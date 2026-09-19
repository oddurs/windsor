// record.cpp — three feet behind the tailpipe.
//
// This file contains no acoustics. Every sample it writes was computed by
// `exhaust.hpp` out of gas leaving eight cylinders at moments decided by
// `crankshaft.hpp`, and all this does is hold the throttle, drain the two
// banks, add them together and put a RIFF header on the front.
//
// That is the claim the whole project is making, so it is worth stating in the
// place where it is cashed: nothing here knows what a V8 sounds like. There is
// no sample, no oscillator, no filter tuned by ear, no table of harmonics.
// There is a crankshaft with its throws at 90°, and the noise is what that
// implies.
//
// ── Two channels, because there are two exhausts ──────────────────────────
//
// A V8 does not have an exhaust. It has two, one per bank, running down
// opposite sides of the car, and on a cross-plane crank they are permanently
// out of step with each other — that is the second half of the burble, the
// first being that neither bank is even on its own.
//
// So there are two microphones, one behind each tailpipe, about a metre apart.
// Each hears its own pipe close and the other one further away: quieter by the
// extra distance, and later by the time sound takes to cross the gap. That
// delay is the whole of the stereo image. Nothing here is widened, panned or
// spread; the two channels are simply what two microphones in two places would
// have recorded, and the space between them is the space between the pipes.
//
// It is also the most honest possible presentation of the thesis. Summing the
// banks to mono recombines them toward an even train and cancels much of the
// unevenness — which is a real effect, and is why people argue about H-pipes,
// but it is throwing away the evidence. In stereo the two pulse trains stay
// apart, where you can hear them disagree.
//
// Run it twice:
//
//      windsor record
//      windsor record --flat
//
// Same block, same bore, same cam, same fuel, same everything. One part is
// different and it is not the part that makes the sound — it is the part that
// decides when the sound happens.

#include "apps.hpp"
#include "wav.hpp"
#include <algorithm>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>
#include <vector>

using namespace engine;
using namespace engine::si;

namespace {

// A drive cycle worth listening to: let it warm through, idle, hold a cruise,
// one pull to the redline, then off the throttle and back down onto the
// overrun.
//
// The first phase is NOT RECORDED, and that is not tidying up after a bad
// result — it is where the microphone goes.
//
// Everything in this engine starts from nothing. The delay lines are full of
// silence, the filters are at zero, the pipes are at atmosphere and the crank
// is turning at a speed nobody chose. The first cylinder to fire does so into
// pipes that have never carried a wave, and the transient that produces is
// five times louder than the idle that follows it — loud enough to set the
// normalisation for the whole file single-handed and make everything after it
// quiet. Then the engine spends the next two seconds hunting its way to an
// idle, and a hunting engine is a broadband one: three quarters of the energy
// in that stretch was above 1.4 kHz, against a twentieth of a percent once it
// had settled.
//
// None of that is the engine. It is the engine being switched on. You do not
// put a microphone behind a car and start the tape before the starter motor.
struct Phase {
    const char* what;
    double      seconds;
    double      throttle;
    double      driveline;   // kg·m² the engine is dragging behind it
    bool        recorded;
};

// The driveline column is the gearbox. An engine idles in neutral, where it
// is turning nothing but its own flywheel and will snap to the limiter in a
// quarter of a second if you ask it to; it pulls in gear, where it is turning
// a ton and a half of car seen through the square of the ratio and takes three
// seconds to do the same thing. Same engine, same torque, and the difference
// between a blip and a pull is entirely this one number.
//
// It also decides how long the idle takes to find itself, which is why the
// warm-up above is as long as it is.
constexpr Phase script[] = {
    { "warming up", 4.0, 0.00, 0.6, false },   // simulated, not recorded
    { "idle",       2.0, 0.00, 0.6, true  },   // neutral
    { "cruise",     1.6, 0.22, 2.2, true  },   // in gear from here
    { "pull",       3.2, 1.00, 2.2, true  },
    { "overrun",    2.2, 0.00, 2.2, true  },
};

// A slow envelope on the louder of the two channels, and one gain applied to
// both. Two things about it are easy to get wrong and this file got both of
// them wrong first, so they are written down where the constants are.
//
// The threshold has to be a fraction of the material, not a number. Samples
// arrive here in whatever units the physics left them in — this engine peaks
// near two hundred of them — so a fixed 0.06 put the whole recording thirty to
// forty decibels into gain reduction and turned three-to-one into a cube root.
// That is not levelling, it is flattening, and it is audible as flattening.
//
// And the release has to be far longer than the CYCLE, not than the gap between
// pulses. A cross-plane bank repeats every two revolutions — 174 ms at idle —
// so a release of 250 ms rides the burble itself, turning up through the long
// gap and down on the pair that follows it. That is precisely the shape the
// recording exists to show, being ironed out by the thing meant to make it
// audible. A second and a half is longer than anything the engine does at idle
// and shorter than the drive cycle, so phases get levelled against each other
// and the rhythm inside them is left alone.
void level(std::vector<double>& left, std::vector<double>& right) {
    constexpr double below   = 0.10;    // threshold, 20 dB under the loudest
    constexpr double ratio   = 3.0;
    constexpr double attack  = 0.010;   // seconds
    constexpr double release = 1.500;

    double peak = 0.0;
    for (std::size_t i = 0; i < left.size(); ++i)
        peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
    if (peak <= 0.0) return;
    const double threshold = below * peak;

    const double up   = 1.0 - std::exp(-1.0 / (attack  * 44100.0));
    const double down = 1.0 - std::exp(-1.0 / (release * 44100.0));

    // The envelope has to START somewhere, and starting it at zero is a bug
    // with a sound. For the first few milliseconds it is below the threshold,
    // so the gain is 1 while every sample after it is being turned down — and
    // those few milliseconds then become the loudest thing in the file, set
    // the normalisation single-handed, and push the entire recording down
    // behind a thump. Charge it from the opening instead, so the compressor
    // is already holding the signal it is about to be handed.
    double envelope = 0.0;
    for (std::size_t i = 0; i < std::min<std::size_t>(left.size(), 4410); ++i)
        envelope = std::max(envelope, std::max(std::abs(left[i]), std::abs(right[i])));

    for (std::size_t i = 0; i < left.size(); ++i) {
        const double loudest = std::max(std::abs(left[i]), std::abs(right[i]));
        envelope += (loudest > envelope ? up : down) * (loudest - envelope);

        double gain = 1.0;
        if (envelope > threshold)
            gain = std::pow(threshold / envelope, 1.0 - 1.0 / ratio);

        left[i]  *= gain;
        right[i] *= gain;
    }
}

const char* output_path(int argc, char** argv) {
    for (int i = 0; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], "--out") == 0) return argv[i + 1];
    return app::has_flag(argc, argv, "--flat") ? "windsor-flat.wav" : "windsor.wav";
}

} // namespace

