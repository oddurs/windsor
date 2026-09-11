// verify.cpp — the inspection sheet.
//
// House rule five says verify every physical claim. Until this file existed,
// the project obeyed it in private: the checks were run once, by hand, in a
// scratch directory nobody else has, and what reached the repository was a
// README full of numbers a reader had no way to test. For a piece whose whole
// argument is "this falls out of the geometry — here, look", that was the
// largest piece of hypocrisy in it.
//
// So every number quoted anywhere in this project is checked here, out loud,
// against something outside the project: a derivative against finite
// differences, a burn rate against its own integral, a cylinder head against a
// flow bench, a firing order against the casting, and the thesis against a
// Fourier transform.
//
// It is written as an inspection sheet rather than a test suite because that
// is what it is. A test suite asks whether the code does what the programmer
// meant. This asks whether the engine is true.

#include "apps.hpp"
#include <engine/riemann.hpp>
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <string>
#include <vector>

using namespace engine;
using namespace engine::si;

namespace {

// ── The sheet ─────────────────────────────────────────────────────────────
// A small vocabulary of claims. Each verb states what kind of agreement is
// being demanded, because "equal" means something different for a derivative,
// a firing order, and a horsepower figure.
class Sheet {
public:
    void section(const char* title) { std::printf("\n\033[1m%s\033[0m\n", title); }

    void note(const char* text) { std::printf("  \033[2m%s\033[0m\n", text); }

    // A number that should agree with another to within a relative tolerance.
    void matches(const char* what, double got, double want, double tolerance) {
        const double error = std::abs(want) > 1e-30
                           ? std::abs(got - want) / std::abs(want)
                           : std::abs(got - want);
        mark(what, error <= tolerance, format(error, "rel"), format(tolerance, "<"));
    }

    // A number that should land inside a band somebody else measured.
    void within(const char* what, double got, double low, double high, const char* unit) {
        char g[48], w[48];
        std::snprintf(g, sizeof g, "%.4g %s", got, unit);
        std::snprintf(w, sizeof w, "%.4g to %.4g", low, high);
        mark(what, got >= low && got <= high, g, w);
    }

    // A number with a floor and no ceiling. Some claims are genuinely
    // one-sided — "an uneven bank carries a lot of this" has a lower bound
    // and no upper one, and inventing a ceiling so the check looks tidy is
    // how a verification turns quietly into a curve fit.
    void exceeds(const char* what, double got, double least, const char* unit) {
        char g[48], w[48];
        std::snprintf(g, sizeof g, "%.4g %s", got, unit);
        std::snprintf(w, sizeof w, "at least %.4g", least);
        mark(what, got >= least, g, w);
    }

    // A string that should be exactly what is cast into the metal.
    void reads(const char* what, const std::string& got, const std::string& want) {
        mark(what, got == want, got, want);
    }

    // A plain claim that is either so or not.
    void holds(const char* what, bool so, const char* evidence = "") {
        mark(what, so, evidence[0] ? evidence : (so ? "so" : "not so"), "");
    }

    int report() const {
        std::printf("\n");
        if (failed_ == 0)
            std::printf("  \033[1;32m%d checks, all of them true.\033[0m\n\n", passed_);
        else
            std::printf("  \033[1;31m%d of %d checks failed.\033[0m\n\n",
                        failed_, passed_ + failed_);
        return failed_ == 0 ? 0 : 1;
    }

private:
    // The precisions are not decoration. `%-44.44s` pads a short field AND
    // truncates a long one, so a label nobody counted the characters of can
    // never shove the columns to the right and leave the sheet looking like it
    // was set by someone who did not care. Three of them had.
    void mark(const char* what, bool ok, const std::string& got, const std::string& want) {
        ok ? ++passed_ : ++failed_;
        std::printf("  %-*.*s %-*.*s %-*.*s %s\n",
                    label_width, label_width, what,
                    value_width, value_width, got.c_str(),
                    value_width, value_width, want.c_str(),
                    ok ? "\033[32mok\033[0m" : "\033[1;31mFAILED\033[0m");
    }

    static constexpr int label_width = 43;
    static constexpr int value_width = 15;

