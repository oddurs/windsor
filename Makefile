# windsor — a Ford 302, modelled from first principles, for no reason.
#
# No dependencies, no build system, no configure step. A C++23 compiler and
# make. This should still work in fifteen years.

CXX      ?= c++
CXXFLAGS ?= -std=c++23 -O2 -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast -Wdouble-promotion -Iinclude
SOURCES  := $(wildcard apps/*.cpp)
HEADERS  := $(wildcard include/engine/*.hpp) $(wildcard apps/*.hpp)

windsor: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@

clean:
	rm -f windsor *.wav

.PHONY: clean
