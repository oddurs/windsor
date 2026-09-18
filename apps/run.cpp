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
#include <csignal>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

using namespace engine;
using namespace engine::si;

namespace {

// ── Borrowing someone's terminal ──────────────────────────────────────────
//
// This is the only part of the project that takes a terminal over rather than
// printing to one, and everything it takes it has to give back — including
// when it is interrupted, which is how it will usually end.
//
// It was once left running and came back to several thousand duplicated copies
// of the cluster scrolled down the history, which is what happens when a
// program that redraws in place turns out not to be drawing in place at all.
// Every way that can happen is handled here.
//
//   NOT A TERMINAL. Redirected or piped, the cursor codes are inert text and
//   thirty frames a second simply accumulate, forever. A pipe gets one frame.
//
//   TOO SHORT A WINDOW. Home the cursor and print twenty-one lines into twenty
//   and the terminal scrolls one line per frame: the cursor keeps going home to
//   the top of what is visible, the frame keeps landing lower, and the
//   scrollback fills at about half a megabyte a minute.
//
//   THE ALTERNATE SCREEN, which makes both of those harmless anyway. It is the
//   buffer vim and less and top borrow. It has no scrollback of its own, so
//   nothing drawn here can reach the history, and leaving it restores exactly
//   the screen that was there before — prompt, output and all, as though this
//   had never run.
//
//   INTERRUPTION. Ctrl-C raises SIGINT, which by default kills the process
//   where it stands: no destructor, and a terminal left in raw mode with no
//   echo and no cursor for its owner to fix by feel. The handler sets a flag,
//   the loop notices and returns, and the destructor does the restoring — in
//   the one place that knows how.
//
//   RESIZE. The next frame repaints from a cleared screen, because half a
//   frame at the old width is worse than a blink.
volatile std::sig_atomic_t interrupted = 0;
volatile std::sig_atomic_t resized     = 0;

extern "C" void on_interrupt(int) { interrupted = 1; }
extern "C" void on_resize(int)    { resized     = 1; }

// How tall a frame is, checked against the window rather than hoped about. If
// a line is ever added below without this following it, the alternate screen is
// the net: the frame scrolls where nobody can see it and the history stays
// clean either way.
constexpr int frame_height = 21;

int window_rows() {
    winsize w{};
    return ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 ? w.ws_row : 0;
}

class Screen {
public:
    Screen() {
        tcgetattr(STDIN_FILENO, &saved_);
        termios raw = saved_;
        raw.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        std::signal(SIGINT,   on_interrupt);
        std::signal(SIGTERM,  on_interrupt);
        std::signal(SIGHUP,   on_interrupt);
        std::signal(SIGWINCH, on_resize);

        std::printf("\033[?1049h\033[?25l\033[2J\033[H");
        std::fflush(stdout);
    }

    ~Screen() {
        std::printf("\033[?25h\033[0m\033[?1049l");
        std::fflush(stdout);
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_);
    }

    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

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

// One frame. Every line ends with \033[K and the frame with \033[J, so a
// line that gets shorter cannot leave yesterday's characters behind it, and
// neither can a frame.
void draw(const Engine& e, double throttle, double redline) {
    // ── Draw ──────────────────────────────────────────────────────────────
    std::printf("  \033[1m%s\033[0m   %s   firing order ",
                e.name(), name_of(e.crankshaft().plane()));
    {
        const auto order = e.crankshaft().firing_order();
        for (int i = 0; i < cylinder_count; ++i)
            std::printf("%d%s", order[i], i + 1 < cylinder_count ? "-" : "");
    }
    std::printf("            \033[K\n\033[K\033[K\n");

    std::printf("  rpm   ");
    bar(e.rpm() / redline, 46, e.rpm() > redline * 0.93 ? "\033[1;31m" : "\033[1;32m");
    std::printf("  \033[1m%5.0f\033[0m\033[K\n", e.rpm());

    std::printf("  load  ");
    bar(throttle, 46, "\033[0;33m");
    std::printf("  \033[1m%3.0f%%\033[0m %-9s\033[K\n\033[K\033[K\n", throttle * 100.0,
                throttle > 0.99 ? "wide open" : throttle < 0.01 ? "closed" : "part");

    std::printf("   cyl        bore, TDC to BDC              stroke      pressure\033[K\n");
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
        std::printf(" %5.1f bar\033[K\n", as::bar(cyl.pressure()));
    }

