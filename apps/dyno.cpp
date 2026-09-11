// dyno.cpp — a water brake on the flywheel.
//
// A dynamometer does not ask an engine what torque it is making. It cannot;
// there is no such measurement. What it does is load the engine — historically
// by throttling water through a paddle wheel bolted to the output shaft —
// until the crank speed stops changing, at which point the torque the engine
// makes and the torque the brake absorbs are known to be equal because the
// flywheel is not accelerating. Then it weighs the brake's reaction arm, which
// is a force at a known radius, which is a torque.
//
// That is the entire method, and this file is a faithful copy of it: a
// proportional-integral controller standing in for a man with a valve,
// holding the speed still and reading off the load.
//
// Power is never measured. It is always and only computed, from torque and
// speed, by the definition P = τω — which is why every dyno graph's two curves
// cross at 5252 rpm, that being the speed at which the conversion constant
// between pound-feet and horsepower happens to equal one. It means nothing at
// all about the engine. It is a unit artefact that a century of people have
// squinted at looking for significance.

#include "apps.hpp"
#include <algorithm>
#include <cmath>
#include <deque>
#include <cstdio>
#include <vector>

using namespace engine;
using namespace engine::si;

namespace {

struct Point { double rpm, torque, power, vacuum, advance, peak, knock, spread; };

// How close counts as arrived, how long it has to stay there first, and how
// many cycles to average once it has. The speed window is 15 rpm rather than
// something prouder because the torque reading only refreshes once per cycle,
// so the load is always a cycle behind the speed and the pair of them circle
// each other in a small limit cycle forever. Demanding they stop entirely
// costs half a minute and buys a tenth of a newton-metre.
constexpr double on_the_nose     = 25.0;   // rpm, and he does not care about these
constexpr double still_enough    = 0.004;  // the beam has stopped drifting
constexpr std::size_t watched_cycles = 8;  // how long he watches before believing it
constexpr int    cycles_averaged = 20;     // and how many he then averages

// Hold the speed until it stops arguing, then read the arm.
//
// A real operator does not take a number the instant the tachometer reaches
// the figure he wanted. He holds it there, watches the beam settle, and then
// watches it a while longer, because an engine that has just been dragged down
// to 3500 rpm is still full of the transient — manifold pressure recovering,
// pipes still ringing with the last load change — and the arm will wander for
// a second or two afterwards. Read it too early and you will publish a dip in
// your torque curve that exists nowhere but in your own impatience.
//
// So: bring it to speed, discard what the needle does next, and then average
// what it does after that, one reading per completed cycle. The spread across
// those readings is reported too, because a figure whose spread is large is
// a figure that has not settled and should not be believed.
Point measure(Engine& e, double target_rpm) {
    const double step = 0.25_deg;

    // The valve. At a steady speed the brake must be absorbing exactly what
    // the engine is making — that is what steady means — so the load is set to
    // the engine's own last reading and then trimmed for whatever speed error
    // remains. It converges in a few cycles because the feedforward term is
    // already the right answer; all the trim has to do is decide which way to
    // lean. A pure integrator has to discover the right answer from scratch,
    // which is why the first version of this took twenty-eight seconds and
    // still could not hold 3500 rpm.
    const auto hold = [&] {
        const double error = e.rpm() - target_rpm;
        e.brake_torque(std::clamp(e.torque() + error * 0.6, -150.0, 1200.0));
        e.step(step);
    };

    // Drag it to the speed, and then watch the beam until it stops moving.
    //
    // Waiting on the TACHOMETER to hold perfectly still does not work and the
    // first two versions of this function both tried it. The torque reading
    // only refreshes once per cycle, so the load is permanently one cycle
    // behind the speed and the two of them circle each other in a small limit
    // cycle that never quite closes. Demanding it close cost half a minute a
    // sweep and bought nothing.
    //
    // What an operator actually watches is the beam. The speed can wander a
    // few rpm and he does not care; what he is waiting for is the arm to stop
    // drifting, because that is the number he is going to write down.
    std::deque<double> recent;
    unsigned long long seen = e.cycles();
    for (int guard = 0; guard < 1500000; ++guard) {
        hold();
        if (e.cycles() == seen) continue;
        seen = e.cycles();

        recent.push_back(e.torque());
        if (recent.size() > watched_cycles) recent.pop_front();
        if (recent.size() < watched_cycles) continue;
        if (std::abs(e.rpm() - target_rpm) > on_the_nose) continue;

        const auto [low, high] = std::minmax_element(recent.begin(), recent.end());
        if (*high - *low < still_enough * *high) break;   // the needle has stopped
    }

    // Now write it down, one reading per completed cycle.
    std::vector<double> readings;
    for (int guard = 0; guard < 400000 && int(readings.size()) < cycles_averaged; ++guard) {
        hold();
        if (e.cycles() != seen) {
            seen = e.cycles();
            readings.push_back(e.torque());
        }
    }

    double mean = 0.0;
    for (double t : readings) mean += t;
    mean /= double(readings.size());

    double spread = 0.0;
    for (double t : readings) spread = std::max(spread, std::abs(t - mean));

    return { e.rpm(), mean, mean * e.crank_speed(),
             e.manifold_vacuum(), e.spark_advance(), e.peak_pressure(),
             e.knock_severity(), spread / std::max(mean, 1.0) };
}

// Two curves, one axis, drawn the way a dyno sheet draws them.
void plot(const std::vector<Point>& pts) {
    constexpr int rows = 16;

    double t_max = 0.0, p_max = 0.0;
    for (const Point& p : pts) {
        t_max = std::max(t_max, as::lbft(p.torque));
        p_max = std::max(p_max, as::hp(p.power));
    }
    const double t_top = std::ceil(t_max / 50.0) * 50.0;
    const double p_top = std::ceil(p_max / 50.0) * 50.0;

    const auto row_of = [&](double value, double top) {
        return std::clamp(static_cast<int>(value / top * rows), 0, rows - 1);
    };

    std::printf("\n  \033[33mlb-ft\033[0m%*s\033[36mhp\033[0m\n",
                static_cast<int>(pts.size()) * 3 + 2, "");

    for (int r = rows - 1; r >= 0; --r) {
        std::printf("  %4.0f |", t_top * (r + 1.0) / rows);
        for (const Point& p : pts) {
            const bool torque = row_of(as::lbft(p.torque), t_top) == r;
            const bool power  = row_of(as::hp(p.power),    p_top) == r;
            if (torque && power) std::printf("\033[32m #\033[0m ");   // they cross
            else if (torque)     std::printf("\033[33m *\033[0m ");
            else if (power)      std::printf("\033[36m o\033[0m ");
            else                 std::printf("   ");
        }
        std::printf("| %4.0f\n", p_top * (r + 1.0) / rows);
    }

    std::printf("       ");
    for (std::size_t i = 0; i < pts.size(); ++i) std::printf("---");
    std::printf("\n       ");
    for (const Point& p : pts) std::printf("%3.0f", p.rpm / 100.0);
    std::printf("   rpm/100\n");
    std::printf("\n       \033[33m *\033[0m torque    \033[36m o\033[0m power    \033[32m #\033[0m both\n");
}

} // namespace

