// windsor.hpp — the engine itself, as built.
//
// A Ford 302, which is to say 301.6 cubic inches, which is to say the small
// block Ford introduced as a 221 in 1962, bored and stroked through a 260, a
// 289 and a 302, and built without interruption until 2001. It is the engine
// under the Fox-body Mustang, the Bronco, the Cortina that shouldn't have had
// it, and a substantial fraction of every hot rod built in the last fifty
// years. It is not the best V8 ever made and it is close to the most useful.
//
// Every figure below is the real one, from the 1968 shop manual, the casting,
// or the cam card. Where a figure has been chosen rather than measured — the
// burn duration, the friction coefficients, the wall temperature — it is
// marked, because the difference between a specification and a calibration is
// the difference between a model and a fit.

#pragma once

#include <engine/engine.hpp>

namespace engine::windsor {

using namespace si::literals;

// ── The bottom end ────────────────────────────────────────────────────────
// 4.000 × 3.000, oversquare by exactly a third — a short-stroke engine that
// wants to rev and a block with very little room to stroke it, which is why
// the 351 needed a taller deck and became a different engine.
inline constexpr Geometry short_block() {
    return Geometry{
        Bore{4.000_in},            // standard, before any overbore
        Stroke{3.000_in},
        RodLength{5.090_in},       // 302; the 289 used the same rod
        CompressionRatio{9.5}      // 1968 2V, on leaded regular
    };
}

// ── The camshaft ──────────────────────────────────────────────────────────
// A stock hydraulic flat-tappet grind. Wide separation for a smooth idle and
// a vacuum figure the power brakes can live on, and modest duration because
// this engine was sold to people who wanted it to start in February.
inline constexpr Camshaft stock_cam() {
    return Camshaft::from_card(
        266.0_deg, 266.0_deg,      // advertised duration, in / ex
        0.375_in,  0.375_in,       // lift AT THE VALVE, after 1.6:1 rockers
        112.0_deg,                 // intake centreline, ATDC
        112.0_deg,                 // lobe separation
        1.780_in,  1.450_in);      // valve head diameters, stock heads
}

// ── The crankshaft ────────────────────────────────────────────────────────
//
// Four throws at 90°, cast nodular iron, and the reason this engine sounds the
// way it does. The rod table below is the only place in the project where the
// arrangement of the engine is written down, and the firing order is NOT in
// it — the firing order comes out of `Crankshaft` when you ask.
//
// It comes out 1-5-4-2-6-3-7-8, which is what is cast into the intake.

inline Crankshaft cross_plane_crank() {
    return Crankshaft{
        90.0_deg,
        { 45.0_deg, 315.0_deg, 135.0_deg, 225.0_deg },
        {{  // cylinder      journal  bank          revolution
            /* 1 */         { 0, Bank::right, 0 },
            /* 2 */         { 1, Bank::right, 0 },
            /* 3 */         { 2, Bank::right, 1 },
            /* 4 */         { 3, Bank::right, 0 },
            /* 5 */         { 0, Bank::left,  0 },
            /* 6 */         { 1, Bank::left,  1 },
            /* 7 */         { 2, Bank::left,  1 },
            /* 8 */         { 3, Bank::left,  1 },
        }},
        0.20                       // kg·m², crank + damper + flywheel + clutch
    };
}

// The 1982 5.0 H.O. camshaft, on the identical forging above. Ford changed
// nothing but which cylinder on each pin fires first, and got the 351W firing
// order, 1-3-7-2-6-5-4-8. The engine sounds exactly the same, because the
// crank is exactly the same, because the sound was never in the cam.
inline Crankshaft cross_plane_crank_ho() {
    return Crankshaft{
        90.0_deg,
        { 45.0_deg, 315.0_deg, 135.0_deg, 225.0_deg },
        {{  { 0, Bank::right, 0 }, { 1, Bank::right, 0 },
            { 2, Bank::right, 0 }, { 3, Bank::right, 1 },
            { 0, Bank::left,  1 }, { 1, Bank::left,  1 },
            { 2, Bank::left,  0 }, { 3, Bank::left,  1 }, }},
        0.20
    };
}

// ── The experiment ────────────────────────────────────────────────────────
//
// The same block, the same rods, the same pistons, the same cam. One billet
// crankshaft with all four throws in a single plane, which people do actually
// grind for small block Fords, and which requires a rebalance and a stiffer
// engine mount and produces an engine that will not idle politely.
//
// Fit it and every bank interval goes to 180°. That is the only thing that
// changes, and it changes everything you can hear.
inline Crankshaft flat_plane_crank() {
    return Crankshaft{
        90.0_deg,
        { 45.0_deg, 225.0_deg, 45.0_deg, 225.0_deg },
        {{  { 0, Bank::right, 0 }, { 1, Bank::right, 0 },
            { 2, Bank::right, 1 }, { 3, Bank::right, 1 },
            { 0, Bank::left,  0 }, { 1, Bank::left,  0 },
            { 2, Bank::left,  1 }, { 3, Bank::left,  1 }, }},
        0.16                       // lighter: a flat crank needs less
                                   // counterweight, having less to cancel
    };
}

// ── Everything else ───────────────────────────────────────────────────────

inline Engine::Specification specification(Crankshaft crank, const char* name) {
    return Engine::Specification{
        short_block(),
        stock_cam(),
        std::move(crank),

        // CALIBRATED, not specified. A 1968 wedge chamber with the plug off to
        // one side and a quench pad on the other burns slowly; 70° is what it
        // takes to put peak pressure 13° after TDC on 34° of advance, which is
        // where the indicator cards say it was.
        Wiebe{70.0_deg},

        Fuel::gasoline(),
        Distributor{Distributor::stock_302()},
        Friction{Friction::pushrod_v8()},

        Induction::Setup{
            2.0_L,                 // under a cast-iron 2V intake
            0.056_m,               // both barrels of an Autolite 2100, as one
            25.0e-6,               // idle bypass: the curb idle screw
            320.0_K                // charge temperature after the hot intake
        },

        Exhaust::Setup{
            0.80_m,   1.625_in,    // primaries — a mid-length header
            1.50_m,   2.500_in,    // collector back to the open end
            3.0_L,                 // the collector as a plenum
            0.0020,                // m², what the system leaves open
            900.0_K,               // CALIBRATED: pipe gas temperature
            44100.0                // the microphone
        },

        0.780,                     // kg reciprocating: piston, pin, rings,
                                   // and the small end's share of the rod
        400.0_K,                   // CALIBRATED: mean wall temperature
        name
    };
}

inline Engine stock()      { return Engine{specification(cross_plane_crank(),    "302 Windsor")}; }
inline Engine high_output(){ return Engine{specification(cross_plane_crank_ho(), "302 H.O.")}; }
inline Engine flat_crank() { return Engine{specification(flat_plane_crank(),     "302, flat crank")}; }

} // namespace engine::windsor
