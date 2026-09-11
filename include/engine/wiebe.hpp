// wiebe.hpp — how fast the fire spreads.
//
// Ivan Wiebe was a Soviet engine researcher who, in the 1950s, noticed that
// combustion in a cylinder is a chain reaction, and that chain reactions have
// a shape. He wrote down that shape:
//
//      x(θ) = 1 − exp( −a · ((θ − θ₀)/Δθ)^(m+1) )
//
// x is the fraction of the fuel that has burned. It is an S: slow to catch
// after the spark, violent through the middle, and a long reluctant tail as
// the flame runs out of room in the crevices and the boundary layer.
//
// This equation is not derived from anything. There is no chemistry in it, no
// flame speed, no turbulence, no geometry of the chamber. It is a curve that
// was fitted to pressure traces because it had the right shape, and it has
// outlived every mechanistic model that was supposed to replace it, because
// the shape is right and three constants are enough.
//
//      a   — completeness. How far down the tail you cut. a = 5 burns 99.3%
//            of the charge; a = 6.908 burns 99.9%. The rest goes out of the
//            pipe as unburned hydrocarbons, and is what a catalyst is for.
//      m   — the form factor. Low m fires early and fades; high m hangs back
//            and then goes off. Petrol engines sit near 2; diesels, with their
//            sharp premixed spike, are modelled with two Wiebes added together.
//      Δθ  — the burn duration in crank angle, typically 40–60°. It barely
//            changes with engine speed, which is the single most important
//            fact about spark ignition: the flame takes roughly the same
//            NUMBER OF DEGREES at 6000 rpm as at 1500, because the turbulence
//            that carries it scales with piston speed. That near-invariance is
//            why one mechanical advance curve could serve a whole rev range,
//            and why distributors worked at all.

#pragma once

#include <cmath>
#include <engine/si.hpp>

namespace engine {

class Wiebe {
public:
    // duration    — crank angle from spark to the end of the burn
    // efficiency  — fraction of fuel consumed by the end (0.993 is the classic)
    // form        — the exponent m
    constexpr Wiebe(double duration, double efficiency = 0.993, double form = 2.0)
        : duration_{duration}
        , completeness_{-std::log(1.0 - efficiency)}
        , form_{form}
    {}

    constexpr double duration() const { return duration_; }

    // Mass fraction burned, `since_ignition` crank radians after the spark.
    // Zero before the spark and clamped at the top: a charge cannot burn twice.
    double burned_fraction(double since_ignition) const {
        if (since_ignition <= 0.0)        return 0.0;
        if (since_ignition >= duration_)  return 1.0 - std::exp(-completeness_);
        const double tau = since_ignition / duration_;
        return 1.0 - std::exp(-completeness_ * std::pow(tau, form_ + 1.0));
    }

    // dx/dθ — the rate of burning, which is the rate of heat release, which is
    // the shape of the pressure rise, which is what you hear as knock when it
    // happens too fast and as a misfire when it does not happen at all.
    double burn_rate(double since_ignition) const {
        if (since_ignition <= 0.0 || since_ignition >= duration_) return 0.0;
        const double tau = since_ignition / duration_;
        const double p   = std::pow(tau, form_);
        return completeness_ * (form_ + 1.0) / duration_ * p
             * std::exp(-completeness_ * p * tau);
    }

private:
    double duration_, completeness_, form_;
};

} // namespace engine
