// port.hpp — the bottleneck.
//
// A naturally aspirated engine is a pump that is not allowed to use a pump.
// Its only means of filling a cylinder is to open a hole and let one
// atmosphere push. Every cubic centimetre of air that gets in has to be
// dragged through the gap between a valve and its seat, and that gap is the
// single tightest constraint on the whole machine. Everything upstream — the
// cam, the manifold, the carburettor — exists to serve it, and everything
// downstream is decided by how much got through.
//
// ── Curtain area ──────────────────────────────────────────────────────────
//
// A valve lifting off its seat does not open a circle. It opens a cylindrical
// curtain around its rim, of circumference πD and height equal to the lift:
//
//      A_curtain = π · D · L
//
// which grows without limit as L grows, and is therefore a lie past about a
// quarter of the valve diameter. Beyond L/D ≈ 0.25 the curtain is wider than
// the throat behind the valve and the restriction moves upstream into the
// port, where it stays no matter how much more lift you buy. This is why cam
// lift has diminishing returns, why porting a head is worth more than a bigger
// cam on an engine that is already breathing, and why the head is always what
// finally limits an engine.
//
// ── Discharge coefficient ─────────────────────────────────────────────────
//
// Cd is the fraction of the geometric area the gas actually uses. It is
// measured on a flow bench, one valve at a time, at a standard depression, and
// it is the number cylinder-head work is bought and sold by. It is never 1. At
// small lift the flow clings to the seat and Cd is high but the area is
// nothing; as lift grows the jet separates from the seat angle and Cd falls
// away. The product Cd·A — effective area — peaks and then goes flat, and the
// lift at which it goes flat is the lift the cam should have.
//
// ── The flow itself ───────────────────────────────────────────────────────
//
// Compressible flow through an orifice, from the energy equation for an
// isentropic expansion — de Saint-Venant and Wantzel, 1839, and unimproved
// since. There are two regimes. Below a pressure ratio of about 0.528 the flow
// chokes: the gas leaves the throat at exactly the local speed of sound and
// pulling harder downstream cannot make more flow arrive, because the news
// that you pulled cannot travel upstream faster than sound. An exhaust valve
// cracking open at 60 bar into an atmospheric pipe is choked, violently and
// briefly, and that moment — blowdown — is the sharp leading edge of every
// exhaust pulse, and therefore of the noise.

#pragma once

#include <algorithm>
#include <cmath>
#include <engine/charge.hpp>
#include <engine/si.hpp>

namespace engine {

namespace port {

// Discharge coefficient as a function of lift over valve diameter.
// A piecewise fit to the shape every flow bench in the world produces: rising
// fast off the seat, peaking near L/D ≈ 0.12, then sagging as the jet detaches.
inline double discharge_coefficient(double lift_over_diameter) {
    constexpr double peak_at = 0.10;   // L/D where Cd is best
    constexpr double peak_cd = 0.68;

    const double x = std::clamp(lift_over_diameter, 0.0, 0.40);

    // Off the seat the flow is still attached and turning neatly through the
    // seat angle, and Cd climbs.
    if (x <= peak_at) return 0.58 + (peak_cd - 0.58) * (x / peak_at);

    // Past the peak the jet separates and a growing share of the curtain is
    // carrying recirculation rather than air. A good head sags slowly; this is
    // most of what porting buys.
    //
    // CALIBRATED, and against the only thing there is to calibrate against: a
    // flow bench. Stock 302 C8OE-F castings are published at about 150 cfm at
    // 0.400 in of lift and 28 inches of water, and 0.90 is the sag that puts
    // this curve there. It was 1.32 for a while, which flowed 139, and the
    // error went unnoticed because the camshaft in `windsor.hpp` was carrying
    // a wrong lift figure at the time and never asked the head for more.
    return std::max(0.30, peak_cd - 0.90 * (x - peak_at));
}

// The area the gas really has. The curtain, discounted by Cd, and capped by the
// throat behind the valve — which is where the restriction goes to live once
// the curtain outgrows it.
inline double effective_area(double lift, double valve_diameter) {
    if (lift <= 0.0) return 0.0;
    // The throat is the narrowed bore just behind the seat, conventionally
    // ground to about 88% of the valve head diameter. Once the curtain exceeds
    // it, lifting the valve further opens nothing at all.
    constexpr double throat_ratio = 0.88;
    const double throat_diameter  = throat_ratio * valve_diameter;

    const double curtain = si::pi * valve_diameter * lift;
    const double throat  = si::pi * 0.25 * throat_diameter * throat_diameter;
    const double cd      = discharge_coefficient(lift / valve_diameter);
    return cd * std::min(curtain, throat);
}

// The pressure ratio at which the throat goes sonic and stops listening.
inline double critical_ratio(double gamma) {
    return std::pow(2.0 / (gamma + 1.0), gamma / (gamma - 1.0));
}

// Mass flow through an orifice, positive in the direction upstream → downstream.
//
// Signs matter here more than anywhere else in the project. Flow reverses
// constantly in a running engine: intake charge is pushed back out of the port
// before the valve closes, exhaust is drawn back into the cylinder during
// overlap, and an engine modelled without reverse flow will show a volumetric
// efficiency it cannot achieve and an idle it does not have. So this function
// takes both states, works out for itself which way the gas wants to go, and
// returns a signed answer.
inline double mass_flow(double area,
                        double p_a, double T_a,     // one side
                        double p_b, double T_b)     // the other
{
    if (area <= 0.0) return 0.0;

    // Gas goes downhill. Which side is uphill is not ours to assume.
    const bool   forward  = p_a >= p_b;
    const double p_up     = forward ? p_a : p_b;
    const double T_up     = forward ? T_a : T_b;
    const double p_down   = forward ? p_b : p_a;
    const double sign     = forward ? +1.0 : -1.0;

    if (p_up <= 0.0 || T_up <= 0.0) return 0.0;

    const double g  = air::gamma(T_up);
    const double pr = std::clamp(p_down / p_up, 0.0, 1.0);

    const double flux = p_up / std::sqrt(air::R * T_up);

    if (pr <= critical_ratio(g)) {
        // Choked. The throat is at Mach 1 and the downstream pressure has
        // stopped being part of the conversation.
        return sign * area * flux * std::sqrt(g)
             * std::pow(2.0 / (g + 1.0), (g + 1.0) / (2.0 * (g - 1.0)));
    }

    // Subsonic. Both pressures matter, and the flow vanishes smoothly as the
    // ratio approaches one, which is how it should be: equal pressures, no flow.
    const double term = std::pow(pr, 2.0 / g) - std::pow(pr, (g + 1.0) / g);
    return sign * area * flux
         * std::sqrt(std::max(0.0, 2.0 * g / (g - 1.0) * term));
}

} // namespace port
} // namespace engine