int app::record(int argc, char** argv) {
    Engine e = engine_from_flags(argc, argv);
    e.mixture(1.05);

    const double redline = 6200.0;
    const double step    = 0.25_deg;

    std::vector<double> from_left, from_right;
    Exhaust& left  = e.bank(Bank::left);
    Exhaust& right = e.bank(Bank::right);

    std::printf("\n\033[1m%s\033[0m — %s crankshaft\n",
                e.name(), name_of(e.crankshaft().plane()));
    std::printf("  each bank fires at ");
    {
        const auto gaps = e.crankshaft().firing_intervals(Bank::right);
        for (std::size_t i = 0; i < gaps.size(); ++i)
            std::printf("%.0f%s", as::deg(gaps[i]), i + 1 < gaps.size() ? " - " : " deg\n\n");
    }

    for (const Phase& phase : script) {
        const double until = phase.seconds;
        double elapsed = 0.0;

        std::printf("  %-11s%s", phase.what, phase.recorded ? "" : "\033[2m");
        std::fflush(stdout);

        e.load_inertia(phase.driveline);

        while (elapsed < until) {
            e.throttle(phase.throttle);

            // A rev limiter, which on a 1968 engine was the valve springs and
            // the owner's nerve. Here it is a fuel cut, which is what makes
            // the stuttering noise everyone recognises.
            if (e.rpm() > redline) e.throttle(0.0);

            const double dt = step / e.crank_speed();
            e.step(step);
            elapsed += dt;

            // Drain both banks. A listener standing behind the car hears the
            // sum of two exhausts, and on a cross-plane engine those two are
            // permanently out of step with each other — which is the second
            // half of the burble, the first being that neither is even.
            const std::size_t n = std::min(left.samples().size(), right.samples().size());
            if (phase.recorded)
                for (std::size_t i = 0; i < n; ++i) {
                    from_left.push_back(left.samples()[i]);
                    from_right.push_back(right.samples()[i]);
                }
            left.clear_samples();
            right.clear_samples();
        }
        std::printf("%5.0f rpm\033[0m\n", e.rpm());
    }

    // ── The two microphones ───────────────────────────────────────────────
    //
    // Both microphones hear BOTH tailpipes. That is the whole of it, and the
    // first version of this got it wrong by giving each mic its own pipe plus
    // a little of the other: a crossfeed with a delay on it is a comb filter,
    // it cancelled where the delay ran to half a wavelength, and it left the
    // two channels ANTI-correlated — which sounds hollow and disappears
    // entirely the moment anyone plays it in mono.
    //
    // Done properly there is nothing to invent. Four paths, pipe to mic, each
    // with a length. Amplitude falls as one over that length because sound
    // spreads over a sphere. Arrival is late by the length over the speed of
    // sound. The stereo image is then not an effect applied to the recording —
    // it is the geometry, and if you move the microphones it moves.
    constexpr double track   = 1.00;   // m between the two tailpipes
    constexpr double spacing = 0.60;   // m between the two microphones
    constexpr double behind  = 1.50;   // m back from the bumper
    constexpr double in_air  = 343.0;  // m/s, out in the cold

    const auto path_from = [&](double pipe_x, double mic_x) {
        return std::hypot(behind, pipe_x - mic_x);
    };

    struct Path { double gain; std::size_t lag; };
    const auto route = [&](double pipe_x, double mic_x) {
        const double d = path_from(pipe_x, mic_x);
        return Path{ 1.0 / d, static_cast<std::size_t>(d / in_air * 44100.0) };
    };

    const Path left_to_left   = route(-0.5 * track, -0.5 * spacing);
    const Path right_to_left  = route(+0.5 * track, -0.5 * spacing);
    const Path left_to_right  = route(-0.5 * track, +0.5 * spacing);
    const Path right_to_right = route(+0.5 * track, +0.5 * spacing);

    const std::size_t frames = from_left.size();
    std::vector<double> channel_left(frames), channel_right(frames);

    const auto heard = [&](const std::vector<double>& source, const Path& p, std::size_t i) {
        return i >= p.lag ? source[i - p.lag] * p.gain : 0.0;
    };

    for (std::size_t i = 0; i < frames; ++i) {
        channel_left [i] = heard(from_left,  left_to_left,   i)
                         + heard(from_right, right_to_left,  i);
        channel_right[i] = heard(from_left,  left_to_right,  i)
                         + heard(from_right, right_to_right, i);
    }

    // ── Levelling ─────────────────────────────────────────────────────────
    //
    // A recording decision and not a physical one, said plainly. An idle is
    // genuinely thirty times quieter than a pull to the redline, and a file
    // that preserves that faithfully is a file whose first four seconds are
    // inaudible on a laptop. So the same gentle compression every recording of
    // every car has ever had: a slow envelope, three to one above the
    // threshold, applied identically to both channels so the image does not
    // move when the engine gets loud.
    level(channel_left, channel_right);

    // CALIBRATED. One full-scale reference for both crankshafts, because the
    // two files exist to be compared and a file scaled to its own peak cannot
    // be. Measured, the flat crank peaks at 69.3 against the cross-plane's
    // 60.1 while carrying LESS energy — a cleaner train with taller spikes —
    // so scaling each to its own peak turned a true 0.95 dB difference in
    // loudness into 2.2 dB, all of it against the flat crank. In an A/B, 2 dB
    // reads as better rather than different, and that is the one conclusion
    // this recording must not put in the listener's ear for him.
    //
    // It is a number, so it can rot: if a change to the model makes a take
    // louder than this, the take would clip, and the run says so and stops
    // rather than quietly flattening its own peaks.
    constexpr double full_scale = 80.0;

    double loudest = 0.0;
    for (std::size_t i = 0; i < channel_left.size(); ++i)
        loudest = std::max(loudest, std::max(std::abs(channel_left[i]), std::abs(channel_right[i])));
    if (loudest > full_scale) {
        std::fprintf(stderr,
                     "this take peaks at %.1f, over the %.1f reference in record.cpp.\n"
                     "raise the reference and re-record BOTH cranks, or they stop comparing.\n",
                     loudest, full_scale);
        return 1;
    }

    const char* path = output_path(argc, argv);
    if (!wav::write(path, channel_left, channel_right, 44100, full_scale)) {
        std::fprintf(stderr, "could not write %s\n", path);
        return 1;
    }

    std::printf("\n  %s — %.1f seconds, stereo, %zu frames at 44.1 kHz\n",
                path, channel_left.size() / 44100.0, channel_left.size());
    std::printf("\033[2m  Two microphones %.2f m apart, %.2f m behind two tailpipes %.2f m\n",
                spacing, behind, track);
    std::printf("  apart. The stereo is the geometry. Nothing in this file knows what a\n");
    std::printf("  V8 sounds like.\033[0m\n\n");
    return 0;
}
