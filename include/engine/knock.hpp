// knock.hpp — the constraint that decides everything.
//
// Compression ratio, spark advance, and what the pump charges for the fuel are
// not three separate decisions. They are one decision, and this file is it.
//
// ── What is actually happening ────────────────────────────────────────────
//
// The spark lights one point in the chamber and a flame front walks outward
// from it at ten or twenty metres a second. Ahead of that front is the END
// GAS: the last of the mixture, sitting in the far corner of the chamber, and
// being compressed not only by the piston but by the expansion of everything
// that has already burned. It gets very hot — twelve hundred kelvin is
// ordinary — and it sits there, waiting its turn, cooking.
//
// Petrol does not need a spark. Given enough temperature and enough time it
// will light on its own; that is what a diesel is. So the whole of spark
// ignition is a race: the flame front has to arrive and consume the end gas
// before the end gas decides to go by itself.
//
// If it loses that race, the end gas does not burn — it detonates, all of it
// at once, at something like a thousand metres a second. The pressure spike
// rings the block like a bell, which is the sound, and the shockwave strips
// the insulating boundary layer off the piston crown, which is the damage. A
// sustained detonation will melt a hole through the top of a piston in under a
// minute. The noise and the destruction are the same event.
//
// ── Livengood and Wu, 1955 ────────────────────────────────────────────────
//
// The difficulty is that autoignition is not a threshold. The end gas does not
// light when it gets to some temperature; it lights when it has spent enough
// time hot, and "enough" depends on how hot, continuously, over a history that
// is changing every degree of crank rotation.
//
// John Livengood and Paul Wu proposed that you simply keep a running account.
// If τ(p, T) is how long this mixture would take to light if held at these
// conditions, then in each instant dt it uses up the fraction dt/τ of its
// patience. Integrate:
//
//      ∫ dt / τ(p, T)   =   1      →  it goes off
//
// That is the whole model. It has no chemistry in it. It is the same species
// of idea as Wiebe's curve and Woschni's coefficient — a thing that turned out
// to be true enough, for long enough, that the mechanistic models built to
// replace it are still mostly used to calibrate it.
//
// The delay τ comes from Douaud and Eyzat, 1978, who fitted it on a CFR engine:
//
//      τ = 17.68 · (ON/100)^3.402 · p^(−1.7) · exp(3800/T)     ms, atm, K
//
// Note the exponent on octane. Going from 91 to 100 octane nearly doubles τ —
// which is the entire commercial value of the number on the pump, and why a
// tenth of a point of it is worth arguing about.
//
// ── Why this file is worth having ─────────────────────────────────────────
//
// Without it, nothing in the project stops you. Raise the compression ratio to
// fourteen, wind in sixty degrees of advance, and the model will hand you the
// extra power with a straight face — because the only thing that was ever
// going to object is the fuel, and the fuel was not being asked.
//
// With it, three numbers that were arbitrary become a system that has to
// agree with itself: 9.5:1, 34° of total advance, and the octane rating of
// what was in the tank in 1968. Change any one and the others have to move.
//
// ── What the number means, and what it does not ───────────────────────────
//
// An awkward result, reported rather than tidied away.
//
// Run this model on the engine as Ford built it — 9.5:1, 94 RON leaded
// regular, 34° of total advance — and the integral comes out around 4.8 at
// wide open throttle. Taken at face value that says a stock 1968 302 detonated
// itself to pieces every time anyone opened the secondaries, which it
// conspicuously did not.
//
// The integral is not wrong; the threshold is borrowed. Douaud and Eyzat fitted
// that leading constant of 17.68 on a CFR engine — a single-cylinder laboratory
// instrument with a variable head, a chamber nothing like a 1968 Ford wedge,
// and a thermal environment nothing like a cast-iron V8 at full load. The
// value "1" means autoignition in THAT engine. Carried to another chamber
// without being re-fitted, the correlation keeps its shape and loses its
// calibration, and comes out several times pessimistic.
//
// Two of those times are this project's own doing and are worth naming. The
// engine here makes about ten percent more power than the real one and runs
// correspondingly higher peak pressures, and τ goes as p^−1.7, so a sixty-bar
// peak where the real engine saw fifty is on its own worth a factor of 1.6.
// And the end gas is treated as isentropic from inlet valve closing with no
// heat lost to the chamber walls it is pressed against, which makes it hotter
// than it is, inside an exponential.
//
// The temptation is to divide by 4.8 and call the stock engine 1.0. That would
// be fitting the model to flatter itself, which is the one thing house rule
// two exists to forbid, and it would destroy the only thing this file is
// actually good for. So the constant stays as Douaud and Eyzat published it
// and the number is reported raw.
//
// Read it as an INDEX, not a verdict. It cannot tell you whether this engine
// knocks. It can tell you, reliably and with the right sensitivities, that
// this engine knocks MORE at 1500 rpm than at 4500 because the end gas has
// more milliseconds to sit there cooking; more on 87 octane than on 94; more
// at eleven to one than at nine and a half; and a great deal more with sixteen
// extra degrees of advance wound into it. Those are the questions compression
// ratio and spark advance are actually chosen by, and they are comparisons.
//
// ── One thing worth knowing about octane numbers ──────────────────────────
//
// There are two scales and the pump uses a third. RESEARCH octane (RON) is
// measured at 600 rpm and light load; MOTOR octane (MON) at 900 rpm with a
// heated intake, and is always the lower and harsher number. An American pump
// advertises the average of the two, (R+M)/2, called AKI, which is why the
// same fuel is 87 in Michigan and 91 in Milan.
//
// Douaud and Eyzat fitted against RON, so RON is what this file wants, and
// handing it an AKI figure will make the fuel look about four points worse
// than it is and the engine detonate on paper when it never did in a car.

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
