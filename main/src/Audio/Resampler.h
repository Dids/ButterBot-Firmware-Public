#ifndef BUTTERBOT_FIRMWARE_RESAMPLER_H
#define BUTTERBOT_FIRMWARE_RESAMPLER_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

/**
 * Streaming polyphase windowed-sinc resampler, 22050 Hz -> 16000 Hz (320:441), int16 mono.
 *
 * Low-pass at ~7 kHz with a Kaiser window, so the hiss of eSpeak's formant voice above the
 * new Nyquist frequency doesn't alias back down. Keeps its history between process() calls,
 * so audio can be fed in arbitrary chunks; call reset() at the start of each utterance.
 */
class Resampler {
public:
	static constexpr uint32_t InRate = 22050;
	static constexpr uint32_t OutRate = 16000;
	static constexpr uint32_t Up = 320;
	static constexpr uint32_t Down = 441;
	static constexpr uint32_t Taps = 32; // per phase
	static constexpr float Cutoff = 7000.0f;
	static_assert((uint64_t) InRate * Up == (uint64_t) OutRate * Down);

	Resampler();

	void reset();

	/**
	 * @param out must hold at least maxOutput(count) samples
	 * @return number of samples written to out
	 */
	size_t process(const int16_t* in, size_t count, int16_t* out);

	static constexpr size_t maxOutput(size_t inCount){ return inCount * Up / Down + 1; }

private:
	// coeffs[phase * Taps + k] multiplies the input sample k steps back
	std::unique_ptr<float[]> coeffs;

	// Delay line written twice so the newest Taps samples are always contiguous
	std::array<float, 2 * Taps> history{};
	uint32_t pos = 0;
	uint32_t phase = 0; // time of the next output, in 1/Up input samples after the newest one
};


#endif //BUTTERBOT_FIRMWARE_RESAMPLER_H
