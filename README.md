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

| | each bank hears |
|---|---|
| **cross-plane** (throws at 0°, 90°, 180°, 270°) | 90° — 180° — 270° — 180° |
| **flat-plane** (throws at 0°, 180°, 0°, 180°) | 180° — 180° — 180° — 180° |

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

Forty-two checks, in a third of a second, every one of them against something
outside the project — a derivative against finite differences, a burn rate
against its own integral, a cylinder head against a flow bench, a firing order
against the casting, and the thesis against a Fourier transform.

Nothing in the model was fitted to any of it. A selection:

**The cylinder head, against a flow bench.** The port model is curtain area,
a discharge coefficient, and a throat cap. Run it at 28 inches of water:

| lift | model | published, stock C8OE castings |
|---|---|---|
| 0.375 in (the cam's peak) | 146 cfm | ~150 cfm at 0.400 in |

**The engine, against Ford.** `./windsor dyno`:

| | model | Ford, 1968 302-2V (gross) |
|---|---|---|
| peak torque | 310 lb-ft @ 2000 | 300 lb-ft @ 2600 |
| peak power | 236 hp @ 5000 | 210 hp @ 4600 |

About ten percent optimistic, with no fitting anywhere. The missing ten
percent has names, and they are listed below.

**The indicator card.** At 3000 rpm, wide open: peak pressure 55 bar at 13°
after top dead centre, 50% mass burned at 8° ATDC, 2945 K. Which is what a
pressure trace off a 1968 wedge chamber looks like.

**The balance, which is why the crank exists.** The same rod table, asked a
different question — `balance.hpp` resolves each piston's inertia force along
its own bore axis and Fourier-transforms the sum over a revolution:

| at 3000 rpm | cross-plane | flat-plane |
|---|---|---|
| primary force | 0 N | 0 N |
| **secondary force** | **0 N** | **5000 N** |
| what it traces | a circle — a counterweight opposes it | a line — nothing can |

Half a tonne, a hundred times a second, and no counterweight on a shaft
turning at ω can oppose a force that goes at 2ω. The flat crank sounds better
and shakes. That is the bill for the noise, and it is why almost nobody pays it.

**The sound, against itself.** The whole point. Half-order energy against
whole-order energy at idle — half-orders being the signature of a pulse train
that repeats every *two* revolutions instead of one:

| | cross-plane | flat-plane | |
|---|---|---|---|
| one bank alone | **2.13** | **0.025** | 86× |
| both banks summed | **0.135** | **0.053** | 2.5× |

with 4.5 and 5.0 order standing 19 and 21 dB louder on the cross-plane crank.

That second row is itself a real result: summing two banks recombines them
toward an even train and cancels much of the unevenness, which is why people
have argued about H-pipes and true duals for sixty years.

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
  woschni.hpp      heat going where it does no good
  camshaft.hpp     the only part of the engine that decides anything
  port.hpp         the bottleneck
  cylinder.hpp     one cylinder, as an open system
  friction.hpp     the tax
  induction.hpp    the throttle, and the vacuum behind it
  ignition.hpp     deciding when, with no way of knowing
  exhaust.hpp      four pipes, a collector, and the noise
  engine.hpp       the assembly
  windsor.hpp      the engine itself, as built

apps/
  spec.cpp         the shop manual page
  dyno.cpp         a water brake on the flywheel
  run.cpp          a gauge cluster, wired to the sensors
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
- **Nonlinear gas dynamics.** The waveguide is linear, so the source has to be
  clamped at Mach 1 — past which what leaves the valve is a jet, not a wave.
  Real engine gas-dynamics codes use method of characteristics. This is why
  header tuning here is worth ~2% instead of ~8%.
- **Knock.** Needs end-gas chemistry, and is the constraint that actually
  decides compression ratio.
- **Blow-by, oil temperature, crankshaft torsion, dissociation above 2000 K.**

None of these would change the shape of the project.

---

## Build

```
make
./windsor spec       what was built, and what falls out of it
./windsor dyno       put it on a water brake and sweep it
./windsor run        watch it idle  (SPACE for throttle, Q to stop)
./windsor record     stand behind it with a microphone
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