    std::printf("\n  manifold vacuum  %5.1f kPa      spark advance  %4.1f deg BTDC\033[K\n",
                as::kPa(e.manifold_vacuum()), as::deg(e.spark_advance()));

    // Torque and peak pressure are cycle quantities: they do not exist
    // until a cycle has finished. Until then the engine is holding its
    // initial guesses, and printing those as though they were readings is
    // how a gauge lies. A dash is the honest thing to show a driver who
    // has just turned the key.
    if (e.cycles() == 0) {
        std::printf("  peak cylinder    %5s bar      torque         %4s Nm    %4s hp\033[K\n",
                    "--", "--", "--");
    } else {
        std::printf("  peak cylinder    %5.1f bar      torque         %4.0f Nm    %4.0f hp\033[K\n",
                    as::bar(e.peak_pressure()), e.torque(), as::hp(e.power()));
    }
    std::printf("  backpressure     %5.1f kPa      crank angle    %4.0f deg\033[K\n",
                as::kPa(e.bank(Bank::right).backpressure()), as::deg(e.crank_angle()));

    std::printf("\n  \033[2m0-9 throttle    +/- trim    SPACE wide open    Q stop\033[0m\033[K\n");
    std::fflush(stdout);
    std::printf("\033[J");
}

} // namespace

int app::run(int argc, char** argv) {
    Engine e = engine_from_flags(argc, argv);
    e.mixture(1.05);
    e.load_inertia(1.2);

    const double step     = 0.5_deg;
    const double redline  = 6200.0;
    const double frame    = 1.0 / 30.0;

    double throttle  = 0.0;
    bool   running   = true;

    // ── Refuse to animate into something that is not a screen ─────────────
    // Redirected or piped, the cursor codes are inert and the frames simply
    // pile up. One frame is the useful thing to hand a pipe, so that is what
    // a pipe gets.
    if (!isatty(STDOUT_FILENO)) {
        draw(e, throttle, redline);
        std::printf("\n  Not a terminal, so one frame. Run it in one to watch it turn.\n");
        return 0;
    }

    if (const int rows = window_rows(); rows > 0 && rows < frame_height) {
        std::fprintf(stderr,
            "  This window is %d lines and the cluster needs %d. Make it taller:\n"
            "  drawn into a window too short, it scrolls a copy of itself every frame.\n",
            rows, frame_height);
        return 1;
    }

    Screen screen;
    auto  wall      = std::chrono::steady_clock::now();

    while (running && !interrupted) {
        // A resize invalidates the whole screen; repaint from a clean one
        // rather than leave half a frame at the old width.
        if (resized) { resized = 0; std::printf("\033[2J"); }

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
        switch (const int key = screen.key()) {
            case ' ':  throttle = (throttle < 0.99) ? 1.0 : 0.0;      break;
            case '+': case '=': throttle = std::min(1.0, throttle + 0.05); break;
            case '-': case '_': throttle = std::max(0.0, throttle - 0.05); break;
            case 'q': case 3: running = false;                        break;
            default:
                if (key >= '0' && key <= '9') throttle = (key - '0') / 9.0;
                break;
        }

        std::printf("\033[H");
        draw(e, throttle, redline);


        wall += std::chrono::microseconds(static_cast<long>(frame * 1e6));
        std::this_thread::sleep_until(wall);
    }
    return 0;
}
