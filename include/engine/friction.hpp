// friction.hpp — the tax.
//
// Everything so far has computed indicated work: what the gas did to the
// piston. None of it reaches the flywheel. Between the crown of the piston and
// the output shaft sit sixteen rings dragging on eight bores, five main
// bearings and eight rod bearings swimming in oil, a valvetrain lifting
// sixteen springs twice per cycle against their own inertia, an oil pump, a
// water pump, and an alternator. They are paid first.
//
// At peak power the tax is perhaps a tenth of the gross. At idle it is all of
// it — an idling engine makes exactly enough to overcome its own friction and
// not one watt more, which is the definition of idling, and the reason an
// engine stops when you stall it rather than slowing gracefully.
//
// ── Chen–Flynn ────────────────────────────────────────────────────────────
//
// A friction mean effective pressure, in the same units as the work it is
// subtracted from, fitted by Chen and Flynn in 1965 to motoring tests:
//
//      FMEP = A + B·p_max + C·Sp + D·Sp²
//
//   A   the constant drag. Bearings, seals, pumps. Present at any speed.
//   B   proportional to peak cylinder pressure — the rings and the main
//       bearings are squeezed harder when the cylinder above them fires
//       harder, so making more power costs more friction, directly. This is
//       the term that makes a supercharger less profitable than it looks.
//   C   proportional to mean piston speed. Hydrodynamic shear: the oil film
//       between ring and bore, being sheared.
//   D   proportional to the square. Turbulent churning, windage, oil thrown
//       off the crank and hit again by it. This is why the friction curve
//       turns sharply upward near the redline and why an engine has a speed
//       past which it makes less power than it did before.
//
// Like Woschni, this is a fit, not a law, and the coefficients are the fit's
// entire content. The ones below are for a pushrod V8 with flat tappets and
// two valves per cylinder — a valvetrain with a great deal of reciprocating
// iron in it, which is most of why A and C are as large as they are.

#pragma once

#include <engine/si.hpp>

namespace engine {

class Friction {
public:
    struct Coefficients {
        double constant;        // Pa
        double per_peak_pressure;      // dimensionless
        double per_piston_speed;       // Pa/(m/s)
        double per_piston_speed_sq;    // Pa/(m/s)²
    };

    // A pushrod V8, flat tappet cam, two valves, accessories driven.
    static constexpr Coefficients pushrod_v8() {
        return { 0.50e5, 0.006, 0.085e5, 0.00090e5 };
    }

    constexpr explicit Friction(Coefficients c) : c_{c} {}

    // Friction mean effective pressure, Pa.
    double mean_effective_pressure(double peak_cylinder_pressure,
                                   double mean_piston_speed) const
    {
        return c_.constant
             + c_.per_peak_pressure      * peak_cylinder_pressure
             + c_.per_piston_speed       * mean_piston_speed
             + c_.per_piston_speed_sq    * mean_piston_speed * mean_piston_speed;
    }

    // The same thing as a torque, which is how the crankshaft would rather
    // hear it. An mep is a work per unit swept volume; a four-stroke sweeps its
    // whole displacement once every two revolutions, so the torque it costs is
    //
    //      τ = mep · V_displacement / 4π
    //
    // and that 4π — rather than 2π — is the four-stroke's entire handicap
    // written as a single constant.
    double torque(double peak_cylinder_pressure,
                  double mean_piston_speed,
                  double total_displacement) const
    {
        return mean_effective_pressure(peak_cylinder_pressure, mean_piston_speed)
             * total_displacement / (2.0 * si::two_pi);
    }

private:
    Coefficients c_;
};

} // namespace engine
