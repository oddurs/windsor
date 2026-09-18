import * as stylex from '@stylexjs/stylex';
import type { ReactNode } from 'react';

import { color, font, leading, size, space } from '@/design/tokens.stylex';

// Five equations, and no more than five.
//
// Notation earns its place when a relation is genuinely two-dimensional — a
// quotient, a root, an integral with limits — and costs the reader when it is
// used for something a sentence already says plainly. Firing intervals are
// arithmetic and stay in prose. These five are not.
//
// They live together in one file because they are one idea: the argument the
// engine is actually making, written the way it was derived.

function Equation({ children, note }: { children: ReactNode; note?: string }) {
  return (
    <figure {...stylex.props(s.figure)}>
      <div {...stylex.props(s.display)}>{children}</div>
      {note ? <figcaption {...stylex.props(s.note)}>{note}</figcaption> : null}
    </figure>
  );
}

// A differential, as a component rather than a value, so that mfrac always
// receives exactly the two children it is defined to take.
const D = ({ of }: { of: string }) => (
  <mrow>
    <mi>d</mi>
    <mi>{of}</mi>
  </mrow>
);

const dTheta = (
  <mrow>
    <mi>d</mi>
    <mi>θ</mi>
  </mrow>
);

// ── The first law, per radian of crank ──────────────────────────────────────
//
// Set as five separate rows against their own annotations rather than as one
// long line, because that is how it is read: a left-hand side, and then five
// things that happen to a box of gas, each of which is a file.

const terms: readonly [ReactNode, string][] = [
  [
    <mrow key="work">
      <mo>−</mo>
      <mi>p</mi>
      <mfrac>
        <D of="V" />
        {dTheta}
      </mfrac>
    </mrow>,
    'the piston, taking or giving work',
  ],
  [
    <mrow key="burn">
      <mo>+</mo>
      <mfrac>
        <mrow>
          <mi>d</mi>
          <msub>
            <mi>Q</mi>
            <mtext>burn</mtext>
          </msub>
        </mrow>
        {dTheta}
      </mfrac>
    </mrow>,
    'the fire — wiebe.hpp',
  ],
  [
    <mrow key="wall">
      <mo>−</mo>
      <mfrac>
        <mrow>
          <mi>d</mi>
          <msub>
            <mi>Q</mi>
            <mtext>wall</mtext>
          </msub>
        </mrow>
        {dTheta}
      </mfrac>
    </mrow>,
    'the coolant, stealing — woschni.hpp',
  ],
  [
    <mrow key="in">
      <mo>+</mo>
      <mo>∑</mo>
      <mfrac>
        <mrow>
          <mi>d</mi>
          <msub>
            <mi>m</mi>
            <mi>i</mi>
          </msub>
        </mrow>
        {dTheta}
      </mfrac>
      <msub>
        <mi>h</mi>
        <mi>i</mi>
      </msub>
    </mrow>,
    'gas arriving, carrying enthalpy — port.hpp',
  ],
  [
    <mrow key="mass">
      <mo>−</mo>
      <mi>u</mi>
      <mfrac>
        <D of="m" />
        {dTheta}
      </mfrac>
    </mrow>,
    'the same gas, accounted as mass',
  ],
];

export function FirstLaw() {
  return (
    <figure {...stylex.props(s.figure)}>
      <div {...stylex.props(s.ledger)}>
        <div {...stylex.props(s.lhs)}>
          <math display="block">
            <mrow>
              <mi>m</mi>
              <msub>
                <mi>c</mi>
                <mi>v</mi>
              </msub>
              <mfrac>
        <D of="T" />
        {dTheta}
      </mfrac>
              <mo>=</mo>
            </mrow>
          </math>
        </div>
        {terms.map(([term, says], i) => (
          <div key={i} {...stylex.props(s.row)}>
            <math display="block">{term}</math>
            <span {...stylex.props(s.says)}>{says}</span>
          </div>
        ))}
      </div>
    </figure>
  );
}

// ── The linkage ─────────────────────────────────────────────────────────────

