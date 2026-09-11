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
#include <cstring>
#include <vector>

using namespace engine;
using namespace engine::si;

namespace {

// A drive cycle worth listening to: settle, idle, one pull to the redline,
// then off the throttle and back down onto the overrun.
struct Phase { const char* what; double seconds; double throttle; };

constexpr Phase script[] = {
    { "settling",  1.5, 0.00 },
    { "idle",      1.8, 0.00 },
    { "pull",      3.6, 1.00 },
    { "overrun",   2.2, 0.00 },
};

const char* output_path(int argc, char** argv) {
    for (int i = 0; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], "--out") == 0) return argv[i + 1];
    return app::has_flag(argc, argv, "--flat") ? "windsor-flat.wav" : "windsor.wav";
}

} // namespace

int app::record(int argc, char** argv) {
    Engine e = engine_from_flags(argc, argv);
    e.mixture(1.05);

    // Third gear, roughly: a ton and a half of car seen through the square of
    // the ratio. Without it the engine hits the limiter in a quarter second
    // and there is nothing to hear.
    e.load_inertia(2.0);

    const double redline = 6200.0;
    const double step    = 0.25_deg;

    std::vector<double> track;
    Exhaust& left  = e.bank(Bank::left);
    Exhaust& right = e.bank(Bank::right);

    std::printf("\n\033[1m%s\033[0m — %s crankshaft\n",
                e.name(), name_of(e.crankshaft().plane()));
    std::printf("  each bank fires at ");
    for (double g : e.crankshaft().firing_intervals(Bank::right)) std::printf("%.0f ", as::deg(g));
    std::printf("deg\n\n");

    for (const Phase& phase : script) {
        const double until = phase.seconds;
        double elapsed = 0.0;

        std::printf("  %-9s ", phase.what);
        std::fflush(stdout);

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
            for (std::size_t i = 0; i < n; ++i)
                track.push_back(left.samples()[i] + right.samples()[i]);
            left.clear_samples();
            right.clear_samples();
        }
        std::printf("%5.0f rpm\n", e.rpm());
    }

    const char* path = output_path(argc, argv);
    if (!wav::write(path, track, 44100)) {
        std::fprintf(stderr, "could not write %s\n", path);
        return 1;
    }

    std::printf("\n  %s — %.1f seconds, %zu samples at 44.1 kHz\n",
                path, track.size() / 44100.0, track.size());
    std::printf("\033[2m  Nothing in this file knows what a V8 sounds like.\033[0m\n\n");
    return 0;
}
