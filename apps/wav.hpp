// wav.hpp — 44 bytes of header and then the numbers.
//
// A RIFF wave file is a tiny format with an undeserved reputation, and writing
// one by hand takes less code than linking something that will write it for
// you. Four-character chunk tags, little-endian throughout because it came
// from a PC, two nested chunks, and then raw samples. That is the whole
// specification, and this is the whole implementation.
//
// Doing it here rather than pulling in a library is not stubbornness. It is
// the fifth house rule: this repository should still build in fifteen years,
// and every dependency is a bet that something else will too.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include <vector>

namespace wav {

// Signed 16-bit PCM, stereo.
//
// Both channels are normalised by the SAME factor. Scaling them independently
// would make each one as loud as it could be and destroy the only thing a
// second channel is for: the difference between them.
//
// `reference` extends that rule across FILES. Left at zero, each file is scaled
// to its own peak, which is right for a single recording and wrong for two that
// are meant to be compared: it makes every take equally loud and throws away
// the difference in loudness between them, which is a real difference. Given a
// reference instead, that number is full scale and the take lands wherever it
// honestly falls beneath it.
inline bool write(const char* path,
                  std::span<const double> left,
                  std::span<const double> right,
                  int sample_rate,
                  double reference = 0.0)
{
    constexpr int channels = 2;
    const std::size_t frames = std::min(left.size(), right.size());

    double peak = 0.0;
    for (double s : left)  peak = std::max(peak, std::abs(s));
    for (double s : right) peak = std::max(peak, std::abs(s));
    if (peak <= 0.0) peak = 1.0;
    const double full = reference > 0.0 ? reference : peak;
    const double gain = 0.89 * 32767.0 / full;

    const std::uint32_t data_bytes = static_cast<std::uint32_t>(frames * 4);
    const std::uint32_t byte_rate  = static_cast<std::uint32_t>(sample_rate) * 4;

    std::FILE* f = std::fopen(path, "wb");
    if (!f) return false;

    const auto u32 = [&](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
    const auto u16 = [&](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
    const auto tag = [&](const char* t)   { std::fwrite(t, 1, 4, f); };

    tag("RIFF");  u32(36 + data_bytes);  tag("WAVE");
    tag("fmt ");  u32(16);
    u16(1);                                   // PCM, uncompressed
    u16(channels);
    u32(static_cast<std::uint32_t>(sample_rate));
    u32(byte_rate);
    u16(channels * 2);                               // block align
    u16(16);                                         // bits per sample
    tag("data");  u32(data_bytes);

    const auto quantise = [&](double v) {
        return static_cast<std::int16_t>(std::clamp(v * gain, -32768.0, 32767.0));
    };

    std::vector<std::int16_t> pcm;
    pcm.reserve(frames * 2);
    for (std::size_t i = 0; i < frames; ++i) {
        pcm.push_back(quantise(left[i]));
        pcm.push_back(quantise(right[i]));
    }

    std::fwrite(pcm.data(), 2, pcm.size(), f);
    std::fclose(f);
    return true;
}

} // namespace wav
