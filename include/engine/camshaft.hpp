// camshaft.hpp — the only part of the engine that decides anything.
//
// The crankshaft says when each cylinder fires. The camshaft says what an
// engine IS. Two 302s with identical blocks, cranks, pistons, heads and
// carburettors will make peak power 2500 rpm apart if you grind the cams
// differently, and the one that makes more at 6000 will be undriveable at
// idle. There is no cam that is good everywhere. Every camshaft ever ground is
// an argument about which rpm the owner actually cares about, settled in steel.
//
// ── What is on a cam card ─────────────────────────────────────────────────
//
// A cam is not sold by its shape. It is sold by four numbers:
//
//   LIFT       How far the valve leaves its seat, at the valve — which is the
//              lobe lift multiplied by the rocker ratio, and a figure quoted
//              at the lobe by anyone trying to make a cam sound smaller than
//              it is. More lift is more area for longer. It is also more
//              spring pressure, more valvetrain load, and eventually a valve
//              introduced to a piston.
//
//   DURATION   How long the valve is off its seat, in crank degrees. Note
//              CRANK degrees: the cam turns at half engine speed, so a 266°
//              cam lobe occupies 133° of camshaft. Duration is the single
//              number that moves an engine's powerband, and it moves it up.
//
//   CENTRELINE The crank angle of peak lift. Intake centreline is quoted after
//              TDC, exhaust before it. Advancing the cam (smaller intake
//              centreline) builds low-end torque and bleeds off the top;
//              retarding it does the reverse. This is the adjustment that can
//              be made with the engine in the car, and the reason adjustable
//              timing gears exist.
//
//   LSA        Lobe separation angle: the angle between the two centrelines,
//              ground into the cam and unchangeable afterward. Tight LSA
//              (106°) gives more overlap — a sharper torque peak, a rougher
//              idle, the lope. Wide LSA (114°) gives a broad soft curve and an
//              idle you could balance a coin on. A stock engine wants 112–114.
//              A drag engine wants 106. The lope people love in a muscle car
//              IS the overlap, and the overlap is a manufacturing defect
//              deliberately preserved because it sells cars.
//
// ── Overlap ───────────────────────────────────────────────────────────────
//
// Near TDC between exhaust and intake, both valves are open at once. This is
// not an oversight. A column of exhaust gas leaving at 500 m/s has momentum,
// and if the intake cracks open while it is still leaving, the departing gas
// pulls fresh charge in behind it and scavenges the chamber. Get it right at
// the rpm you care about and the cylinder fills past 100% on atmospheric
// pressure alone. Get it wrong — which is to say, run the same cam at idle —
// and exhaust reverses back up the intake, dilutes the charge, and the engine
// hunts and stumbles and makes that noise.
//
// ── The lift curve ────────────────────────────────────────────────────────
//
// Modelled here as a raised cosine: zero lift, zero velocity at both flanks,
// peak at the centreline, symmetric. A real lobe is not symmetric. It has an
// opening ramp that takes up lash gently and a slower closing ramp that sets
// the valve down at under 0.5 m/s instead of hammering the seat, and the flank
// between them is as close to a straight line as the follower and the spring
// will tolerate — because the area under this curve is the only thing the
// engine actually cares about, and a triangle has more area than a sine. The
// pursuit of that area, against valve float and spring surge and the finite
// stiffness of a pushrod, is the whole discipline of cam design and it is not
// in this file. What is here is the shape a cam would have if steel were
// infinitely stiff, which is the honest idealisation to start from.

#pragma once

#include <cmath>
#include <engine/si.hpp>

namespace engine {

// One lobe. Angles are crank angles, measured within the 720° cycle from the
// firing TDC of the cylinder this lobe serves.
class Lobe {
public:
    // centreline — crank angle of peak lift, relative to firing TDC
    // duration   — crank degrees the valve is off its seat
    // max_lift   — at the valve, after the rocker
    // diameter   — valve head diameter, which sets the flow area
    constexpr Lobe(double centreline, double duration, double max_lift, double diameter)
        : centreline_{centreline}, duration_{duration}
        , max_lift_{max_lift}, diameter_{diameter}
    {}

    constexpr double centreline() const { return centreline_; }
    constexpr double duration()   const { return duration_;   }
    constexpr double max_lift()   const { return max_lift_;   }
    constexpr double diameter()   const { return diameter_;   }

    constexpr double opens()  const { return centreline_ - duration_ * 0.5; }
    constexpr double closes() const { return centreline_ + duration_ * 0.5; }

    // Valve lift at a crank angle measured from this cylinder's firing TDC.
    // The angle is folded into the cycle before use, because a lobe that opens
    // at −16° and one that opens at 704° are the same lobe.
    double lift(double theta) const {
        const double cycle = 2.0 * si::two_pi;
        double phase = std::fmod(theta - centreline_, cycle);
        if (phase >  cycle * 0.5) phase -= cycle;
        if (phase < -cycle * 0.5) phase += cycle;

        const double half = duration_ * 0.5;
        if (std::abs(phase) >= half) return 0.0;

        return max_lift_ * 0.5 * (1.0 + std::cos(si::pi * phase / half));
    }

    bool is_open(double theta) const { return lift(theta) > 0.0; }

private:
    double centreline_, duration_, max_lift_, diameter_;
};

// The two lobes serving one cylinder. Every cylinder in this engine gets the
// same pair, because they share a shaft — which is itself an assumption worth
// naming, since it is exactly the assumption that variable cam phasing and
// cylinder deactivation exist to break.
class Camshaft {
public:
    constexpr Camshaft(Lobe intake, Lobe exhaust)
        : intake_{intake}, exhaust_{exhaust} {}

    constexpr const Lobe& intake()  const { return intake_;  }
    constexpr const Lobe& exhaust() const { return exhaust_; }

    // The angle between the two peaks, ground into the billet. See above.
    constexpr double lobe_separation_angle() const {
        return 0.5 * (intake_.centreline() - exhaust_.centreline());
    }

    // Crank degrees with both valves off their seats. The lope.
    constexpr double overlap() const {
        const double o = exhaust_.closes() - intake_.opens();
        return o > 0.0 ? o : 0.0;
    }

    // Build a cam the way a catalogue lists one: durations, lifts, and where
    // the intake peak sits. Exhaust centreline follows from the LSA.
    static constexpr Camshaft from_card(double intake_duration,
                                        double exhaust_duration,
                                        double intake_lift,
                                        double exhaust_lift,
                                        double intake_centreline,   // after TDC
                                        double lobe_separation,
                                        double intake_valve_diameter,
                                        double exhaust_valve_diameter)
    {
        const double exhaust_centreline = intake_centreline - 2.0 * lobe_separation;
        return Camshaft{
            Lobe{intake_centreline,  intake_duration,  intake_lift,  intake_valve_diameter},
            Lobe{exhaust_centreline, exhaust_duration, exhaust_lift, exhaust_valve_diameter}
        };
    }

private:
    Lobe intake_, exhaust_;
};

} // namespace engine
