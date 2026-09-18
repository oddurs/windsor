import * as stylex from '@stylexjs/stylex';

// The whole design system. Seven greys for the page, a solved palette for the
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
// A code block is not paper, but it is not a lightbox either. The ground is
// neutral and one step darker than the page in both themes, so a panel settles
// into the sheet instead of cutting a hole in it.
//
// The hues are solved rather than chosen. Picking seven colours by eye gives
// seven different perceptual lightnesses — the first attempt at this spread 22
// points of L* — and the eye reads that as some of them shouting. So each hue
// is held at ONE lightness and its channels solved to land there: L* 44 on the
// light ground, L* 78 on the dark. Every hue in a theme therefore sits within
// 0.05 of the same contrast, measured, which is what balance actually is.
//
//   light: every hue 5.0:1 on the panel, comments 4.7, text 14.2
//   dark:  every hue 11.0:1, comments 4.9, text 13.6
export const term = stylex.defineVars({
  ground: { default: '#f3f3f0', [dark]: '#0a0a0a' },
  border: { default: '#e5e5e1', [dark]: '#1f1f1f' },
  glow: { default: '#e6e6e0', [dark]: '#1c1c1c' },

  text: { default: '#232322', [dark]: '#d6d6d2' },
  bright: { default: '#0f0f0e', [dark]: '#f2f2ee' },
  fog: { default: '#6d6d68', [dark]: '#7f7f79' },
  punctuation: { default: '#5c5c58', [dark]: '#9a9a94' },

  coral: { default: '#ac4939', [dark]: '#efb3a9' },
  amber: { default: '#7e652a', [dark]: '#e2bc65' },
  mint: { default: '#277551', [dark]: '#3dda91' },
  cyan: { default: '#2b7182', [dark]: '#73cee4' },
  sky: { default: '#3b67b2', [dark]: '#a8c2ef' },
  violet: { default: '#8948c2', [dark]: '#d5b4f1' },
  rose: { default: '#af3a84', [dark]: '#efadd7' },
});

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
