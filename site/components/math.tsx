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
//
// ── Setting it properly ─────────────────────────────────────────────────────
//
// A quantity is italic and everything else is upright. That is not decoration,
// it is the distinction the notation carries: `d` is an operator, `sin` is a
// function, and the `v` in cᵥ is the word "volume" abbreviated — none of them
// are quantities, so none of them lean. The two invisible operators do the rest
// of the work: U+2062 between factors, U+2061 between a function and what it is
// applied to, which is what tells the renderer how much air to leave.

const Rm = ({ children }: { children: string }) => (
  <mi mathvariant="normal">{children}</mi>
);

const Times = () => <mo>{'⁢'}</mo>;
const Of = () => <mo>{'⁡'}</mo>;

// An upright d, per ISO 80000-2, bound to what it differentiates.
const D = ({ of }: { of: string }) => (
  <mrow>
    <Rm>d</Rm>
    <Times />
    <mi>{of}</mi>
  </mrow>
);

const dTheta = (
  <mrow>
    <Rm>d</Rm>
    <Times />
    <mi>θ</mi>
  </mrow>
);

// sin²θ, and not sinθ². The exponent belongs to the function, and putting it
// after the argument says something else entirely.
const SinSquared = () => (
  <mrow>
    <msup>
      <mi mathvariant="normal">sin</mi>
      <mn>2</mn>
    </msup>
    <Of />
    <mi>θ</mi>
  </mrow>
);

function Equation({ children, note }: { children: ReactNode; note?: string }) {
  return (
    <figure {...stylex.props(s.figure)}>
      <div {...stylex.props(s.display)}>{children}</div>
      {note ? <figcaption {...stylex.props(s.note)}>{note}</figcaption> : null}
    </figure>
  );
}

// ── The first law, per radian of crank ──────────────────────────────────────
//
// Set as five rows against their own annotations rather than as one long line,
// because that is how it is read: a left-hand side, and then five things that
// happen to a box of gas, each of which is a file. The signs align down the
// left, which is the only reason the column is there.

const terms: readonly [ReactNode, string][] = [
  [
    <mrow key="work">
      <mo>−</mo>
      <mi>p</mi>
      <Times />
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
          <Rm>d</Rm>
          <Times />
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
          <Rm>d</Rm>
          <Times />
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
      <mo largeop="true">∑</mo>
      <mfrac>
        <mrow>
          <Rm>d</Rm>
          <Times />
          <msub>
            <mi>m</mi>
            <mi>i</mi>
          </msub>
        </mrow>
        {dTheta}
      </mfrac>
      <Times />
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
      <Times />
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
              <Times />
              <msub>
                <mi>c</mi>
                <mtext>v</mtext>
              </msub>
              <Times />
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
    <Equation note="s is how far the piston has come down from top dead centre; a is the crank throw, l the rod, A the bore area, and Vc the clearance volume.">
      <math display="block">
        <mrow>
          <mi>s</mi>
          <Of />
          <mo stretchy="false">(</mo>
          <mi>θ</mi>
          <mo stretchy="false">)</mo>
          <mo>=</mo>
          <mo stretchy="false">(</mo>
          <mi>a</mi>
          <mo>+</mo>
          <mi>l</mi>
          <mo stretchy="false">)</mo>
          <mo>−</mo>
          <mi>a</mi>
          <Times />
          <mi mathvariant="normal">cos</mi>
          <Of />
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
              <Times />
              <SinSquared />
            </mrow>
          </msqrt>
        </mrow>
      </math>
      <math display="block">
        <mrow>
          <mi>V</mi>
          <Of />
          <mo stretchy="false">(</mo>
          <mi>θ</mi>
          <mo stretchy="false">)</mo>
          <mo>=</mo>
          <msub>
            <mi>V</mi>
            <mtext>c</mtext>
          </msub>
          <mo>+</mo>
          <mi>A</mi>
          <Times />
          <mi>s</mi>
          <Of />
          <mo stretchy="false">(</mo>
          <mi>θ</mi>
          <mo stretchy="false">)</mo>
        </mrow>
      </math>
    </Equation>
  );
}

// ── Woschni, and why it is the one exception to the SI rule ─────────────────

export function Woschni() {
  const power = (base: string, exponent: string) => (
    <>
      <msup>
        <mi>{base}</mi>
        <mrow>
          {exponent.startsWith('−') ? <mo>−</mo> : null}
          <mn>{exponent.replace('−', '')}</mn>
        </mrow>
      </msup>
      <Times />
    </>
  );
  return (
    <Equation note="B in metres, p in KILOPASCALS, T in kelvin, w in metres per second — and it returns watts per square metre kelvin.">
      <math display="block">
        <mrow>
          <mi>h</mi>
          <mo>=</mo>
          <mn>3.26</mn>
          <Times />
          {power('B', '−0.2')}
          {power('p', '0.8')}
          {power('T', '−0.55')}
          <msup>
            <mi>w</mi>
            <mn>0.8</mn>
          </msup>
        </mrow>
      </math>
    </Equation>
  );
}

// ── Livengood–Wu ────────────────────────────────────────────────────────────

export function LivengoodWu() {
  return (
    <Equation note="τ is the ignition delay at the pressure and temperature of that instant, so the end gas spends a little of its patience at every one of them.">
      <math display="block">
        <mrow>
          <munderover>
            <mo largeop="true">∫</mo>
            <mn>0</mn>
            <msub>
              <mi>t</mi>
              <mtext>ign</mtext>
            </msub>
          </munderover>
          <mfrac>
            <mrow>
              <Rm>d</Rm>
              <Times />
              <mi>t</mi>
            </mrow>
            <mrow>
              <mi>τ</mi>
              <Of />
              <mo stretchy="false">(</mo>
              <mi>p</mi>
              <mo separator="true">,</mo>
              <mi>T</mi>
              <mo stretchy="false">)</mo>
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
  // Display maths is indented the way a quotation is. It is a held-out thing,
  // and holding it out is most of what makes it readable.
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
    paddingInlineStart: space.md,
    rowGap: space.sm,
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
    paddingBlockEnd: space.xs,
    paddingInlineStart: space.md,
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
    paddingInlineStart: space.line,
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
    maxWidth: '28rem',
    paddingBlockStart: space.xs,
    paddingInlineStart: space.md,
    textWrap: 'pretty',
  },
});
