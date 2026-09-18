import * as stylex from '@stylexjs/stylex';

// The whole design system. Seven greys for the page, a lit palette for the
// terminal, two families, seven sizes and
// a spacing scale built on the line. Everything on the site is assembled out of
// this file and nothing else, which is the same discipline the engine applies
// to units: one surface, no exceptions.

const dark = '@media (prefers-color-scheme: dark)';

// Greys, not colours. The subject is a cast-iron block and a paper shop manual,
// and neither has ever needed a hue to be understood.
//
// The three text tones are set by measurement and not by eye. Against paper,
// light mode: ink 18.0:1, muted 6.2:1, faint 4.6:1 — and 4.5 is the floor for
// small text, which is what faint is for. It was 4.19 until I checked. A
// caption nobody can read is not restraint, it is a caption nobody can read.
export const color = stylex.defineVars({
  paper: { default: '#fcfcfb', [dark]: '#101010' },
  raised: { default: '#f5f5f3', [dark]: '#191919' },
  ink: { default: '#141414', [dark]: '#eaeae7' },
  muted: { default: '#5f5f5d', [dark]: '#9a9a96' },
  faint: { default: '#737370', [dark]: '#7d7d79' },
  rule: { default: '#e4e4e1', [dark]: '#272727' },
  hairline: { default: '#efefec', [dark]: '#1e1e1e' },
});

// ── The terminal ────────────────────────────────────────────────────────────
//
// The page is paper and stays monochrome. A code block is not paper — it is a
// terminal, and a terminal has always been lit rather than printed. So these do
// not change with the theme: the panel is dark in both, because that is what
// the thing being quoted actually looks like, and because it gives the one
// place on the site that carries colour somewhere to put it.
//
// Seven hues at roughly one lightness, which is what keeps a palette this
// saturated from coming apart. Every one of them is load-bearing: each marks
// something the language or the program already distinguished.
//
// Measured against the ground: text 14.8:1, the hues 7.1 to 12.8, and fog —
// which is what comments are set in — 5.5:1. Comments carry the argument in
// this project, so they are the last thing that should be hard to read.
export const term = stylex.defineVars({
  ground: '#0c1014',
  border: '#1d2630',
  glow: '#151d26',

  text: '#dbe4ec',
  bright: '#f4f8fb',
  fog: '#7a8ba1',
  punctuation: '#8a9bb0',

  cyan: '#57e5ff',
  mint: '#52e8a5',
  amber: '#ffc857',
  coral: '#ff6e7f',
  violet: '#c792ff',
  sky: '#79aaff',
  rose: '#ff86d0',
});

// System fonts. The page should look like it belongs to the machine it is read
// on, and a downloaded typeface is a dependency that arrives late or not at all.
export const font = stylex.defineVars({
  text: 'system-ui, -apple-system, "Segoe UI", Roboto, Helvetica, Arial, sans-serif',
  mono: 'ui-monospace, SFMono-Regular, "SF Mono", Menlo, Consolas, "Liberation Mono", monospace',
});

// Seven sizes. Six of them sit inside one octave, close enough that they cannot
// shout at each other, and the seventh is allowed to — because a page needs one
// note of real contrast or everything on it reads as the same thing said twice.
export const size = stylex.defineVars({
  micro: '0.75rem',
  tiny: '0.8125rem',
  small: '0.875rem',
  base: '0.9375rem',
  lead: '1.0625rem',
  head: '1.75rem',
  display: '2rem',
});

export const leading = stylex.defineVars({
  flat: '1.1',
  tight: '1.25',
  snug: '1.45',
  prose: '1.65',
});

// Built on the line, not on powers of two. Body text sets a 25px line, so `line`
// is that line and everything else is a simple fraction or multiple of it. The
// page then has a rhythm rather than a set of arbitrary gaps.
export const space = stylex.defineVars({
  hair: '0.25rem',
  xs: '0.5rem',
  sm: '0.75rem',
  md: '1rem',
  line: '1.5rem',
  lg: '2rem',
  xl: '3rem',
  section: '4.5rem',
  page: '6rem',
});

// About sixty-eight characters at the base size. Long enough for an argument,
// short enough that the eye finds the next line without being told.
export const layout = stylex.defineVars({
  column: '36rem',
  gutter: '1.5rem',
});
