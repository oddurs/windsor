# Reading order

The source is the document. These files are only a map into it.

Every header opens with prose explaining why the part exists and what it is
arguing with, and they were written to be read in this order — each one
assuming the last.

## The spine

|                    |                                                                                                                               |
| ------------------ | ----------------------------------------------------------------------------------------------------------------------------- |
| **si.hpp**         | The one rule everything else depends on: inside the engine, everything is SI. Read it first or nothing later will make sense. |
| **geometry.hpp**   | The crank-slider, which is the entire machine. One equation, and every other file is decoration on it.                        |
| **crankshaft.hpp** | The thesis. Why a cross-plane V8 burbles, and why it refuses to let you type in a firing order.                               |
| **balance.hpp**    | The other half of the thesis: what the cross-plane crank *cost*, and why almost nobody buys the alternative.                  |

Stop here and you have the argument. Everything below is the machine that
makes it audible.

## One cylinder

|                  |                                                                                                                            |
| ---------------- | -------------------------------------------------------------------------------------------------------------------------- |
| **charge.hpp**   | The working fluid. An engine does not burn petrol, it heats air.                                                           |
| **wiebe.hpp**    | How fast the fire spreads — a curve fitted in the 1950s that outlived everything meant to replace it.                      |
| **woschni.hpp**  | The third of the fuel that goes into the coolant. Also the one sanctioned exception to the SI rule, and it says so loudly. |
| **camshaft.hpp** | The only part of the engine that decides anything. Two engines identical but for this make peak power 2500 rpm apart.      |
| **port.hpp**     | The bottleneck. A naturally aspirated engine is a pump forbidden from using a pump.                                        |
| **cylinder.hpp** | All of the above, assembled: five terms of the first law per radian of crank.                                              |

## The rest of the machine

|                   |                                                                                                                        |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------- |
| **induction.hpp** | The throttle, and the vacuum behind it — the most wasteful control mechanism never replaced.                           |
| **ignition.hpp**  | Deciding when to light it, with no way of knowing. Flyweights and a rubber diaphragm.                                  |
| **knock.hpp**     | The constraint that actually decides compression ratio. Read the note on why its number is an index and not a verdict. |
| **friction.hpp**  | The tax. At idle it is all of it.                                                                                      |
| **exhaust.hpp**   | Four pipes, a collector, and the noise. Where the project cashes in.                                                   |
| **riemann.hpp**   | Its foil: the nonlinear solver that says what the one above costs. See [reverted.md](reverted.md).                     |
| **engine.hpp**    | The assembly, and the loop — a running engine is two statements chasing each other.                                    |
| **windsor.hpp**   | The engine itself, as built. Every figure is Ford's except the few marked otherwise.                                   |

## The instruments

`apps/` holds six, and the engine knows about none of them. It is a sealed
mechanism that turns and gets hot; these are bolted on afterward, and none may
reach into the physics to make its own job easier.

`spec` the shop manual page · `dyno` a water brake on the flywheel ·
`run` a live gauge cluster · `card` Watt's indicator diagram ·
`record` two microphones behind the tailpipes · `verify` the inspection sheet.

Read `verify.cpp` last. Its entry point is the contents page of everything the
project claims, and every claim there is checked against something outside it.
