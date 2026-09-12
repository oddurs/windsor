// exhaust.hpp — four pipes, a collector, and the noise.
//
// This is where the project cashes in. Everything upstream has been arranging
// for eight cylinders to open their exhaust valves at particular moments. This
// file is what a bystander hears when they do.
//
// ── Two jobs, kept apart ──────────────────────────────────────────────────
//
// The exhaust system does two unrelated things and this class does both
// without letting them touch.
//
// The first is thermodynamic. It is the far side of the exhaust valve, the
// `Boundary` a cylinder pushes against, and it has a backpressure the engine
// must work to overcome. That is a plenum with a mass and a temperature, and
// it is dull and important.
//
// The second is acoustic, and is the reason anyone cares about V8s.
//
// ── Why an exhaust makes a sound ──────────────────────────────────────────
//
// The valve cracks open with sixty atmospheres behind it. The flow chokes
// instantly — sonic at the seat, and no downstream condition can make it go
// faster — and a near-discontinuity in pressure sets off down the primary at
// the local speed of sound, which in 900 K exhaust gas is about 590 m/s,
// nearly twice what it is in the air outside. That is blowdown, and it is the
// leading edge of every pulse. What follows, as the piston pushes the rest
// out, is gentle by comparison and contributes the low end.
//
// So an exhaust note is a train of sharp pulses. Its pitch is the rate they
// arrive at. Its timbre is what the pipes did to them on the way out.
//
// ── The waveguide ─────────────────────────────────────────────────────────
//
// A pipe is two delay lines, one per direction. This is not a filter that
// sounds a bit like a pipe; it is the exact solution to the one-dimensional
// wave equation, sampled — d'Alembert's 1747 result that any solution is a
// left-mover plus a right-mover, taken at 176 kHz instead of in the continuum.
//
// Where pipes meet, waves scatter. At a junction of N pipes, continuity of
// pressure and conservation of volume flow give exactly one answer:
//
//      p_junction = 2·Σ(Yᵢ·pᵢ⁺) / Σ(Yᵢ)        Yᵢ = Aᵢ / (ρc), the admittance
//      pᵢ⁻ = p_junction − pᵢ⁺
//
// and that expression contains every tuning effect a header has. A wave
// arriving from a narrow primary into a wide collector finds more room than it
// came from, and the reflection it sends home is INVERTED — a rarefaction,
// running back toward the cylinder that made it. Time that to arrive during
// overlap and it reaches into the chamber, pulls the residuals out and the
// next charge in. That is scavenging. It is why a set of headers has a length,
// and why the length is chosen for an engine speed rather than for an engine.
//
// At the open end the wave meets infinite area and reflects almost perfectly
// inverted, which is why a tailpipe has a note of its own: an organ pipe,
// stopped at one end by a cylinder head.
//
// ── The loop is closed ────────────────────────────────────────────────────
//
// The waveguide does not merely listen to the ports. It is what they push
// against: each cylinder's exhaust `Boundary` is the pressure at the closed
// end of its own primary — the collector's mean, plus whatever wave is
// standing at that valve at that instant. A cylinder blowing down sends a wave
// away, the junction sends part of it back inverted, and the cylinder that is
// by then on overlap breathes better or worse for it.
//
// A mass flow becomes a travelling wave of p⁺ = ρcu, where u = ṁ/ρA and the
// density cancels — the small piece of luck that makes this tractable.
//
// ── The clamp, and what it costs ──────────────────────────────────────────
//
// One limit cannot be skipped. A plane wave cannot carry particle velocity
// faster than sound; past Mach 1 what leaves the valve is a jet, not a wave.
// A linear waveguide has no way to become a shock, so left unbounded it
// reports thirty-five bar in a header that has never seen three, the cylinder
// finds it cannot exhale against its own exhaust, and the engine makes
// negative power with complete confidence. So u is clamped at the local sound
// speed, capping the source at ρc² = γp̄ — about 1.8 bar over a mean of 1.3,
// which is what a primary actually sees.
//
// That clamp is the honest boundary of a linear model, and `riemann.hpp` is
// what it costs, measured rather than asserted. `windsor verify` sends a real
// blowdown front down a primary with no clamp at all and clocks it at
// 1409 m/s — Mach 2.4, against 586 for sound in the gas ahead. It is a shock.
// It outruns its own sound because its crest is hotter than the gas in front
// and is carried forward by the flow behind, and that is why the crack of an
// exhaust is sharper at the tailpipe than it was at the valve.
//
// Nothing here can produce that. Two delay lines propagate at one speed
// forever, which is what makes them exact for a linear wave and wrong for
// this one. The nonlinear solver was built, verified against Sod's shock tube
// to one part in a hundred thousand, and wired in here in place of the delay
// lines. It cost seven times the runtime and collapsed the difference between
// the two crankshafts — the measurement this whole project exists to make —
// from 118 to 1. It was reverted. A delay line has no numerical dissipation:
// it is the exact solution to the linear problem, not an approximation to it,
// and for a problem that is mostly linear propagation with occasional violence
// the cruder-looking model is the more faithful one.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <vector>
#include <engine/charge.hpp>
#include <engine/cylinder.hpp>
#include <engine/si.hpp>

