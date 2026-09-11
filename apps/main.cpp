#include "apps.hpp"
#include <cstring>
#include <cstdio>

namespace app {

bool has_flag(int argc, char** argv, const char* flag) {
    for (int i = 0; i < argc; ++i) if (std::strcmp(argv[i], flag) == 0) return true;
    return false;
}

engine::Engine engine_from_flags(int argc, char** argv) {
    if (has_flag(argc, argv, "--flat")) return engine::windsor::flat_crank();
    if (has_flag(argc, argv, "--ho"))   return engine::windsor::high_output();
    return engine::windsor::stock();
}

} // namespace app

static int usage() {
    std::puts(R"(windsor — a Ford 302, modelled from first principles, for no reason.

    windsor spec        what was built, and what falls out of it
    windsor dyno        put it on a water brake and sweep it
    windsor run         watch it idle
    windsor record      stand behind it with a microphone
    windsor card        let it draw its own picture, as Watt did
    windsor verify      check every number this project quotes

Options
    --flat              fit the billet flat-plane crankshaft instead
    --ho                the 1982 H.O. camshaft on the stock crank

The engine is the same in every case. Only the crank changes, and only the
crank has ever decided what a V8 sounds like.)");
    return 1;
}

int main(int argc, char** argv) {
    if (argc < 2) return usage();
    const char* verb = argv[1];

    if (std::strcmp(verb, "spec")   == 0) return app::spec  (argc, argv);
    if (std::strcmp(verb, "dyno")   == 0) return app::dyno  (argc, argv);
    if (std::strcmp(verb, "run")    == 0) return app::run   (argc, argv);
    if (std::strcmp(verb, "record") == 0) return app::record(argc, argv);
    if (std::strcmp(verb, "verify") == 0) return app::verify(argc, argv);
    if (std::strcmp(verb, "card")   == 0) return app::card  (argc, argv);

    return usage();
}
