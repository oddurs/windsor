// card.cpp — the indicator diagram.
//
// In the 1790s James Watt and John Southern bolted a small pencil to a spring
// loaded by boiler pressure, and a drum to the engine that rotated with the
// piston, and let the machine draw its own picture. Pressure up the page,
// volume across it, one closed loop per cycle. Two hundred and thirty years
// later there is still no better way to see what a cylinder is doing, and
// every quantity an engine is judged by is a feature of this one shape.
//
// Watt kept the method secret for years, because the area inside the loop is
// the work done in a cycle, and being able to measure the work an engine
// actually did — rather than argue about it — is what let him sell engines on
// the promise of coal saved. This picture is the reason the thermodynamics of
// engines is an experimental science and not a branch of opinion.
//
// ── What to look at ───────────────────────────────────────────────────────
//
// The loop goes CLOCKWISE. Up the right side as the piston compresses, a near-
// vertical jump at the top where the fire goes in, down the left as the gas
// expands against a retreating piston, and a horizontal step out at the bottom
// when the exhaust valve opens. Enclosed area is work out.
//
// Below it, small and flat and easily missed, is the second loop — the
// exhaust and intake strokes. In a throttled petrol engine this one runs
// ANTICLOCKWISE, which means its area is work IN: the engine spending energy
// to breathe, every cycle, whether it fired or not. That is the pumping loss,
// it is the price of controlling an engine by suffocating it, and on a big
// engine at light load it is most of the fuel you are buying.
//
// The two cards below are the same cylinder at the same speed, at two throttle
// positions, drawn to the same scale. Everything interesting is in the
// difference between them.

#include "apps.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace engine;
using namespace engine::si;

namespace {

struct Trace {
    std::vector<double> volume, pressure, angle;
    double gross_mep = 0.0, pumping_mep = 0.0, peak = 0.0;
};

// One complete cycle of cylinder number one, after it has stopped changing.
Trace take_card(Engine& e, double rpm, double throttle) {
    e.throttle(throttle);
    e.mixture(1.05);
    for (int i = 0; i < 260000; ++i) {
        e.brake_torque(std::clamp(e.torque() + (e.rpm() - rpm) * 0.6, -150.0, 1200.0));
        e.step(0.25_deg);
    }

    // Wind to this cylinder's own top dead centre so the card starts where the
    // engineer expects it to.
    const double tdc = e.cylinder(1).firing_tdc();
    while (std::abs(e.crank_angle() - tdc) > 0.3_deg) e.step(0.25_deg);

    Trace card;
    const Geometry& g = e.geometry();

    for (int i = 0; i < 2880; ++i) {
        const double local = std::fmod(e.crank_angle() - tdc + 4.0 * two_pi, 2.0 * two_pi);
        const double V = e.cylinder(1).volume();
        const double p = e.cylinder(1).pressure();

        card.volume.push_back(V);
        card.pressure.push_back(p);
        card.angle.push_back(local);
        card.peak = std::max(card.peak, p);

        // p·dV, split the way a four-stroke splits: the closed strokes either
        // side of firing TDC are the power loop, the two in between are the
        // engine breathing. Sign falls out of dV/dθ; nothing needs negating.
        const double work = p * g.dV_dtheta(local) * 0.25_deg;
        const bool   closed = (local < pi) || (local > 3.0 * pi);
        (closed ? card.gross_mep : card.pumping_mep) += work;

        e.step(0.25_deg);
    }

    card.gross_mep   /= g.swept_volume();
    card.pumping_mep /= g.swept_volume();
    return card;
}

// Draw one card. Pressure is logarithmic up the page — a choice, and one that
// has to be admitted: on a linear axis the whole pumping loop is a hairline
// half a millimetre thick at the bottom of a sixty-bar diagram, which is
// exactly why it is so easy to forget it is there. Log makes it visible and
// costs the property that area equals work, so the work is printed underneath
// in numbers instead. It also has a bonus: a polytropic compression, pV^n
// constant, is a STRAIGHT LINE on log-log, and the compression stroke here
// really is one.
void draw(const Trace& card, const Geometry& g, const char* title, double ceiling) {
    constexpr int rows = 18, cols = 60;
    char  glyph[rows][cols];
    char  tint [rows][cols];
    for (int r = 0; r < rows; ++r) {
        std::memset(glyph[r], ' ', cols);
        std::memset(tint [r], 0,   cols);
    }

    const double v_min = g.clearance_volume();
    const double v_max = g.clearance_volume() + g.swept_volume();
    const double p_min = 0.20e5, p_max = ceiling;

    for (std::size_t i = 0; i < card.volume.size(); ++i) {
        const double vx = (card.volume[i] - v_min) / (v_max - v_min);
        const double py = (std::log(std::clamp(card.pressure[i], p_min, p_max))
                         - std::log(p_min)) / (std::log(p_max) - std::log(p_min));

        const int x = std::clamp(int(vx * (cols - 1)), 0, cols - 1);
        const int y = std::clamp(int((1.0 - py) * (rows - 1)), 0, rows - 1);

        const double deg = as::deg(card.angle[i]);
        const char   phase = deg < 180.0 ? 'P' : deg < 360.0 ? 'E' : deg < 540.0 ? 'I' : 'C';
        glyph[y][x] = (phase == 'P') ? '#' : '.';
        tint [y][x] = phase;
    }

    std::printf("\n  \033[1m%s\033[0m\n", title);
    for (int r = 0; r < rows; ++r) {
        const double p = std::exp(std::log(p_min)
                       + (1.0 - double(r) / (rows - 1)) * (std::log(p_max) - std::log(p_min)));
        std::printf("  %5.1f |", as::bar(p));
        for (int c = 0; c < cols; ++c) {
            if (glyph[r][c] == ' ') { std::printf(" "); continue; }
            const char* colour = tint[r][c] == 'P' ? "\033[1;31m"
                               : tint[r][c] == 'E' ? "\033[0;90m"
                               : tint[r][c] == 'I' ? "\033[0;36m" : "\033[0;34m";
            std::printf("%s%c\033[0m", colour, glyph[r][c]);
        }
        std::printf("\n");
    }
    std::printf("    bar +");
    for (int c = 0; c < cols; ++c) std::printf("-");
    std::printf("\n         TDC%*s%*s\n", (cols / 2) - 1, "volume", (cols / 2) - 8, "BDC");

    std::printf("\n         gross %5.2f bar   pumping %+5.2f bar   net %5.2f bar   peak %.1f bar\n",
                as::bar(card.gross_mep), as::bar(card.pumping_mep),
                as::bar(card.gross_mep + card.pumping_mep), as::bar(card.peak));
}

} // namespace

