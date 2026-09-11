// induction.hpp — the throttle, and the vacuum behind it.
//
// A petrol engine is controlled by making it difficult to breathe. That is the
// whole mechanism. There is no other lever: the throttle does not add fuel, it
// does not change the timing, it does not alter the compression. It closes a
// plate across the only path air has into the engine, and everything else
// follows from the pressure that collapses behind it.
//
// This is a spectacularly wasteful way to control an engine and it has never
// been replaced. At light load the manifold sits at a third of an atmosphere
// and every intake stroke is the piston pulling a partial vacuum against
// atmospheric pressure on its underside — the pumping loop, a negative area on
// the indicator card that is subtracted from every cycle whether it fired or
// not. Cruising down a motorway, a large engine spends more energy breathing
// than a small one spends moving the car. Diesels do not do this; they throttle
// on fuel alone and their manifolds sit at atmosphere, which is most of why
// they are more efficient at part load and nearly none of why they are more
// efficient at full.
//
// ── What manifold vacuum is for ───────────────────────────────────────────
//
// The depression behind the plate is not merely a symptom. Before there were
// sensors it was the engine's only broadcast signal, and an extraordinary
// amount of machinery was built to listen to it: the vacuum advance on the
// distributor, which read light load and gave the slow lean mixture more time
// to burn; the power valve in the carburettor, which read the vacuum
// collapsing under acceleration and dumped in extra fuel; the brake booster;
// the heater controls; the transmission modulator. An engine with a vacuum
// leak misbehaves in a dozen unrelated ways at once because a dozen unrelated
// systems were all listening to the same pipe.
//
// ── The model ─────────────────────────────────────────────────────────────
//
// A butterfly, and a plenum behind it with a mass and a temperature of its
// own. Air enters through the plate from the atmosphere and leaves through
// eight intake ports on a schedule set by the camshaft, and the plenum
// pressure is whatever those two disagree by. That disagreement is manifold
// vacuum, and because the plenum has real volume it has real lag, which is
// why an engine takes a moment to answer the throttle.
//
// ── Not modelled: the runners ─────────────────────────────────────────────
//
// The plenum here is one well-stirred volume, and every cylinder drinks from
// it directly. In the engine there is a foot of cast iron between the two, and
// the column of air standing in it has mass and a resonance of its own. Slow
// the flow down near the end of the intake stroke and that column keeps coming
// anyway, packing charge in after the piston has stopped helping — which is
// why the inlet valve is left open 70° past bottom dead centre, and why a
// tunnel ram is tall.
//
// It was built twice and neither version shipped, which is worth writing down
// properly because the measurements are the useful part.
//
// AS A WAVEGUIDE — two delay lines per runner, an inverting reflection at the
// plenum where the area opens out, the same machinery the exhaust primaries
// use. It worked, and it tuned in the right direction: longer runners helped
// 2500 to 4500 rpm and hurt 5500, exactly the trade a manifold makes. It moved
// peak torque to 306 lb-ft at 2486 rpm against Ford's 295 at 2400, which is
// the closest this model has ever come. And it could not be held below 2000
// rpm at any damping: the loop rang and the torque swung between 137 and 465
// Nm in a limit cycle seventeen cycles long.
//
// AS AN INERTANCE — dropping the resonance and keeping only the mass, which
// for a runner this short is the larger effect anyway. Written as
// p = p_plenum − Λ·dṁ/dt it is an algebraic loop, since the ṁ being
// differentiated is the flow the cylinder computes FROM the pressure being
// solved for, and it produced NaN in under a second. Written properly — the
// runner flow as a state pushed by the pressure difference, a small port
// volume as a compliance, which together are a Helmholtz resonator and are
// what a runner actually is — it integrated stably at 1500 and 4000 rpm with
// no cycle-to-cycle variation at all. But it moved peak torque DOWN to 1484
// rpm, away from the 2400 it was meant to be chasing.
//
// So: one version that was accurate and unstable, one that was stable and less
// accurate than having no runners at all, and four times the runtime for
// either. Neither is worth what it costs yet. What is missing in both is that
// the eight runners should meet each other AT the plenum and share it —
// a proper junction, as the exhaust primaries have at their collector — rather
// than each seeing a private boundary condition. That is probably the whole
// difference, and it is the next thing to try.

#pragma once

#include <algorithm>
#include <cmath>
#include <engine/charge.hpp>
#include <engine/cylinder.hpp>
#include <engine/port.hpp>
#include <engine/si.hpp>

namespace engine {

class Induction {
public:
    struct Setup {
        double plenum_volume;     // m³ — the space under the carburettor
        double throttle_bore;     // m  — total, across all barrels
        double venturi_area;      // m² — total, and the real limit. See below.
        double idle_bypass_area;  // m² — the leak that keeps it running closed
        double manifold_heating;  // K  — a cast-iron intake with an exhaust
                                  //      crossover cast into its floor,
                                  //      deliberately warming the charge so
                                  //      the fuel stays evaporated instead of
                                  //      puddling in the runners on a cold
                                  //      morning. It costs power every mile
                                  //      afterwards and it is why the engine
                                  //      starts in February
        Fuel   fuel;
    };

