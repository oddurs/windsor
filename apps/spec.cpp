// spec.cpp — the shop manual page.
//
// Prints what was specified, and then what was never specified and came out
// anyway. The line between the two is the whole argument of the project, and
// it is drawn across the middle of this page.

#include "apps.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace engine;
using namespace engine::si;

namespace {

void rule(const char* title) {
    std::printf("\n\033[1m%s\033[0m\n", title);
}

const char* ordinal_gap(double rad) {
    static char buf[8];
    std::snprintf(buf, sizeof buf, "%.0f", as::deg(rad));
    return buf;
}


// The oldest way of telling two crankshafts apart: stand at the front of the
// engine and look down the length of the shaft. The throws either make a cross
// or they make a line, and that is the entire taxonomy.
void down_the_nose(const Crankshaft& ck) {
    constexpr int rows = 7, cols = 27;
    char canvas[rows][cols + 1];
    for (int r = 0; r < rows; ++r) {
        std::memset(canvas[r], ' ', cols);
        canvas[r][cols] = '\0';
    }

    const double reference = ck.throws()[0];
    for (double psi : ck.throws()) {
        const double angle = psi - reference;
        // Stepped in whole units, because a float loop that ends on its own
        // last value is a loop that does not reach it. Characters are about
        // twice as tall as they are wide, so the horizontal step is doubled
        // to keep a circle circular.
        constexpr int tip = 9;
        for (int step = 2; step <= tip; ++step) {
            const double radius = step / 3.0;
            const int x = 13 + static_cast<int>(std::lround(radius * 2.0 * std::sin(angle)));
            const int y = 3  - static_cast<int>(std::lround(radius * std::cos(angle)));
            if (x >= 0 && x < cols && y >= 0 && y < rows)
                canvas[y][x] = (step == tip) ? 'O' : '.';
        }
    }
    canvas[3][13] = '+';

    std::printf("      looking down the nose of the crank\n\n");
    for (int r = 0; r < rows; ++r) std::printf("      \033[1m%s\033[0m\n", canvas[r]);
}

const char* shape_of(const Harmonic& h) {
    if (h.peak_force < 1.0 && h.peak_couple < 1.0) return "nothing";
    if (h.eccentricity < 0.25) return "a circle   counterweights take it";
    if (h.eccentricity > 0.90) return "a line     nothing can touch it";
    return "an ellipse";
}

} // namespace

