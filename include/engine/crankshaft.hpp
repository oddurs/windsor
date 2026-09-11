// crankshaft.hpp — where the sound comes from.
//
// This is the most important file in the project, and it contains almost no
// physics. It is bookkeeping about angles. But every characteristic noise a V8
// has ever made is decided here, and nowhere else.
//
// ── The forging ───────────────────────────────────────────────────────────
//
// A V8 crankshaft has four rod journals. Each journal carries two connecting
// rods, one from each bank, side by side on the same pin. Eight rods, four
// pins. That sharing is the whole reason a V engine is short.
//
// The four pins sit at angles around the crank — the throws. You have two
// sensible choices, and the entire character of the engine follows from which
// one you forge:
//
//      CROSS-PLANE      throws at 0°, 90°, 180°, 270°
//                       Viewed down the nose, a cross. Detroit, 1923, and
//                       nearly every American V8 since.
//
//      FLAT-PLANE       throws at 0°, 180°, 0°, 180°
//                       Viewed down the nose, a line. Maranello, and every
//                       racing V8 that has ever mattered.
//
// ── Why it makes no difference, and all the difference ────────────────────
//
// Both cranks fire evenly. In a 90° V8 the bank angle is exactly half the
// 180° a four-stroke needs between adjacent firings, so either crank produces
// a power stroke every 90° of rotation, eight times per 720° cycle, perfectly
// spaced. At the flywheel the two engines are indistinguishable in rhythm.
//
// But an engine does not exhale through its flywheel. It exhales through two
// exhaust manifolds, one per bank, and the manifold does not hear the engine —
// it hears its own four cylinders. Ask each bank what it heard:
//
//      CROSS-PLANE bank:   90° — 180° — 270° — 180°       lopsided
//      FLAT-PLANE  bank:  180° — 180° — 180° — 180°       even
//
// That is the entire difference. That is the burble and that is the shriek.
// A cross-plane bank coughs twice in quick succession, waits three quarters of
// a turn, and coughs again — a limping, syncopated pulse train that beats
// against the even pulse train from the other bank, and the interference is
// the sound of a Mustang idling outside a diner. A flat-plane bank exhales in
// perfect metronomic thirds of a revolution and the two banks lock in phase,
// and that clean stack of harmonics at twice the frequency is the sound of a
// 458 leaving a tunnel.
//
// The cross-plane crank was not adopted for its voice. It was adopted because
// throws at 90° let the counterweights cancel the secondary shaking forces of
// all eight pistons, which is why an American V8 can idle at 600 rpm on soft
// mounts without walking across the engine bay, and why a flat-plane V8 needs
// a stiff subframe and a driver who does not mind. The sound was a side effect.
// It has outlived every reason it was made.
//
// ── What this file does ───────────────────────────────────────────────────
//
// It refuses to let you type in a firing order.
//
// A firing order is not a design input; it is a result. You forge a crank, you
// hang the rods on it, you grind a cam that decides which of each pin's two
// cylinders fires on the first revolution and which on the second, and the
// firing order is whatever falls out. That is why Ford and Chevrolet, using
// cranks of identical geometry, ended up with 1-5-4-2-6-3-7-8 and
// 1-8-4-3-6-5-7-2: same forging, different cam. So here you specify the crank
// and the cam, and the firing order is derived, and you may check it against
// the number cast into the intake manifold.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <engine/si.hpp>