namespace engine {

// A one-pole lowpass, specified by the frequency it turns over at rather than
// by a bare coefficient, because a bare coefficient is a number nobody can
// check and a corner frequency is a claim about the world.
class OnePole {
public:
    void corner(double hertz, double rate) {
        a_ = 1.0 - std::exp(-si::two_pi * hertz / rate);
    }
    double operator()(double x) { y_ += a_ * (x - y_); return y_; }
private:
    double a_ = 1.0, y_ = 0.0;
};

// Three poles in series, which is what it takes to throw something away
// properly. See the decimator below for why one is not enough.
class Cascade {
public:
    void corner(double hertz, double rate) {
        for (OnePole& p : pole_) p.corner(hertz, rate);
    }
    double operator()(double x) {
        for (OnePole& p : pole_) x = p(x);
        return x;
    }
private:
    std::array<OnePole, 3> pole_{};
};

// A travelling wave, stored as the samples it has not yet arrived as.
class DelayLine {
public:
    explicit DelayLine(std::size_t samples)
        : buffer_(std::max<std::size_t>(samples, 1), 0.0) {}

    // Against the wall is a boundary layer a fraction of a millimetre thick,
    // where viscosity holds the gas still and conduction holds it at the
    // wall's temperature, and every pass leaves some of the wave in it. The
    // loss grows as the square root of frequency, so a pipe is a lowpass
    // filter made of steel. It is why a long system sounds darker than a short
    // one, and why standing behind open headers is unpleasant in a way that
    // standing behind a car is not.
    void absorbs_above(double hertz, double rate) { loss_.corner(hertz, rate); }

    double read() { return loss_(buffer_[index_]); }
    void   write(double v) { pending_ = v; }
    void   advance() {
        buffer_[index_] = pending_;
        index_ = (index_ + 1) % buffer_.size();
    }
    std::size_t length() const { return buffer_.size(); }

private:
    std::vector<double> buffer_;
    std::size_t index_   = 0;
    double      pending_ = 0.0;
    OnePole     loss_;
};

// One bank: four primaries into a collector into a tailpipe.
class Exhaust {
public:
    struct Setup {
        double primary_length;     // m, valve to collector — the tuned length
        double primary_diameter;   // m
        double collector_length;   // m, collector to the open end
        double collector_diameter; // m
        double plenum_volume;      // m³, the thermodynamic collector
        double outlet_area;        // m², the restriction to atmosphere
        double gas_temperature;    // K, what the pipes settle at
        double muffler_cutoff;     // Hz — see below. Zero for open headers.
        double sample_rate;        // Hz
    };

    // The waveguide runs faster than the file it writes. Three reasons, all
    // of them the same reason: at 6000 rpm a quarter-degree of crank is under
    // seven microseconds and the crank must not be able to step over a sound
    // sample; a delay line rounded to the nearest whole sample is a pipe cut
    // to the nearest whole sample, and at 44.1 kHz that is nearly a centimetre
    // of header; and a closed feedback loop wants headroom above the
    // frequencies it cares about. Output is decimated back down.
    static constexpr int oversample = 4;

