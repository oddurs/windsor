# windsor

A Ford 302, modelled from first principles in C++23, for no reason.

```
make && ./windsor spec
```

No dependencies. No build system. No website. It is an engine, and it runs.

---

## The argument

A cross-plane V8 burbles. A flat-plane V8 shrieks. Everyone knows this and
almost nobody can say why, and the reason is not in the exhaust, the camshaft,
the displacement or the fuel. It is four numbers on a crankshaft.

A 90° V8 fires **evenly on either crank** — a power stroke every 90° of
rotation, eight per cycle, perfectly spaced. At the flywheel the two engines
are identical in rhythm. There is no measurement you can take at the output
shaft that tells them apart.

But an engine does not exhale through its flywheel. It exhales through two
exhaust manifolds, and a manifold is not connected to an engine — it is
connected to *four cylinders*, and it only ever hears those four:

|                                                 | each bank hears           |
| ----------------------------------------------- | ------------------------- |
| **cross-plane** (throws at 0°, 90°, 180°, 270°) | 90° — 180° — 270° — 180°  |
| **flat-plane** (throws at 0°, 180°, 0°, 180°)   | 180° — 180° — 180° — 180° |

That is the entire difference. A cross-plane bank coughs twice in quick
succession, waits three quarters of a turn, and coughs again — a limping,
syncopated pulse train, beating against the other bank's, never resolving.
That is the sound of a Mustang idling outside a diner. A flat-plane bank
exhales in perfect thirds of a revolution, both banks lock in phase, and the
harmonics stack cleanly at twice the frequency. That is the sound of a 458
leaving a tunnel.

So: if the model is honest, changing **one part** should change the sound and
nothing else.

```
./windsor record          # the factory cross-plane crank
./windsor record --flat   # a billet flat crank, same block, same everything
```

---

## It refuses to let you type in a firing order

A firing order is not a design input. It is a *result*. You forge a crank, you
hang eight rods on four journals, you grind a cam that decides which cylinder
on each pin fires on the first revolution and which on the second — and the
firing order is whatever falls out.

So `windsor.hpp` specifies the forging, and nothing else:

```cpp
Crankshaft{
    90.0_deg,
    { 45.0_deg, 315.0_deg, 135.0_deg, 225.0_deg },
    {{  // cylinder      journal  bank          revolution
        /* 1 */         { 0, Bank::right, 0 },
        /* 2 */         { 1, Bank::right, 0 },
        /* 3 */         { 2, Bank::right, 1 },
        ...
```

and `./windsor spec` reports what comes out of it:

```
  ---- everything above was specified. everything below came out. ----

CRANKSHAFT
  forging              cross-plane
  firing order         1-5-4-2-6-3-7-8
  the engine fires     every 90 deg, eight times per cycle

WHAT EACH BANK HEARS
  right bank   cyl 1 4 2 3   fires at   180 -  90 - 180 - 270
  left  bank   cyl 5 6 7 8   fires at   270 - 180 -  90 - 180

  LOPSIDED. Each bank coughs twice in quick succession, waits three
  quarters of a turn, and coughs again.
  This is the burble.
```

`1-5-4-2-6-3-7-8` is what is cast into a 1968 302 intake manifold. It was
never typed in.

Pass `--ho` and you get the 1982 5.0 H.O. camshaft on the **identical
forging** — which is exactly what Ford did — and the order becomes
`1-3-7-2-6-5-4-8`, the 351W order. Same crank, different cam, different order,
*identical sound*, because the sound was never in the cam.

---

## Does it work?

```
./windsor verify
```

Fifty-five checks, in a couple of seconds, every one of them against something
outside the project — a derivative against finite differences, a burn rate
against its own integral, a cylinder head against a flow bench, a firing order
against the casting, and the thesis against a Fourier transform.

Nothing in the model was fitted to any of it. A selection:

**The cylinder head, against a flow bench.** The port model is curtain area, a
discharge coefficient and a throat cap. The sag of that coefficient past its
peak is the one number in it fitted to anything, and it is fitted to this:

