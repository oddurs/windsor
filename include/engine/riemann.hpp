// riemann.hpp — what two bodies of gas do to each other.
//
// `exhaust.hpp` models a pipe as two delay lines, one per direction, which is
// d'Alembert's 1747 result sampled: any solution of the wave equation is a
// left-mover plus a right-mover, and neither one cares what the other is
// doing. It is exact, it is fast, and it is a linear theory.
//
// An exhaust pipe is not a linear problem. When the valve cracks with sixty
// bar behind it, the gas that leaves is not a small perturbation about a mean —
// it is a substantial fraction of the mean, moving at the speed of sound, and
// the assumption the delay lines are built on has quietly failed. That failure
// is why `exhaust.hpp` has to clamp its source at Mach 1: not as a tuning
// constant, but because a plane wave that carries supersonic flow is not a
// plane wave, and the linear model has no way to become anything else.
//
// ── What it should become ─────────────────────────────────────────────────
//
// A sound wave travels at the local speed of sound, RELATIVE TO THE GAS IT IS
// IN. Both halves of that sentence are the problem. The crest of a pressure
// wave is hotter than the trough, so sound moves faster there; and the crest
// is also being carried forward by the gas motion behind it, while the trough
// is not. The top of the wave therefore overtakes the bottom. It leans
// forward, and leans further, and the front face gets steeper the further it
// travels — until it is vertical, and a vertical pressure front is a shock.
//
// This is why the crack of an exhaust is sharper at the tailpipe than it was
// at the valve, which is the opposite of what every intuition about
// attenuation suggests, and it is the reason an engine sounds like an engine
// and not like a loudspeaker playing one.
//
// ── Riemann, 1860 ─────────────────────────────────────────────────────────
//
// Bernhard Riemann asked what happens when two uniform bodies of gas, each at
// its own pressure, density and velocity, are placed in contact and released.
// The answer is always the same three things, fanning out from where they met:
//
//      a shock or an expansion running one way,
//      a shock or an expansion running the other,
//      and between them a CONTACT — a surface where the temperature and
//      density jump but the pressure and velocity do not,
//
// which is simply the original boundary, drifting along with the flow. Solve
// that one problem and you can solve any flow at all, by chopping the pipe
// into cells, treating every face between two cells as its own little Riemann
// problem, and doing it again a microsecond later. That is a Godunov scheme,
// and it is the reason this file can do what the delay lines cannot: shocks
// are not a special case in it. They are what it makes on its own.
//
// The solver here is HLL — Harten, Lax and van Leer, 1983 — which does not
// resolve the contact and is the more robust for it. It is roughly thirty
// lines, and every one of them is conservation of something.

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include <engine/charge.hpp>
#include <engine/si.hpp>

namespace engine {

// The conserved quantities. Not pressure and velocity — those are what you
// want to LOOK at, and they are not what is conserved. Mass, momentum and
// energy are, and a scheme that advances anything else will quietly
// manufacture or destroy one of them at every shock.
struct Conserved {
    double mass      = 0.0;   // ρ
    double momentum  = 0.0;   // ρu
    double energy    = 0.0;   // ρE, total: internal plus kinetic
};

// The same gas, in the variables a person can read.
struct Primitive {
    double density = 1.0, velocity = 0.0, pressure = si::p_atmosphere;
};

class Gas {
public:
    static constexpr double gamma = 1.33;   // hot exhaust products, near enough

    static Primitive primitive(const Conserved& u) {
        Primitive q;
        q.density  = std::max(u.mass, 1e-6);
        q.velocity = u.momentum / q.density;
        q.pressure = std::max((gamma - 1.0)
                   * (u.energy - 0.5 * q.density * q.velocity * q.velocity), 1e2);
        return q;
    }

    static Conserved conserved(const Primitive& q) {
        return { q.density,
                 q.density * q.velocity,
                 q.pressure / (gamma - 1.0)
                   + 0.5 * q.density * q.velocity * q.velocity };
    }

    static double sound_speed(const Primitive& q) {
        return std::sqrt(gamma * q.pressure / q.density);
    }


