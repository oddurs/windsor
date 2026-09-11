// run.cpp — a gauge cluster, wired to the sensors.
//
// The engine runs in real time and this draws what it is doing, thirty times a
// second. Nothing here computes anything: every number on the screen was
// already true inside the engine before this file asked for it.
//
// What is worth watching is the firing order marching around the eight bores —
// 1-5-4-2-6-3-7-8, never two adjacent, because a crankshaft that fired
// neighbouring cylinders in sequence would load the main bearings between them
// twice in a row and rock the whole block. Every firing order ever cast into
// an intake manifold is a compromise between bearing loads, crank torsion, and
// keeping the intake charge from being stolen by the cylinder next door.

#include "apps.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <termios.h>
#include <thread>
#include <unistd.h>

using namespace engine;
using namespace engine::si;

namespace {

// Put the terminal into raw mode so a keystroke arrives without a newline, and
// —- importantly — put it back afterward whatever happens.
class RawTerminal {
public:
    RawTerminal() {
        tcgetattr(STDIN_FILENO, &saved_);
        termios raw = saved_;
        raw.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        std::printf("\033[?25l");        // hide the cursor
    }
    ~RawTerminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_);
        std::printf("\033[?25h\033[0m\n");
    }
    RawTerminal(const RawTerminal&) = delete;
    RawTerminal& operator=(const RawTerminal&) = delete;

    int key() const {
        char c;
        return ::read(STDIN_FILENO, &c, 1) == 1 ? c : 0;
    }
private:
    termios saved_{};
};

// Which quarter of the cycle a cylinder is in, from its own crank angle.
struct Phase { const char* name; const char* colour; };

Phase phase_of(double local) {
    const double deg = as::deg(local);
    if (deg < 180.0) return { "power   ", "\033[1;31m" };
    if (deg < 360.0) return { "exhaust ", "\033[0;90m" };
    if (deg < 540.0) return { "intake  ", "\033[0;36m" };
    return                  { "compress", "\033[0;34m" };
}

double fold_cycle(double a) {
    const double cycle = 2.0 * two_pi;
    a = std::fmod(a, cycle);
    return a < 0 ? a + cycle : a;
}

void bar(double fraction, int width, const char* colour) {
    const int filled = std::clamp(static_cast<int>(fraction * width), 0, width);
    std::printf("%s", colour);
    for (int i = 0; i < width; ++i) std::printf("%s", i < filled ? "█" : "░");
    std::printf("\033[0m");
}

} // namespace

int app::run(int argc, char** argv) {
    Engine e = engine_from_flags(argc, argv);
    e.mixture(1.05);
    e.load_inertia(1.2);

    RawTerminal terminal;
    std::printf("\033[2J");

    const double step     = 0.5_deg;
    const double redline  = 6200.0;
    const double frame    = 1.0 / 30.0;

    double throttle  = 0.0;
    bool   running   = true;
    auto  wall      = std::chrono::steady_clock::now();

    while (running) {
        // ── Advance the engine by one frame of real time ──────────────────
        double simulated = 0.0;
        while (simulated < frame) {
            e.throttle(e.rpm() > redline ? 0.0 : throttle);
            simulated += step / e.crank_speed();
            e.step(step);
            e.bank(Bank::left).clear_samples();     // nobody is listening
            e.bank(Bank::right).clear_samples();
        }

        // A throttle is a plate on a spindle and it is not a switch. The
        // model has always known that — `Induction::open_area` opens as
        // (1 − cos α) across ninety degrees of rotation, which is why the
        // first ten degrees of pedal do almost nothing and the next ten do
        // everything — but this instrument used to offer only shut and wide
        // open, which threw away every interesting part of it. The digits are
        // tenths of throttle; you can sit it at a cruise and watch the vacuum
        // advance wind in.
        switch (const int key = terminal.key()) {
            case ' ':  throttle = (throttle < 0.99) ? 1.0 : 0.0;      break;
            case '+': case '=': throttle = std::min(1.0, throttle + 0.05); break;
            case '-': case '_': throttle = std::max(0.0, throttle - 0.05); break;
            case 'q': case 3: running = false;                        break;
            default:
                if (key >= '0' && key <= '9') throttle = (key - '0') / 9.0;
                break;
        }

        // ── Draw ──────────────────────────────────────────────────────────
        std::printf("\033[H");
        std::printf("  \033[1m%s\033[0m   %s   firing order ",
                    e.name(), name_of(e.crankshaft().plane()));
        for (int c : e.crankshaft().firing_order()) std::printf("%d", c);
        std::printf("            \n\n");

        std::printf("  rpm   ");
        bar(e.rpm() / redline, 46, e.rpm() > redline * 0.93 ? "\033[1;31m" : "\033[1;32m");
        std::printf("  \033[1m%5.0f\033[0m\n", e.rpm());

        std::printf("  load  ");
        bar(throttle, 46, "\033[0;33m");
        std::printf("  \033[1m%3.0f%%\033[0m %-9s\n\n", throttle * 100.0,
                    throttle > 0.99 ? "wide open" : throttle < 0.01 ? "closed" : "part");

        std::printf("   cyl        bore, TDC to BDC              stroke      pressure\n");
        for (int c : e.crankshaft().firing_order()) {
            const Cylinder& cyl = e.cylinder(c);
            const double local  = fold_cycle(e.crank_angle() - cyl.firing_tdc());
            const Phase  ph     = phase_of(local);

            const double travel = cyl.geometry().piston_displacement(local)
                                / cyl.geometry().stroke();

            // The piston, drawn where it actually is in the bore.
            const int width = 24;
            const int at    = std::clamp(static_cast<int>(travel * (width - 1)), 0, width - 1);

            std::printf("    %d   %s│", c, ph.colour);
            for (int i = 0; i < width; ++i)
                std::printf("%s", i == at ? "█" : (i < at ? "─" : " "));
            std::printf("│\033[0m  %s%-8s\033[0m  ", ph.colour, ph.name);

            bar(as::bar(cyl.pressure()) / 70.0, 16, ph.colour);
            std::printf(" %5.1f bar\n", as::bar(cyl.pressure()));
        }

        std::printf("\n  manifold vacuum  %5.1f kPa      spark advance  %4.1f deg BTDC\n",
                    as::kPa(e.manifold_vacuum()), as::deg(e.spark_advance()));
        std::printf("  peak cylinder    %5.1f bar      torque         %4.0f Nm    %4.0f hp\n",
                    as::bar(e.peak_pressure()), e.torque(), as::hp(e.power()));
        std::printf("  backpressure     %5.1f kPa      crank angle    %4.0f deg\n",
                    as::kPa(e.bank(Bank::right).backpressure()), as::deg(e.crank_angle()));

        std::printf("\n  \033[2m0-9 throttle    +/- trim    SPACE wide open    Q stop\033[0m\n");
        std::fflush(stdout);

        wall += std::chrono::microseconds(static_cast<long>(frame * 1e6));
        std::this_thread::sleep_until(wall);
    }
    return 0;
}