namespace engine {

inline constexpr int cylinder_count = 8;
inline constexpr int journal_count  = 4;

// Which cylinder head a cylinder lives under. The two banks are what the
// crankshaft's symmetry is hidden from; see above.
enum class Bank { left, right };

constexpr const char* name_of(Bank b) { return b == Bank::left ? "left" : "right"; }

// How a cylinder is hung on the crank. Three facts and no more.
struct Rod {
    int  journal;      // 0..3 — which pin this rod shares
    Bank bank;         // which head it fires under
    int  revolution;   // 0 or 1 — does it fire on the first or second turn?
                       // This is the camshaft's only say in the matter, and it
                       // is the difference between a Ford and a Chevrolet.
};

// The shape of the forging, named for what you see down the nose of it.
enum class Plane { cross, flat, irregular };

constexpr const char* name_of(Plane p) {
    switch (p) {
        case Plane::cross: return "cross-plane";
        case Plane::flat:  return "flat-plane";
        default:           return "irregular";
    }
}

class Crankshaft {
public:
    // vee          — the included angle between the banks. 90° for everything
    //                in this file; 60° and 72° V8s exist and are compromises.
    // throws       — the four journal angles, in the order they sit along the
    //                shaft from the nose. Absolute values are arbitrary; only
    //                the differences matter.
    // rods         — cylinder 1..8, in cylinder-number order.
    // inertia      — polar moment of the rotating assembly: crank, flywheel,
    //                clutch, damper. This is what resists every power stroke
    //                and what smooths an idle. A light flywheel is why a race
    //                engine blips and a truck engine lugs.
    Crankshaft(double vee,
               std::array<double, journal_count> throws,
               std::array<Rod, cylinder_count>   rods,
               double inertia)
        : vee_{vee}, throws_{throws}, rods_{rods}, inertia_{inertia}
    {
        const double half_vee = vee_ * 0.5;

        // Derive every cylinder's firing top-dead-centre from the forging.
        //
        //      φ = (ψ ± ½V  mod 360°) + 360°·rev
        //
        // The pin angle ψ says when the piston would reach TDC if its bore
        // were vertical; the half-vee leans the bore off vertical and shifts
        // that moment. Both together fix the TDC within a revolution — and a
        // piston reaches TDC twice a cycle, once on compression with fire
        // above it and once on overlap with nothing. The revolution flag is
        // the camshaft choosing which of those two is the live one, so it is
        // added after the fold into a single turn, not before: `revolution`
        // means the first or second turn of the cycle, exactly as it sounds.
        // Left bank leads, right bank trails: a sign convention, and the only
        // arbitrary line in this file.
        for (int c = 0; c < cylinder_count; ++c) {
            const Rod&   rod   = rods_[c];
            const double lean  = (rod.bank == Bank::left) ? +half_vee : -half_vee;
            tdc_[c] = wrap_turn(throws_[rod.journal] + lean)
                    + rod.revolution * si::two_pi;
        }

        validate();
    }

    // ── The forging, as specified ─────────────────────────────────────────
    double vee_angle()   const { return vee_;     }
    double inertia()     const { return inertia_; }

    // The four pin angles as forged, in the order they sit along the shaft
    // from the nose. Absolute values are arbitrary; only the spread matters,
    // and the spread is the whole engine.
    const std::array<double, journal_count>& throws() const { return throws_; }
    Bank   bank_of(int cylinder)    const { return rods_[cylinder - 1].bank;    }
    int    journal_of(int cylinder) const { return rods_[cylinder - 1].journal; }

    // The crank angle, within the 720° cycle, at which this cylinder's piston
    // arrives at top dead centre with a fresh charge above it and both valves
    // shut. Every other event in the cylinder is quoted relative to this.
    double firing_tdc(int cylinder) const { return tdc_[cylinder - 1]; }

    // ── What falls out of it ──────────────────────────────────────────────

    // The number cast into the intake manifold. Derived, never declared.
    std::array<int, cylinder_count> firing_order() const {
        std::array<int, cylinder_count> order{};
        for (int i = 0; i < cylinder_count; ++i) order[i] = i + 1;
        std::sort(order.begin(), order.end(),
                  [&](int a, int b) { return tdc_[a - 1] < tdc_[b - 1]; });
        return order;
    }

    // The gaps between firings of the whole engine, in the order they occur.
    // For any 90° V8 on any crank this is eight even 90° intervals, which is
    // precisely why the difference between the two cranks is inaudible here
    // and unmistakable one file downstream.
    std::array<double, cylinder_count> firing_intervals() const {
        return intervals_of(firing_order());
    }

    // The cylinders of one bank, in the order that bank fires them.
    std::array<int, 4> firing_order(Bank bank) const {
        std::array<int, 4> order{};
        int n = 0;
        for (int c = 1; c <= cylinder_count; ++c)
            if (bank_of(c) == bank) order[n++] = c;
        std::sort(order.begin(), order.end(),
                  [&](int a, int b) { return tdc_[a - 1] < tdc_[b - 1]; });
        return order;
    }

