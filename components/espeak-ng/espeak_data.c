#include "espeak_data.h"
#include "config-espk.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <esp_log.h>
#include <esp_partition.h>
#include <esp_vfs.h>

// Read-only view of the image built by tools/pack_data.py, mapped straight from flash

static const char* TAG = "espeak_data";

#define DATA_MAGIC "ESPK"
#define DATA_VERSION 1
#define NAME_LEN 56
#define DATA_MAX_FDS 4

typedef struct {
	char magic[4];
	uint32_t version;
	uint32_t count;
	uint32_t reserved;
} data_header_t;

typedef struct {
	char name[NAME_LEN];
	uint32_t offset;
	uint32_t size;
} data_entry_t;

typedef struct {
	const data_entry_t* entry;
	size_t pos;
} open_file_t;

static const uint8_t* base;
static const data_entry_t* table;
static uint32_t count;
static open_file_t fds[DATA_MAX_FDS];

static const data_entry_t* find(const char* name){
	while(*name == '/') name++;
	for(uint32_t i = 0; i < count; i++){
		if(strncmp(table[i].name, name, NAME_LEN) == 0) return &table[i];
	}
	return NULL;
}

static bool isRoot(const char* path){
	return path[0] == 0 || strcmp(path, "/") == 0;
}

static int vfsOpen(const char* path, int flags, int mode){
	if((flags & O_ACCMODE) != O_RDONLY){
		errno = EROFS;
		return -1;
	}
	const data_entry_t* entry = find(path);
	if(entry == NULL){
		errno = ENOENT;
		return -1;
	}
	for(int fd = 0; fd < DATA_MAX_FDS; fd++){
		if(fds[fd].entry == NULL){
			fds[fd].entry = entry;
			fds[fd].pos = 0;
			return fd;
		}
	}
	errno = ENFILE;
	return -1;
}

static open_file_t* getFile(int fd){
	if(fd < 0 || fd >= DATA_MAX_FDS || fds[fd].entry == NULL){
		errno = EBADF;
		return NULL;
	}
	return &fds[fd];
}

static ssize_t vfsRead(int fd, void* dst, size_t size){
	open_file_t* f = getFile(fd);
	if(f == NULL) return -1;
	size_t left = f->entry->size - f->pos;
	if(size > left) size = left;
	memcpy(dst, base + f->entry->offset + f->pos, size);
	f->pos += size;
	return (ssize_t) size;
}

static off_t vfsLseek(int fd, off_t offset, int whence){
	open_file_t* f = getFile(fd);
	if(f == NULL) return -1;
	off_t pos;
	switch(whence){
		case SEEK_SET: pos = offset; break;
		case SEEK_CUR: pos = (off_t) f->pos + offset; break;
		case SEEK_END: pos = (off_t) f->entry->size + offset; break;
		default:
			errno = EINVAL;
			return -1;
	}
	if(pos < 0 || pos > (off_t) f->entry->size){
		errno = EINVAL;
		return -1;
	}
	f->pos = (size_t) pos;
	return pos;
}

static int vfsClose(int fd){
	open_file_t* f = getFile(fd);
	if(f == NULL) return -1;
	f->entry = NULL;
	return 0;
}

static void fillStat(const data_entry_t* entry, struct stat* st){
	memset(st, 0, sizeof(*st));
	if(entry == NULL){
		st->st_mode = S_IFDIR | 0555;
		return;
	}
	st->st_mode = S_IFREG | 0444;
	st->st_size = entry->size;
}

static int vfsFstat(int fd, struct stat* st){
	open_file_t* f = getFile(fd);
	if(f == NULL) return -1;
	fillStat(f->entry, st);
	return 0;
}

static int vfsStat(const char* path, struct stat* st){
	// eSpeak checks that its data path is a directory before using it
	if(isRoot(path)){
		fillStat(NULL, st);
		return 0;
	}
	const data_entry_t* entry = find(path);
	if(entry == NULL){
		errno = ENOENT;
		return -1;
	}
	fillStat(entry, st);
	return 0;
}

void* espeak_mem_map(const char* path, int* len){
	static const char prefix[] = PATH_ESPEAK_DATA "/";
	if(base == NULL || path == NULL || strncmp(path, prefix, sizeof(prefix) - 1) != 0) return NULL;

	const data_entry_t* entry = find(path + sizeof(prefix) - 1);
	if(entry == NULL) return NULL;

	if(len) *len = (int) entry->size;
	return (void*) (base + entry->offset);
}

esp_err_t espeak_data_mount(void){
	if(base != NULL) return ESP_OK;

	const esp_partition_t* part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "espeak");
	if(part == NULL){
		ESP_LOGE(TAG, "espeak partition not found");
		return ESP_ERR_NOT_FOUND;
	}

	const void* ptr;
	esp_partition_mmap_handle_t handle;
	esp_err_t err = esp_partition_mmap(part, 0, part->size, ESP_PARTITION_MMAP_DATA, &ptr, &handle);
	if(err != ESP_OK){
		ESP_LOGE(TAG, "mmap failed: %s", esp_err_to_name(err));
		return err;
	}

	const data_header_t* header = (const data_header_t*) ptr;
	if(memcmp(header->magic, DATA_MAGIC, 4) != 0 || header->version != DATA_VERSION
	   || sizeof(data_header_t) + header->count * sizeof(data_entry_t) > part->size){
		ESP_LOGE(TAG, "espeak partition holds no valid data image, flash it with idf.py flash");
		esp_partition_munmap(handle);
		return ESP_ERR_INVALID_STATE;
	}

	const data_entry_t* entries = (const data_entry_t*) (header + 1);
	for(uint32_t i = 0; i < header->count; i++){
		if(entries[i].offset + entries[i].size > part->size || (entries[i].offset & 3) != 0){
			ESP_LOGE(TAG, "corrupt entry %.*s", NAME_LEN, entries[i].name);
			esp_partition_munmap(handle);
			return ESP_ERR_INVALID_STATE;
		}
	}

	static const esp_vfs_t vfs = {
		.flags = ESP_VFS_FLAG_DEFAULT | ESP_VFS_FLAG_READONLY_FS,
		.open = &vfsOpen,
		.read = &vfsRead,
		.lseek = &vfsLseek,
		.close = &vfsClose,
		.fstat = &vfsFstat,
		.stat = &vfsStat,
	};

	base = (const uint8_t*) ptr;
	table = entries;
	count = header->count;

	err = esp_vfs_register(PATH_ESPEAK_DATA, &vfs, NULL);
	if(err != ESP_OK){
		ESP_LOGE(TAG, "VFS register failed: %s", esp_err_to_name(err));
		base = NULL;
		esp_partition_munmap(handle);
		return err;
	}

	ESP_LOGI(TAG, "mounted %lu files at %s", (unsigned long) count, PATH_ESPEAK_DATA);
	return ESP_OK;
}
