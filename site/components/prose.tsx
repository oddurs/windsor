import * as stylex from '@stylexjs/stylex';
import type { ReactNode } from 'react';

import { color, font, layout, leading, size, space } from '@/design/tokens.stylex';

// Every page is built from these and nothing else. They are deliberately few:
// a heading, a paragraph, a rule, a table, a command, an aside. A page that
// needs a seventh shape usually needs a shorter argument instead.

export function Article({ children }: { children: ReactNode }) {
  return <article {...stylex.props(s.article)}>{children}</article>;
}

export function Title({
  children,
  sub,
}: {
  children: ReactNode;
  sub?: ReactNode;
}) {
  return (
    <header {...stylex.props(s.masthead)}>
      <h1 {...stylex.props(s.title)}>{children}</h1>
      {sub ? <p {...stylex.props(s.sub)}>{sub}</p> : null}
    </header>
  );
}

// Sections are numbered in the margin the way a manual numbers them: it tells
// you where you are without a heading having to raise its voice.
export function Section({
  index,
  heading,
  children,
}: {
  index: string;
  heading: string;
  children: ReactNode;
}) {
  return (
    <section {...stylex.props(s.section)}>
      <h2 {...stylex.props(s.heading)}>
        <span {...stylex.props(s.index)}>{index}</span>
        {heading}
      </h2>
      {children}
    </section>
  );
}

export function P({ children }: { children: ReactNode }) {
  return <p {...stylex.props(s.p)}>{children}</p>;
}

export function Lede({ children }: { children: ReactNode }) {
  return <p {...stylex.props(s.p, s.lede)}>{children}</p>;
}

export function Code({ children }: { children: ReactNode }) {
  return <code {...stylex.props(s.code)}>{children}</code>;
}

export function Note({ children }: { children: ReactNode }) {
  return <aside {...stylex.props(s.note)}>{children}</aside>;
}

export function Quote({ children }: { children: ReactNode }) {
  return <blockquote {...stylex.props(s.quote)}>{children}</blockquote>;
}

export function A({ href, children }: { href: string; children: ReactNode }) {
  return (
    <a href={href} {...stylex.props(s.a)}>
      {children}
    </a>
  );
}

// Figures are quoted with the command that prints them. The number on the page
// is a copy; the command is the original, and anyone can run it.
export function Table({
  head,
  rows,
  from,
}: {
  head: readonly ReactNode[];
  rows: readonly (readonly ReactNode[])[];
  from?: string;
}) {
  return (
    <figure {...stylex.props(s.figure)}>
      <div {...stylex.props(s.scroll)}>
        <table {...stylex.props(s.table)}>
          <thead>
            <tr>
              {head.map((cell, i) => (
                <th key={i} {...stylex.props(s.th, i > 0 && s.right)}>
                  {cell}
                </th>
              ))}
            </tr>
          </thead>
          <tbody>
            {rows.map((row, r) => (
              <tr key={r}>
                {row.map((cell, i) => (
                  <td
                    key={i}
                    {...stylex.props(s.td, i > 0 && s.right, i > 0 && s.num)}
                  >
                    {cell}
                  </td>
                ))}
              </tr>
            ))}
          </tbody>
        </table>
      </div>
      {from ? <figcaption {...stylex.props(s.caption)}>{from}</figcaption> : null}
    </figure>
  );
}

// A two-column list of a term and what it is. Used for the file manifest, where
// the names are the argument and the descriptions are the footnotes.
export function Terms({
  items,
  code = false,
}: {
  items: readonly { term: string; body: ReactNode }[];
  code?: boolean;
}) {
  return (
    <dl {...stylex.props(s.dl)}>
      {items.map((item) => (
        <div key={item.term} {...stylex.props(s.dlRow)}>
          <dt {...stylex.props(s.dt, code && s.dtCode)}>{item.term}</dt>
          <dd {...stylex.props(s.dd)}>{item.body}</dd>
        </div>
      ))}
    </dl>
  );
}

