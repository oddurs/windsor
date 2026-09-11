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
// ── The one-way street ────────────────────────────────────────────────────
//
// A real admission. The waveguide LISTENS to the exhaust ports; it does not
// push back on them. The cylinders see the collector plenum's slowly-varying
// backpressure, not the sharp returning waves the waveguide computes. So this
// model will reproduce the sound of scavenging faithfully and will not credit
// the engine with the power that scavenging makes, which means header length
// changes the note here and does not change the torque curve.
//
// Closing that loop means running the gas dynamics and the cylinder solution
// together at the waveguide's timestep, and it is the single largest honest
// improvement available to this project. It is not done, and this comment is
// the place where that is admitted rather than quietly glossed.

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

    explicit Exhaust(Setup s) : s_{s} {
        const double c = air::speed_of_sound(s_.gas_temperature);

        const auto samples = [&](double length) {
            return static_cast<std::size_t>(std::round(length / c * s_.sample_rate));
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

    // Advance the whole system by dt seconds of real time.
    void advance(double dt) {
        double total = 0.0;
        for (double f : port_flow_) total += f;

        // Plenum: filled by the ports, emptied through whatever the rest of
        // the system leaves open. A straight pipe and a glasspack breathe;
        // a restrictive muffler is how a manufacturer buys quiet with power.
        const double p = air::pressure(mass_, s_.gas_temperature, s_.plenum_volume);
        const double out = port::mass_flow(s_.outlet_area,
                                           p, s_.gas_temperature,
                                           si::p_atmosphere, si::T_standard);
        mass_ = std::max(mass_ + (total - std::max(out, 0.0)) * dt, 1e-9);

        // Acoustic: run whole audio samples, carrying the remainder forward so
        // the sound clock never drifts against the crank.
        audio_debt_ += dt * s_.sample_rate;
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
            primary_out_[i].write(returning * closed_end_reflection + port_flow_[i]);
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
        samples_.push_back(leaving - previous_mouth_);
        previous_mouth_ = leaving;

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
    static constexpr double open_end_reflection   = 0.72;

    Setup s_;
    std::vector<DelayLine> primary_out_, primary_back_;
    std::unique_ptr<DelayLine> collector_out_, collector_back_;

    double admittance_primary_ = 0.0, admittance_collector_ = 0.0;
    std::array<double, 4> port_flow_{};
    double mass_ = 0.0;
    double audio_debt_ = 0.0;
    double lowpass_state_ = 0.0, previous_mouth_ = 0.0;
    std::vector<double> samples_;
};

} // namespace engine
