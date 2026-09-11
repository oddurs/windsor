// apps.hpp — the instruments.
//
// The engine is a sealed mechanism. It turns, it gets hot, and it knows
// nothing whatever about being observed. Everything in this directory is
// something you bolt onto it afterward, and none of it is allowed to reach
// into the physics to make its own job easier.
//
//   spec     the shop manual page. What was built, and what falls out of it.
//   dyno     a water brake on the flywheel. Loads the engine until the speed
//            holds still, then weighs the arm.
//   run      a gauge cluster wired to the sensors, drawn at 30 frames a second.
//   record   a microphone three feet behind the tailpipe.

#pragma once
#include <engine/windsor.hpp>

namespace app {
int spec  (int argc, char** argv);
int dyno  (int argc, char** argv);
int run   (int argc, char** argv);
int record(int argc, char** argv);
int verify(int argc, char** argv);
int card  (int argc, char** argv);

// Every instrument can be clipped to either crankshaft, because the whole
// point of the project is the comparison.
engine::Engine engine_from_flags(int argc, char** argv);
bool has_flag(int argc, char** argv, const char* flag);
}
