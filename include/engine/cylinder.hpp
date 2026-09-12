// cylinder.hpp — one cylinder, as an open system.
//
// Everything to this point has been a part on a bench. This is where they are
// bolted together and the result is asked to account for itself, once per
// small rotation of the crank, forever.
//
// ── The bookkeeping ───────────────────────────────────────────────────────
//
// A cylinder is a box of gas whose walls move, which leaks at two places on a
// schedule, which is heated from within on purpose and cooled from without by
// accident. The first law, written for such a box per radian of crank:
//
//      m·cᵥ·dT/dθ  =  −p·dV/dθ              the piston, taking or giving work
//                     + dQ_burn/dθ           the fire
//                     − dQ_wall/dθ           the coolant, stealing
//                     + Σ (dmᵢ/dθ)·hᵢ        gas arriving, carrying enthalpy
//                     − u·dm/dθ              the same gas, accounted as mass
//
// That is the whole model. Five terms, and each one is a file already written:
// `geometry.hpp` supplies dV/dθ, `wiebe.hpp` the burn, `woschni.hpp` the loss,
// `port.hpp` the flows, `charge.hpp` the gas that carries them.
//
// ── Why open, and not the cycle in the textbook ───────────────────────────
//
// The air-standard Otto cycle is a closed system: a fixed mass, compressed,
// heated at constant volume, expanded, cooled. It is taught first because it
// gives η = 1 − r^(1−γ) in one line, and that line is genuinely the reason
// compression ratio matters. But it cannot tell you anything an engine builder
// wants to know, because everything an engine builder cares about happens
// during the half of the cycle the closed model deletes — the half where the
// valves are open and the mass is changing.
//
// Volumetric efficiency, overlap, scavenging, the entire effect of a camshaft,
// the reason an engine has a powerband at all: all of it is gas exchange, and
// all of it requires the mass in the box to be a variable. So it is one.
//
// ── Sign conventions, stated once ─────────────────────────────────────────
//
//   θ        crank angle from THIS cylinder's firing TDC, within [0, 720°)
//   V        increases as the piston descends
//   dV/dθ    positive on a downstroke
//   torque   positive when the cylinder is driving the crank
//   mass flow positive INTO the cylinder, at both valves
//
// That last one is worth dwelling on. It would be more natural to make exhaust
// flow positive outward, and it would be wrong, because during overlap the
// exhaust valve flows both directions within a few degrees and a convention
// that has to be negated halfway through an event is a convention that will
// eventually be negated in only one of the three places it appears.

#pragma once

#include <algorithm>
#include <engine/camshaft.hpp>
#include <engine/charge.hpp>
#include <engine/geometry.hpp>
#include <engine/knock.hpp>
#include <engine/port.hpp>
#include <engine/si.hpp>
#include <engine/wiebe.hpp>
#include <engine/woschni.hpp>

namespace engine {

// The conditions on the far side of each valve. The cylinder does not know
// where these come from — a plenum, a manifold, the atmosphere — and must not.
struct Boundary {
    double pressure;      // Pa
    double temperature;   // K
};

// What one step of one cylinder did — one point on an indicator card, which is
// the instrument James Watt invented in the 1790s to draw a p–V loop with a
// pencil on a rotating drum while the engine ran, and which remains the only
// honest picture of what a cylinder is doing. The engine collects these;
// nothing else may reach inside the cylinder to find out.
struct Indication {
    double torque;          // N·m on the crank, gas and inertia together
    double intake_flow;     // kg/s, positive into the cylinder
    double exhaust_flow;    // kg/s, positive into the cylinder (so: usually negative)
    double pressure;        // Pa in the chamber, after the step
    double temperature;     // K
    double volume;          // m³
    double heat_release;    // W from combustion
    double wall_loss;       // W into the coolant
    double exhaust_open;    // 0..1, how far off its seat the exhaust valve is
};

class Cylinder {
public:
    struct Setup {
        Geometry geometry;
        Camshaft camshaft;
        Wiebe    combustion;
        Fuel     fuel;
        double   firing_tdc;              // this cylinder's place in the cycle
        double   reciprocating_mass;      // piston + pin + rings + small end of rod
        double   wall_temperature;        // K, the mean the gas sees
        double   crankcase_pressure;      // Pa, pushing back on the piston underside
    };

    explicit Cylinder(Setup setup) : s_{setup}, burning_{setup.combustion} {
        // Start cold and full of atmosphere, which is what an engine that has
        // been sitting overnight actually contains.
        charge_.mass        = air::mass(si::p_atmosphere, si::T_standard,
                                        s_.geometry.volume(0.0));
        charge_.temperature = si::T_standard;
        charge_.burned      = 0.0;
        reference_ = { si::p_atmosphere, si::T_standard, s_.geometry.volume(0.0) };
    }

