# windsor

A Ford 302 Windsor, modelled from first principles in C++23, for the sound.

This is not a simulator that happens to be readable. It is a piece of writing
that happens to run. If a change makes the engine more accurate and the source
less beautiful, it is the wrong change.

---

## What the thing is

Eight cylinders, a cross-plane crank, a cam, two exhaust manifolds. The physics
is honest — crank-slider kinematics, an open-system energy balance per cylinder,
Wiebe combustion, Woschni heat loss, compressible flow past the valves — but
accuracy is a means, not the end. The end is that a reader who has never opened
a shop manual finishes the repository understanding why an American V8 burbles.

The thesis lives in `crankshaft.hpp` and everything else serves it:

> A 90° V8 fires evenly every 90° on *either* crank. But each bank only hears
> its own four cylinders — 90-180-270-180 on a cross-plane, 180-180-180-180 on
> a flat one. That lopsided pulse train is the entire difference between a
> Mustang and a Ferrari, and it falls out of geometry or it doesn't.

Change the crank, and the sound changes. If it ever stops doing that, the model
has quietly stopped being true and the project is over.

---

## The homage

This is C++, deliberately, and it is meant as a thank-you letter to Bjarne
Stroustrup. Which means specific things, not vibes:

**Represent ideas directly in code.** `Bore` and `Stroke` are distinct types
because a bore is not a stroke and you should not be able to swap them by
accident. A `Lobe` knows its own centreline. A `Crankshaft` refuses to be
constructed with two rods on one journal. The domain is in the type system, and
the compiler is a machinist checking your work.

**Zero overhead.** Every abstraction here must compile to the arithmetic you
would have written by hand and no more. `Geometry::volume(θ)` is a class method
on an object with eight members and it becomes a handful of instructions.
If an abstraction costs a cycle it cannot justify, it is the wrong abstraction —
not because the cycle matters at 300 rpm-seconds, but because paying for
nothing is the habit that makes people write C instead.

**Prefer compile-time.** `consteval` literals. `constexpr` constructors. A
`Geometry` built entirely at compile time from figures on a spec sheet. Errors
that a compiler can catch should never reach a test.

**Don't make it look like C.** No output parameters, no raw owning pointers, no
`#define`, no arrays that decay. Values, references, `std::array`, and names.

---

## House rules

### 1. Inside the engine, everything is SI

Metres, kilograms, seconds, kelvin, pascals, radians. Not in a field, not in a
parameter, not in an intermediate. Inches and bar and rpm and horsepower exist
at exactly two places — the spec a user types in, and the number printed back
out — and `si.hpp` owns both surfaces.

The one sanctioned exception is `woschni.hpp`, which must be fed kilopascals
because the correlation is dimensionally inhomogeneous. It converts at its own
boundary and says so, loudly, in a comment. If you find yourself wanting a
second exception, you are about to introduce a bug that will take an afternoon
to find.

### 2. Derive, never declare

A firing order is not an input. You forge a crank, you grind a cam, and the
firing order is whatever falls out — which is why Ford and Chevrolet got
different orders from identical forgings. The same rule applies everywhere:
if a number is a consequence of other numbers, compute it. The moment you type
a derived constant in as a literal, the model stops being a model and becomes a
lookup table with opinions.

Corollary: when a derivation reproduces something real — 301.6 cubic inches,
firing order 1-5-4-2-6-3-7-8, peak effective area landing exactly on the lift
Ford chose — say so in the commit message. Those are the moments the project
is for.

### 3. Comments carry the argument

Every header opens with prose explaining *why the part exists and what it is
arguing with*. Not what the code does — the code does that. The register is a
service manual written by someone who loves the machine: real figures, real
part names, real tolerances, and an honest account of what has been left out.

Say what is not modelled and why. Wrist-pin offset is missing from
`geometry.hpp` and the file admits it. An unstated simplification is a lie; a
stated one is a design decision.

Never write a comment that restates the line beneath it.

### 4. No dependencies

The standard library and nothing else. No CMake requirement, no audio library,
no plotting library. `make && ./windsor` on a clean machine with a C++23
compiler. A WAV file is a 44-byte header and some samples; write it yourself.
The repository should still build in fifteen years.

### 5. Verify every physical claim

Before a number appears in a comment or a README, check it. Analytic
derivatives get compared against finite differences. Burn rates get integrated
and compared against burn fractions. Firing orders get printed and checked
against the casting. The shop-manual voice is only earned if the figures are
right; a beautiful comment attached to wrong arithmetic is the worst thing this
project could contain.

### 6. Every figure quoted outside the code is a copy, and copies rot

The README quotes dozens of numbers — flow bench cfm, peak torque, half-order
ratios, pumping mean effective pressures. Every one of them is a *copy* of
something the program prints, and the program changes.

They have drifted three times, and always the same way: an edit anchored on a
string that had since been reformatted, applied with a plain replace and no
check, which silently did nothing. The README went on quoting 320 lb-ft for an
engine making 306, and a flow figure at a lift the camshaft no longer had.

So: after any change to the model, re-run `./windsor verify`, `./windsor dyno`
and `./windsor card`, and reconcile every number in the README against what
they actually printed. And when editing prose programmatically, assert the
anchor exists. An edit that silently does nothing is worse than one that fails,
because you will believe it worked.

### 7. One idea per file

A file is named for a part or a person — `camshaft.hpp`, `wiebe.hpp`,
`woschni.hpp`. Files named after the people who fitted the correlations are
named that way on purpose: those are empirical curves that someone measured,
not laws, and the credit is also a warning.

---

## Commits

Atomic, one subsystem each, written in the same voice as the code. Present
tense, no ceremony, and say what the part *does* rather than what you did to it.

    The crankshaft, and where the sound comes from
    Blowdown chokes; reverse flow goes negative
    Admit that wrist-pin offset is missing

Not `feat: add crankshaft module` and not `fix stuff`. The log is part of the
piece.

Never commit a state that does not build.

---

## Layout

    include/engine/     the machine. header-only, one part per file.
    apps/               the instruments you bolt onto it.
    Makefile            `make`. that's it.

There is no src/. Every part of the engine is a header, because every part of
the engine is small enough to be read in one sitting, and splitting a thing
that size across two files buys nothing but a place for them to disagree.

The engine knows nothing about output. It is a sealed mechanism that turns and
gets hot. The six apps are witnesses to it — a shop manual page, a dyno on the
flywheel, a gauge cluster on the sensors, an indicator card on the cylinder, a
microphone behind the pipe, and an inspection sheet — and none of them may
reach into the physics to make their own job easier.
