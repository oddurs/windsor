// geometry.hpp — the crank-slider.
//
// Everything a piston engine does, it does because of this linkage. A crank
// throw of radius a sweeps a circle; a rod of length l connects it to a piston
// constrained to a line. That is the entire machine. Combustion, valve timing,
// firing order and exhaust note are all decoration on top of one equation:
//
//      x(θ) = a·cos θ + √(l² − a² sin²θ)
//
// x is the distance from the crank centreline to the wrist pin. It is largest
// at top dead centre (a + l) and smallest at bottom dead centre (l − a), and
// the difference between those two is the stroke, 2a, which is why a stroke is
// always exactly twice the crank throw and why you cannot stroke an engine
// without moving the rod journal.
//
// The linkage is asymmetric in a way that matters. The piston is not a sine
// wave. It spends longer near BDC than near TDC, it accelerates harder on the
// way down from TDC than it does coming up from BDC, and the whole of that
// asymmetry is governed by one dimensionless number:
//
//      λ = a / l          (the rod ratio, inverted; "rod-stroke ratio" is l/a)
//
// A long rod (small λ) dwells the piston at TDC, which suits high rpm and a
// slow burn. A short rod (large λ) yanks it away, which makes torque and eats
// cylinder walls. Nothing in this file decides which is better. It only
// reports, exactly, what the linkage does.
//
// Not modelled: wrist-pin offset (desaxé). Real pistons carry 0.5–1.5 mm of
// offset toward the thrust side to soften piston slap at TDC. It shifts true
// TDC a fraction of a degree off the crank's TDC and makes the up-stroke and
// down-stroke slightly unequal. It is a real effect and it is left out here
// deliberately, because it buys a rattle you cannot hear and costs the closed
// form above. If you add it, everything downstream still works.

#pragma once

#include <cmath>
#include <engine/si.hpp>

namespace engine {

// Named parameters. A cylinder is quoted as bore × stroke and you should not
// be able to swap them by accident.
struct Bore             { double m;     };  // cylinder diameter
struct Stroke           { double m;     };  // full travel, TDC to BDC
struct RodLength        { double m;     };  // centre to centre
struct CompressionRatio { double ratio; };  // swept+clearance over clearance

// The fixed geometry of one cylinder. Nothing here moves; it is the block,
// the crank throw and the rod, at rest on the bench.
class Geometry {
public:
    constexpr Geometry(Bore b, Stroke s, RodLength r, CompressionRatio cr)
        : bore_{b.m}
        , stroke_{s.m}
        , rod_{r.m}
        , crank_radius_{s.m * 0.5}
        , piston_area_{si::pi * 0.25 * b.m * b.m}
        , swept_volume_{si::pi * 0.25 * b.m * b.m * s.m}
        , clearance_volume_{swept_volume_ / (cr.ratio - 1.0)}
        , compression_ratio_{cr.ratio}
    {}

    // ── Dimensions as specified ───────────────────────────────────────────
    constexpr double bore()              const { return bore_;              }
    constexpr double stroke()            const { return stroke_;            }
    constexpr double rod_length()        const { return rod_;               }
    constexpr double crank_radius()      const { return crank_radius_;      }  // a = stroke/2
    constexpr double piston_area()       const { return piston_area_;       }
    constexpr double swept_volume()      const { return swept_volume_;      }  // one cylinder
    constexpr double clearance_volume()  const { return clearance_volume_;  }  // at TDC
    constexpr double compression_ratio() const { return compression_ratio_; }

    // λ = a/l. Small is a long rod. Typical production: 0.25 to 0.33.
    constexpr double rod_ratio() const { return crank_radius_ / rod_; }

    // Bore/stroke character. Over 1.0 is oversquare — short stroke, big valves,
    // revs. Under 1.0 is undersquare — long stroke, torque, a low ceiling.
    constexpr double bore_stroke_ratio() const { return bore_ / stroke_; }

    // ── The linkage, as a function of crank angle ─────────────────────────
    // θ is measured from top dead centre of THIS cylinder, positive in the
    // direction of rotation. Callers hold the whole-engine crank angle and
    // subtract each cylinder's own TDC before arriving here; see crankshaft.hpp.

    // Distance from crank centreline to wrist pin. Maximum a+l at TDC.
    double pin_distance(double theta) const {
        const double a = crank_radius_, l = rod_;
        const double s = a * std::sin(theta);
        return a * std::cos(theta) + std::sqrt(l * l - s * s);
    }

    // How far the piston has descended from TDC. 0 at TDC, `stroke` at BDC.
    double piston_displacement(double theta) const {
        return (crank_radius_ + rod_) - pin_distance(theta);
    }

    // Chamber volume above the piston, including the clearance volume.
    // This is the V in every thermodynamic expression in this project.
    double volume(double theta) const {
        return clearance_volume_ + piston_area_ * piston_displacement(theta);
    }

    // dV/dθ — the rate the chamber opens, per radian of crank. Positive on the
    // way down. This is what turns pressure into torque: the work a gas does on
    // the crank in a small rotation is p·dV, so the instantaneous torque a
    // cylinder makes is p·(dV/dθ). The whole engine is that one product,
    // integrated, eight times over.
    double dV_dtheta(double theta) const {
        const double a = crank_radius_, l = rod_;
        const double sin_t = std::sin(theta), cos_t = std::cos(theta);
        const double root  = std::sqrt(l * l - a * a * sin_t * sin_t);
        return piston_area_ * a * sin_t * (1.0 + a * cos_t / root);
    }

    // Piston velocity, positive downward, at a given crank speed.
    double piston_velocity(double theta, double omega) const {
        return dV_dtheta(theta) / piston_area_ * omega;
    }

    // Piston acceleration at constant crank speed. This is the number that
    // decides what a connecting rod is made of. At 7000 rpm a 3-inch stroke
    // turns the piston around at roughly 3000 g, twice per revolution, forever.
    double piston_acceleration(double theta, double omega) const {
        const double a = crank_radius_, l = rod_;
        const double lam = a / l;
        // Second-order expansion is not used; this is the exact derivative.
        const double s2 = std::sin(theta) * std::sin(theta);
        const double root = std::sqrt(1.0 - lam * lam * s2);
        const double term = std::cos(theta)
                          + lam * (std::cos(2.0 * theta) + lam * lam * s2 * s2)
                            / (root * root * root);
        return omega * omega * a * term;
    }

    // Mean piston speed at a given crank speed: the single number that most
    // honestly predicts whether an engine will survive. Production engines live
    // below about 20 m/s. Formula One lives near 25. Nothing lives above 30.
    double mean_piston_speed(double omega) const {
        return 2.0 * stroke_ * (omega / si::two_pi);
    }

    // Wetted surface the hot gas can lose heat through: head, crown, and the
    // liner exposed so far. Used by the heat-transfer model, which is the
    // reason a small-bore engine runs cooler and a hemi runs hotter.
    double heat_transfer_area(double theta) const {
        const double head_and_crown = 2.0 * piston_area_;
        const double exposed_liner  = si::pi * bore_
                                    * (clearance_volume_ / piston_area_
                                       + piston_displacement(theta));
        return head_and_crown + exposed_liner;
    }

private:
    double bore_, stroke_, rod_, crank_radius_;
    double piston_area_, swept_volume_, clearance_volume_, compression_ratio_;
};

} // namespace engine