    // ── Observation ───────────────────────────────────────────────────────
    const Geometry& geometry() const { return s_.geometry; }
    double firing_tdc()        const { return s_.firing_tdc; }
    double pressure()          const { return charge_.pressure(volume_now_); }
    double temperature()       const { return charge_.temperature; }
    double volume()            const { return volume_now_; }
    double burned_fraction()   const { return charge_.burned; }

    // How much of the end gas's patience was spent on the last cycle. One is
    // detonation; anything over about 0.8 is an engine that will knock on a
    // hot day, on a hill, in traffic. See `knock.hpp`.
    double knock_severity()    const { return knock_.severity(); }
    double trapped_air()       const { return trapped_mass_; }


    // ── The step ──────────────────────────────────────────────────────────
    // engine_theta  — the whole engine's crank angle, [0, 720°)
    // dtheta        — the step, in radians of crank
    // omega         — crank speed, rad/s. The bridge between angle and time:
    //                 everything physical happens per second, everything
    //                 geometric happens per radian, and ω is the exchange rate.
    Indication step(double engine_theta, double dtheta, double omega,
                const Boundary& intake, const Boundary& exhaust,
                double spark_advance, double equivalence_ratio)
    {
        const double theta = fold(engine_theta - s_.firing_tdc);

        const double V     = s_.geometry.volume(theta);
        const double dVdth = s_.geometry.dV_dtheta(theta);
        volume_now_        = V;

        double p = charge_.pressure(V);
        const double T = charge_.temperature;

        // ── Valves ────────────────────────────────────────────────────────
        const double lift_in  = s_.camshaft.intake().lift(theta);
        const double lift_ex  = s_.camshaft.exhaust().lift(theta);
        const double area_in  = port::effective_area(lift_in,  s_.camshaft.intake().diameter());
        const double area_ex  = port::effective_area(lift_ex,  s_.camshaft.exhaust().diameter());
        const bool   open     = (area_in > 0.0) || (area_ex > 0.0);

        // Flows, signed into the cylinder. `mass_flow` is given the outside
        // first and the cylinder second, so its positive direction is inward.
        double mdot_in = port::mass_flow(area_in, intake.pressure,  intake.temperature,  p, T);
        double mdot_ex = port::mass_flow(area_ex, exhaust.pressure, exhaust.temperature, p, T);

        const double dt = dtheta / omega;

        // A cylinder cannot exhale more than it contains. Blowdown is violent
        // enough that an unclamped explicit step can ask it to, and the gas law
        // will then return a negative pressure and the engine will explode in a
        // way that is entirely numerical.
        clamp_outflow(mdot_in, mdot_ex, dt);

        // ── Fresh charge, and therefore fuel ──────────────────────────────
        // The carburettor meters against air that actually came down the port,
        // not against whatever is in the cylinder — most of which, at overlap,
        // is last cycle's exhaust.
        //
        // NET flow, signed. It is tempting to count only what comes in, and it
        // is wrong: at low speed the piston starts back up the bore long before
        // the inlet valve closes and shoves a measurable fraction of the charge
        // back out into the runner. That is reversion, it is the reason a big
        // cam idles badly and the reason volumetric efficiency falls off at the
        // bottom of the rev range as well as the top, and an accumulator that
        // ignores the negative flows will report an engine filling its
        // cylinders essentially perfectly at 1000 rpm, which no engine does.
        if (lift_in > 0.0) inducted_mass_ += mdot_in * dt;
        if (opened_intake(theta, dtheta)) { inducted_mass_ = 0.0; ignited_ = false; charge_.burned = 0.0; }

        // ── Inlet valve closing: the cylinder is sealed and countable ─────
        // The one instant per cycle at which the question "how much air is in
        // there?" has an answer. Before it the valve is open and the number is
        // still moving; after it the charge is trapped and the rest of the
        // cycle is arithmetic. So the figure is latched here, and `trapped_air`
        // reports the latched one — not the live accumulator, which if sampled
        // mid-induction will report anything at all.
        if (closed_intake(theta, dtheta)) {
            knock_.reset();               // a fresh charge, a fresh account
            trapped_mass_ = inducted_mass_;
            fuel_mass_ = s_.fuel.mass_for(trapped_mass_, equivalence_ratio);
            reference_ = { p, T, V };     // Woschni's anchor, and the motoring datum
        }

        // ── Spark ─────────────────────────────────────────────────────────
        // Advance is quoted before top dead centre because the fire takes time
        // and the piston will not wait. Light it early enough that peak
        // pressure lands about 15° after TDC, where the crank has leverage;
        // light it too early and the rising pressure fights the piston still
        // coming up, which is heard as knock and felt as a hole in the piston.
        const double spark_at = fold(-spark_advance);
        if (!ignited_ && fuel_mass_ > 0.0 && crossed(theta, spark_at, dtheta)) {
            ignited_     = true;
            since_spark_ = 0.0;
            // The pace is fixed at the moment of lighting, by what is in there.
            burning_     = s_.combustion.paced_for(equivalence_ratio);
        }

        double burn_power = 0.0;
        if (ignited_) {
            const double dx = burning_.burn_rate(since_spark_) * dtheta;
            charge_.burned += dx;
            burn_power      = fuel_mass_ * s_.fuel.lower_heating_value
                            * s_.fuel.combustion_efficiency(equivalence_ratio)
                            * dx / dt;
            since_spark_   += dtheta;
        }

        // ── The end gas, cooking ──────────────────────────────────────────
        // The mixture the flame has not reached yet is being compressed by the
        // flame, and is deciding whether to wait. See `knock.hpp`.
        knock_.elapse(dt, p,
                      Knock::end_gas_temperature(reference_.temperature,
                                                 reference_.pressure, p),
                      s_.fuel.research_octane, charge_.burned);

        // ── Heat into the coolant ─────────────────────────────────────────
        const double p_motored = Woschni::motored_pressure(reference_, V);
        const double h_conv    = Woschni::coefficient(
            s_.geometry.bore(), p, T,
            s_.geometry.mean_piston_speed(omega), s_.geometry.swept_volume(),
            p_motored, reference_, open);
        const double wall_power =
            h_conv * s_.geometry.heat_transfer_area(theta) * (T - s_.wall_temperature);

        // ── The first law ─────────────────────────────────────────────────
        const double dm_in = mdot_in * dt;
        const double dm_ex = mdot_ex * dt;
        const double dm    = dm_in + dm_ex;

        // Enthalpy rides in with whatever arrives; what leaves takes the
        // cylinder's own state with it.
        const double h_in = air::cp(mdot_in > 0 ? intake.temperature  : T) * (mdot_in > 0 ? intake.temperature  : T);
        const double h_ex = air::cp(mdot_ex > 0 ? exhaust.temperature : T) * (mdot_ex > 0 ? exhaust.temperature : T);
        const double u    = air::cv(T) * T;

        const double dU = -p * dVdth * dtheta
                        + (burn_power - wall_power) * dt
                        + dm_in * h_in + dm_ex * h_ex;

        const double new_mass = std::max(charge_.mass + dm, 1e-9);
        const double dT       = (dU - u * dm) / (charge_.mass * air::cv(T));

        charge_.mass        = new_mass;
        charge_.temperature = std::clamp(charge_.temperature + dT, 200.0, 4000.0);

        // ── Torque ────────────────────────────────────────────────────────
        // Gas on the crown, less crankcase pressure on its underside, times the
        // leverage the linkage currently offers. Plus the reciprocating mass,
        // which borrows torque on the way up and returns it on the way down and
        // nets to nothing over a cycle — but not within one, and the within is
        // what an idle sounds like.
        p = charge_.pressure(V);
        const double gas_torque = (p - s_.crankcase_pressure) * dVdth;
        const double inertia_torque =
            -s_.reciprocating_mass * s_.geometry.piston_acceleration(theta, omega)
             * dVdth / s_.geometry.piston_area();


        return Indication{
            gas_torque + inertia_torque,
            mdot_in, mdot_ex,
            p, charge_.temperature, V,
            burn_power, wall_power,
            lift_ex / s_.camshaft.exhaust().max_lift()
        };
    }

private:
    static double fold(double a) {
        const double cycle = 2.0 * si::two_pi;
        a = std::fmod(a, cycle);
        return a < 0 ? a + cycle : a;
    }