    explicit Induction(Setup s) : s_{s} {
        mass_        = air::mass(si::p_atmosphere, charge_temperature(), s_.plenum_volume);
        temperature_ = charge_temperature();
    }

    // What the mixture is set to. The carburettor's business, and the plenum's
    // only because evaporating fuel chills what it evaporates into.
    void mixture(double equivalence_ratio) { phi_ = equivalence_ratio; }

    // Ambient, plus what the manifold adds, less what the fuel takes back as
    // it boils. Three numbers pulling in two directions, and the net of them
    // is the density of everything that goes into the engine.
    double charge_temperature() const {
        return si::T_standard + s_.manifold_heating - s_.fuel.charge_cooling(phi_);
    }

    // What the intake ports see. This is the `Boundary` a cylinder is handed.
    Boundary boundary() const {
        return { air::pressure(mass_, temperature_, s_.plenum_volume), temperature_ };
    }

    double pressure() const { return air::pressure(mass_, temperature_, s_.plenum_volume); }

    // Manifold vacuum, quoted the way a gauge on the dashboard quotes it:
    // how far BELOW atmosphere, in pascals. A healthy idle is 55–70 kPa of
    // vacuum; a flat one means a leak, a burnt valve, or a cam with far more
    // overlap than its owner admits.
    double vacuum() const { return si::p_atmosphere - pressure(); }

    // throttle — 0 (closed, idling on the bypass) to 1 (wide open)
    // port_flow — total mass flow leaving the plenum into all eight cylinders,
    //             positive outward, which during overlap is genuinely negative
    //             in some of them as exhaust reverses up the port.
    void step(double dt, double throttle, double port_flow) {
        const double p = pressure();

        const double in = port::mass_flow(open_area(throttle),
                                          si::p_atmosphere, si::T_standard,
                                          p, temperature_);

        // Energy in the plenum. Incoming air carries its enthalpy; outgoing
        // takes the plenum's. The manifold itself is treated as isothermal at
        // the charge temperature — a cast-iron intake bolted between two
        // cylinder heads has enormous thermal mass and does not care what one
        // intake stroke did to it.
        mass_ = std::max(mass_ + (in - port_flow) * dt, 1e-9);
        temperature_ += (temperature_ - charge_temperature()) * -std::min(1.0, dt * 50.0);
    }

private:
    // A butterfly plate does not open linearly. Near closed it is a slot
    // whose area goes as (1 − cos α), which is why the first ten degrees of
    // pedal travel do almost nothing and the next ten do everything, and why
    // progressive linkages and, eventually, drive-by-wire had to be invented.
    //
    // ── And the plate is not the restriction ──────────────────────────────
    //
    // Wide open, the throttle plate is edge-on to the airflow and barely there.
    // What is still there, at every throttle position, is the VENTURI: the
    // waist the barrel is narrowed to so that the air speeds up through it and
    // drops enough pressure to pull fuel out of a hole in the side. A
    // carburettor has to restrict the engine in order to work at all. That is
    // the deal it makes — it meters fuel by measuring airflow, and the only
    // way it can measure airflow is by getting in its way.
    //
    // The 1968 302-2V ran an Autolite 2100 with two 1.08-inch venturis, rated
    // at 287 cfm. Two of those come to 11.8 cm², against 24.6 cm² of throttle
    // bore — so wide open, better than half the restriction in the induction
    // system is a hole that cannot be opened, and the engine is breathing
    // through it at 6000 rpm exactly as hard as it is at 1000.
    //
    // This is most of why a 2V engine makes its power at 4400 rpm and the
    // otherwise identical 4V makes more of it at 4800, and it is the whole
    // reason the first thing anyone ever did to one of these was throw the
    // carburettor away.
    //
    // Two restrictions in series do not add; their reciprocal squares do,
    // because each one costs a pressure drop and the drops are what add.
    double open_area(double throttle) const {
        const double alpha = std::clamp(throttle, 0.0, 1.0) * (si::pi * 0.5);
        const double bore_area = si::pi * 0.25 * s_.throttle_bore * s_.throttle_bore;
        const double plate = s_.idle_bypass_area + bore_area * (1.0 - std::cos(alpha));

        if (s_.venturi_area <= 0.0) return plate;
        return 1.0 / std::sqrt(1.0 / (plate * plate)
                             + 1.0 / (s_.venturi_area * s_.venturi_area));
    }

    Setup  s_;
    double phi_ = 1.0;
    double mass_;
    double temperature_;
};

} // namespace engine
