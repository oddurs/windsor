// knock.hpp — the constraint that decides everything.
//
// Compression ratio, spark advance, and what the pump charges for the fuel are
// not three decisions. They are one, and this file is it.
//
// ── What is happening ─────────────────────────────────────────────────────
//
// The spark lights one point in the chamber and a flame walks outward at ten
// or twenty metres a second. Ahead of it is the END GAS: the last of the
// mixture, in the far corner, compressed not only by the piston but by the
// expansion of everything that has already burned. Twelve hundred kelvin is
// ordinary. It sits there, waiting its turn, cooking.
//
// Petrol does not need a spark. Given enough temperature and enough time it
// lights on its own — that is what a diesel is. So spark ignition is a race:
// the flame has to arrive and consume the end gas before the end gas goes by
// itself. Lose that race and it does not burn, it detonates, all at once, at a
// thousand metres a second. The pressure spike rings the block, which is the
// sound, and the shockwave strips the insulating boundary layer off the piston
// crown, which is the damage. They are the same event.
//
// ── Livengood and Wu, 1955 ────────────────────────────────────────────────
//
// Autoignition is not a threshold. The end gas does not light at some
// temperature; it lights when it has spent enough time hot, and "enough"
// depends on how hot, continuously, over a history changing every degree of
// crank rotation.
//
// Livengood and Wu proposed keeping a running account. If τ(p,T) is how long
// this mixture would take to light if held at these conditions, then in each
// instant dt it spends the fraction dt/τ of its patience:
//
//      ∫ dt / τ(p, T)  =  1      →  it goes off
//
// There is no chemistry in it. It is the same species of idea as Wiebe's curve
// and Woschni's coefficient — a thing true enough for long enough that the
// mechanistic models built to replace it are mostly used to calibrate it.
//
// The delay comes from Douaud and Eyzat, 1978, fitted on a CFR engine:
//
//      τ = 17.68 · (ON/100)^3.402 · p^(−1.7) · exp(3800/T)     ms, atm, K
//
// Note the exponent on octane. Going from 91 to 100 nearly doubles τ, which is
// the entire commercial value of the number on the pump.
//
// ── An index, not a verdict ───────────────────────────────────────────────
//
// Run this on the engine as Ford built it and the integral comes out near 5,
// which would say a stock 302 detonated itself to pieces every time anyone
// opened the secondaries. It did not.
//
// The integral is fine; the threshold is borrowed. Douaud and Eyzat fitted
// that leading constant on a CFR engine — a single-cylinder laboratory
// instrument with a chamber nothing like a Ford wedge. The value 1 means
// autoignition in THAT engine. Carried elsewhere unre-fitted, the correlation
// keeps its shape and loses its calibration.
//
// Dividing by 5 to make the stock engine read 1.0 would be fitting the model
// to flatter itself, which is the one thing house rule two exists to forbid.
// So the constant stays as published and the number is reported raw. Read it
// as an index: it cannot tell you whether this engine knocks, but it will tell
// you, with the right sensitivities, that it knocks more at 1500 rpm than at
// 4500 because the end gas has more milliseconds to cook; more on 87 octane
// than on 94; more at eleven to one than at nine and a half; and a great deal
// more with sixteen extra degrees wound into it. Those are the questions
// compression ratio and advance are actually chosen by, and they are all
// comparisons.
//
// ── Octane numbers ───────────────────────────────────────────────────────
//
// There are two scales and the pump uses a third. RESEARCH octane is measured
// at 600 rpm and light load; MOTOR octane at 900 rpm with a heated intake, and
// is always the harsher number. An American pump advertises their average,
// which is why the same fuel is 87 in Michigan and 91 in Milan.
//
// Douaud and Eyzat fitted against RON, so RON is what this file wants. Hand it
// a pump figure and the fuel looks four points worse than it is, and the
// engine detonates on paper when it never did in a car.

#pragma once

#include <algorithm>
#include <cmath>
#include <engine/charge.hpp>
#include <engine/si.hpp>

namespace engine {

class Knock {
public:
    // The end gas, unburned, ahead of the flame. It has not been heated by
    // combustion — it is being squeezed by it. Treated as an isentropic
    // compression from the last moment the cylinder's contents were known,
    // which is inlet valve closing, and which is the same anchor Woschni's
    // correlation uses. One reference state, two entirely different uses.
    static double end_gas_temperature(double reference_temperature,
                                      double reference_pressure,
                                      double pressure)
    {
        if (reference_pressure <= 0.0 || pressure <= 0.0) return reference_temperature;
        const double gamma = air::gamma(reference_temperature);
        return reference_temperature
             * std::pow(pressure / reference_pressure, (gamma - 1.0) / gamma);
    }

    // How long this mixture would take to light itself, held here.
    // Douaud & Eyzat. Like Woschni, the correlation is not dimensionally
    // homogeneous and demands its own units — atmospheres and kelvin in,
    // milliseconds out — so the conversions happen here, at the boundary,
    // and SI is what crosses the door in both directions.
    static double ignition_delay(double pressure,
                                 double end_gas_temperature,
                                 double research_octane)
    {
        const double p_atm = std::max(pressure / si::p_atmosphere, 1e-3);
        const double T     = std::max(end_gas_temperature, 200.0);

        constexpr double milliseconds = 1e-3;
        return 17.68 * milliseconds
             * std::pow(research_octane / 100.0, 3.402)
             * std::pow(p_atm, -1.7)
             * std::exp(3800.0 / T);
    }

    // A cylinder's running account, reset every cycle at inlet valve closing.
    //
    // The integral is only accumulated while there IS end gas — from the
    // moment the charge is trapped until the flame has consumed it. An engine
    // whose mixture is entirely burned has nothing left to detonate, which is
    // why a fast-burning chamber tolerates more compression than a slow one,
    // and why the whole industry spent the 1980s moving the spark plug toward
    // the middle of the chamber.
    class Account {
    public:
        void reset() { consumed_ = 0.0; worst_ = 0.0; }

        void elapse(double dt, double pressure, double end_gas_T,
                    double research_octane, double burned_fraction)
        {
            if (burned_fraction >= end_gas_consumed) return;  // nothing left to go off
            const double tau = ignition_delay(pressure, end_gas_T, research_octane);
            if (tau <= 0.0) return;
            consumed_ += dt / tau;
            worst_ = std::max(worst_, consumed_);
        }

        // How much of the end gas's patience was spent. An index, not a
        // verdict — see the long note above on why there is deliberately no
        // `knocking()` here returning a bool. This file is not entitled to
        // that opinion about this engine, and pretending otherwise would be
        // the most confident lie in the project.
        double severity() const { return worst_; }

    private:
        // A Wiebe burn never quite reaches 1; the last fraction of a percent
        // is the crevice volume, and it was never going to detonate anyway.
        static constexpr double end_gas_consumed = 0.98;

        double consumed_ = 0.0, worst_ = 0.0;
    };
};

} // namespace engine