    explicit Exhaust(Setup s) : s_{s}, rate_{s.sample_rate * oversample} {
        const double c = air::speed_of_sound(s_.gas_temperature);
        sound_speed_ = c;

        const auto samples = [&](double length) {
            return static_cast<std::size_t>(std::round(length / c * rate_));
        };

        for (int i = 0; i < 4; ++i) {
            primary_out_.emplace_back(samples(s_.primary_length));
            primary_back_.emplace_back(samples(s_.primary_length));
        }
        collector_out_  = std::make_unique<DelayLine>(samples(s_.collector_length));
        collector_back_ = std::make_unique<DelayLine>(samples(s_.collector_length));

        const double Ap = si::pi * 0.25 * s_.primary_diameter   * s_.primary_diameter;
        const double Ac = si::pi * 0.25 * s_.collector_diameter * s_.collector_diameter;
        admittance_primary_   = Ap;    // ρc is common to every port and cancels
        admittance_collector_ = Ac;
        primary_area_         = Ap;

        mass_ = air::mass(si::p_atmosphere, s_.gas_temperature, s_.plenum_volume);

        // ── The silencer ──────────────────────────────────────────────────
        //
        // Every version of this file until now vented the collector straight
        // at the atmosphere, which is to say the engine was running open
        // headers, which is exactly why it sounded like it.
        //
        // A real muffler is a reactive device — a box of baffles and reversals
        // working by area change, whose transmission loss is a comb set by its
        // chamber lengths. That geometry is not modelled. Its effect is: three
        // poles of loss above a corner. This is the one place in the project
        // where a part is represented by its behaviour instead of its shape,
        // and it is marked as such rather than dressed up.
        //
        // The corner matters more than it looks, because the ear does. A
        // whistle 35 dB below the firing frequency is not quieter than the
        // firing frequency — hearing is about that much more sensitive at
        // 5 kHz than at 46 Hz. Loudness is not energy, and a model can be
        // right about energy and still be unlistenable.
        if (s_.muffler_cutoff > 0.0) silencer_.corner(s_.muffler_cutoff, rate_);

        // Two corners, both of them claims about the world rather than knobs.
        // The pipe's own losses, and the frequency above which the mouth stops
        // getting better at radiating — c/2πa, in ambient air, not in the hot
        // gas inside the pipe, because the sound has left by then.
        constexpr double ambient_sound = 343.0;

        // The corner a pipe absorbs above, longer pipe darker. These are the
        // two numbers in this file that were set by ear rather than derived,
        // and they are marked as such: the √f boundary-layer law gives the
        // SHAPE, an exhaust system's real attenuation depends on its bends,
        // its welds and its silencer, and none of that is modelled.
        constexpr double absorption_metre_hz = 3200.0;   // CALIBRATED
        for (auto& d : primary_out_)  d.absorbs_above(absorption_metre_hz / s_.primary_length, rate_);
        for (auto& d : primary_back_) d.absorbs_above(absorption_metre_hz / s_.primary_length, rate_);
        collector_out_ ->absorbs_above(absorption_metre_hz / s_.collector_length, rate_);
        collector_back_->absorbs_above(absorption_metre_hz / s_.collector_length, rate_);

        for (OnePole& f : source_) f.corner(3000.0, rate_);
        mouth_.corner(2500.0, rate_);
        antialias_.corner(10000.0, rate_);
        radiation_.corner(ambient_sound / (si::two_pi * 0.5 * s_.collector_diameter), rate_);
    }

    // ── The thermodynamic half ────────────────────────────────────────────

    Boundary boundary() const {
        return { air::pressure(mass_, s_.gas_temperature, s_.plenum_volume),
                 s_.gas_temperature };
    }

    double backpressure() const {
        return air::pressure(mass_, s_.gas_temperature, s_.plenum_volume)
             - si::p_atmosphere;
    }

    // seat      — 0..3, which primary. The ORDER cylinders are assigned to
    //             seats is the header's business and changes nothing acoustic,
    //             since all four primaries are the same length. On a real
    //             engine they are not, quite, and equal-length headers cost
    //             what they cost precisely because making them so is hard.
    // mass_flow — kg/s leaving the cylinder, positive outward.
    void receive(int seat, double mass_flow, double valve_open) {
        port_flow_[seat] = mass_flow;
        port_open_[seat] = valve_open;
    }

    // What the exhaust valve of the cylinder on this seat is actually pushing
    // against: the collector's mean backpressure, plus the wave standing in
    // the pipe. This is the number that makes a header a tuned part rather
    // than a drain.
    Boundary port_boundary(int seat) const {
        const double mean = air::pressure(mass_, s_.gas_temperature, s_.plenum_volume);
        return { std::max(mean + valve_pressure_[seat], 2.0e4), s_.gas_temperature };
    }

