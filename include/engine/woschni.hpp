// woschni.hpp — heat going where it does no good.
//
// About a third of the fuel's energy leaves as work at the crank. About a
// third leaves hot out of the exhaust. The last third crosses the chamber
// walls into the coolant and is thrown away into the air through the radiator,
// and this file is the size of that third.
//
// It matters more than it sounds. Heat lost during combustion and expansion is
// lost pressure, and lost pressure is lost torque directly. It is also the
// reason an engine makes more power on a cold morning, the reason a small-bore
// engine is more efficient than a big-bore one at the same displacement, and
// the reason the first firing stroke after a cold start makes almost nothing.
//
// Gerhard Woschni published the correlation in 1967, and it is still the one
// people use:
//
//      h = 3.26 · B^(−0.2) · p^(0.8) · T^(−0.55) · w^(0.8)
//
// A WARNING, and the one place in this project where the SI rule in si.hpp
// bends. This equation is not dimensionally homogeneous. It is a curve fit,
// and the constant 3.26 carries whatever units are needed to make the two
// sides balance — which means it is only correct if you feed it bore in
// METRES, pressure in KILOPASCALS, temperature in KELVIN and velocity in
// METRES PER SECOND, whereupon it returns W/(m²·K). Feed it pascals and it
// will return a number four thousand times too large, silently, and your
// engine will run stone cold and make far too much power.
//
// This is the difference between a law and a correlation, and it is worth
// stating plainly: everything else in this project is a statement about how
// the world is. This is a statement about what Woschni measured. The
// conversion to kPa happens inside this file, at the boundary, where such
// things belong.

#pragma once

#include <cmath>
#include <engine/si.hpp>

namespace engine {

class Woschni {
public:
    // The gas velocity scale w has two parts.
    //
    //   C₁ · Sp          the piston dragging the charge around. Always present.
    //                    2.28 with the valves shut; 6.18 during gas exchange,
    //                    when the incoming jet stirs the chamber far harder
    //                    than the piston does.
    //
    //   C₂ · ΔV·T₁/(p₁V₁) · (p − p_motored)
    //                    combustion. The term is the excess of the firing
    //                    pressure over what the same cylinder would have shown
    //                    with no spark — that is, the pressure rise the flame
    //                    itself caused — and it stands in for the expansion of
    //                    the burned gas shoving the unburned charge against
    //                    the walls. It is zero until the mixture lights, and
    //                    it is the reason heat loss triples the instant it does.

    static constexpr double C1_closed   = 2.28;
    static constexpr double C1_exchange = 6.18;
    static constexpr double C2_burning  = 3.24e-3;   // m/(s·K)

    // reference_* is the cylinder state at inlet valve closing: the last moment
    // the cylinder held a known mass at a known condition, and the anchor the
    // motored pressure is projected forward from.
    struct Reference {
        double pressure;    // Pa at IVC
        double temperature; // K  at IVC
        double volume;      // m³ at IVC
    };

    // h, the convective coefficient, in W/(m²·K).
    //   bore              m
    //   pressure          Pa      (converted to kPa below — see the warning)
    //   temperature       K
    //   mean_piston_speed m/s
    //   swept_volume      m³
    //   motored_pressure  Pa      — what this cylinder would read, unfired
    //   valves_open       true during intake and exhaust
    static double coefficient(double bore,
                              double pressure,
                              double temperature,
                              double mean_piston_speed,
                              double swept_volume,
                              double motored_pressure,
                              const Reference& reference,
                              bool valves_open)
    {
        const double C1 = valves_open ? C1_exchange : C1_closed;

        double w = C1 * mean_piston_speed;
        if (const double rise = pressure - motored_pressure; rise > 0.0) {
            w += C2_burning * swept_volume * reference.temperature
               / (reference.pressure * reference.volume) * rise;
        }

        // The boundary. Pascals in, kilopascals to Woschni.
        const double p_kPa = pressure * 1e-3;

        return 3.26
             * std::pow(bore,        -0.2)
             * std::pow(p_kPa,        0.8)
             * std::pow(temperature, -0.55)
             * std::pow(std::max(w, 1.0), 0.8);
    }

    // The pressure an unfired cylinder would show, carried forward from the
    // reference state by a polytropic compression. n = 1.32 rather than γ
    // because a motored cylinder is losing heat the whole way up, so it arrives
    // at TDC cooler and softer than an adiabatic compression would predict.
    static double motored_pressure(const Reference& reference, double volume) {
        constexpr double n = 1.32;
        return reference.pressure * std::pow(reference.volume / volume, n);
    }
};

} // namespace engine
