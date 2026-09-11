// si.hpp — the unit convention of this engine.
//
// There is exactly one rule here, and every other file depends on it:
//
//      INSIDE THE ENGINE, EVERYTHING IS SI.
//
// Metres, kilograms, seconds, kelvin, pascals, joules, radians. Always.
// No inches, no bar, no degrees, no rpm, no horsepower — not in a struct
// field, not in a parameter, not in an intermediate. Those units exist only
// at the two surfaces of this program: the specification a user types in,
// and the number printed back out. Conversion happens at the surface and
// nowhere else.
//
// This is not pedantry. An engine is a machine where a bore is quoted in
// thousandths of an inch, boost in bar, torque in pound-feet and heat in
// BTU, all on the same page of the same manual, and every one of those is
// correct in its own context. The only way to keep the arithmetic in the
// middle honest is to refuse to let any of them in.

#pragma once

#include <numbers>

namespace engine::si {

// ── Physical constants ────────────────────────────────────────────────────
// Values are the engineering ones, not the CODATA ones; this is a machine,
// not a metrology lab.

inline constexpr double pi            = std::numbers::pi;
inline constexpr double two_pi        = 2.0 * pi;

inline constexpr double R_universal   = 8.314462618;   // J/(mol·K)
inline constexpr double g_standard    = 9.80665;       // m/s²
inline constexpr double p_atmosphere  = 101325.0;      // Pa, ISO 2533 sea level
inline constexpr double T_standard    = 288.15;        // K, 15 °C

// ── Literals: the surface where non-SI enters ─────────────────────────────
// Written so that a specification reads the way it is stamped on the part.
//
//      Bore   bore   {4.000_in};
//      Stroke stroke {3.000_in};
//      auto   redline = 6500.0_rpm;

inline namespace literals {

// Length
consteval double operator""_m  (long double v) { return double(v);            }
consteval double operator""_mm (long double v) { return double(v) * 1e-3;     }
consteval double operator""_in (long double v) { return double(v) * 0.0254;   }
consteval double operator""_ft (long double v) { return double(v) * 0.3048;   }

// Volume
consteval double operator""_L  (long double v) { return double(v) * 1e-3;     }
consteval double operator""_cc (long double v) { return double(v) * 1e-6;     }
consteval double operator""_ci (long double v) { return double(v) * 1.6387064e-5; } // cubic inch

// Mass
consteval double operator""_kg (long double v) { return double(v);            }
consteval double operator""_g  (long double v) { return double(v) * 1e-3;     }
consteval double operator""_lb (long double v) { return double(v) * 0.45359237; }

// Angle — the engine's native coordinate. Stored in radians; quoted, always
// and everywhere in the trade, in degrees. Both revolutions of the four-stroke
// cycle are addressable: 720.0_deg is a full cycle, not an error.
consteval double operator""_deg(long double v) { return double(v) * pi / 180.0; }
consteval double operator""_rad(long double v) { return double(v);            }
consteval double operator""_rev(long double v) { return double(v) * two_pi;   }

// Angular velocity. `_rpm` is how every engine on earth is discussed and
// how not one of them is computed.
consteval double operator""_rpm(long double v)        { return double(v) * two_pi / 60.0; }
consteval double operator""_rpm(unsigned long long v) { return double(v) * two_pi / 60.0; }

// Pressure
consteval double operator""_Pa  (long double v) { return double(v);           }
consteval double operator""_kPa (long double v) { return double(v) * 1e3;     }
consteval double operator""_bar (long double v) { return double(v) * 1e5;     }
consteval double operator""_psi (long double v) { return double(v) * 6894.757; }
consteval double operator""_atm (long double v) { return double(v) * p_atmosphere; }

// Temperature. Absolute only: there is no such thing as a negative kelvin in
// a cylinder, and a gas law fed celsius will lie to you quietly for a while
// and then produce a negative pressure.
consteval double operator""_K (long double v) { return double(v);             }
consteval double operator""_C (long double v) { return double(v) + 273.15;    }

// Energy, power, torque, force
consteval double operator""_J   (long double v) { return double(v);           }
consteval double operator""_kJ  (long double v) { return double(v) * 1e3;     }
consteval double operator""_W   (long double v) { return double(v);           }
consteval double operator""_kW  (long double v) { return double(v) * 1e3;     }
consteval double operator""_hp  (long double v) { return double(v) * 745.6999; } // mechanical
consteval double operator""_Nm  (long double v) { return double(v);           }
consteval double operator""_lbft(long double v) { return double(v) * 1.3558179; }
consteval double operator""_N   (long double v) { return double(v);           }

// Time
consteval double operator""_s  (long double v) { return double(v);            }
consteval double operator""_ms (long double v) { return double(v) * 1e-3;     }

} // namespace literals

// ── The other surface: SI back out to the page ────────────────────────────
// These are the only functions in the project permitted to divide by a
// conversion factor. Read them as "quote this in".

namespace as {
constexpr double mm  (double m)   { return m   * 1e3;               }
constexpr double in  (double m)   { return m   / 0.0254;            }
constexpr double L   (double m3)  { return m3  * 1e3;               }
constexpr double cc  (double m3)  { return m3  * 1e6;               }
constexpr double ci  (double m3)  { return m3  / 1.6387064e-5;      }
constexpr double deg (double rad) { return rad * 180.0 / pi;        }
constexpr double rpm (double w)   { return w   * 60.0 / two_pi;     }
constexpr double bar (double Pa)  { return Pa  * 1e-5;              }
constexpr double kPa (double Pa)  { return Pa  * 1e-3;              }
constexpr double psi (double Pa)  { return Pa  / 6894.757;          }
constexpr double C   (double K)   { return K   - 273.15;            }
constexpr double kW  (double W)   { return W   * 1e-3;              }
constexpr double hp  (double W)   { return W   / 745.6999;          }
constexpr double lbft(double Nm)  { return Nm  / 1.3558179;         }
constexpr double g   (double kg)  { return kg  * 1e3;               }
} // namespace as

} // namespace engine::si