    // Advance the whole system by dt seconds of real time.
    void advance(double dt) {
        double total = 0.0;
        for (double f : port_flow_) total += f;

        // Plenum: filled by the ports, emptied through whatever the rest of
        // the system leaves open. A straight pipe and a glasspack breathe;
        // a restrictive muffler is how a manufacturer buys quiet with power.
        const double p = air::pressure(mass_, s_.gas_temperature, s_.plenum_volume);
        mean_pressure_ = p;
        mean_density_  = p / (air::R * s_.gas_temperature);
        const double out = port::mass_flow(s_.outlet_area,
                                           p, s_.gas_temperature,
                                           si::p_atmosphere, si::T_standard);
        mass_ = std::max(mass_ + (total - std::max(out, 0.0)) * dt, 1e-9);

        // Acoustic: run whole audio samples, carrying the remainder forward so
        // the sound clock never drifts against the crank.
        //
        // The port flows are RAMPED across those samples rather than held.
        // They arrive once per crank step and the waveguide runs several times
        // faster than that, so holding them makes a staircase — and at idle
        // that staircase has its steps 17 kHz apart, squarely inside the audio
        // band and aliasing badly. The gas leaving a valve does not move in
        // steps; the steps are an artefact of two clocks meeting, and the
        // interpolation is what removes the artefact rather than the sound.
        audio_debt_ += dt * rate_;
        const int ticks = static_cast<int>(audio_debt_);
        audio_debt_ -= ticks;

        for (int k = 0; k < ticks; ++k) {
            const double across = (k + 1.0) / ticks;
            for (int i = 0; i < 4; ++i) {
                ramped_[i] = previous_flow_[i]
                           + across * (port_flow_[i] - previous_flow_[i]);
                open_[i]   = previous_open_[i]
                           + across * (port_open_[i] - previous_open_[i]);
            }
            tick();
        }
        previous_flow_ = port_flow_;
        previous_open_ = port_open_;
    }