const s = stylex.create({
  article: {
    marginInline: 'auto',
    maxWidth: layout.column,
    paddingInline: layout.gutter,
  },
  masthead: {
    paddingBlockEnd: space.lg,
  },
  // The one place the scale is allowed to jump. Everything else on the page
  // sits inside a single octave; this does not, and that is what makes the top
  // of a page read as the top of a page.
  title: {
    fontSize: size.head,
    fontWeight: 600,
    letterSpacing: '-0.018em',
    lineHeight: leading.tight,
    margin: 0,
    textWrap: 'balance',
  },
  sub: {
    color: color.muted,
    fontSize: size.lead,
    lineHeight: leading.snug,
    marginBlockEnd: 0,
    marginBlockStart: space.sm,
    maxWidth: '30rem',
    textWrap: 'pretty',
  },
  section: {
    borderTopColor: color.rule,
    borderTopStyle: 'solid',
    borderTopWidth: 1,
    marginBlockStart: space.section,
    paddingBlockStart: space.md,
  },
  heading: {
    alignItems: 'baseline',
    display: 'flex',
    fontSize: size.lead,
    fontWeight: 600,
    letterSpacing: '-0.006em',
    lineHeight: leading.snug,
    marginBlock: 0,
    textWrap: 'balance',
  },
  // The number sits in the margin where there is room for it and folds back
  // into the line where there is not.
  index: {
    color: color.faint,
    flexShrink: 0,
    fontSize: size.micro,
    fontVariantNumeric: 'tabular-nums',
    fontWeight: 400,
    marginInlineStart: {
      default: '-1.75rem',
      '@media (max-width: 44rem)': 0,
    },
    paddingInlineEnd: {
      default: 0,
      '@media (max-width: 44rem)': space.xs,
    },
    width: {
      default: '1.75rem',
      '@media (max-width: 44rem)': 'auto',
    },
  },
  // One line between paragraphs, because the line is what the page is measured
  // in. Anything less and the block sets solid; anything more and it drifts.
  p: {
    marginBlock: space.line,
    textWrap: 'pretty',
  },
  lede: {
    color: color.ink,
    fontSize: size.lead,
    letterSpacing: '-0.004em',
  },
  code: {
    fontFamily: font.mono,
    fontSize: '0.9em',
    wordBreak: 'break-word',
  },
  note: {
    borderInlineStartColor: color.rule,
    borderInlineStartStyle: 'solid',
    borderInlineStartWidth: 2,
    color: color.muted,
    fontSize: size.small,
    lineHeight: leading.snug,
    marginBlock: space.lg,
    paddingInlineStart: space.md,
    textWrap: 'pretty',
  },
  quote: {
    borderInlineStartColor: color.faint,
    borderInlineStartStyle: 'solid',
    borderInlineStartWidth: 2,
    fontSize: size.base,
    lineHeight: leading.snug,
    marginBlock: space.lg,
    marginInline: 0,
    paddingInlineStart: space.md,
    textWrap: 'pretty',
  },
  a: {
    color: color.ink,
    textDecorationColor: { default: color.faint, ':hover': color.ink },
    textDecorationLine: 'underline',
    textDecorationThickness: '1px',
    textUnderlineOffset: '0.18em',
    transitionDuration: '120ms',
    transitionProperty: 'text-decoration-color',
  },
  figure: {
    marginBlock: space.lg,
    marginInline: 0,
  },
  scroll: {
    overflowX: 'auto',
  },
  table: {
    borderCollapse: 'collapse',
    fontSize: size.small,
    lineHeight: leading.snug,
    width: '100%',
  },
  th: {
    borderBottomColor: color.ink,
    borderBottomStyle: 'solid',
    borderBottomWidth: 1,
    color: color.muted,
    fontSize: size.micro,
    fontWeight: 400,
    letterSpacing: '0.02em',
    paddingBlock: space.xs,
    paddingInline: 0,
    textAlign: 'start',
    whiteSpace: 'nowrap',
  },
  td: {
    borderBottomColor: color.hairline,
    borderBottomStyle: 'solid',
    borderBottomWidth: 1,
    paddingBlock: space.sm,
    paddingInline: 0,
    verticalAlign: 'baseline',
  },
  right: {
    paddingInlineStart: space.md,
    textAlign: 'end',
  },
  num: {
    fontVariantNumeric: 'tabular-nums',
    whiteSpace: 'nowrap',
  },
  caption: {
    color: color.faint,
    fontFamily: font.mono,
    fontSize: size.micro,
    paddingBlockStart: space.xs,
  },
  dl: {
    marginBlock: space.lg,
  },
  dlRow: {
    borderTopColor: color.hairline,
    borderTopStyle: 'solid',
    borderTopWidth: 1,
    columnGap: space.md,
    display: 'grid',
    gridTemplateColumns: {
      default: '9rem 1fr',
      '@media (max-width: 34rem)': '1fr',
    },
    paddingBlock: space.sm,
  },
  dt: {
    color: color.ink,
    fontSize: size.small,
    lineHeight: leading.snug,
  },
  dtCode: {
    fontFamily: font.mono,
    fontSize: size.tiny,
    lineHeight: '1.6',
  },
  dd: {
    color: color.muted,
    fontSize: size.small,
    lineHeight: leading.snug,
    marginInlineStart: 0,
    textWrap: 'pretty',
  },
});
