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
// The first is thermodynamic: it is the far side of the exhaust valve, the
// `Boundary` a cylinder pushes against, and it has a pressure — backpressure —
// that the engine must work to overcome and that determines how much burnt gas
// is left in the chamber to dilute the next charge. That is a plenum with a
// mass and a temperature, and it is boring and important.
//
// The second is acoustic, and is the reason anyone cares about V8s.
//
// ── Why an exhaust makes a sound at all ───────────────────────────────────
//
// The valve cracks open with sixty atmospheres behind it. The flow chokes
// instantly — sonic at the seat, and no downstream condition can make it flow
// faster — and a near-discontinuity in pressure sets off down the primary pipe
// at the local speed of sound, which in 900 K exhaust gas is about 590 m/s,
// nearly twice what it is in the air outside. That step is blowdown, and it is
// the leading edge of every pulse. What follows it, as the piston comes up the
// bore and pushes the rest out, is comparatively gentle and contributes the
// low end.
//
// So an exhaust note is a train of sharp pulses. Its pitch is the rate the
// pulses arrive. Its timbre is what the pipes did to them on the way out.
//
// ── The waveguide ─────────────────────────────────────────────────────────
//
// Pipes are modelled as digital waveguides: a pressure wave travelling one way
// is a delay line, and a pipe is two of them, one in each direction. This is
// not an analogy or a filter that sounds a bit like a pipe. It is the exact
// solution to the one-dimensional wave equation, sampled — d'Alembert's 1747
// result, that any solution is the sum of a left-mover and a right-mover, with
// the sampling done at 44.1 kHz instead of in the continuum.
//
// Where pipes meet, waves scatter. At a junction of N pipes, continuity of
// pressure and conservation of volume flow give exactly one answer:
//
//      p_junction = 2·Σ(Yᵢ·pᵢ⁺) / Σ(Yᵢ)        Yᵢ = Aᵢ / (ρc), the admittance
//      pᵢ⁻ = p_junction − pᵢ⁺
//
// and that single expression contains every tuning effect a header has. A wave
// arriving from a small primary into a large collector sees more area than it
// came from, and the reflection comes back INVERTED — a rarefaction, running
// back up the pipe toward the cylinder it came from. Time that returning
// suction to arrive at the exhaust valve during overlap and it will reach into
// the chamber and pull the residuals out and the next charge in. That is
// scavenging; it is worth real power over a narrow band; and it is the entire
// reason a set of headers has a length, and why that length is chosen for an
// rpm rather than for the engine.
//
// At the open end of the tailpipe the wave meets infinite area and reflects
// almost perfectly inverted, which is why an exhaust pipe has a resonant note
// of its own — an organ pipe, stopped at one end by a cylinder head.
//
// ── The loop, closed ──────────────────────────────────────────────────────
//
// The waveguide does not merely listen to the exhaust ports. It is what they
// push against.
//
// Each cylinder's exhaust `Boundary` is the pressure at the closed end of its
// own primary pipe — the slowly-varying mean backpressure of the collector,
// plus whatever wave happens to be standing at that valve at that instant. So
// a cylinder blowing down sends a wave away down its pipe, the junction sends
// part of it back inverted, and when it arrives the cylinder that is by then
// on its overlap feels it, and breathes better or worse for it.
//
// This costs more than it looks. The cylinder solution and the gas dynamics
// have to be integrated together, the waveguide has to run fast enough that
// the crank cannot step over it at 6000 rpm, and a mass flow has to be
// converted into a travelling wave with the right magnitude rather than an
// arbitrary one:
//
//      p⁺ = ρ·c·u = ρ·c·(ṁ / ρA) = c·ṁ / A
//
// — where the density cancels, which is the small piece of luck that makes
// this tractable.
//
// ── The clamp, and what it costs ──────────────────────────────────────────
//
// One limit has to be respected or the whole thing detonates. A plane wave
// cannot carry particle velocity faster than sound — past Mach 1 what is
// leaving the valve is a jet, not a wave, and its surplus momentum is spent
// stirring the pipe rather than travelling down it. A linear waveguide has no
// way to become a shock, so left unbounded it will happily report thirty-five
// bar in a header that has never seen three, the cylinder will find it cannot
// exhale against its own exhaust, and the engine will make negative power with
// complete confidence. It did exactly that, once.
//
// So u is clamped at the local speed of sound, capping the source at ρc² = γp̄
// — about 1.8 bar over a mean of 1.3, which is what a primary pipe actually
// sees at blowdown.
//
// That clamp is the honest boundary of a linear model, and `riemann.hpp` is
// what it costs, measured rather than asserted. Run `windsor verify`: a real
// blowdown front, solved without any clamp at all, travels down a primary at
// 1409 m/s — Mach 2.4, against 586 m/s for sound in the gas ahead of it. It is
// a shock. It outruns its own sound because its crest is hotter than the gas
// in front and is carried forward by the flow behind, and that is why the
// crack of an exhaust is sharper at the tailpipe than it was at the valve.
//
// Nothing in this file can produce that. Two delay lines propagate at exactly
// one speed, forever, which is what makes them exact for a linear wave and
// wrong for this one.
//
// ── And why it is still the model that ships ──────────────────────────────
//
// The nonlinear solver was built, verified against Sod's shock tube to one
// part in a hundred thousand, and wired into this engine in place of the delay
// lines. It worked. Pipe pressures came out physically correct without a clamp
// anywhere, and header length finally became worth something real — 14% of
// torque across a length scan, peaking at 1.2 m at 4500 rpm where the old
// builder's rule of thumb predicts 1.10 m, against 1.8% and no peak at all
// from the delay lines.
//
// It was reverted anyway, for two reasons and a lesson.
//
// It cost seven times the runtime. And it destroyed the one measurement this
// entire project exists to make: the flat-plane bank's half-order share, which
// ought to be nearly nothing, went from 0.02 to 3.8, and the ratio between the
// two crankshafts collapsed to 1 — from 118 as the model stood that day, and
// from 26 as it stands now that the radiation is modelled properly, but the
// number that matters is the 1. Somewhere in the coupled pipes the
// scheme manufactures cycle-to-cycle variation that an evenly-firing bank does
// not have, and an engine that cannot tell the two crankshafts apart is of no
// use here however good its shocks are.
//
// The lesson is the interesting part. A delay line has NO numerical
// dissipation — it is the exact solution to the linear problem, not an
// approximation to it. A finite-volume scheme, however carefully limited, is
// diffusive everywhere. For a problem that is mostly linear propagation with
// occasional violence, the cruder-looking model is the more faithful one over
// most of the cycle, and the first-order version of the "better" solver was
// measurably WORSE at header tuning than the delay lines it replaced.
//
// That is not an argument against ever doing it properly. It is a record of
// what was tried, what it bought, and what it broke.

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

    // A pipe is not a perfect conductor of sound. Against the wall there is a
    // boundary layer a fraction of a millimetre thick where the gas is held
    // still by viscosity and held at the wall's temperature by conduction, and
    // every pass down the pipe leaves some of the wave in it. The loss grows
    // as the square root of frequency, so a pipe is a lowpass filter made of
    // steel — which is why a long system sounds darker than a short one, why a
    // megaphone is bright and a two-chamber muffler is not, and why standing
    // behind open headers is unpleasant in a way that standing behind a car is
    // not.
    //
    // Without this the model delivers the sharp edge of every blowdown to the
    // tailpipe undiminished, the radiation shelf above passes all of it, and
    // the result is 12 dB of hiss at 1.8 kHz standing over the firing
    // frequency. It sounded, accurately, like a hairdryer.
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
        // at the atmosphere, which is to say the engine has been running open
        // headers, which is exactly why it sounded like it. An open-header V8
        // is genuinely unpleasant to stand behind — that is not a modelling
        // artefact, it is the reason mufflers were invented and the reason
        // they are required by law.
        //
        // A real one is a reactive device: a box with baffles and perforated
        // tubes and one or two reversals, which works by area changes rather
        // than absorption, and whose transmission loss is a comb of peaks and
        // troughs determined by chamber lengths. That geometry is not modelled
        // here. What is modelled is its effect — three poles of transmission
        // loss above a corner — and this is the one place in the project where
        // a component is represented by its behaviour instead of its shape.
        // It is marked as such rather than dressed up.
        //
        // The corner matters more than it looks because the ear does. A
        // whistle sitting 35 dB below the firing frequency sounds no quieter
        // than the firing frequency does, because human hearing is roughly
        // that much more sensitive at 5 kHz than at 46 Hz. Loudness is not
        // energy, and a model that is right about energy can still be
        // unlistenable.
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
            // Smoothed, and this is the last artefact of two clocks meeting.
            //
            // The port flows arrive once per crank step — 16.7 kHz at idle —
            // and are ramped between arrivals, which sounds like enough and is
            // not, because the radiation model downstream DIFFERENTIATES. The
            // derivative of a piecewise-linear ramp is a staircase, so the
            // interpolation is undone on the way out and the crank's step rate
            // is printed straight into the audio band, where it aliased and
            // put nearly a third of the recording's energy at 16 kHz.
            //
            // A real exhaust valve event lasts about ten milliseconds and its
            // fastest genuine feature, blowdown, takes a millisecond or so.
            // There is nothing physical in a port flow above about three
            // kilohertz, so what is above three kilohertz is the solver
            // talking about itself, and it is removed here.
            const double injected = source_[i](mean_density_ * sound_speed_ * velocity);

            // While the valve is open the end of the pipe is not closed — it
            // is coupled to half a litre of cylinder, and a wave arriving
            // there is partly swallowed instead of bounced. Only once the
            // valve is on its seat is this a closed end.
            //
            // It has to be a continuous function of the lift, and getting that
            // wrong is audible. The first version of this switched between the
            // two values the instant any flow appeared — a jump from 0.96 to
            // 0.45 in one sample, sixteen times a cycle, inside a resonant
            // delay loop. A step discontinuity in a loop gain is a click, a
            // click is broadband, and those clicks were putting over half the
            // recording's energy above 8 kHz. A valve does not snap open; it
            // has a lift curve, and the pipe's termination follows it.
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
        // An open pipe radiates the RATE OF CHANGE of the volume flow leaving
        // it, not the flow itself — a pipe blowing steadily makes no sound at
        // all. So the microphone begins as a differentiator, and that is why
        // an exhaust note is all edge.
        //
        // But only up to a point, and missing the point is what made the first
        // version of this file sound like a hairdryer. A differentiator rises
        // at six decibels per octave FOREVER, and an engine whose output has
        // been differentiated with nothing to stop it puts three quarters of
        // its energy above 1.4 kHz, where a real V8 at idle has almost none.
        //
        // The physics that stops it: a pipe mouth only radiates like a point
        // source while it is acoustically small compared to the wavelength —
        // ka ≪ 1. Once the wavelength is down to the size of the pipe, the
        // mouth is no longer a point, radiation efficiency stops climbing, and
        // the response goes flat. The corner sits at
        //
        //      f = c / 2πa
        //
        // which for a two-and-a-half inch tailpipe, in ambient air rather than
        // in the hot gas inside, is about 1.7 kHz. Above that the pipe is
        // simply not getting any better at being a loudspeaker.
        const double leaving  = (1.0 + open_end_reflection) * at_mouth;
        const double radiated = radiation_(leaving - previous_mouth_);
        previous_mouth_ = leaving;

        // ── Down to the file's rate ───────────────────────────────────────
        //
        // The waveguide runs at four times the sample rate of the file, and
        // everything above half the FILE's rate has to be gone before the
        // rates are allowed to meet. Anything left folds: it does not vanish,
        // it reappears at a frequency it never had, and it sounds like a
        // whistle because that is exactly what it is.
        //
        // There was one. The crank steps 16.5 thousand times a second at idle,
        // that rate is stamped on the port flows, and its third harmonic at
        // 49 kHz folded straight down to 5.3 kHz and sat there five decibels
        // under the firing frequency, which is audible and awful.
        //
        // The boxcar average that used to be the whole of the filtering here
        // is a very poor lowpass — barely four decibels down at the frequency
        // it most needs to stop. Three poles at ten kilohertz put the folding
        // band forty decibels further down, which is enough, and they cost
        // nothing that anyone wanted to hear: there is no exhaust note above
        // ten kilohertz.
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
