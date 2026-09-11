// charge.hpp — the working fluid, and what is dissolved in it.
//
// An engine does not burn petrol. It heats air. The fuel is a way of putting
// heat into the air quickly and at a chosen moment, and everything the engine
// produces, it produces because hot air pushes harder than cold air. Change
// nothing but the temperature of the gas in the cylinder and the crankshaft
// cannot tell what you burned.
//
// The gas is treated as ideal, which it very nearly is at the pressures a
// naturally aspirated engine reaches, and as having a specific heat that
// varies with temperature, which it emphatically does. That second concession
// is not fussiness. The ratio of specific heats, γ, is the exponent on every
// compression and expansion in the cycle, and it falls from 1.40 in a cold
// intake charge to about 1.25 in 2500 K combustion products. An air-standard
// cycle computed at a constant γ = 1.4 will promise you roughly a fifth more
// thermal efficiency than the engine can actually deliver, and the whole of
// that missing fifth is this one number sagging as the gas gets hot.

#pragma once

#include <algorithm>
#include <cmath>
#include <engine/si.hpp>

namespace engine {

// ── The fuel ──────────────────────────────────────────────────────────────
// Pump petrol, as sold. Not iso-octane; the surrogate a chemist would use has
// a slightly different heating value and a much better octane rating than what
// comes out of a nozzle.
struct Fuel {
    double lower_heating_value;   // J/kg — the useful figure. The higher value
                                  // counts the latent heat of the water vapour
                                  // in the exhaust, which leaves out of the
                                  // tailpipe as steam and is never recovered.
    double stoichiometric_ratio;  // kg air per kg fuel for complete combustion

    // Latent heat of vaporisation, J/kg. Every drop of petrol that goes into
    // an engine arrives as a liquid and has to be boiled before it can burn,
    // and the heat to boil it is stolen from the air it is boiling into. This
    // is not a detail. See `charge_cooling` below.
    double latent_heat;

    // RESEARCH octane, not the number on the pump. An American pump advertises
    // (RON+MON)/2, which is about four points lower, which is why the same
    // fuel is 87 in Michigan and 91 in Milan. The knock model was fitted
    // against RON and will detonate a perfectly good engine on paper if handed
    // the other one. See `knock.hpp`.
    double research_octane;

    // What came out of a pump in 1968, with tetraethyl lead still in it.
    // Leaded regular ran about 94 RON — better than most unleaded regular
    // today, which is the quiet reason a 9.5:1 engine of this era was
    // perfectly happy and its 1975 descendant had to drop to 8.0:1.
    static constexpr Fuel gasoline() {
        return { 44.0e6, 14.7, 350.0e3, 94.0 };
    }

    // Fuel mass to burn with a given mass of air at a given equivalence ratio.
    // φ = 1 is stoichiometric. φ > 1 is rich: surplus fuel, cooler burn, more
    // power — a well-known and slightly embarrassing fact, since the extra
    // fuel makes power mostly by refusing to burn and cooling the charge as it
    // evaporates. Peak torque lives near φ = 1.1, peak efficiency near 0.9.
    constexpr double mass_for(double air_mass, double equivalence_ratio) const {
        return air_mass * equivalence_ratio / stoichiometric_ratio;
    }

