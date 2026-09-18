// MathML, declared once so the pages can be written in it.
//
// Every browser has rendered MathML natively for years, which makes a
// typesetting library the wrong answer here: it would be the only dependency
// on the site that the platform already ships, and the heaviest.
//
// React's types do not cover these elements, so this is the whole cost of
// using them.
import type { DetailedHTMLProps, HTMLAttributes } from 'react';

type MathMLProps = DetailedHTMLProps<HTMLAttributes<HTMLElement>, HTMLElement>;

declare module 'react' {
  namespace JSX {
    interface IntrinsicElements {
      math: MathMLProps & { display?: 'block' | 'inline' };
      mfrac: MathMLProps;
      mi: MathMLProps & { mathvariant?: string };
      mn: MathMLProps;
      mo: MathMLProps & { stretchy?: 'true' | 'false' };
      mrow: MathMLProps;
      msqrt: MathMLProps;
      mspace: MathMLProps & { width?: string };
      msub: MathMLProps;
      msup: MathMLProps;
      mtext: MathMLProps;
      munderover: MathMLProps;
    }
  }
}
