// engine.hpp — the assembly.
//
// Nothing new is decided here. Every physical claim this project makes has
// already been made in a file of its own; this one bolts them together in the
// arrangement Ford chose and turns the result over.
//
// What it adds is the loop, and the loop is the last honest idea in the
// project: a running engine is a fixed point. Eight cylinders each make torque
// as a function of how fast the crank is turning; the crank turns at a speed
// determined by the torque it is given; and the whole machine is those two
// statements chasing each other several thousand times a second and never
// quite catching up. An idle is that chase reaching a stable orbit. A stall is
// it failing to.
//
// ── The integration variable ──────────────────────────────────────────────
//
// Crank angle, not time. Every geometric fact in this engine — valve lift,
// chamber volume, spark, the firing order — is a function of θ and has no
// opinion about seconds. Every physical fact — heat transfer, mass flow, the
// acceleration of the flywheel — is a function of t and has no opinion about
// degrees. The crank speed ω is the exchange rate between them, and it appears
// in exactly one line of this file:
//
//      dt = dθ / ω
//
// Stepping in angle rather than time means the resolution automatically
// follows the engine: at 6000 rpm the solver takes steps ten times shorter in
// seconds than at 600, which is precisely what the physics needs, for free.
//
// ── What it does not model ────────────────────────────────────────────────
//
// Blow-by past the rings. Oil temperature and its effect on friction, which is
// most of why a cold engine is slow. Knock, which would need an end-gas
// chemistry model and is the constraint that actually decides compression
// ratio. The torsional wind-up of the crankshaft itself, which is real enough
// that a V8's front damper is not decoration. Any of these could go in and
// none of them would change the shape of the project.

#pragma once

#include <algorithm>
#include <array>
#include <engine/balance.hpp>
#include <engine/crankshaft.hpp>
#include <engine/cylinder.hpp>
#include <engine/exhaust.hpp>
#include <engine/friction.hpp>
#include <engine/ignition.hpp>
#include <engine/induction.hpp>
#include <engine/si.hpp>

namespace engine {

class Engine {
public:
    struct Specification {
        Geometry            geometry;
        Camshaft            camshaft;
        Crankshaft          crankshaft;
        Wiebe               combustion;
        Fuel                fuel;
        Distributor         distributor;
        Friction            friction;
        Induction::Setup    induction;

        // Two systems, not one. A dual exhaust is never symmetric: the left
        // side has to get around the starter and the bellhousing and the right
        // side does not, and the two pipes reach the back of the car having
        // travelled measurably different distances. Nobody builds them equal
        // because nobody can, and it turns out to matter — see the note in
        // `record.cpp` about what happens when you add two identical banks
        // together.
        Exhaust::Setup      exhaust_left;
        Exhaust::Setup      exhaust_right;
        double              reciprocating_mass;
        double              bore_spacing;       // centre to centre in a bank
        double              wall_temperature;
        const char*         name;
    };

    explicit Engine(Specification spec)
        : spec_{spec}
        , induction_{spec.induction}
        , banks_{Exhaust{spec.exhaust_left}, Exhaust{spec.exhaust_right}}
        , omega_{600.0 * si::two_pi / 60.0}
    {
        for (int c = 1; c <= cylinder_count; ++c) {
            cylinders_.emplace_back(Cylinder::Setup{
                spec_.geometry, spec_.camshaft, spec_.combustion, spec_.fuel,
                spec_.crankshaft.firing_tdc(c),
                spec_.reciprocating_mass, spec_.wall_temperature,
                si::p_atmosphere
            });

            // Each cylinder is plumbed to a seat on its own bank's header. The
            // seat index is arbitrary because all four primaries are cut to
            // the same length — that being the entire point of a set of
            // equal-length headers, and the reason they cost what they do.
            const Bank bank = spec_.crankshaft.bank_of(c);
            const int  side = (bank == Bank::left) ? 0 : 1;
            seat_[c - 1] = { side, seats_used_[side]++ };
        }
    }

    // ── Controls ──────────────────────────────────────────────────────────
    void throttle(double fraction) { throttle_ = std::clamp(fraction, 0.0, 1.0); }
    void mixture(double phi)       { phi_ = phi; induction_.mixture(phi); }

    // What the dynamometer's water brake is absorbing. A dyno does not measure
    // an engine's torque by asking it; it loads the engine until the speed
    // holds still, and then weighs the arm. Set this to zero and the engine
    // free-revs, which is the other thing a dyno is for.
    void brake_torque(double nm) { brake_ = nm; }

    // Everything bolted downstream of the flywheel, reflected back through
    // whatever gear it is in. In neutral this is nothing and a V8 will snap to
    // the rev limiter in a quarter of a second, which is exactly what a blip
    // sounds like. In third gear it is the entire car — a ton and a half of
    // steel, seen through the square of the gear ratio — and the same engine
    // takes three seconds to do the same thing. The engine cannot tell the
    // difference and does not need to; it is one number added to the crank's
    // own polar moment.
    void load_inertia(double kg_m2) { load_inertia_ = std::max(kg_m2, 0.0); }

    // ── Observation ───────────────────────────────────────────────────────
    double crank_angle() const { return theta_; }
    double crank_speed() const { return omega_; }
    double rpm()         const { return omega_ * 60.0 / si::two_pi; }