    // How far the charge is chilled by the fuel evaporating into it.
    //
    //      ΔT = m_fuel · h_fg / (m_air · c_p)
    //
    // and at stoichiometric that is about 24 K of free intercooling, which
    // makes the charge denser, which lets more air into the cylinder, which
    // makes more power than the fuel would have made by burning.
    //
    // It is also why running rich makes power. The two comments in this file
    // that say so have been saying so since the first week without anything
    // behind them; this is the something. Past stoichiometric the surplus fuel
    // cannot burn — `combustion_efficiency` below says as much — but it can
    // still evaporate, and it goes on chilling the charge and quieting the end
    // gas long after it has stopped contributing any heat. A racer running
    // 12:1 air-fuel is not burning that extra petrol. He is air-conditioning
    // with it.
    // Not all of it evaporates where it would do good. Up to about
    // stoichiometric, a warm manifold gets essentially the whole charge into
    // vapour before the inlet valve. Past that the air is close to saturated
    // with petrol at manifold temperature, and the surplus stays liquid — it
    // wets the runner walls, arrives as droplets, and boils inside the
    // cylinder during compression, where the cooling comes too late to have
    // let any more air in. So the useful half of it stops counting. This is
    // why a carburettor's fuel film lags the throttle, and why the accelerator
    // pump exists to cover the moment it does.
    constexpr double charge_cooling(double equivalence_ratio) const {
        constexpr double cp_air   = 1005.0;    // J/(kg·K), near ambient
        constexpr double in_port  = 0.5;       // of the surplus, past stoich

        const double evaporating = equivalence_ratio <= 1.0
            ? equivalence_ratio
            : 1.0 + (equivalence_ratio - 1.0) * in_port;

        return evaporating / stoichiometric_ratio * latent_heat / cp_air;
    }

    // How much of it actually burns.
    //
    // Never all of it. Even with oxygen to spare, a couple of percent survives:
    // fuel hiding in the crevice above the top ring where the flame is too
    // narrow to enter, and a quenched layer a few tenths of a millimetre thick
    // against every wall, where the metal pulls heat out of the flame front
    // faster than the reaction can make it. That fuel leaves unburned, and it
    // is most of what a catalytic converter is there to finish.
    //
    // Past stoichiometric it is no longer a matter of geometry. There is
    // simply not enough oxygen in the cylinder, and the surplus fuel leaves as
    // carbon monoxide and hydrogen no matter how good the chamber is. The
    // burned fraction falls as 1/φ, which is why running rich makes power by a
    // route that has nothing to do with burning more fuel — the extra petrol
    // makes power by evaporating, cooling the charge, and letting more air in.
    constexpr double combustion_efficiency(double equivalence_ratio) const {
        constexpr double crevice_and_quench = 0.98;
        return equivalence_ratio <= 1.0
             ? crevice_and_quench
             : crevice_and_quench / equivalence_ratio;
    }
};

// ── The gas ───────────────────────────────────────────────────────────────

namespace air {

inline constexpr double R = 287.05;   // J/(kg·K), specific gas constant, dry air

// γ(T). A straight-line fit through the real curve, which is what every
// engine-cycle code in the world actually uses, and which is accurate to about
// one percent from ambient to 3000 K — far better than the combustion model it
// feeds. It is clamped below because the fit, extended, would eventually pass
// through 1.0 and then the expansion would do no work at all.
inline double gamma(double T) {
    return std::max(1.24, 1.400 - 7.0e-5 * (T - 300.0));
}

inline double cv(double T) { return R / (gamma(T) - 1.0); }
inline double cp(double T) { return cv(T) * gamma(T);     }

// pV = mRT, in its three useful rearrangements. Named rather than written
// inline because a sign or a swap here is undetectable downstream — it simply
// produces an engine that makes slightly the wrong power, forever.
inline double pressure   (double mass, double T, double V) { return mass * R * T / V;    }
inline double temperature(double mass, double p, double V) { return p * V / (mass * R); }
inline double mass       (double p, double T, double V)    { return p * V / (R * T);    }

// Speed of sound. The pressure waves in the manifolds travel at this, and
// because it goes as √T the exhaust — at 900 K — carries sound at nearly twice
// the speed the intake does. That asymmetry is why exhaust tuning works over a
// wide rev range and intake tuning does not.
inline double speed_of_sound(double T) { return std::sqrt(gamma(T) * R * T); }

} // namespace air

// ── The contents of one cylinder at one instant ───────────────────────────
// Mass, temperature and volume are the state. Pressure is not stored, because
// storing it would let it drift out of agreement with the other three, and a
// cylinder whose pressure disagrees with its own gas law is a bug that will
// take an afternoon to find.
struct Charge {
    double mass;         // kg of gas in the cylinder
    double temperature;  // K
    double burned;       // 0..1 mass fraction of the fuel consumed so far

    double pressure(double volume) const {
        return air::pressure(mass, temperature, volume);
    }
};

} // namespace engine
