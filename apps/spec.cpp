// spec.cpp — the shop manual page.
//
// Prints what was specified, and then what was never specified and came out
// anyway. The line between the two is the whole argument of the project, and
// it is drawn across the middle of this page.

#include "apps.hpp"
#include <cstdio>

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
    std::printf("  duration (adv.)      %.0f deg in / %.0f deg ex\n",
                as::deg(cam.intake().duration()), as::deg(cam.exhaust().duration()));
    std::printf("  lift at the valve    %.3f in / %.3f in\n",
                as::in(cam.intake().max_lift()), as::in(cam.exhaust().max_lift()));
    std::printf("  lobe separation      %.0f deg\n", as::deg(cam.lobe_separation_angle()));
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

    std::printf("\n\033[2m  The engine fires evenly either way. Nothing at the flywheel can tell\n");
    std::printf("  these two apart. Run `windsor record` and `windsor record --flat`.\033[0m\n\n");
    return 0;
}