export function CrankSlider() {
  return (
    <Equation note="s is how far the piston has come down from top dead centre; a is the crank throw, l the rod, A the bore area, Vc the clearance volume.">
      <math display="block">
        <mrow>
          <mi>s</mi>
          <mo>(</mo>
          <mi>θ</mi>
          <mo>)</mo>
          <mo>=</mo>
          <mo>(</mo>
          <mi>a</mi>
          <mo>+</mo>
          <mi>l</mi>
          <mo>)</mo>
          <mo>−</mo>
          <mi>a</mi>
          <mi>cos</mi>
          <mi>θ</mi>
          <mo>−</mo>
          <msqrt>
            <mrow>
              <msup>
                <mi>l</mi>
                <mn>2</mn>
              </msup>
              <mo>−</mo>
              <msup>
                <mi>a</mi>
                <mn>2</mn>
              </msup>
              <msup>
                <mrow>
                  <mi>sin</mi>
                  <mi>θ</mi>
                </mrow>
                <mn>2</mn>
              </msup>
            </mrow>
          </msqrt>
        </mrow>
      </math>
      <math display="block">
        <mrow>
          <mi>V</mi>
          <mo>(</mo>
          <mi>θ</mi>
          <mo>)</mo>
          <mo>=</mo>
          <msub>
            <mi>V</mi>
            <mi>c</mi>
          </msub>
          <mo>+</mo>
          <mi>A</mi>
          <mi>s</mi>
          <mo>(</mo>
          <mi>θ</mi>
          <mo>)</mo>
        </mrow>
      </math>
    </Equation>
  );
}

// ── Woschni, and why it is the one exception to the SI rule ─────────────────

export function Woschni() {
  const power = (base: string, exponent: string) => (
    <msup>
      <mi>{base}</mi>
      <mrow>
        {exponent.startsWith('−') ? <mo>−</mo> : null}
        <mn>{exponent.replace('−', '')}</mn>
      </mrow>
    </msup>
  );
  return (
    <Equation note="B in metres, p in KILOPASCALS, T in kelvin, w in m/s — and it returns W/(m²·K).">
      <math display="block">
        <mrow>
          <mi>h</mi>
          <mo>=</mo>
          <mn>3.26</mn>
          {power('B', '−0.2')}
          {power('p', '0.8')}
          {power('T', '−0.55')}
          {power('w', '0.8')}
        </mrow>
      </math>
    </Equation>
  );
}

// ── Livengood–Wu ────────────────────────────────────────────────────────────

export function LivengoodWu() {
  return (
    <Equation note="τ is the ignition delay at the pressure and temperature of that instant.">
      <math display="block">
        <mrow>
          <munderover>
            <mo>∫</mo>
            <mn>0</mn>
            <msub>
              <mi>t</mi>
              <mtext>ign</mtext>
            </msub>
          </munderover>
          <mfrac>
            <mrow>
              <mi>d</mi>
              <mi>t</mi>
            </mrow>
            <mrow>
              <mi>τ</mi>
              <mo>(</mo>
              <mi>p</mi>
              <mo>,</mo>
              <mi>T</mi>
              <mo>)</mo>
            </mrow>
          </mfrac>
          <mo>=</mo>
          <mn>1</mn>
        </mrow>
      </math>
    </Equation>
  );
}

// ── The line the textbook is taught for ─────────────────────────────────────

export function OttoEfficiency() {
  return (
    <math display="inline" {...stylex.props(s.inline)}>
      <mrow>
        <mi>η</mi>
        <mo>=</mo>
        <mn>1</mn>
        <mo>−</mo>
        <msup>
          <mi>r</mi>
          <mrow>
            <mn>1</mn>
            <mo>−</mo>
            <mi>γ</mi>
          </mrow>
        </msup>
      </mrow>
    </math>
  );
}

const s = stylex.create({
  figure: {
    marginBlock: space.lg,
    marginInline: 0,
  },
  display: {
    color: color.ink,
    display: 'grid',
    fontFamily: font.text,
    fontSize: size.lead,
    justifyItems: 'start',
    overflowX: {
      default: 'visible',
      '@media (max-width: 34rem)': 'auto',
    },
    rowGap: space.xs,
  },
  inline: {
    fontFamily: font.text,
    fontSize: '1em',
  },
  // The left-hand side once, then one row per term, each against the thing it
  // is. The source comment is laid out this way and it was right.
  ledger: {
    display: 'grid',
    overflowX: {
      default: 'visible',
      '@media (max-width: 34rem)': 'auto',
    },
    rowGap: space.hair,
  },
  lhs: {
    color: color.ink,
    fontSize: size.lead,
    paddingBlockEnd: space.hair,
  },
  row: {
    alignItems: 'baseline',
    columnGap: space.md,
    display: 'grid',
    fontSize: size.lead,
    gridTemplateColumns: {
      default: 'max-content 1fr',
      '@media (max-width: 34rem)': '1fr',
    },
    paddingInlineStart: space.md,
    rowGap: 0,
  },
  says: {
    color: color.muted,
    fontFamily: font.text,
    fontSize: size.tiny,
    lineHeight: leading.snug,
  },
  note: {
    color: color.faint,
    fontSize: size.micro,
    lineHeight: leading.snug,
    paddingBlockStart: space.xs,
  },
});
