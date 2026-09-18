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

export function Cmd({ children }: { children: ReactNode }) {
  return <pre {...stylex.props(s.cmd)}>{children}</pre>;
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
    paddingBlockEnd: space.xl,
  },
  title: {
    fontSize: size.head,
    fontWeight: 600,
    letterSpacing: '-0.01em',
    lineHeight: leading.tight,
    margin: 0,
  },
  sub: {
    color: color.muted,
    fontSize: size.base,
    lineHeight: leading.snug,
    marginBlock: space.sm,
    maxWidth: '32rem',
  },
  section: {
    borderTopColor: color.rule,
    borderTopStyle: 'solid',
    borderTopWidth: 1,
    marginBlockStart: space.section,
    paddingBlockStart: space.lg,
  },
  heading: {
    alignItems: 'baseline',
    display: 'flex',
    fontSize: size.base,
    fontWeight: 600,
    lineHeight: leading.snug,
    marginBlock: 0,
  },
  // The number is set in the margin where there is room for it and folded back
  // into the line where there is not.
  index: {
    color: color.faint,
    flexShrink: 0,
    fontSize: size.micro,
    fontWeight: 400,
    fontVariantNumeric: 'tabular-nums',
    marginInlineStart: {
      default: '-1.75rem',
      '@media (max-width: 44rem)': 0,
    },
    paddingInlineEnd: {
      default: 0,
      '@media (max-width: 44rem)': space.sm,
    },
    width: {
      default: '1.75rem',
      '@media (max-width: 44rem)': 'auto',
    },
  },
  p: {
    marginBlock: space.lg,
  },
  lede: {
    color: color.ink,
    fontSize: size.lead,
  },
  cmd: {
    backgroundColor: color.raised,
    borderColor: color.rule,
    borderRadius: 2,
    borderStyle: 'solid',
    borderWidth: 1,
    fontFamily: font.mono,
    fontSize: size.tiny,
    lineHeight: leading.snug,
    marginBlock: space.lg,
    overflowX: 'auto',
    paddingBlock: space.md,
    paddingInline: space.lg,
  },
  code: {
    fontFamily: font.mono,
    fontSize: '0.92em',
    wordBreak: 'break-word',
  },
  note: {
    borderInlineStartColor: color.rule,
    borderInlineStartStyle: 'solid',
    borderInlineStartWidth: 1,
    color: color.muted,
    fontSize: size.small,
    lineHeight: leading.snug,
    marginBlock: space.xl,
    paddingInlineStart: space.lg,
  },
  quote: {
    borderInlineStartColor: color.faint,
    borderInlineStartStyle: 'solid',
    borderInlineStartWidth: 1,
    fontSize: size.base,
    marginBlock: space.xl,
    marginInline: 0,
    paddingInlineStart: space.lg,
  },
  a: {
    color: color.ink,
    textDecorationColor: { default: color.faint, ':hover': color.ink },
    textDecorationLine: 'underline',
    textDecorationThickness: '1px',
    textUnderlineOffset: '0.2em',
  },
  figure: {
    marginBlock: space.xl,
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
    paddingBlock: space.sm,
    paddingInline: 0,
    textAlign: 'start',
    whiteSpace: 'nowrap',
  },
  td: {
    borderBottomColor: color.rule,
    borderBottomStyle: 'solid',
    borderBottomWidth: 1,
    paddingBlock: space.sm,
    paddingInline: 0,
    verticalAlign: 'baseline',
  },
  right: {
    paddingInlineStart: space.lg,
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
    paddingBlockStart: space.sm,
  },
  dl: {
    marginBlock: space.xl,
  },
  dlRow: {
    borderTopColor: color.rule,
    borderTopStyle: 'solid',
    borderTopWidth: 1,
    display: 'grid',
    columnGap: space.lg,
    gridTemplateColumns: {
      default: '8.5rem 1fr',
      '@media (max-width: 34rem)': '1fr',
    },
    paddingBlock: space.md,
  },
  dt: {
    color: color.ink,
    fontSize: size.small,
    lineHeight: leading.snug,
  },
  dtCode: {
    fontFamily: font.mono,
    fontSize: size.tiny,
    lineHeight: leading.prose,
  },
  dd: {
    color: color.muted,
    fontSize: size.small,
    lineHeight: leading.snug,
    marginInlineStart: 0,
  },
});