int app::card(int argc, char** argv) {
    constexpr double at_rpm = 2000.0;

    Engine wide = engine_from_flags(argc, argv);
    Engine shut = engine_from_flags(argc, argv);

    const Trace open   = take_card(wide, at_rpm, 1.00);
    const Trace closed = take_card(shut, at_rpm, 0.06);
    const double ceiling = std::max(open.peak, closed.peak) * 1.1;

    std::printf("\n\033[1mindicator card\033[0m — %s, cylinder 1, %.0f rpm\n", wide.name(), at_rpm);
    std::printf("\033[2m  pressure is logarithmic, so area is not work; the work is printed below\033[0m\n");

    draw(open,   wide.geometry(), "WIDE OPEN THROTTLE", ceiling);
    draw(closed, shut.geometry(), "THROTTLE CLOSED",    ceiling);

    std::printf("\n  \033[1;31m#\033[0m power   \033[0;90m.\033[0m exhaust   "
                "\033[0;36m.\033[0m intake   \033[0;34m.\033[0m compression\n");

    const double cost = 100.0 * std::abs(closed.pumping_mep)
                      / std::max(closed.gross_mep, 1.0);
    std::printf("\n  Throttled, the engine spends \033[1m%.0f%%\033[0m of everything it makes on\n", cost);
    std::printf("  breathing. Wide open it spends %.0f%%. That is the whole cost of\n",
                100.0 * std::abs(open.pumping_mep) / std::max(open.gross_mep, 1.0));
    std::printf("  controlling a petrol engine by making it difficult to breathe, and it\n");
    std::printf("  is why a diesel, which throttles on fuel alone, is more efficient at\n");
    std::printf("  part load and barely more efficient at full.\n\n");
    return 0;
}
