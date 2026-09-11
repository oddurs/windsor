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

// Signed 16-bit PCM, one channel. Samples arrive in whatever arbitrary units
// the physics produced them in and are normalised to just under full scale,
// because the absolute sound pressure three feet behind a tailpipe is not a
// number this project has any business claiming to know.
inline bool write(const char* path, std::span<const double> samples, int sample_rate) {
    double peak = 0.0;
    for (double s : samples) peak = std::max(peak, std::abs(s));
    if (peak <= 0.0) peak = 1.0;
    const double gain = 0.89 * 32767.0 / peak;

    const std::uint32_t data_bytes = static_cast<std::uint32_t>(samples.size() * 2);
    const std::uint32_t byte_rate  = static_cast<std::uint32_t>(sample_rate) * 2;

    std::FILE* f = std::fopen(path, "wb");
    if (!f) return false;

    const auto u32 = [&](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
    const auto u16 = [&](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
    const auto tag = [&](const char* t)   { std::fwrite(t, 1, 4, f); };

    tag("RIFF");  u32(36 + data_bytes);  tag("WAVE");
    tag("fmt ");  u32(16);
    u16(1);                                   // PCM, uncompressed
    u16(1);                                   // mono
    u32(static_cast<std::uint32_t>(sample_rate));
    u32(byte_rate);
    u16(2);                                   // block align
    u16(16);                                  // bits per sample
    tag("data");  u32(data_bytes);

    std::vector<std::int16_t> pcm;
    pcm.reserve(samples.size());
    for (double s : samples)
        pcm.push_back(static_cast<std::int16_t>(
            std::clamp(s * gain, -32768.0, 32767.0)));

    std::fwrite(pcm.data(), 2, pcm.size(), f);
    std::fclose(f);
    return true;
}

} // namespace wav
