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
// now have to be integrated together, the waveguide has to run fast enough
// that the crank cannot step over it at 6000 rpm, and a mass flow has to be
// converted into a travelling wave with the right magnitude rather than an
// arbitrary one:
//
//      p⁺ = ρ·c·u,        u = ṁ / ρA
//
// with one limit that has to be respected or the whole thing detonates. A
// plane wave cannot carry particle velocity faster than sound — past Mach 1
// what is leaving the valve is a jet, not a wave, and its surplus momentum is
// spent stirring the pipe rather than travelling down it. A linear waveguide
// has no way to become a shock, so left unbounded it will happily report
// thirty-five bar in a header that has never seen three, the cylinder will
// find it cannot exhale against its own exhaust, and the engine will make
// negative power with complete confidence. It did exactly that, once.
//
// So u is clamped at the local speed of sound, which caps the source at
//
//      p⁺max = ρc² = γ·p̄
//
// — about 1.8 bar over a mean of 1.3, which is what a primary pipe actually
// sees at blowdown. The clamp is the honest boundary of a linear model, not a
// tuning constant, and the real fix is a nonlinear method-of-characteristics
// solver, which is what proper engine gas-dynamics codes are.
//
// What it buys is that header length now changes the torque curve and not
// only the note, that the pulses leaving one bank are no longer all identical,
// and that the difference between the two crankshafts survives being summed
// with the other bank. It did not, before.

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

// A travelling wave, stored as the samples it has not yet arrived as.
class DelayLine {
public:
    explicit DelayLine(std::size_t samples)
        : buffer_(std::max<std::size_t>(samples, 1), 0.0) {}

    double read()  const { return buffer_[index_]; }
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
    void receive(int seat, double mass_flow) {
        port_flow_[seat] = mass_flow;
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
        audio_debt_ += dt * rate_;
        while (audio_debt_ >= 1.0) {
            tick();
            audio_debt_ -= 1.0;
        }
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
                port_flow_[i] / (mean_density_ * primary_area_),
                -sound_speed_, sound_speed_);
            const double injected = mean_density_ * sound_speed_ * velocity;

            // While the valve is open the end of the pipe is not closed — it
            // is coupled to half a litre of cylinder, and a wave arriving
            // there is partly swallowed instead of bounced. Only once the
            // valve is on its seat is this a closed end.
            const double reflection = (port_flow_[i] != 0.0)
                                    ? open_valve_reflection : closed_end_reflection;
            const double outgoing = returning * reflection + injected;

            primary_out_[i].write(outgoing);
            valve_pressure_[i] = outgoing + returning;   // what the valve feels
        }

        // ── The open end of the tailpipe ──────────────────────────────────
        // Infinite area ahead: the wave inverts and comes back. Not quite
        // perfectly — some of it escapes as sound, and that escaping fraction
        // is the entire output of this file.
        const double at_mouth = collector_out_->read();
        collector_back_->write(-open_end_reflection * lowpass(at_mouth));

        // An open pipe radiates the RATE OF CHANGE of the volume flow leaving
        // it, not the flow itself — a pipe blowing steadily makes no sound at
        // all. So the microphone is a differentiator, and that is why an
        // exhaust note is all edge.
        const double leaving  = (1.0 + open_end_reflection) * at_mouth;
        const double radiated = leaving - previous_mouth_;
        previous_mouth_ = leaving;

        // Decimate back to the file's rate. A boxcar average over the
        // oversampled run is a crude anti-alias filter and an honest one: it
        // is exactly what a microphone diaphragm too heavy to follow the top
        // octave would do.
        decimator_ += radiated;
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

    // Pipes are not lossless and neither is the air at the mouth. High
    // frequencies are absorbed on every pass, which is why a long system
    // sounds darker than a short one and why an open header is so bright it
    // is unpleasant.
    double lowpass(double x) {
        lowpass_state_ += 0.42 * (x - lowpass_state_);
        return lowpass_state_;
    }

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
    std::array<double, 4> port_flow_{};
    std::array<double, 4> valve_pressure_{};
    double decimator_ = 0.0;
    int    decimation_count_ = 0;
    double mass_ = 0.0;
    double audio_debt_ = 0.0;
    double lowpass_state_ = 0.0, previous_mouth_ = 0.0;
    std::vector<double> samples_;
};

} // namespace engine
