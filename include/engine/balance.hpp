// balance.hpp — why the cross-plane crank exists at all.
//
// This file exists to stop `crankshaft.hpp` from getting away with something.
//
// That file claims, in prose, that the cross-plane crankshaft was not adopted
// for its voice — that it was adopted because throws at 90° let an engine hold
// still, and that the sound was a side effect which has outlived every reason
// it was made. It is the most important claim in the project and until now it
// was the one thing the code asked you to take on trust. House rule two says
// derive, never declare, and the file that states the rule most loudly was the
// one breaking it.
//
// So: same rod table, same four throws, same eight cylinders. A different
// question asked of them.
//
// ── What shakes ───────────────────────────────────────────────────────────
//
// A piston does not travel sinusoidally, and that single fact is the origin of
// every balance problem a reciprocating engine has ever had. If it did, one
// counterweight would cancel it and the subject would not exist. Instead the
// crank-slider produces
//
//      acceleration ≈ ω²·r·( cos θ  +  λ·cos 2θ  +  … )
//                            ↑          ↑
//                        PRIMARY     SECONDARY
//
// two terms that behave completely differently and must be killed separately.
//
// The PRIMARY term goes once per revolution, in step with the crank. Being
// synchronous with the shaft, it can be cancelled by putting metal on the
// shaft — a counterweight opposite the throw, which is why crankshafts are
// lumpy. Easy, and every engine does it.
//
// The SECONDARY term goes TWICE per revolution, and that is the whole problem:
// nothing bolted to a shaft turning at ω can cancel a force at 2ω. It arises
// because the rod swings — the piston is dragged from top dead centre faster
// than it is returned to bottom, since the rod is at its most oblique halfway
// down. Its size is set entirely by λ, the rod ratio, which is the mechanical
// reason a long rod runs smoother and has nothing to do with dwell.
//
// You have exactly three options for a secondary: arrange the cylinders so
// they cancel each other, add a pair of balance shafts geared to turn at twice
// engine speed in opposite directions (Lanchester, 1904 — and what every large
// four-cylinder has hiding in its sump), or let it shake.
//
// ── The claim under test ──────────────────────────────────────────────────
//
// A flat-plane V8 is two flat-plane four-cylinder engines sharing a crank. An
// inline four has a notoriously unbalanced secondary — that is the buzz in
// every four-cylinder ever built, and the reason balance shafts were invented.
// A cross-plane V8 is not two of anything; its four throws point four
// different ways.
//
// Whether that makes the difference the prose claims is not asserted here. It
// is computed, over one revolution, from the exact linkage rather than the
// two-term approximation above, and whatever comes out is what this file
// prints — including if it is inconvenient.
//
// ── What is left out, deliberately ────────────────────────────────────────
//
// Rotating mass: the crank throws themselves and the big end of each rod
// travel in a circle, producing a force of constant magnitude that rotates
// with the shaft. It is completely cancelled by counterweights on every
// production engine as a matter of course, and including it would only measure
// how well the counterweights were drawn, which is not a property of the
// firing arrangement. The interesting question — the one that distinguishes
// one crankshaft from another — is entirely about the reciprocating mass.

#pragma once

#include <array>
#include <cmath>
#include <complex>
#include <engine/crankshaft.hpp>
#include <engine/geometry.hpp>
#include <engine/si.hpp>

namespace engine {

// A force in the plane perpendicular to the crankshaft — the plane the engine
// mounts have to hold still. x across the vee, y up through it.
struct Shake {
    double across = 0.0;
    double up     = 0.0;

    double magnitude() const { return std::hypot(across, up); }
};

// What one harmonic of the shaking does over a revolution.
struct Harmonic {
    double peak_force;   // N, the largest the mounts ever see from this order
    double peak_couple;  // N·m, the largest rocking moment about the block centre

    // A harmonic whose force traces a circle is a rotating imbalance, and one
    // that traces a line is an oscillation along a single direction. The
    // difference matters to a person choosing engine mounts, and it is the
    // difference between an engine that wobbles and one that buzzes.
    double eccentricity; // 0 = a pure circle, 1 = a straight line
};

class Balance {
public:
    // reciprocating_mass — piston, pin, rings, and the small end's share of rod
    // bore_spacing       — centre to centre between adjacent bores in a bank,
    //                      4.380 in on a small block Ford. This is what turns
    //                      a set of forces into a set of couples, and it is why
    //                      a long engine rocks more than a short one.
    Balance(const Crankshaft& crank,
            const Geometry&   geometry,
            double            reciprocating_mass,
            double            bore_spacing)
        : crank_{crank}, geometry_{geometry}
        , mass_{reciprocating_mass}, spacing_{bore_spacing}
    {}

    // The instantaneous shaking force, summed over all eight cylinders, at a
    // given crank angle and speed.
    //
    // Each piston, being accelerated, pushes back on the block along its own
    // bore axis with m·a. The bores point in two directions half a vee apart,
    // so the eight contributions are added as VECTORS and not as numbers —
    // which is the entire mechanism by which cylinders cancel one another, and
    // the reason the vee angle is as much a balance decision as a packaging one.
    Shake force(double theta, double omega) const {
        Shake total;
        for (int c = 1; c <= cylinder_count; ++c) {
            const Shake axis = bore_axis(c);
            const double a = geometry_.piston_acceleration(local_angle(c, theta), omega);
            total.across += mass_ * a * axis.across;
            total.up     += mass_ * a * axis.up;
        }
        return total;
    }