    // THE RESULT. The pulse train one exhaust manifold actually receives.
    // Cross-plane: 90-180-270-180. Flat-plane: 180-180-180-180.
    // Everything this project makes audible is contained in these four numbers.
    std::array<double, 4> firing_intervals(Bank bank) const {
        return intervals_of(firing_order(bank));
    }

    // Read the shape back off the forging rather than trusting the label.
    Plane plane() const {
        std::array<double, journal_count> t = throws_;
        const double reference = t[0];          // absolute throw angles are
        for (double& x : t) x = wrap_turn(x - reference);   // arbitrary; only
        std::sort(t.begin(), t.end());                      // the spread counts

        auto near = [](double a, double b) { return std::abs(a - b) < 1e-9; };
        const double q = si::pi / 2.0;

        if (near(t[0], 0) && near(t[1], q) && near(t[2], 2 * q) && near(t[3], 3 * q))
            return Plane::cross;
        if (near(t[0], 0) && near(t[1], 0) && near(t[2], si::pi) && near(t[3], si::pi))
            return Plane::flat;
        return Plane::irregular;
    }

    // Is every bank's pulse train even? The one-line statement of the thesis.
    bool banks_fire_evenly() const {
        for (Bank b : {Bank::left, Bank::right}) {
            const auto gaps = firing_intervals(b);
            for (double g : gaps)
                if (std::abs(g - gaps[0]) > 1e-9) return false;
        }
        return true;
    }

private:
    // Angles in this class live in [0, 720°) — the four-stroke cycle — because
    // a cylinder's events do not repeat every revolution, they repeat every
    // two, and folding them into 360° is how you lose an engine's whole cycle.
    static double wrap_cycle(double a) {
        const double cycle = 2.0 * si::two_pi;
        a = std::fmod(a, cycle);
        return a < 0 ? a + cycle : a;
    }

    // ...except the throws themselves, which are features of a shaft that
    // turns once per revolution and know nothing of the cycle.
    static double wrap_turn(double a) {
        a = std::fmod(a, si::two_pi);
        return a < 0 ? a + si::two_pi : a;
    }

    template <std::size_t N>
    std::array<double, N> intervals_of(const std::array<int, N>& order) const {
        std::array<double, N> gaps{};
        const double cycle = 2.0 * si::two_pi;
        for (std::size_t i = 0; i < N; ++i) {
            const double here = tdc_[order[i] - 1];
            const double next = tdc_[order[(i + 1) % N] - 1];
            gaps[i] = wrap_cycle(next - here);
            if (gaps[i] < 1e-9) gaps[i] = cycle;   // a lone cylinder waits a full cycle
        }
        return gaps;
    }

    // A crank that cannot exist should not compile into an engine that runs.
    void validate() const {
        std::array<int, journal_count> per_journal{};
        for (const Rod& r : rods_) {
            if (r.journal < 0 || r.journal >= journal_count)
                throw std::invalid_argument{"rod hung on a journal that is not on this crank"};
            if (r.revolution != 0 && r.revolution != 1)
                throw std::invalid_argument{"a four-stroke cylinder fires on revolution 0 or 1"};
            ++per_journal[r.journal];
        }
        for (int n : per_journal)
            if (n != 2)
                throw std::invalid_argument{"every rod journal carries exactly two rods"};

        // No two cylinders may want the same instant: an engine that fires two
        // cylinders at once has thrown a rod through the block by lunchtime.
        for (int a = 0; a < cylinder_count; ++a)
            for (int b = a + 1; b < cylinder_count; ++b)
                if (std::abs(tdc_[a] - tdc_[b]) < 1e-9)
                    throw std::invalid_argument{"two cylinders share a firing angle"};
    }

    double                              vee_;
    std::array<double, journal_count>   throws_;
    std::array<Rod, cylinder_count>     rods_;
    double                              inertia_;
    std::array<double, cylinder_count>  tdc_{};
};

} // namespace engine
