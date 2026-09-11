// ignition.hpp — deciding when, with no way of knowing.
//
// A flame takes time to cross a chamber — roughly two thousandths of a second,
// and remarkably little less at 6000 rpm than at 1000, because the turbulence
// that carries the flame front scales with piston speed almost exactly as fast
// as the time available shrinks. So the burn occupies a roughly constant
// NUMBER OF DEGREES, and the spark must therefore be thrown a roughly constant
// number of degrees early — except that "roughly" hides everything interesting.
//
// You want peak pressure at about 15° after top dead centre. Earlier and you
// are pushing on a piston that is still coming up, which wastes the pressure
// and, if you are enthusiastic about it, detonates the end gas and takes the
// top off a piston in about ninety seconds. Later and the gas expands into a
// cylinder that is already running away downward, and the pressure that would
// have been torque goes out of the exhaust valve as heat.
//
// ── The mechanism, which knew nothing ─────────────────────────────────────
//
// A distributor had no sensors. It had no idea what the engine was doing. It
// solved the problem with two mechanical guesses, and they were good enough to
// run essentially every car on earth for seventy years.
//
//   CENTRIFUGAL ADVANCE. A pair of flyweights under the points plate, held in
//   by springs. As the shaft spins faster they swing out and rotate the cam
//   ahead of the shaft. That is the whole of it: an analogue computer whose
//   program is the spring rate, and whose output is more advance at more rpm.
//   The curve is shaped by using two springs of different stiffness, so one
//   governs the early rise and the other takes over and limits the total — and
//   recurving a distributor means literally fitting lighter springs.
//
//   VACUUM ADVANCE. A diaphragm connected to the manifold, pulling the points
//   plate around. At light load the mixture in the cylinder is thin and
//   heavily diluted with residual exhaust and burns slowly, so it needs more
//   time — and at light load there is manifold vacuum to detect exactly that
//   condition. So the engine advances when it is cruising and retards the
//   instant the throttle opens and the vacuum collapses. It is a load sensor
//   made of a rubber diaphragm and a spring, and it was so nearly right that
//   the first electronic engine managements had to work hard to beat it.
//
// Total advance at a motorway cruise might be 50°: the spark fires while the
// piston is still a seventh of a stroke from the top and a cylinder's worth of
// mixture burns almost entirely before the crank reaches TDC. At full throttle
// it drops to 34°, because a full cylinder burns fast and does not need the
// help — and because at full throttle, 50° would destroy the engine.

#pragma once

#include <algorithm>
#include <engine/si.hpp>

namespace engine {

class Distributor {
public:
    struct Curve {
        double initial;             // static advance, set by turning the body
        double centrifugal_max;     // additional, all-in by `all_in_at`
        double all_in_at;           // rad/s where the weights hit their stops
        double vacuum_max;          // additional, at full manifold vacuum
        double vacuum_full_at;      // Pa of vacuum for the full amount
    };

    // A 1968 302-2V, as it left Dearborn: 12° initial, 22° of weights all in by
    // 3000 rpm for 34° total at full throttle, and 15° of vacuum on top of that
    // for cruising.
    static constexpr Curve stock_302() {
        using namespace si::literals;
        return { 12.0_deg, 22.0_deg, 3000.0_rpm, 15.0_deg, 55.0_kPa };
    }

    constexpr explicit Distributor(Curve c) : c_{c} {}

    // Crank radians before top dead centre to throw the spark.
    double advance(double omega, double manifold_vacuum) const {
        const double centrifugal = c_.centrifugal_max
            * std::clamp(omega / c_.all_in_at, 0.0, 1.0);

        const double vacuum = c_.vacuum_max
            * std::clamp(manifold_vacuum / c_.vacuum_full_at, 0.0, 1.0);

        return c_.initial + centrifugal + vacuum;
    }

private:
    Curve c_;
};

} // namespace engine