int app::dyno(int argc, char** argv) {
    Engine e = engine_from_flags(argc, argv);
    e.throttle(1.0);
    e.mixture(1.05);          // slightly rich, which is where torque lives

    std::printf("\n\033[1m%s\033[0m — wide open throttle, water brake, %.1f L\n",
                e.name(), as::L(e.displacement()));
    std::printf("\n       torque      power         BMEP vacuum advance peak p  knock  brake\n");
    std::printf("   rpm  lb-ft   Nm    hp    kW    bar    kPa     deg    bar  index   hold\n");

    std::vector<Point> points;
    for (double rpm = 1000.0; rpm <= 6000.5; rpm += 500.0) {
        const Point p = measure(e, rpm);
        points.push_back(p);
        std::printf("  %4.0f %6.1f %4.0f %5.1f %5.1f %6.2f %6.1f %7.1f %6.1f %6.2f %5.1f%%\n",
                    p.rpm, as::lbft(p.torque), p.torque, as::hp(p.power), as::kW(p.power),
                    as::bar(p.torque * 2.0 * two_pi / e.displacement()),
                    as::kPa(p.vacuum), as::deg(p.advance), as::bar(p.peak), p.knock, 100.0 * p.spread);
        std::fflush(stdout);
    }

    const Point& best_t = *std::max_element(points.begin(), points.end(),
        [](const Point& a, const Point& b) { return a.torque < b.torque; });
    const Point& best_p = *std::max_element(points.begin(), points.end(),
        [](const Point& a, const Point& b) { return a.power < b.power; });

    plot(points);

    std::printf("\n  peak torque   %.0f lb-ft at %.0f rpm\n", as::lbft(best_t.torque), best_t.rpm);
    std::printf("  peak power    %.0f hp    at %.0f rpm\n",   as::hp(best_p.power),   best_p.rpm);
    std::printf("\n\033[2m  Ford rated the 1968 302-2V at 210 hp / 4400 and 295 lb-ft / 2400, gross.\n");
    std::printf("  Nothing here was fitted to those numbers.\n\n");
    std::printf("  The knock index is worst at the bottom of the range, where the end gas\n");
    std::printf("  has the most milliseconds to sit and cook. It is an index and not a\n");
    std::printf("  verdict; see the note in knock.hpp about whose engine the threshold\n");
    std::printf("  of 1.0 belongs to.\033[0m\n\n");
    return 0;
}
