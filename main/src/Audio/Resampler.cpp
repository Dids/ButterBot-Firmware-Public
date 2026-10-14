#include "Resampler.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

// Zeroth order modified Bessel function of the first kind, for the Kaiser window
double besselI0(double x){
	double sum = 1.0, term = 1.0;
	for(int k = 1; k < 50; k++){
		term *= (x / (2.0 * k)) * (x / (2.0 * k));
		sum += term;
		if(term < sum * 1e-12) break;
	}
	return sum;
}

constexpr double KaiserBeta = 7.0; // ~70 dB stopband

}

Resampler::Resampler() : coeffs(new float[Up * Taps]){
	constexpr uint32_t Length = Up * Taps;
	constexpr double Center = (Length - 1) / 2.0;
	const double fc = Cutoff / ((double) InRate * Up); // normalized to the upsampled rate
	const double i0Beta = besselI0(KaiserBeta);

	for(uint32_t p = 0; p < Up; p++){
		double sum = 0;
		for(uint32_t k = 0; k < Taps; k++){
			const double n = p + (double) k * Up - Center;
			const double x = 2.0 * fc * n;
			const double sinc = n == 0 ? 1.0 : std::sin(std::numbers::pi * x) / (std::numbers::pi * x);
			const double r = n / Center;
			const double window = besselI0(KaiserBeta * std::sqrt(std::max(0.0, 1.0 - r * r))) / i0Beta;
			const double h = sinc * window;
			coeffs[p * Taps + k] = (float) h;
			sum += h;
		}
		// Unity DC gain on every phase, otherwise the phase pattern shows up as a whine
		for(uint32_t k = 0; k < Taps; k++){
			coeffs[p * Taps + k] = (float) (coeffs[p * Taps + k] / sum);
		}
	}
}

void Resampler::reset(){
	history.fill(0);
	pos = 0;
	phase = 0;
}

size_t Resampler::process(const int16_t* in, size_t count, int16_t* out){
	size_t written = 0;

	for(size_t i = 0; i < count; i++){
		pos = pos == 0 ? Taps - 1 : pos - 1;
		history[pos] = history[pos + Taps] = in[i];

		// Down > Up, so each input sample yields at most one output
		if(phase < Up){
			const float* c = &coeffs[phase * Taps];
			const float* x = &history[pos];
			float acc = 0;
			for(uint32_t k = 0; k < Taps; k++){
				acc += c[k] * x[k];
			}
			acc = std::round(acc);
			out[written++] = (int16_t) std::clamp(acc, -32768.0f, 32767.0f);
			phase += Down;
		}
		phase -= Up;
	}

	return written;
}
