import * as stylex from '@stylexjs/stylex';

// The whole design system. Six greys, two families, seven sizes, eight steps.
// Everything on the site is assembled out of this file and nothing else, which
// is the same discipline the engine applies to units: one surface, no exceptions.

const dark = '@media (prefers-color-scheme: dark)';

// Greys, not colours. The subject is a cast-iron block and a paper shop manual,
// and neither has ever needed a hue to be understood.
export const color = stylex.defineVars({
  paper: { default: '#fcfcfb', [dark]: '#0e0e0e' },
  raised: { default: '#f4f4f2', [dark]: '#171717' },
  ink: { default: '#141414', [dark]: '#e9e9e6' },
  muted: { default: '#6a6a68', [dark]: '#8e8e8b' },
  faint: { default: '#9b9b98', [dark]: '#676764' },
  rule: { default: '#e3e3e0', [dark]: '#242424' },
});

// System fonts. The page should look like it belongs to the machine it is read
// on, and a downloaded typeface is a dependency that arrives late or not at all.
export const font = stylex.defineVars({
  text: 'system-ui, -apple-system, "Segoe UI", Roboto, Helvetica, Arial, sans-serif',
  mono: 'ui-monospace, SFMono-Regular, "SF Mono", Menlo, Consolas, "Liberation Mono", monospace',
});

// A narrow ladder: seven sizes inside a single octave, each about an eighth
// larger than the last. Steps that close cannot shout, so hierarchy has to come
// from weight, colour and space — which is the constraint that keeps a page calm.
export const size = stylex.defineVars({
  micro: '0.75rem',
  tiny: '0.8125rem',
  small: '0.875rem',
  base: '0.9375rem',
  lead: '1rem',
  title: '1.125rem',
  head: '1.3125rem',
});

export const leading = stylex.defineVars({
  tight: '1.3',
  snug: '1.45',
  prose: '1.65',
});

export const space = stylex.defineVars({
  xs: '0.25rem',
  sm: '0.5rem',
  md: '0.75rem',
  lg: '1rem',
  xl: '1.5rem',
  xxl: '2.25rem',
  section: '3.5rem',
  page: '5rem',
});

// About sixty-two characters at the base size. Long enough for an argument,
// short enough that the eye finds the next line without being told.
export const layout = stylex.defineVars({
  column: '36rem',
  gutter: '1.5rem',
});
