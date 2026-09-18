# windsor

A Ford 302, modelled from first principles in C++23, for no reason.

```
make && ./windsor spec
```

No dependencies. Nothing to configure. No website. It is an engine, and it runs.

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

|                                             | each bank hears       |
| ------------------------------------------- | --------------------- |
| **cross-plane** (throws at 0, 90, 180, 270) | 90 - 180 - 270 - 180  |
| **flat-plane** (throws at 0, 180, 0, 180)   | 180 - 180 - 180 - 180 |

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
    {{
    //  cylinder    journal   bank          revolution
        /* 1 */     { 0,      Bank::right,  0 },
        /* 2 */     { 1,      Bank::right,  0 },
        /* 3 */     { 2,      Bank::right,  1 },
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

Sixty-three checks in a couple of seconds, every one against something outside
the project: a derivative against finite differences, a burn rate against its
own integral, a cylinder head against a flow bench, a firing order against the
casting, a shock tube against its exact solution, and the thesis against a
Fourier transform. Nothing in the model was fitted to any of it.

**The cylinder head, against a flow bench.** Curtain area, a discharge
coefficient, a throat cap. How fast that coefficient sags past its peak is the
only number in the port model fitted to anything, and this is what it is fitted
to:

| lift                      | model     | published, stock C8OE-F |
| ------------------------- | --------- | ----------------------- |
| 0.400 in                  | 153.5 cfm | ~150 cfm                |
| 0.426 in (the cam's peak) | 149.9 cfm | —                       |

**The engine, against Ford.**

|             | model            | Ford, 1968 302-2V (gross) |
| ----------- | ---------------- | ------------------------- |
| peak torque | 307 lb-ft @ 1985 | 295 lb-ft @ 2400          |
| peak power  | 206 hp @ 4994    | 210 hp @ 4400             |

Within four percent and two, with nothing fitted. The magnitudes are close and
the curve is too broad — torque peaks 400 rpm low, power 600 high — which is the
intake runner tuning still missing. It was ten percent optimistic with the power
peak a thousand rpm high until the carburettor's venturis went in: a two-barrel
engine breathes through a hole that cannot be opened.

**The indicator card.** `./windsor card` draws Watt's diagram, wide open and
throttled, to the same scale. At 3000 rpm the trace peaks at 64 bar, 9.5° after
top dead centre — late enough to use the crank's leverage, early enough that the
end gas has not gone off by itself. Throttled:

| at 2000 rpm | gross     | pumping       | net       |
| ----------- | --------- | ------------- | --------- |
| wide open   | 11.99 bar | −0.08 bar     | 11.90 bar |
| throttled   | 0.97 bar  | **−0.93 bar** | 0.03 bar  |

Ninety-six percent of everything it makes, spent on suffocating itself.

**The fuel, which is what stops the engine.** Livengood–Wu over Douaud–Eyzat:
the end gas spends `dt/τ` of its patience each instant and goes off when the
account reaches one. It reports an index and not a verdict — the threshold
belongs to the CFR engine the correlation was fitted on — but the comparisons
are what compression ratio and advance are actually chosen by, and they all come
out right. Knock worsens at low rpm, with advance, with compression, with
cheaper fuel, and eases on a rich mixture. Which makes 9.5:1, 34° of advance and
what was in the tank in 1968 one decision instead of three.

**The balance, which is why the crank exists.** The same rod table, asked a
different question:

| at 3000 rpm         | cross-plane                       | flat-plane           |
| ------------------- | --------------------------------- | -------------------- |
| primary force       | 0 N                               | 0 N                  |
| **secondary force** | **0 N**                           | **5000 N**           |
| what it traces      | a circle — counterweights take it | a line — nothing can |

Half a tonne, a hundred times a second, and nothing bolted to a shaft turning at
ω can oppose a force that goes at 2ω. The flat crank sounds better and shakes.
That is the bill for the noise, and why almost nobody pays it.

**The sound, against itself.** Half-order energy against whole-order energy at
idle, half-orders being the signature of a train that repeats every *two*
revolutions instead of one:

|                | cross-plane | flat-plane |      |
| -------------- | ----------- | ---------- | ---- |
| one bank alone | **0.70**    | **0.0009** | 742× |

Ninety-nine percent of the energy sits below 90 Hz, where the burble lives,
and nothing measurable at all above 1.4 kHz.

`./windsor record` writes stereo, because a V8 does not have an exhaust — it has
two, down opposite sides of the car, permanently out of step on a cross-plane
crank. Two microphones 0.6 m apart, 1.5 m behind two tailpipes 1.0 m apart, each
hearing both pipes: quieter by the extra distance, later by the time sound takes
to cross the gap. Nothing is widened or panned; the image is the geometry, and
moving the microphones moves it. Summing to mono recombines the banks toward an
even train and cancels much of the unevenness — real, and why people have argued
about H-pipes for sixty years, but it throws the evidence away.

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
about being observed. The six apps are instruments bolted to it afterward, and
none of them may reach into the physics to make its own job easier.

Every header opens with prose explaining why the part exists and what it is
arguing with. [docs/reading-order.md](docs/reading-order.md) is the path
through them; [docs/reverted.md](docs/reverted.md) is the two largest pieces of
work in the project, neither of which is in it; and
[docs/changing-it.md](docs/changing-it.md) is how to build a different engine.

---

## The physics, briefly

Each cylinder is an **open thermodynamic system**, integrated in crank angle,
five terms of the first law per radian:

```
m·cᵥ·dT/dθ  =  −p·dV/dθ           the piston, taking or giving work
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
pressure standing in *its own primary pipe*, so a header has a length that
matters and not merely a note. Only by about two percent here, for reasons
under *nonlinear gas dynamics* below.

γ varies with temperature, falling from 1.400 in a cold intake charge to 1.246
in combustion products. A cycle computed at a constant 1.4 will promise you a
fifth more thermal efficiency than any engine can deliver.

---

## What it does not model

Stated plainly, because an unstated simplification is a lie and a stated one is
a design decision. Each is named in the file where it bites.

- **Wrist-pin offset.** Real pistons carry 0.5–1.5 mm toward the thrust side.
  It buys a rattle you cannot hear and costs a closed-form solution.

- **Intake runner tuning.** Built three times, shipped none. The third gave the
  eight runners one shared junction — the same scattering node the exhaust uses
  for its collector — which was the hypothesis the second attempt ended on. It
  was only half right. Sharing redistributes energy between runners; it does not
  dissipate any, and eight low-loss pipes passing a wave around still have it.

  What mattered more was **length**. At 0.30 m the ram gain came out at 34%
  where a stock log manifold is worth a few percent, and that is what made the
  engine unholdable — because a torque-controlled brake on a *rising* torque
  curve is an unstable equilibrium, and ram tuning is exactly what makes a
  curve locally rise. Shortened to 0.18 m the gain falls to 4% and the curve
  comes out smooth.

  It still could not be held between 1000 and 1500 rpm, at any controller gain,
  and I do not know why. That is written down in `induction.hpp` as the next
  thing to find out rather than explained away.

- **Nonlinear gas dynamics.** `riemann.hpp` is a second-order finite-volume
  Euler solver, verified against Sod's shock tube to one part in 10⁵ and
  conserving mass and energy to machine precision. Wired in place of the delay
  lines it removed the Mach 1 clamp and made header length worth 14% of torque —
  and collapsed the difference between the two crankshafts, which is the one
  measurement this project exists to make. So it was reverted. A delay line has
  *no* numerical dissipation: it is the exact solution to the linear problem,
  not an approximation to it, and for a problem that is mostly linear
  propagation the cruder-looking model is the more faithful one.

  What survives is a measurement. `./windsor verify` clocks a real blowdown
  front at **1409 m/s** — Mach 2.4, against 586 for sound in the gas ahead of
  it. It is a shock. The shipped model cannot make one, and now says so with a
  number instead of an apology.

- **Blow-by, oil temperature, crankshaft torsion, dissociation above 2000 K.**

None of these would change the shape of the project.

---

## Build

```
make
./windsor spec       what was built, and what falls out of it
./windsor dyno       put it on a water brake and sweep it
./windsor run        watch it idle  (0-9 throttle, SPACE wide open, Q quit)
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