    // Did this step pass over a given angle? Events are instants and steps are
    // finite, so an event is "crossed", never "equalled".
    static bool crossed(double theta, double mark, double dtheta) {
        const double d = fold(theta - mark);
        return d < dtheta;
    }

    // An event is an instant; a step is finite. The window must therefore be
    // exactly one step wide — narrower and the event is missed on most cycles,
    // wider and it fires twice. Passing the step in is not an inconvenience,
    // it is the only correct width there is.
    bool opened_intake(double theta, double dtheta) const {
        return crossed(theta, fold(s_.camshaft.intake().opens()), dtheta);
    }
    bool closed_intake(double theta, double dtheta) const {
        return crossed(theta, fold(s_.camshaft.intake().closes()), dtheta);
    }

    void clamp_outflow(double& mdot_in, double& mdot_ex, double dt) const {
        const double leaving = -(std::min(mdot_in, 0.0) + std::min(mdot_ex, 0.0)) * dt;
        const double available = charge_.mass * 0.25;   // never more than a quarter per step
        if (leaving > available && leaving > 0.0) {
            const double scale = available / leaving;
            if (mdot_in < 0) mdot_in *= scale;
            if (mdot_ex < 0) mdot_ex *= scale;
        }
    }


    Setup  s_;
    Wiebe  burning_;          // this cycle's burn, paced by this cycle's mixture
    Charge charge_{};
    Woschni::Reference reference_{};

    double volume_now_       = 0.0;
    double inducted_mass_    = 0.0;
    double trapped_mass_     = 0.0;
    double fuel_mass_        = 0.0;
    double since_spark_      = 0.0;
    bool   ignited_          = false;
    Knock::Account knock_{};

};

} // namespace engine
