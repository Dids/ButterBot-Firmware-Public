#!/usr/bin/env python3
"""Packs eSpeak-NG data files into a flat image for the espeak flash partition.

Layout (little endian):
  header: magic "ESPK", version, file count, reserved   (4 x uint32)
  table:  name (56 bytes, NUL padded), offset, size      (64 bytes per file)
  data:   file contents, each starting on a 4-byte boundary

Offsets are from the start of the image. eSpeak reads its tables as int/short
straight from flash, so the alignment matters (Xtensa faults on unaligned loads).
"""

import argparse
import os
import struct
import sys

MAGIC = b"ESPK"
VERSION = 1
NAME_LEN = 56
HEADER = struct.Struct("<4sIII")
ENTRY = struct.Struct("<%dsII" % NAME_LEN)


def align4(n):
	return (n + 3) & ~3


def collect(root):
	files = []
	for dirpath, dirnames, filenames in os.walk(root):
		dirnames.sort()
		for fn in sorted(filenames):
			path = os.path.join(dirpath, fn)
			name = os.path.relpath(path, root).replace(os.sep, "/")
			files.append((name, path))
	return files


def pack(root, out, max_size):
	files = collect(root)
	if not files:
		sys.exit("pack_data: no files in %s" % root)

	offset = align4(HEADER.size + ENTRY.size * len(files))
	table = []
	blobs = []
	for name, path in files:
		encoded = name.encode("ascii")
		if len(encoded) >= NAME_LEN:
			sys.exit("pack_data: name too long: %s" % name)
		with open(path, "rb") as f:
			data = f.read()
		table.append(ENTRY.pack(encoded, offset, len(data)))
		blobs.append((offset, data))
		offset = align4(offset + len(data))

	if max_size and offset > max_size:
		sys.exit("pack_data: image is %d bytes, partition holds %d" % (offset, max_size))

	image = bytearray(offset)
	HEADER.pack_into(image, 0, MAGIC, VERSION, len(files), 0)
	pos = HEADER.size
	for entry in table:
		image[pos:pos + ENTRY.size] = entry
		pos += ENTRY.size
	for off, data in blobs:
		image[off:off + len(data)] = data

	tmp = out + ".tmp"
	with open(tmp, "wb") as f:
		f.write(image)
	os.replace(tmp, out)
	print("pack_data: %d files, %d bytes -> %s" % (len(files), len(image), out))


def main():
	parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
	parser.add_argument("datadir", help="directory with the data files, relative paths become file names")
	parser.add_argument("output", help="output image")
	parser.add_argument("--max-size", type=lambda s: int(s, 0), default=0, help="fail if the image is larger")
	args = parser.parse_args()
	pack(args.datadir, args.output, args.max_size)


if __name__ == "__main__":
	main()
