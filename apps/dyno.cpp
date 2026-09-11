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
#include <cstdio>
#include <vector>

using namespace engine;
using namespace engine::si;

namespace {

struct Point { double rpm, torque, power, vacuum, advance, peak; };

// Hold the speed until it stops arguing, then read the arm.
Point measure(Engine& e, double target_rpm) {
    const double omega = target_rpm * two_pi / 60.0;
    const double step  = 0.25_deg;

    double integral = 0.0;
    for (int i = 0; i < 320000; ++i) {
        const double error = e.crank_speed() - omega;
        integral += error * 1e-4;
        e.brake_torque(std::clamp(180.0 + error * 3.0 + integral, -120.0, 900.0));
        e.step(step);
    }
    return { e.rpm(), e.torque(), e.power(),
             e.manifold_vacuum(), e.spark_advance(), e.peak_pressure() };
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
    std::printf("\n   rpm    torque      power      BMEP    vacuum   advance   peak p\n");
    std::printf("         lb-ft   Nm    hp    kW     bar      kPa       deg      bar\n");

    std::vector<Point> points;
    for (double rpm = 1000.0; rpm <= 6000.5; rpm += 500.0) {
        const Point p = measure(e, rpm);
        points.push_back(p);
        std::printf("  %4.0f  %6.1f %5.0f %5.1f %5.1f   %5.2f    %5.1f     %5.1f    %5.1f\n",
                    p.rpm, as::lbft(p.torque), p.torque, as::hp(p.power), as::kW(p.power),
                    as::bar(p.torque * 2.0 * two_pi / e.displacement()),
                    as::kPa(p.vacuum), as::deg(p.advance), as::bar(p.peak));
        std::fflush(stdout);
    }

    const Point& best_t = *std::max_element(points.begin(), points.end(),
        [](const Point& a, const Point& b) { return a.torque < b.torque; });
    const Point& best_p = *std::max_element(points.begin(), points.end(),
        [](const Point& a, const Point& b) { return a.power < b.power; });

    plot(points);

    std::printf("\n  peak torque   %.0f lb-ft at %.0f rpm\n", as::lbft(best_t.torque), best_t.rpm);
    std::printf("  peak power    %.0f hp    at %.0f rpm\n",   as::hp(best_p.power),   best_p.rpm);
    std::printf("\n\033[2m  Ford published, 1968 302-2V, gross: 210 hp at 4600, 300 lb-ft at 2600.\n");
    std::printf("  Nothing here was fitted to those numbers.\033[0m\n\n");
    return 0;
}