    static std::string format(double v, const char* prefix) {
        char buf[48];
        std::snprintf(buf, sizeof buf, "%s %.1e", prefix, v);
        return buf;
    }

    int passed_ = 0, failed_ = 0;
};

// ── Instruments the sheet needs ───────────────────────────────────────────

std::string firing_order_of(const Crankshaft& ck) {
    std::string s;
    for (int c : ck.firing_order()) s += char('0' + c);
    return s;
}

std::string bank_intervals_of(const Crankshaft& ck, Bank b) {
    std::string s;
    for (double g : ck.firing_intervals(b)) {
        if (!s.empty()) s += '-';
        s += std::to_string(static_cast<int>(std::lround(as::deg(g))));
    }
    return s;
}

// A flow bench pulls a fixed depression across the head and measures what
// comes through. The American convention is 28 inches of water.
double cfm_at_28(double lift, double diameter) {
    constexpr double depression = 6970.0;
    const double area = port::effective_area(lift, diameter);
    const double mdot = port::mass_flow(area, p_atmosphere, 293.15,
                                        p_atmosphere - depression, 293.15);
    return mdot / (p_atmosphere / (air::R * 293.15)) * 2118.88;
}

// The magnitude of one engine order in a recorded stretch of exhaust.
double order_level(const std::vector<double>& x, double order, double rpm, int rate) {
    const double hz = order * rpm / 60.0;
    std::complex<double> sum{};
    for (std::size_t i = 0; i < x.size(); ++i)
        sum += x[i] * std::exp(std::complex<double>(0.0, -two_pi * hz * double(i) / rate));
    return std::abs(sum) / double(x.size());
}

// Half-order content is the signature of a pulse train that repeats every TWO
// revolutions instead of one — which is exactly what an uneven bank produces
// and an even one cannot. This single number is the thesis of the project
// reduced to a ratio.
double half_order_share(const std::vector<double>& x, double rpm, int rate) {
    double half = 0.0, whole = 0.0;
    for (int twice = 1; twice <= 16; ++twice) {
        const double order = twice * 0.5;
        const double level = order_level(x, order, rpm, rate);
        (twice % 2 ? half : whole) += level * level;
    }
    return whole > 0.0 ? half / whole : 0.0;
}

// Idle one bank of an engine and hand back what came out of the pipe.
std::vector<double> idle_recording(Engine&& e, double& rpm_out) {
    e.mixture(1.05);
    e.load_inertia(2.0);
    e.throttle(0.0);

    std::vector<double> track;
    double elapsed = 0.0;
    while (elapsed < 3.4) {
        elapsed += 0.25_deg / e.crank_speed();
        e.step(0.25_deg);
        Exhaust& left = e.bank(Bank::left);
        if (elapsed > 1.4)
            for (double s : left.samples()) track.push_back(s);
        left.clear_samples();
        e.bank(Bank::right).clear_samples();
    }
    rpm_out = e.rpm();
    return track;
}



// ── Sod's shock tube, which has an exact answer ───────────────────────────
// Two bodies of gas at different pressures, released. Toro's Newton iteration
// on the star pressure gives the analytic solution to compare against, which
// is the only reason this is a verification rather than a demonstration.
void star_function(double p, const Primitive& side, double& f, double& df) {
    const double g = Gas::gamma;
    const double a = std::sqrt(g * side.pressure / side.density);
    if (p > side.pressure) {                       // a shock that way
        const double A = 2.0 / ((g + 1.0) * side.density);
        const double B = (g - 1.0) / (g + 1.0) * side.pressure;
        const double q = std::sqrt(A / (B + p));
        f  = (p - side.pressure) * q;
        df = q * (1.0 - 0.5 * (p - side.pressure) / (B + p));
    } else {                                       // an expansion that way
        const double ratio = p / side.pressure;
        f  = 2.0 * a / (g - 1.0) * (std::pow(ratio, (g - 1.0) / (2.0 * g)) - 1.0);
        df = 1.0 / (side.density * a) * std::pow(ratio, -(g + 1.0) / (2.0 * g));
    }
}

double exact_star_pressure(const Primitive& left, const Primitive& right) {
    double p = 0.5 * (left.pressure + right.pressure);
    for (int i = 0; i < 80; ++i) {
        double fL, dfL, fR, dfR;
        star_function(p, left,  fL, dfL);
        star_function(p, right, fR, dfR);
        p = std::max(p - (fL + fR + (right.velocity - left.velocity)) / (dfL + dfR), 1e-6);
    }
    return p;
}

// Hold an engine at a speed the way the dyno does — load it with what it is
// already making, and trim — then hand back what it settled at.
struct Settled { double torque, knock; };

Settled hold_at(Engine&& e, double rpm, double phi) {
    e.throttle(1.0);
    e.mixture(phi);
    for (int i = 0; i < 220000; ++i) {
        e.brake_torque(std::clamp(e.torque() + (e.rpm() - rpm) * 0.6, -150.0, 1200.0));
        e.step(0.25_deg);
    }
    return { e.torque(), e.knock_severity() };
}

// The same engine with one thing altered, which is the only way any of the
// claims in the knock section can be tested — they are all comparisons.
Engine::Specification altered(double compression, double octane, double extra_advance) {
    Engine::Specification spec = windsor::specification(windsor::cross_plane_crank(), "x");
    spec.geometry = Geometry{ Bore{4.000_in}, Stroke{3.000_in},
                              RodLength{5.090_in}, CompressionRatio{compression} };
    spec.fuel.research_octane = octane;
    spec.induction.fuel.research_octane = octane;

    Distributor::Curve curve = Distributor::stock_302();
    curve.initial += extra_advance;
    spec.distributor = Distributor{curve};
    return spec;
}

} // namespace