    // The rocking moment about the centre of the block. A force on cylinder
    // number one, four bore spacings from cylinder number four, twists the
    // engine about its middle even when the forces themselves sum to nothing.
    // An engine can be perfectly balanced in force and still tear its mounts
    // off, and inline sixes and V12s are prized precisely because they are the
    // arrangements where both go to zero at once.
    Shake couple(double theta, double omega) const {
        Shake total;
        for (int c = 1; c <= cylinder_count; ++c) {
            const Shake  axis = bore_axis(c);
            const double arm  = axial_position(c);
            const double a = geometry_.piston_acceleration(local_angle(c, theta), omega);
            // Moment about the block centre: the component of r × F along the
            // crank axis is what rocks the engine end to end.
            total.across += mass_ * a * axis.up     * arm;
            total.up     -= mass_ * a * axis.across * arm;
        }
        return total;
    }

    // Decompose the shaking into its orders. The exact linkage is sampled over
    // one revolution — piston motion repeats every 360°, not every 720°, since
    // a piston neither knows nor cares which stroke it is on — and the n-th
    // Fourier component is extracted and reported by the largest force it
    // applies over that revolution.
    //
    // Doing it this way rather than reading the coefficients off the cos θ +
    // λ cos 2θ series means the answer includes the higher terms the series
    // truncates, and means it is measured from the same linkage the engine
    // actually runs on rather than from an approximation of it.
    Harmonic order(int n, double omega) const {
        constexpr int samples = 2048;

        std::complex<double> fx{}, fy{}, cx{}, cy{};
        for (int k = 0; k < samples; ++k) {
            const double theta = si::two_pi * k / samples;
            const std::complex<double> turn =
                std::exp(std::complex<double>(0.0, -double(n) * theta));

            const Shake f = force(theta, omega);
            const Shake m = couple(theta, omega);
            fx += f.across * turn;  fy += f.up * turn;
            cx += m.across * turn;  cy += m.up * turn;
        }
        const double scale = 2.0 / samples;
        fx *= scale; fy *= scale; cx *= scale; cy *= scale;

        return { peak_of(fx, fy), peak_of(cx, cy), eccentricity_of(fx, fy) };
    }

    // The orders anyone cares about. Third and above exist and are small
    // enough that no one has ever designed a crankshaft around them.
    Harmonic primary  (double omega) const { return order(1, omega); }
    Harmonic secondary(double omega) const { return order(2, omega); }

private:
    // The unit vector pointing up the bore, away from the crank. Left bank
    // leans one way, right bank the other, by half the included angle — the
    // same sign convention `crankshaft.hpp` uses to place the firing angles,
    // and it has to be the same one or the pistons will be in the wrong holes.
    Shake bore_axis(int cylinder) const {
        const double half_vee = crank_.vee_angle() * 0.5;
        const double lean = (crank_.bank_of(cylinder) == Bank::left) ? +half_vee : -half_vee;
        return { -std::sin(lean), std::cos(lean) };
    }

    // Where along the shaft this cylinder sits, measured from the middle of
    // the block. Both cylinders on a journal are treated as sharing its
    // station; they are in truth a rod's width apart, which produces a small
    // couple about the vertical axis that no one has ever lost sleep over.
    double axial_position(int cylinder) const {
        const double journal = crank_.journal_of(cylinder);
        return (journal - (journal_count - 1) * 0.5) * spacing_;
    }

    // This cylinder's own crank angle. Piston motion has a period of one
    // revolution, so the second-revolution offset in a firing TDC is
    // irrelevant here — and folding it away is the small piece of bookkeeping
    // that makes a four-stroke's balance a single-revolution question.
    double local_angle(int cylinder, double theta) const {
        return theta - crank_.firing_tdc(cylinder);
    }

    // The largest magnitude a harmonic reaches over one revolution. Its tip
    // traces an ellipse; this is the major semi-axis, found by walking it.
    static double peak_of(std::complex<double> cx, std::complex<double> cy) {
        double peak = 0.0;
        constexpr int steps = 720;
        for (int k = 0; k < steps; ++k) {
            const double phase = si::two_pi * k / steps;
            const std::complex<double> turn = std::exp(std::complex<double>(0.0, phase));
            peak = std::max(peak, std::hypot((cx * turn).real(), (cy * turn).real()));
        }
        return peak;
    }

    // How far from circular that ellipse is. A rotating imbalance traces a
    // circle and can in principle be cancelled by a counterweight; a straight
    // line cannot be, and has to be cancelled by another cylinder.
    static double eccentricity_of(std::complex<double> cx, std::complex<double> cy) {
        double hi = 0.0, lo = 1e300;
        constexpr int steps = 720;
        for (int k = 0; k < steps; ++k) {
            const double phase = si::two_pi * k / steps;
            const std::complex<double> turn = std::exp(std::complex<double>(0.0, phase));
            const double r = std::hypot((cx * turn).real(), (cy * turn).real());
            hi = std::max(hi, r);
            lo = std::min(lo, r);
        }
        if (hi < 1e-9) return 0.0;
        return std::sqrt(std::max(0.0, 1.0 - (lo * lo) / (hi * hi)));
    }

    const Crankshaft& crank_;
    const Geometry&   geometry_;
    double mass_, spacing_;
};

} // namespace engine