    // Flux of mass, momentum and energy past a station in the gas.
    static Conserved flux(const Primitive& q) {
        const double e = q.pressure / (gamma - 1.0)
                       + 0.5 * q.density * q.velocity * q.velocity;
        return { q.density * q.velocity,
                 q.density * q.velocity * q.velocity + q.pressure,
                 (e + q.pressure) * q.velocity };
    }
};

// ── The Riemann problem at one face ───────────────────────────────────────
//
// Two states meet. Harten, Lax and van Leer observed that you do not need to
// know exactly what happens between them — you only need to bracket it. Find
// the fastest signal each way, assume one uniform state in between, and let
// conservation across the fan fix its value. It smears a contact surface that
// a more elaborate solver would hold sharp, and in exchange it cannot be made
// to produce a negative density, which is the failure mode that ends most
// attempts at this.
inline Conserved hll_flux(const Primitive& left, const Primitive& right) {
    const double aL = Gas::sound_speed(left);
    const double aR = Gas::sound_speed(right);

    // Davis's estimate of the two outermost signal speeds.
    const double sL = std::min(left.velocity - aL, right.velocity - aR);
    const double sR = std::max(left.velocity + aL, right.velocity + aR);

    if (sL >= 0.0) return Gas::flux(left);    // everything swept downstream
    if (sR <= 0.0) return Gas::flux(right);   // everything swept upstream

    const Conserved fL = Gas::flux(left),  fR = Gas::flux(right);
    const Conserved uL = Gas::conserved(left), uR = Gas::conserved(right);
    const double    d  = sR - sL;

    return {
        (sR * fL.mass     - sL * fR.mass     + sL * sR * (uR.mass     - uL.mass))     / d,
        (sR * fL.momentum - sL * fR.momentum + sL * sR * (uR.momentum - uL.momentum)) / d,
        (sR * fL.energy   - sL * fR.energy   + sL * sR * (uR.energy   - uL.energy))   / d,
    };
}


// ── A pipe full of gas ────────────────────────────────────────────────────
//
// Chopped into cells of equal length. Each holds one uniform state; every face
// between two of them is its own Riemann problem; and the whole of fluid
// dynamics in this project is: solve every face, move what the fluxes say to
// move, repeat. Godunov, 1959.
//
// The timestep is not free. A signal must not cross more than one cell in one
// step, or the scheme is solving a problem in which information arrived before
// it was sent. That is the Courant condition, and it is the reason a fine mesh
// is expensive twice over — more cells, and each of them taking smaller steps.
class Duct {
public:
    Duct(double length, double cells, Primitive fill)
        : dx_{length / cells}
        , cell_(static_cast<std::size_t>(cells) + 2, Gas::conserved(fill))
    {}

    Primitive at(std::size_t i) const { return Gas::primitive(cell_[i + 1]); }
    void      set(std::size_t i, const Primitive& q) { cell_[i + 1] = Gas::conserved(q); }

    // The ghost cells, which are how every boundary condition in this project
    // is expressed: a closed end is a mirror with the velocity reversed, an
    // open end is the atmosphere, and a valve is whatever the cylinder is.
    void left_ghost (const Primitive& q) { cell_.front() = Gas::conserved(q); }
    void right_ghost(const Primitive& q) { cell_.back()  = Gas::conserved(q); }

    Primitive left_face()  const { return Gas::primitive(cell_[1]); }
    Primitive right_face() const { return Gas::primitive(cell_[cell_.size() - 2]); }



    // The largest step the Courant condition allows: a signal must not cross
    // more than one cell in one step, or the scheme is solving a problem in
    // which information arrived before it was sent.
    double courant_limit(double number = 0.6) const {
        double fastest = 1.0;
        for (const Conserved& u : cell_) {
            const Primitive q = Gas::primitive(u);
            fastest = std::max(fastest, std::abs(q.velocity) + Gas::sound_speed(q));
        }
        return number * dx_ / fastest;
    }