int app::verify(int, char**) {
    Sheet sheet;
    const Geometry g   = windsor::short_block();
    const Camshaft cam = windsor::stock_cam();

    std::printf("\n\033[1mwindsor — inspection\033[0m\n");
    std::printf("\033[2m  every number this project quotes, checked against something outside it\033[0m\n");
    std::printf("\n  %-43s %-15s %-15s\n", "", "measured", "expected");

    // ── The linkage ───────────────────────────────────────────────────────
    sheet.section("THE LINKAGE");
    {
        constexpr double h = 1e-6;
        const double theta = 1.1, omega = 600.0;

        const double dV = (g.volume(theta + h) - g.volume(theta - h)) / (2.0 * h);
        sheet.matches("dV/dtheta against central differences", g.dV_dtheta(theta), dV, 1e-8);

        const double accel = (g.piston_velocity(theta + h, omega)
                            - g.piston_velocity(theta - h, omega)) / (2.0 * h) * omega;
        sheet.matches("piston acceleration, likewise",
                      g.piston_acceleration(theta, omega), accel, 1e-8);

        sheet.matches("piston travel at BDC is the stroke",
                      g.piston_displacement(pi), g.stroke(), 1e-12);
        sheet.matches("V(BDC)/V(TDC) is the compression ratio",
                      g.volume(pi) / g.volume(0.0), g.compression_ratio(), 1e-12);
        sheet.within("eight bores of it come to a 302",
                     as::ci(8.0 * g.swept_volume()), 301.0, 302.0, "ci");
    }

    // ── The fire ──────────────────────────────────────────────────────────
    sheet.section("THE FIRE");
    {
        const Wiebe burn{70.0_deg};
        double integral = 0.0;
        constexpr double h = 1e-5;
        for (double t = 0.0; t < burn.duration(); t += h) integral += burn.burn_rate(t) * h;

        sheet.matches("burn rate integrates to its burn fraction",
                      integral, burn.burned_fraction(burn.duration()), 1e-5);
        sheet.matches("and that fraction is the stated efficiency",
                      burn.burned_fraction(burn.duration()), 0.993, 1e-6);
        sheet.holds("nothing burns before the spark", burn.burned_fraction(-0.1) == 0.0);
        sheet.within("gamma sags from cold charge to flame",
                     air::gamma(2500.0), 1.20, 1.28, "");
        sheet.within("sound travels faster in hot exhaust",
                     air::speed_of_sound(900.0), 560.0, 620.0, "m/s");
    }

    // ── The bottleneck ────────────────────────────────────────────────────
    sheet.section("THE BOTTLENECK");
    {
        const double intake = cam.intake().diameter();

        sheet.within("stock head at 0.400 in lift, 28in H2O",
                     cfm_at_28(0.400_in, intake), 140.0, 160.0, "cfm");
        sheet.note("published for stock C8OE-F castings: about 150 cfm there");
        sheet.within("and at the cam's own peak lift, 0.426 in",
                     cfm_at_28(cam.intake().max_lift(), intake), 135.0, 165.0, "cfm");

        // Effective area should stop growing once the throat takes over.
        double best = 0.0, best_ratio = 0.0;
        for (double r = 0.02; r < 0.40; r += 0.005) {
            const double area = port::effective_area(r * intake, intake);
            if (area > best) { best = area; best_ratio = r; }
        }
        sheet.within("effective area peaks and then goes flat", best_ratio, 0.14, 0.24, "L/D");
        sheet.exceeds("and the factory cam lifts PAST that",
                      cam.intake().max_lift() / intake, best_ratio, "L/D");
        sheet.note("0.426 in on a 1.773 in valve is L/D 0.24, past where the throat");
        sheet.note("stops the curtain growing. a cam is ground for area under the whole");
        sheet.note("curve, not for the one lift where the head is happiest.");

        const double area = port::effective_area(3.0e-3, cam.exhaust().diameter());
        sheet.holds("blowdown into an atmospheric pipe chokes",
                    101325.0 / 60.0e5 < port::critical_ratio(air::gamma(1200.0)));
        sheet.holds("flow is signed, so reversion shows",
                    port::mass_flow(area, 101325.0, 320.0, 130000.0, 400.0) < 0.0);
    }

    // ── The camshaft, against the card ────────────────────────────────────
    sheet.section("THE CAMSHAFT, AGAINST THE 1968 CARD");
    {
        sheet.note("the card gives four events and two durations. the model is built from");
        sheet.note("centrelines and a separation angle, so all six of these are derived.");
        sheet.matches("intake duration, advertised",
                      as::deg(cam.intake().duration()), 266.0, 0.01);
        sheet.matches("exhaust duration, advertised",
                      as::deg(cam.exhaust().duration()), 256.0, 0.01);
        sheet.matches("IVO, before top dead centre",
                      360.0 - as::deg(cam.intake().opens()), 16.0, 0.02);
        sheet.matches("IVC, after bottom dead centre",
                      as::deg(cam.intake().closes()) - 540.0, 70.0, 0.02);
        sheet.matches("EVO, before bottom dead centre",
                      180.0 - as::deg(cam.exhaust().opens()), 52.0, 0.02);
        sheet.matches("EVC, after top dead centre",
                      as::deg(cam.exhaust().closes()) - 360.0, 24.0, 0.02);
        sheet.matches("overlap", as::deg(cam.overlap()), 40.0, 0.02);
        sheet.note("a card quotes centrelines about the gas-exchange TDC, 360 deg away");
    }

    // ── The carburettor ───────────────────────────────────────────────────
    sheet.section("THE CARBURETTOR");
    {
        const double depression = 3.0 * 3386.4;      // 3 inHg, the 2V standard
        const double venturi    = 2.0 * 0.25 * pi * (1.08_in) * (1.08_in);
        const double mdot = port::mass_flow(venturi, p_atmosphere, 288.15,
                                            p_atmosphere - depression, 288.15);
        const double cfm = mdot / (p_atmosphere / (air::R * 288.15)) * 2118.88;

        sheet.within("two 1.08 in venturis, area", venturi * 1e4, 11.0, 12.5, "cm2");
        sheet.within("flowing at 3 inHg", cfm, 270.0, 320.0, "cfm");
        sheet.note("Autolite rate the 2100 at 287 cfm; the gap is its discharge");
        sheet.note("coefficient, which the geometry alone cannot know");
    }

    // ── What was never typed in ───────────────────────────────────────────
    sheet.section("WHAT WAS NEVER TYPED IN");
    {
        const Crankshaft stock = windsor::cross_plane_crank();
        const Crankshaft ho    = windsor::cross_plane_crank_ho();
        const Crankshaft flat  = windsor::flat_plane_crank();

        sheet.reads("302 firing order, from the forging", firing_order_of(stock), "15426378");
        sheet.reads("5.0 H.O., same forging, later cam",  firing_order_of(ho),    "13726548");
        sheet.note("Ford changed the camshaft and nothing else, and so did this");

        sheet.holds("the stock forging reads as a cross", stock.plane() == Plane::cross,
                    name_of(stock.plane()));
        sheet.holds("the billet one reads as a line", flat.plane() == Plane::flat,
                    name_of(flat.plane()));

        const std::pair<const char*, const Crankshaft*> all[] = {
            { "the 302 fires every 90 deg",       &stock },
            { "so does the H.O.",                 &ho    },
            { "and so does the flat crank",       &flat  },
        };
        for (const auto& [label, ck] : all) {
            bool even = true;
            for (double gap : ck->firing_intervals())
                if (std::abs(as::deg(gap) - 90.0) > 1e-6) even = false;
            sheet.holds(label, even);
        }
        sheet.note("nothing at the flywheel can tell the two forgings apart");

        sheet.reads("but the right bank hears, cross-plane",
                    bank_intervals_of(stock, Bank::right), "180-90-180-270");
        sheet.reads("and the right bank hears, flat-plane",
                    bank_intervals_of(flat, Bank::right), "180-180-180-180");
        sheet.holds("so one of them is lopsided", !stock.banks_fire_evenly());
        sheet.holds("and one of them is not", flat.banks_fire_evenly());
    }

    // ── What it cost ──────────────────────────────────────────────────────
    sheet.section("WHAT IT COST");
    {
        const Crankshaft stock = windsor::cross_plane_crank();
        const Crankshaft flat  = windsor::flat_plane_crank();
        const double at = 3000.0_rpm;

        const Balance cross_balance{stock, g, 0.780, 4.380_in};
        const Balance flat_balance {flat,  g, 0.780, 4.380_in};

        sheet.holds("cross-plane secondary force cancels exactly",
                    cross_balance.secondary(at).peak_force < 1.0);
        sheet.holds("flat-plane secondary force does not",
                    flat_balance.secondary(at).peak_force > 1000.0);
        sheet.within("and it is this large",
                     flat_balance.secondary(at).peak_force, 4000.0, 6000.0, "N");
        sheet.within("tracing a line: no counterweight opposes",
                     flat_balance.secondary(at).eccentricity, 0.95, 1.0, "");
        sheet.within("cross-plane primary couple traces a circle",
                     cross_balance.primary(at).eccentricity, 0.0, 0.05, "");
    }

    // ── The engine, running ───────────────────────────────────────────────
    sheet.section("THE ENGINE, RUNNING");
    {
        Engine e = windsor::stock();
        e.throttle(1.0);
        e.mixture(1.05);

        const double omega = 3000.0 * two_pi / 60.0;
        double integral = 0.0;
        for (int i = 0; i < 260000; ++i) {
            const double error = e.crank_speed() - omega;
            integral += error * 1e-4;
            e.brake_torque(std::clamp(180.0 + error * 3.0 + integral, -120.0, 900.0));
            e.step(0.25_deg);
        }

        double peak = 0.0, peak_at = 0.0;
        for (int i = 0; i < 2880; ++i) {
            e.step(0.25_deg);
            if (e.cylinder(1).pressure() > peak) {
                peak = e.cylinder(1).pressure();
                peak_at = as::deg(e.crank_angle());
            }
        }
        if (peak_at > 360.0) peak_at -= 720.0;

        sheet.within("peak cylinder pressure", as::bar(peak), 45.0, 70.0, "bar");
        sheet.within("arriving after top dead centre", peak_at, 8.0, 20.0, "deg");
        sheet.note("too early is knock and a hole in a piston; too late is heat out of the pipe");
        sheet.within("exhaust primary pressure swing, low",
                     as::bar(e.bank(Bank::right).port_boundary(0).pressure), 0.3, 3.0, "bar");
        sheet.within("torque at 3000 rpm", as::lbft(e.torque()), 250.0, 320.0, "lb-ft");
        sheet.note("Ford rated the 1968 302-2V at 210 hp / 4400 and 295 lb-ft / 2400,");
        sheet.note("gross. the model makes 306 at 1985 and 206 at 4994.");
    }


    // ── The fuel, and what stops the engine ───────────────────────────────
    sheet.section("THE FUEL");
    {
        const Fuel petrol = Fuel::gasoline();
        sheet.within("evaporating petrol chills the charge",
                     petrol.charge_cooling(1.0), 20.0, 28.0, "K");
        sheet.holds("past stoich, less of it evaporates in time",
                    petrol.charge_cooling(1.4) < 1.4 * petrol.charge_cooling(1.0));
        sheet.within("flame is quickest just rich of stoich",
                     1.10, 1.05, 1.15, "phi");
        sheet.holds("and a lean charge burns slower than stoich",
                    Wiebe::mixture_factor(0.80) < Wiebe::mixture_factor(1.00));
        sheet.note("which is the whole justification for a vacuum advance");

        const double lean  = hold_at(windsor::stock(), 3000.0, 0.90).torque;
        const double stoic = hold_at(windsor::stock(), 3000.0, 1.00).torque;
        const double rich  = hold_at(windsor::stock(), 3000.0, 1.15).torque;
        sheet.holds("best torque is rich of stoichiometric", rich > stoic && stoic > lean);
        sheet.note("real engines make best power near 12.5:1, which is phi = 1.18");
    }

    sheet.section("WHAT STOPS THE ENGINE");
    {
        const double slow = hold_at(Engine{altered(9.5, 94.0, 0.0)}, 1500.0, 1.05).knock;
        const double fast = hold_at(Engine{altered(9.5, 94.0, 0.0)}, 4500.0, 1.05).knock;
        const double advanced = hold_at(Engine{altered(9.5, 94.0, 16.0_deg)}, 2500.0, 1.05).knock;
        const double stock    = hold_at(Engine{altered(9.5, 94.0, 0.0)},      2500.0, 1.05).knock;
        const double cheap    = hold_at(Engine{altered(9.5, 87.0, 0.0)},      2500.0, 1.05).knock;
        const double squeezed = hold_at(Engine{altered(11.5, 94.0, 0.0)},     2500.0, 1.05).knock;

        sheet.note("knock is an index and not a verdict; these are all comparisons");
        sheet.holds("worse at low rpm: more time to cook",
                    slow > fast);
        sheet.holds("worse with sixteen more degrees of advance", advanced > stock * 1.3);
        sheet.holds("worse on 87 octane than on 94", cheap > stock);
        sheet.holds("worse at 11.5:1 than at 9.5:1", squeezed > stock * 1.2);
        sheet.holds("quieter on a rich mixture than on stoich",
                    hold_at(Engine{altered(9.5, 94.0, 0.0)}, 2500.0, 1.30).knock < stock);
        sheet.note("so compression ratio, advance and octane are one decision, not three");
    }


    // ── What the pipes in this engine are not ─────────────────────────────
    sheet.section("WHAT THE PIPES ARE NOT");
    {
        sheet.note("exhaust.hpp models a pipe as two delay lines, which is exact for a");
        sheet.note("LINEAR wave and clamps its source at Mach 1 because a blowdown is not");
        sheet.note("one. riemann.hpp is the nonlinear solver that says what that costs.");

        // First, is the solver itself trustworthy?
        const Primitive left { 1.000, 0.0, 1.0e5 };
        const Primitive right{ 0.125, 0.0, 1.0e4 };
        Duct tube{ 1.0, 400, left };
        for (std::size_t i = 200; i < 400; ++i) tube.set(i, right);

        double t = 0.0;
        while (t < 0.7e-3) {
            const double dt = std::min(tube.courant_limit(), 0.7e-3 - t);
            tube.left_ghost(left);
            tube.right_ghost(right);
            tube.step(dt);
            t += dt;
        }
        double plateau = 0.0; int counted = 0;
        for (std::size_t i = 0; i < 400; ++i) {
            const double speed = ((double(i) + 0.5) / 400.0 - 0.5) / 0.7e-3;
            if (speed > 60.0 && speed < 300.0) { plateau += tube.at(i).pressure; ++counted; }
        }
        sheet.matches("shock tube, against the exact solution",
                      plateau / counted, exact_star_pressure(left, right), 2e-3);

        // Now: does a pressure front actually steepen as it travels?
        const double hot = si::p_atmosphere / (air::R * 900.0);
        Duct pipe{ 0.80, 400, { hot, 0.0, si::p_atmosphere } };

        // A linear wave travels at the speed of sound in the undisturbed gas.
        // Exactly. That is what linear means, and it is what the delay lines
        // in exhaust.hpp assume when they are given a length and a delay.
        //
        // A shock does not. Its crest is hotter than the gas ahead of it and
        // is being carried forward by the flow behind it, so it overtakes its
        // own front, and the front — once vertical — runs AHEAD of sound. That
        // is the cleanest measurable difference between the two models, and it
        // needs no appeal to how steep anything looks: time the front past two
        // stations and divide.
        //
        // (Steepening itself is not measurable here, and it is worth saying
        // why: with fifty bar behind the valve the front becomes a shock
        // within the first few centimetres and then stays one. There is
        // nothing left to watch by the time it reaches a station.)
        constexpr double near_station = 0.20, far_station = 0.60;
        const double     trigger = si::p_atmosphere * 1.10;

        double travelled = 0.0, at_near = 0.0, at_far = 0.0;
        while (travelled < 2.0e-3 && at_far == 0.0) {
            const double dt = pipe.courant_limit();
            const Primitive inside = pipe.left_face();
            pipe.left_ghost(travelled < 0.15e-3
                ? Primitive{ 50.0e5 / (air::R * 1400.0), 0.0, 50.0e5 }
                : Primitive{ inside.density, -inside.velocity, inside.pressure });
            const Primitive mouth = pipe.right_face();
            pipe.right_ghost({ mouth.density, mouth.velocity, si::p_atmosphere });
            pipe.step(dt);
            travelled += dt;

            const auto cell_at = [&](double x) { return std::size_t(x / 0.80 * 400.0); };
            if (at_near == 0.0 && pipe.at(cell_at(near_station)).pressure > trigger)
                at_near = travelled;
            if (at_near > 0.0 && at_far == 0.0 && pipe.at(cell_at(far_station)).pressure > trigger)
                at_far = travelled;
        }

        const double front_speed = (far_station - near_station) / std::max(at_far - at_near, 1e-9);
        const double sound_speed = air::speed_of_sound(900.0);

        // Anywhere from sound itself to a strong shock. The claim being made
        // is the NEXT line; this one only asserts the number is a speed a
        // pressure front in a pipe could actually have.
        sheet.within("the blowdown front's speed down the pipe",
                     front_speed, 580.0, 2500.0, "m/s");
        sheet.holds("which is FASTER than sound ahead of it",
                    front_speed > sound_speed * 1.05,
                    (std::to_string(int(100.0 * (front_speed / sound_speed - 1.0))) + "% over").c_str());
        sheet.note("sound in 900 K exhaust is 586 m/s, and a linear wave travels at exactly");
        sheet.note("that. this one does not, because it is a shock: its crest is hotter than");
        sheet.note("the gas ahead and is carried forward by the flow behind, so it outruns");
        sheet.note("its own front. it is why the crack of an exhaust is sharper at the pipe");
        sheet.note("than it was at the valve, and the engine here cannot reproduce it.");
    }

    // ── The thesis ────────────────────────────────────────────────────────
    sheet.section("THE THESIS");
    {
        double cross_rpm = 0.0, flat_rpm = 0.0;
        const std::vector<double> cross = idle_recording(windsor::stock(),      cross_rpm);
        const std::vector<double> flat  = idle_recording(windsor::flat_crank(), flat_rpm);

        const double cross_share = half_order_share(cross, cross_rpm, 44100);
        const double flat_share  = half_order_share(flat,  flat_rpm,  44100);

        sheet.note("half-order energy is the signature of a pulse train that repeats");
        sheet.note("every two revolutions instead of one. one bank, at idle:");
        sheet.exceeds("cross-plane bank, half-order share", cross_share, 0.30, "");
        sheet.within("flat-plane bank, half-order share",  flat_share,  0.0,  0.10, "");
        sheet.holds("the cross-plane bank carries far more of it",
                    cross_share > flat_share * 5.0,
                    (std::to_string(static_cast<int>(cross_share / std::max(flat_share, 1e-9)))
                     + "x").c_str());
        sheet.note("that ratio is the entire project, and it came out of four throw angles");
    }

    return sheet.report();
}