    // Mean torque at the flywheel over the last complete cycle: indicated work
    // from all eight cylinders, less the friction tax, divided by the 4π of
    // crank rotation a four-stroke needs to collect it.
    double torque() const { return last_cycle_torque_; }

    // How many complete cycles have been integrated. A torque figure only
    // changes when one finishes, so this is how an instrument knows there is
    // a fresh reading on the dial rather than the same one sampled twice.
    unsigned long long cycles() const { return cycles_; }
    double power()  const { return torque() * omega_; }

    double manifold_vacuum() const { return induction_.vacuum(); }
    double spark_advance()   const { return advance_; }
    double peak_pressure()   const { return peak_pressure_; }

    // The worst-off cylinder. Knock is never uniform across eight of them —
    // the ones at the ends of the block run hotter, the ones fed by the long
    // runners run leaner, and an engine is only ever as advanced as its most
    // detonation-prone hole will tolerate.
    double knock_severity() const {
        double worst = 0.0;
        for (const Cylinder& c : cylinders_) worst = std::max(worst, c.knock_severity());
        return worst;
    }

    const Cylinder&   cylinder(int n) const { return cylinders_[n - 1]; }
    const Crankshaft& crankshaft()    const { return spec_.crankshaft; }
    const Geometry&   geometry()      const { return spec_.geometry; }
    const char*       name()          const { return spec_.name; }

    double displacement() const { return spec_.geometry.swept_volume() * cylinder_count; }

    // Ask the same eight rods a different question. See `balance.hpp`.
    Balance balance() const {
        return Balance{spec_.crankshaft, spec_.geometry,
                       spec_.reciprocating_mass, spec_.bore_spacing};
    }

    // The two banks, for anyone holding a microphone.
    Exhaust& bank(Bank b) { return banks_[b == Bank::left ? 0 : 1]; }

    // ── The loop ──────────────────────────────────────────────────────────
    void step(double dtheta) {
        const double dt = dtheta / omega_;

        advance_ = spec_.distributor.advance(omega_, induction_.vacuum());

        const Boundary intake = induction_.boundary();

        double gas_torque = 0.0;
        double port_flow  = 0.0;
        double peak       = 0.0;

        for (int c = 1; c <= cylinder_count; ++c) {
            const auto [side, seat] = seat_[c - 1];

            // Not the collector — this cylinder's own pipe, with whatever
            // wave is standing in it at this instant. See `exhaust.hpp`.
            const Indication ind = cylinders_[c - 1].step(
                theta_, dtheta, omega_, intake, banks_[side].port_boundary(seat),
                advance_, phi_);

            gas_torque += ind.torque;
            port_flow  += ind.intake_flow;        // positive INTO the cylinder,
                                                  // so this is what the plenum lost
            banks_[side].receive(seat, -ind.exhaust_flow);   // and outward here
            peak = std::max(peak, ind.pressure);
        }

        induction_.step(dt, throttle_, port_flow);
        banks_[0].advance(dt);
        banks_[1].advance(dt);

        // Friction is quoted as a mean effective pressure and is therefore a
        // cycle-average quantity being applied instantaneously. That is a small
        // lie: real friction varies enormously within a cycle, peaking when
        // cylinder pressure does. It costs nothing at the flywheel because the
        // flywheel averages it anyway, and it keeps the number honest over the
        // one interval anyone measures it over.
        const double friction_torque = spec_.friction.torque(
            peak_pressure_, spec_.geometry.mean_piston_speed(omega_), displacement());

        // ── The chase ─────────────────────────────────────────────────────
        const double net = gas_torque - friction_torque - brake_;
        const double rotating_inertia = spec_.crankshaft.inertia() + load_inertia_;
        omega_ = std::max(omega_ + net / rotating_inertia * dt, 1.0);

        accumulate(gas_torque, friction_torque, dtheta, peak);

        theta_ += dtheta;
        if (theta_ >= 2.0 * si::two_pi) theta_ -= 2.0 * si::two_pi;
    }

private:
    void accumulate(double gas_torque, double friction_torque, double dtheta, double peak) {
        cycle_work_    += (gas_torque - friction_torque) * dtheta;
        cycle_angle_   += dtheta;
        cycle_peak_     = std::max(cycle_peak_, peak);

        if (cycle_angle_ >= 2.0 * si::two_pi) {
            last_cycle_torque_ = cycle_work_ / cycle_angle_;
            peak_pressure_     = cycle_peak_;
            cycle_work_ = cycle_angle_ = cycle_peak_ = 0.0;
            ++cycles_;
        }
    }

    Specification spec_;
    std::vector<Cylinder>   cylinders_;
    Induction               induction_;
    std::array<Exhaust, 2>  banks_;
    std::array<std::pair<int,int>, cylinder_count> seat_{};
    std::array<int, 2>      seats_used_{0, 0};

    double theta_    = 0.0;
    double omega_    = 0.0;
    double throttle_ = 0.0;
    double phi_      = 1.0;
    double brake_    = 0.0;
    double load_inertia_ = 0.0;
    double advance_  = 0.0;

    double cycle_work_ = 0.0, cycle_angle_ = 0.0, cycle_peak_ = 0.0;
    double last_cycle_torque_ = 0.0;
    unsigned long long cycles_ = 0;
    double peak_pressure_     = 30.0e5;   // a plausible first guess for friction
};

} // namespace engine
