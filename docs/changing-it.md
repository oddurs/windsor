# Fitting a different part

Everything the engine is made of is in `windsor.hpp`, and nothing else needs
touching to build a different one.

## Specification and calibration

The difference matters more here than anywhere else, and the source marks it.

A **specification** is Ford's: a bore, a rod length, a cam card, a carburettor.
It is quoted because it was measured off the engine, and if it is wrong the
model is wrong.

A **calibration** is a number fitted because nothing else could supply it —
burn duration, wall temperature, pipe gas temperature, how fast a discharge
coefficient sags. Every one is marked `CALIBRATED` in the source and says what
it was fitted to. There are five.

Anything unmarked is Ford's. That is the whole contract.

## The catalogue

```cpp
short_block()          4.000 × 3.000, 5.090 in rods, 9.5:1
stock_cam()            266°/256°, 0.426/0.425 in lift, 117° ICL, 110.5° LSA

cross_plane_crank()    the factory forging  →  1-5-4-2-6-3-7-8
cross_plane_crank_ho() the same forging, 1982 cam  →  1-3-7-2-6-5-4-8
flat_plane_crank()     a billet flat crank, same block, same rods

specification(crank, name)   everything else: fuel, distributor, friction,
                             induction, both exhausts
```

and the three engines those compose into: `stock()`, `high_output()`,
`flat_crank()`, which is what `--flat` and `--ho` select.

## Changing something

**A camshaft.** `Camshaft::from_card` takes what a catalogue prints — two
durations, two lifts, the intake centreline, the lobe separation, and the valve
diameters. Note *which* top dead centre a card means: centrelines are quoted
about the gas-exchange TDC, 360° from the firing TDC everything else here uses.
The function converts at that boundary and explains why.

**A crankshaft.** Give `Crankshaft` four journal throws, eight rods, and each
rod's revolution. You cannot give it a firing order; it derives one, and will
refuse to build a crank that cannot exist. Grind the throws into one plane and
every bank interval goes to 180°.

**A carburettor.** The venturi is the limit, not the throttle bore — a two-barrel
engine breathes through a hole that cannot be opened, and it is most of why the
2V made its power at 4400 where the 4V made more at 4800.

**Bore, stroke, compression.** `Geometry` takes them as distinct types so they
cannot be swapped by accident, and derives everything else.

## Afterwards

Run `./windsor verify`. Several of its sixty-three checks are against figures
belonging to *this* engine — the flow bench, the cam card, the firing orders —
and will fail honestly if you have built a different one. That is the check
doing its job, not breaking; edit the expectation to match the engine you meant
to build.

Then reconcile the README, because it quotes numbers the instruments print and
those numbers move. It has drifted three times. House rule six exists for it.
