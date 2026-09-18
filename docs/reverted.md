# Four things that were built and taken out

The two largest pieces of work in this project are not in it. Both were
finished, measured, and removed because the measurement said so.

That is the point of writing them down. A limitation nobody attempted is a
guess; a limitation somebody built, measured and reverted is a result.

## The nonlinear gas solver

`riemann.hpp` — still in the tree, still verified, no longer wired to the
engine.

A second-order MUSCL-Hancock finite-volume Euler solver with an HLL flux.
Verified against Sod's shock tube to **one part in 10⁵**, conserving mass and
energy to **1.5 × 10⁻¹⁴**. Built to answer the one limitation `exhaust.hpp` had
always admitted: its delay lines are linear, and a blowdown is not, so the
source has to be clamped at Mach 1.

Wired in place of the delay lines it worked. No clamp anywhere. Pipe pressures
physically correct. And header length finally became worth something —

|                     | with delay lines | with the solver                                 |
| ------------------- | ---------------- | ----------------------------------------------- |
| header length worth | 1.8%, no peak    | **14% of torque**, peaking at 1.2 m at 4500 rpm |

where the old builder's rule `L = 850·ED/rpm − 3` predicts 1.10 m.

**It was reverted.** It cost seven times the runtime, and it collapsed the one
measurement the project exists to make: the ratio between the two crankshafts
went from 118× to **1×**. An engine that cannot tell the two cranks apart is of
no use here however good its shocks are.

The lesson was worth the detour. A delay line has **no** numerical dissipation —
it is the exact solution to the linear problem, not an approximation to it —
while any finite-volume scheme is diffusive everywhere. For a problem that is
mostly linear propagation with occasional violence, the cruder-looking model is
the more faithful one.

What survives is a measurement. `./windsor verify` sends a real blowdown front
down a primary with no clamp and clocks it at **1409 m/s** — Mach 2.4, against
586 for sound in the gas ahead. It is a shock. The shipped model cannot make
one, and now says so with a number instead of an apology.

## Intake runner tuning, three times

Each attempt left the next one better posed, which is the most a failure can do.

**As a waveguide**, each runner ending at a fixed reflection of its own. It
tuned correctly and put peak torque within **90 rpm** of Ford's. It could not be
held below 2000 rpm at any damping: torque swung between 137 and 465 N·m in a
limit cycle seventeen cycles long.

**As an inertance and a port compliance** — a Helmholtz resonator, which is what
a runner is. Written the obvious way it is an algebraic loop, because the flow
being differentiated is the one the cylinder computes *from* the pressure being
solved for; it produced NaN in under a second. Written properly, with the runner
flow as a state, it was perfectly stable — 0% cycle-to-cycle variation at 1500
and 4000 rpm — and it moved the torque peak the **wrong way**.

**As eight runners meeting at one junction**, which was the hypothesis the
second attempt ended on: they had each been ending at a private wall, as though
the other seven were not there. They were given the scattering node
`exhaust.hpp` already uses for its collector, widened from five ports to nine.

It found two things.

*The junction was not the blocker.* Sharing redistributes energy between
runners; it does not dissipate any. Eight low-loss pipes passing a wave around
still have it afterwards.

*Length mattered more.* At 0.30 m the ram gain came out at **34%** where a stock
log manifold is worth a few percent — and that is what made the engine
unholdable, for a reason worth stating plainly:

> A torque-controlled brake sitting on a **rising** torque curve is an unstable
> equilibrium. Speed up a little, make more torque, speed up more. Ram tuning is
> precisely the thing that makes a torque curve locally rise, so the dyno slid
> into the resonance instead of holding short of it.

That is real engine-and-dynamometer physics, not a defect in either, and it is
why modern cells are speed-controlled. Shortened to 0.18 m the gain falls to
**4%** and the curve comes out smooth.

And it still could not be held between 1000 and 1500 rpm. Not at any controller
gain from 0.6 to 12 N·m per rpm, not with a fresh engine per point, not with the
load lagged.

**I do not know why.** It is written at the bottom of `induction.hpp` as the
next thing to find out, because saying so is better than the explanation one
would have to invent in order not to.