int app::spec(int argc, char** argv) {
    Engine e = engine_from_flags(argc, argv);
    const Geometry&   g  = e.geometry();
    const Crankshaft& ck = e.crankshaft();
    const Camshaft    cam = windsor::stock_cam();

    std::printf("\n\033[1m%s\033[0m\n", e.name());

    rule("SHORT BLOCK");
    std::printf("  bore x stroke        %.3f x %.3f in        %.1f x %.1f mm\n",
                as::in(g.bore()), as::in(g.stroke()), as::mm(g.bore()), as::mm(g.stroke()));
    std::printf("  displacement         %.1f cu in             %.0f cc\n",
                as::ci(e.displacement()), as::cc(e.displacement()));
    std::printf("  compression ratio    %.2f : 1\n", g.compression_ratio());
    std::printf("  clearance volume     %.2f cc\n", as::cc(g.clearance_volume()));
    std::printf("  rod length           %.3f in                lambda = %.4f  (l/a = %.2f)\n",
                as::in(g.rod_length()), g.rod_ratio(), 1.0 / g.rod_ratio());
    std::printf("  bore/stroke          %.3f                   %s\n", g.bore_stroke_ratio(),
                g.bore_stroke_ratio() > 1.0 ? "oversquare" : "undersquare");
    std::printf("  mean piston speed    %.1f m/s at 6000 rpm    (nothing lives above 30)\n",
                g.mean_piston_speed(6000.0_rpm));

    rule("CAMSHAFT");
    std::printf("  duration, advertised %.0f deg in / %.0f deg ex\n",
                as::deg(cam.intake().duration()), as::deg(cam.exhaust().duration()));
    std::printf("  lift at the valve    %.3f in / %.3f in\n",
                as::in(cam.intake().max_lift()), as::in(cam.exhaust().max_lift()));
    std::printf("  lobe separation      %.1f deg\n", as::deg(cam.lobe_separation_angle()));
    std::printf("  timing               IVO %.0f BTDC   IVC %.0f ABDC\n",
                360.0 - as::deg(cam.intake().opens()), as::deg(cam.intake().closes()) - 540.0);
    std::printf("                       EVO %.0f BBDC   EVC %.0f ATDC\n",
                180.0 - as::deg(cam.exhaust().opens()), as::deg(cam.exhaust().closes()) - 360.0);
    std::printf("  overlap              %.0f deg                 %s\n", as::deg(cam.overlap()),
                as::deg(cam.overlap()) < 50 ? "polite" : "the lope");

    // ── The line ──────────────────────────────────────────────────────────
    std::printf("\n\033[2m  ---- everything above was specified. everything below came out. ----\033[0m\n");

    rule("CRANKSHAFT");
    std::printf("  forging              %s\n", name_of(ck.plane()));
    std::printf("  firing order         ");
    const auto order = ck.firing_order();
    for (int i = 0; i < cylinder_count; ++i)
        std::printf("%d%s", order[i], i + 1 < cylinder_count ? "-" : "\n");

    std::printf("  the engine fires     every %s deg, eight times per cycle\n",
                ordinal_gap(ck.firing_intervals()[0]));

    rule("WHAT EACH BANK HEARS");
    std::printf("  A manifold is not connected to an engine. It is connected to four\n");
    std::printf("  cylinders, and it only ever hears those.\n\n");
    for (Bank b : {Bank::right, Bank::left}) {
        const auto cyl = ck.firing_order(b);
        const auto gap = ck.firing_intervals(b);
        std::printf("  %-5s bank   cyl %d %d %d %d   fires at   ",
                    name_of(b), cyl[0], cyl[1], cyl[2], cyl[3]);
        for (int i = 0; i < 4; ++i)
            std::printf("%3.0f%s", as::deg(gap[i]), i < 3 ? " - " : "\n");
    }

    std::printf("\n");
    if (ck.banks_fire_evenly()) {
        std::printf("  \033[1mEVEN.\033[0m Both banks exhale in perfect thirds of a revolution and lock\n");
        std::printf("  in phase with each other. The harmonics stack cleanly at twice the\n");
        std::printf("  frequency and there is nothing left to beat against anything else.\n");
        std::printf("  This is the shriek.\n");
    } else {
        std::printf("  \033[1mLOPSIDED.\033[0m Each bank coughs twice in quick succession, waits three\n");
        std::printf("  quarters of a turn, and coughs again. The two banks are out of step\n");
        std::printf("  with each other and the interference between them never resolves.\n");
        std::printf("  This is the burble.\n");
    }

    // ── And what it cost ──────────────────────────────────────────────────
    rule("WHAT IT COST TO GET THAT");
    std::printf("  A piston does not travel sinusoidally. Kill the once-per-turn term with\n");
    std::printf("  a counterweight and a twice-per-turn one is still there, and nothing\n");
    std::printf("  bolted to a shaft turning at \u03c9 can cancel a force at 2\u03c9.\n\n");

    down_the_nose(ck);

    const Balance balance = e.balance();
    const double  at      = 3000.0_rpm;
    const Harmonic first  = balance.primary(at);
    const Harmonic second = balance.secondary(at);

    std::printf("\n  shaking, at 3000 rpm, from 0.78 kg of reciprocating mass per bore:\n\n");
    std::printf("               force        couple        traces\n");
    std::printf("  primary   %7.0f N    %7.0f Nm     %s\n",
                first.peak_force, first.peak_couple, shape_of(first));
    std::printf("  secondary %7.0f N    %7.0f Nm     %s\n",
                second.peak_force, second.peak_couple, shape_of(second));

    std::printf("\n");
    if (second.peak_force < 1.0) {
        std::printf("  \033[1mNOTHING LEFT OVER.\033[0m The secondaries cancel each other exactly, and\n");
        std::printf("  the primary couple traces a circle, which is a rotating imbalance, which\n");
        std::printf("  a counterweight can be drawn to oppose. The engine holds still. It will\n");
        std::printf("  idle at 600 rpm on soft mounts without walking across the bay.\n\n");
        std::printf("  \033[2mThis, and not the noise, is why Detroit forged crosses.\033[0m\n");
    } else {
        std::printf("  \033[1m%.0f NEWTONS, TWICE PER REVOLUTION\033[0m, along one line, and no\n", second.peak_force);
        std::printf("  counterweight can touch it — a shaft turning once per revolution cannot\n");
        std::printf("  oppose a force that goes twice. It has to be carried by the mounts, the\n");
        std::printf("  subframe, and the driver.\n\n");
        std::printf("  \033[2mThis is the bill for the noise, and it is why almost nobody pays it.\033[0m\n");
    }

    std::printf("\n\033[2m  The engine fires evenly either way. Nothing at the flywheel can tell\n");
    std::printf("  these two apart. Run `windsor record` and `windsor record --flat`.\033[0m\n\n");
    return 0;
}