| lift                      | model   | published, stock C8OE castings |
| ------------------------- | ------- | ------------------------------ |
| 0.375 in (the cam's peak) | 146 cfm | ~150 cfm at 0.400 in           |

**The engine, against Ford.** `./windsor dyno`:

|             | model            | Ford, 1968 302-2V (gross) |
| ----------- | ---------------- | ------------------------- |
| peak torque | 320 lb-ft @ 2000 | 300 lb-ft @ 2600          |
| peak power  | 237 hp @ 5000    | 210 hp @ 4600             |

Within four percent on torque and two on power, with no fitting anywhere —
though both peaks still sit a few hundred rpm high, which is the intake runner
tuning that is still missing. It was ten
percent optimistic with the power peak a thousand rpm too high until the
carburettor's venturis went in — a two-barrel engine breathes through a hole
that cannot be opened, and that hole is most of why the 2V made its power at
4400 where the otherwise identical 4V made more of it at 4800.

**The indicator card.** At 3000 rpm, wide open: peak pressure 55 bar at 13°
after top dead centre, 50% mass burned at 8° ATDC, 2945 K. Which is what a
pressure trace off a 1968 wedge chamber looks like.

**The pumping loss, drawn.** `./windsor card` puts Watt's indicator diagram on
the terminal — the same cylinder at the same speed, wide open and throttled,
to the same scale. Throttled, the intake stroke sinks to 0.2 bar while the
exhaust stroke sits at 1.5, and the anticlockwise loop that opens between them
is work the engine spends on breathing:

| at 2000 rpm | gross     | pumping       | net       |
| ----------- | --------- | ------------- | --------- |
| wide open   | 12.39 bar | −0.03 bar     | 12.36 bar |
| throttled   | 1.20 bar  | **−0.93 bar** | 0.27 bar  |

Seventy-eight percent of everything it makes, spent on suffocating itself. That
is the price of controlling a petrol engine with a plate across its throat,
and it is most of why a diesel is more efficient at part load and barely more
efficient at full.

**The fuel, which is what stops the engine.** The Livengood–Wu integral over
Douaud–Eyzat ignition delay: the end gas spends a fraction `dt/τ` of its
patience each instant, and detonates when the account reaches 1. It reports an
index rather than a verdict — the threshold of 1 belongs to the CFR engine the
correlation was fitted on, and this model runs about five times pessimistic on
a Ford wedge, for reasons `knock.hpp` names rather than divides out. What it
gets right are the comparisons, which is what compression ratio and spark
advance are actually chosen by: knock worsens at low rpm, with advance, with
compression, with cheaper fuel, and eases on a rich mixture.

Which makes three numbers that were arbitrary into one decision that has to
agree with itself: 9.5:1, 34° of total advance, and what was in the tank in 1968.

**The balance, which is why the crank exists.** The same rod table, asked a
different question — `balance.hpp` resolves each piston's inertia force along
its own bore axis and Fourier-transforms the sum over a revolution:

| at 3000 rpm         | cross-plane                       | flat-plane                    |
| ------------------- | --------------------------------- | ----------------------------- |
| primary force       | 0 N                               | 0 N                           |
| **secondary force** | **0 N**                           | **5000 N**                    |
| what it traces      | a circle — counterweights take it | a line — nothing can touch it |

Half a tonne, a hundred times a second, and no counterweight on a shaft
turning at ω can oppose a force that goes at 2ω. The flat crank sounds better
and shakes. That is the bill for the noise, and it is why almost nobody pays it.

**The recording itself.** `./windsor record` writes stereo, because a V8 does
not have an exhaust — it has two, one per bank, down opposite sides of the car,
permanently out of step with each other on a cross-plane crank. There are two
microphones 0.6 m apart, 1.5 m behind two tailpipes 1.0 m apart, and each one
hears both pipes: quieter by the extra distance, later by the time sound takes
to cross the gap. Nothing is widened or panned. The stereo image is the
geometry, and moving the microphones moves it.

Mono-summing the banks recombines them toward an even train and cancels much of
the unevenness — real, and why people argue about H-pipes, but it throws away
the evidence. In stereo the two pulse trains stay apart where you can hear them
disagree.

**The sound, against itself.** The whole point. Half-order energy against
whole-order energy at idle — half-orders being the signature of a pulse train
that repeats every *two* revolutions instead of one:

|                | cross-plane | flat-plane |     |
| -------------- | ----------- | ---------- | --- |
| one bank alone | **1.23**    | **0.047**  | 26× |

The firing fundamental carries 81% of the energy and only 6% sits above
1.4 kHz, which is what a V8 idle looks like on an analyser.

Summing two banks recombines them toward an even train and cancels much of the
unevenness, which is why people have argued about H-pipes and true duals for
sixty years.

---

## What is in it

```
include/engine/
  si.hpp           inside the engine, everything is SI. one rule, no exceptions
  geometry.hpp     the crank-slider, which is the entire machine
  crankshaft.hpp   where the sound comes from
  balance.hpp      why the cross-plane crank exists at all
  charge.hpp       the working fluid, and what is dissolved in it
  wiebe.hpp        how fast the fire spreads
  knock.hpp        the constraint that decides everything
  woschni.hpp      heat going where it does no good
  camshaft.hpp     the only part of the engine that decides anything
  port.hpp         the bottleneck
  cylinder.hpp     one cylinder, as an open system
  friction.hpp     the tax
  induction.hpp    the throttle, and the vacuum behind it
  ignition.hpp     deciding when, with no way of knowing
  riemann.hpp      what two bodies of gas do to each other
  exhaust.hpp      four pipes, a collector, and the noise
  engine.hpp       the assembly
  windsor.hpp      the engine itself, as built

apps/
  spec.cpp         the shop manual page
  dyno.cpp         a water brake on the flywheel
  run.cpp          a gauge cluster, wired to the sensors
  card.cpp         let it draw its own picture, as Watt did
  record.cpp       three feet behind the tailpipe
  verify.cpp       the inspection sheet
```

The engine is a sealed mechanism that turns and gets hot and knows nothing
about being observed. The four apps are instruments bolted to it afterward,
and none of them may reach into the physics to make its own job easier.

Every header opens with prose explaining why the part exists and what it is
arguing with. They are meant to be read in roughly the order above.

---

## The physics, briefly

Each cylinder is an **open thermodynamic system**, integrated in crank angle,
five terms of the first law per radian:

```
m·cᵥ·dT/dθ  =  −p·dV/dθ          the piston, taking or giving work
               + dQ_burn/dθ       the fire            (Wiebe)
               − dQ_wall/dθ       the coolant         (Woschni)
               + Σ (dmᵢ/dθ)·hᵢ    gas arriving        (compressible orifice flow)
               − u·dm/dθ          the same, as mass
```

Open and not closed, because everything anyone cares about happens in the half
of the cycle that the air-standard Otto cycle deletes — the half where the
valves are open and the mass is a variable.

The exhaust is a **digital waveguide**: a pipe is two delay lines, one per
direction, which is d'Alembert's 1747 result sampled at 176 kHz rather than in
the continuum. Junctions scatter by continuity of pressure and conservation of
volume flow. The loop is closed — each cylinder's exhaust boundary is the
pressure standing in *its own primary pipe*, so header length changes the
torque curve and not only the note.

γ varies with temperature, falling from 1.400 in a cold intake charge to 1.246
in combustion products. A cycle computed at a constant 1.4 will promise you a
fifth more thermal efficiency than any engine can deliver.

---

## What it does not model

Stated plainly, because an unstated simplification is a lie and a stated one
is a design decision. Each of these is named in the file where it bites.

- **Wrist-pin offset.** Real pistons carry 0.5–1.5 mm toward the thrust side.
  It buys a rattle you cannot hear and costs a closed-form solution.
- **Intake runner tuning.** Each runner is an organ pipe worth ~10% of peak
  torque in a narrow band. The plenum here is one well-stirred volume.
  This is most of the missing ten percent.
- **Nonlinear gas dynamics** — and this one was built, measured, and reverted,
  which is the most interesting entry on the list. `riemann.hpp` is a
  second-order finite-volume Euler solver, verified against Sod's shock tube to
  one part in 10⁵. Wired into the engine in place of the delay lines it removed
  the Mach 1 clamp entirely and made header length worth **14% of torque**,
  peaking at 1.2 m at 4500 rpm where the old builder's rule predicts 1.10 m —
  against 1.8% and no peak from the waveguide.

  It was reverted anyway. It cost 7× the runtime, and it destroyed the one
  measurement this project exists to make: the flat-plane bank's half-order
  share went from 0.02 to 3.8 and the ratio between the two crankshafts
  collapsed to 1×. An engine that cannot tell the two cranks apart is of no use
  here however good its shocks are.

  The lesson is worth the whole detour: a delay line has **no** numerical
  dissipation — it is the exact solution to the linear problem, not an
  approximation to it — while any finite-volume scheme is diffusive everywhere.
  For a problem that is mostly linear propagation with occasional violence, the
  cruder-looking model is the more faithful one over most of the cycle.

  What survives is the measurement. `./windsor verify` fires a real blowdown
  front down a primary with no clamp at all and clocks it at **1409 m/s** —
  Mach 2.4, against 586 m/s for sound in the gas ahead. It is a shock, it
  outruns its own sound, and it is why the crack of an exhaust is sharper at
  the tailpipe than at the valve. The shipped model cannot produce that, and
  now says so with a number instead of an apology.
- **Blow-by, oil temperature, crankshaft torsion, dissociation above 2000 K.**

None of these would change the shape of the project.

---

## Build

```
make
./windsor spec       what was built, and what falls out of it
./windsor dyno       put it on a water brake and sweep it
./windsor run        watch it idle  (SPACE for throttle, Q to stop)
./windsor card       let it draw its own indicator diagram
./windsor record     stand behind it with two microphones
./windsor verify     check every number this project quotes
```

Flags: `--flat` fits the billet flat-plane crankshaft, `--ho` the 1982 H.O.
camshaft.

A C++23 compiler and `make`. That is the whole of it, and it is deliberate:
every dependency is a bet that something else will still build in fifteen
years. The WAV writer is 44 bytes of header and some samples, written here.

---

## Why

No reason. That is the point. It is an engine that turns over in a terminal
and cannot move anything, burning fuel that does not exist, wasting a third of
it out of a pipe into nowhere, and it does all of that correctly.

MIT.