    void step(double dt) {
        const std::size_t n = cell_.size();
        slope_.assign(n, Conserved{});
        face_l_.resize(n);
        face_r_.resize(n);

        // ── Second order, and why it is not optional ──────────────────────
        //
        // A first-order Godunov scheme assumes each cell holds one uniform
        // state, which means every face is a step even where the flow is
        // perfectly smooth, and the Riemann solver dutifully diffuses it. Over
        // sixty cells of primary pipe a pressure pulse arrives at the collector
        // visibly rounded off, and the reflection it sends home is too soft to
        // scavenge anything. Measured: header length was worth 1.2% of torque
        // with this scheme first-order, against 1.8% from the delay lines it
        // replaced — which is the embarrassing result that a linear model with
        // NO numerical dissipation beats a nonlinear one that is drowning in it.
        //
        // So each cell carries a slope as well as a value, and the states
        // handed to the Riemann solver are read off the ends of that slope
        // rather than from the middle of the cell.
        //
        // The limiter is what keeps that honest. A slope fitted through a
        // shock will overshoot, and an overshoot in density is a negative
        // density one step later. `minmod` takes the shallower of the two
        // one-sided slopes and zero if they disagree in sign — so the
        // reconstruction is second order where the flow is smooth and falls
        // back to first order exactly at the discontinuities, which is where
        // first order was the right answer all along.
        for (std::size_t i = 1; i + 1 < n; ++i) {
            slope_[i].mass     = minmod(cell_[i].mass     - cell_[i-1].mass,
                                        cell_[i+1].mass   - cell_[i].mass);
            slope_[i].momentum = minmod(cell_[i].momentum - cell_[i-1].momentum,
                                        cell_[i+1].momentum - cell_[i].momentum);
            slope_[i].energy   = minmod(cell_[i].energy   - cell_[i-1].energy,
                                        cell_[i+1].energy - cell_[i].energy);
        }

        // Hancock's half step: advance each cell's two extrapolated faces by
        // dt/2 using the flux difference across the cell itself. It costs one
        // extra flux evaluation per cell and buys second order in time as well
        // as space, without the second full pass an ordinary Runge-Kutta needs.
        const double half = 0.5 * dt / dx_;
        for (std::size_t i = 0; i < n; ++i) {
            Conserved left  { cell_[i].mass     - 0.5 * slope_[i].mass,
                              cell_[i].momentum - 0.5 * slope_[i].momentum,
                              cell_[i].energy   - 0.5 * slope_[i].energy };
            Conserved right { cell_[i].mass     + 0.5 * slope_[i].mass,
                              cell_[i].momentum + 0.5 * slope_[i].momentum,
                              cell_[i].energy   + 0.5 * slope_[i].energy };

            const Conserved fl = Gas::flux(Gas::primitive(left));
            const Conserved fr = Gas::flux(Gas::primitive(right));

            face_l_[i] = { left.mass     + half * (fl.mass     - fr.mass),
                           left.momentum + half * (fl.momentum - fr.momentum),
                           left.energy   + half * (fl.energy   - fr.energy) };
            face_r_[i] = { right.mass     + half * (fl.mass     - fr.mass),
                           right.momentum + half * (fl.momentum - fr.momentum),
                           right.energy   + half * (fl.energy   - fr.energy) };
        }

        flux_.resize(n - 1);
        for (std::size_t f = 0; f + 1 < n; ++f)
            flux_[f] = hll_flux(Gas::primitive(face_r_[f]), Gas::primitive(face_l_[f + 1]));

        const double k = dt / dx_;
        for (std::size_t i = 1; i + 1 < n; ++i) {
            cell_[i].mass     -= k * (flux_[i].mass     - flux_[i - 1].mass);
            cell_[i].momentum -= k * (flux_[i].momentum - flux_[i - 1].momentum);
            cell_[i].energy   -= k * (flux_[i].energy   - flux_[i - 1].energy);
            cell_[i].mass      = std::max(cell_[i].mass,   1e-5);
            cell_[i].energy    = std::max(cell_[i].energy, 1e2);
        }
    }

    // Totals, for anyone checking that nothing was invented or lost.
    Conserved contents() const {
        Conserved total;
        for (std::size_t i = 1; i + 1 < cell_.size(); ++i) {
            total.mass     += cell_[i].mass     * dx_;
            total.momentum += cell_[i].momentum * dx_;
            total.energy   += cell_[i].energy   * dx_;
        }
        return total;
    }

private:
    // The shallower of two slopes, or nothing at all if they disagree about
    // which way the gas is going. Van Leer's idea, and the reason this scheme
    // can be second order without inventing gas at a shock.
    static double minmod(double a, double b) {
        if (a * b <= 0.0) return 0.0;
        return std::abs(a) < std::abs(b) ? a : b;
    }

    double dx_;
    std::vector<Conserved> cell_;
    std::vector<Conserved> flux_, slope_, face_l_, face_r_;
};

} // namespace engine