    // Samples produced since the last drain, in arbitrary units; the recorder
    // normalises. The engine does not know these exist.
    const std::vector<double>& samples() const { return samples_; }
    void clear_samples() { samples_.clear(); }

private:
    void tick() {
        // ── Scattering at the collector junction ──────────────────────────
        // Five ports meet: four primaries arriving, and the collector arriving
        // backward off the open end. Pressure is continuous across the joint
        // and volume flow is conserved through it, and those two facts have
        // exactly one solution.
        std::array<double, 4> arriving{};
        double numerator = 0.0;
        for (int i = 0; i < 4; ++i) {
            arriving[i] = primary_out_[i].read();
            numerator  += admittance_primary_ * arriving[i];
        }
        const double from_collector = collector_back_->read();
        numerator += admittance_collector_ * from_collector;

        const double total_admittance = 4.0 * admittance_primary_ + admittance_collector_;
        const double junction = 2.0 * numerator / total_admittance;

        for (int i = 0; i < 4; ++i) primary_back_[i].write(junction - arriving[i]);
        collector_out_->write(junction - from_collector);

        // ── The valve end of each primary ─────────────────────────────────
        // A shut exhaust valve is a closed end: it reflects a pressure wave
        // back with the same sign, near-perfectly, minus a little to wall
        // friction. The port is only open for a third of the cycle, and
        // modelling it as permanently closed is why this model listens but
        // does not push back — see the admission at the top of the file.
        for (int i = 0; i < 4; ++i) {
            const double returning = primary_back_[i].read();

            // A mass flow leaving the valve becomes a travelling wave, up to
            // the point where it stops being one. See the note on the clamp.
            const double velocity = std::clamp(
                ramped_[i] / (mean_density_ * primary_area_),
                -sound_speed_, sound_speed_);
            // Smoothed, and this is where two clocks stop arguing.
            //
            // Port flows arrive once per crank step — 16.7 kHz at idle — and
            // ramping between arrivals is not enough, because the radiation
            // model DIFFERENTIATES, and the derivative of a ramp is a
            // staircase again. The crank's step rate gets printed straight
            // into the audio band.
            //
            // A valve event lasts ten milliseconds and its fastest real
            // feature takes one. Nothing above three kilohertz in a port flow
            // is physical; it is the solver talking about itself.
            const double injected = source_[i](mean_density_ * sound_speed_ * velocity);

            // Open, the end of the pipe is coupled to half a litre of
            // cylinder and swallows part of what arrives; shut, it is a closed
            // end and bounces it. It must follow the lift CONTINUOUSLY. Switch
            // between the two the instant flow appears and you put a step
            // discontinuity in a loop gain sixteen times a cycle, which is a
            // click, and a click is broadband.
            const double reflection = closed_end_reflection
                + open_[i] * (open_valve_reflection - closed_end_reflection);
            const double outgoing = returning * reflection + injected;

            primary_out_[i].write(outgoing);
            valve_pressure_[i] = outgoing + returning;   // what the valve feels
        }

        // ── The open end of the tailpipe ──────────────────────────────────
        // Infinite area ahead: the wave inverts and comes back. Not quite
        // perfectly — some of it escapes as sound, and that escaping fraction
        // is the entire output of this file.
        const double at_mouth = collector_out_->read();
        collector_back_->write(-open_end_reflection * mouth_(at_mouth));

        // ── What actually escapes ─────────────────────────────────────────
        //
        // An open pipe radiates the RATE OF CHANGE of what leaves it, not the
        // amount — a pipe blowing steadily makes no sound at all. So the
        // listener is a differentiator, which is why an exhaust note is all
        // edge. But only up to a point, and a differentiator with nothing to
        // stop it rises at six decibels per octave forever.
        //
        // What stops it is that a mouth only radiates like a point source
        // while it is small compared to the wavelength. Past ka ≈ 1 it is no
        // longer a point, efficiency stops climbing, and the response goes
        // flat. The corner is f = c/2πa: about 1.7 kHz for a two-and-a-half
        // inch tailpipe, in the ambient air outside rather than the hot gas
        // within. Above it the pipe is simply not getting any better at being
        // a loudspeaker.
        const double leaving  = (1.0 + open_end_reflection) * at_mouth;
        const double radiated = radiation_(leaving - previous_mouth_);
        previous_mouth_ = leaving;

        // ── Down to the file's rate ───────────────────────────────────────
        //
        // The waveguide runs four times faster than the file it writes, and
        // everything above half the FILE's rate must be gone before the two
        // rates meet. What is left does not vanish — it folds, reappearing at
        // a frequency it never had, which sounds like a whistle because that
        // is what it is. There was one at 5.3 kHz: the crank steps 16.5
        // thousand times a second at idle, and its third harmonic folded
        // straight down onto it.
        //
        // Three poles at ten kilohertz, where no exhaust note lives anyway.
        // A boxcar average is barely four decibels down at the frequency it
        // most needs to stop.
        decimator_ += antialias_(s_.muffler_cutoff > 0.0 ? silencer_(radiated)
                                                         : radiated);
        if (++decimation_count_ >= oversample) {
            samples_.push_back(decimator_ / oversample);
            decimator_ = 0.0;
            decimation_count_ = 0;
        }

        for (auto& d : primary_out_)  d.advance();
        for (auto& d : primary_back_) d.advance();
        collector_out_->advance();
        collector_back_->advance();
    }

    // Pipes are not lossless and neither is the air at the mouth. A viscous
    // and thermal boundary layer scrubs the high frequencies out on every
    // pass, which is why a long system sounds darker than a short one and why
    // an open header is so bright it is unpleasant to stand behind.

    static constexpr double closed_end_reflection = 0.96;
    static constexpr double open_valve_reflection = 0.45;
    static constexpr double open_end_reflection   = 0.72;

    Setup s_;
    std::vector<DelayLine> primary_out_, primary_back_;
    std::unique_ptr<DelayLine> collector_out_, collector_back_;

    double rate_ = 0.0, sound_speed_ = 0.0, primary_area_ = 1.0;
    double mean_pressure_ = si::p_atmosphere;
    double mean_density_  = 1.0;
    double admittance_primary_ = 0.0, admittance_collector_ = 0.0;
    std::array<double, 4> port_flow_{}, port_open_{};
    std::array<double, 4> previous_flow_{}, previous_open_{};
    std::array<double, 4> ramped_{}, open_{};
    std::array<OnePole, 4> source_{};
    std::array<double, 4> valve_pressure_{};
    OnePole mouth_, radiation_;
    Cascade antialias_, silencer_;
    double decimator_ = 0.0;
    int    decimation_count_ = 0;
    double mass_ = 0.0;
    double audio_debt_ = 0.0;
    double previous_mouth_ = 0.0;
    std::vector<double> samples_;
};

} // namespace engine
