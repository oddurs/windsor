'use client';

import * as stylex from '@stylexjs/stylex';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import type { ReactNode } from 'react';

import { color, font, layout, leading, size, space } from '@/design/tokens.stylex';

// The frame every page hangs in: a name, four ways through it, the source, and
// a rule at each end. A site about a program with one job should not need a
// menu, and these four are the argument in the order it is made — the claim,
// the machine, what it produces, and the proof.
const pages = [
  { href: '/crankshaft', label: 'argument' },
  { href: '/engine', label: 'engine' },
  { href: '/instruments', label: 'instruments' },
  { href: '/evidence', label: 'evidence' },
] as const;

const source = 'https://github.com/oddurs/windsor';

export function Shell({ children }: { children: ReactNode }) {
  const path = usePathname();

  return (
    <div {...stylex.props(s.frame)}>
      <header {...stylex.props(s.header)}>
        <div {...stylex.props(s.bar)}>
          {/* On the cover the name is the page's own heading, so the running
              head steps out of the way. The span holds the nav in place. */}
          {path === '/' ? (
            <span aria-hidden />
          ) : (
            <Link href="/" {...stylex.props(s.wordmark)}>
              windsor
            </Link>
          )}
          <nav {...stylex.props(s.nav)}>
            {pages.map((page) => (
              <Link
                key={page.href}
                href={page.href}
                aria-current={path === page.href ? 'page' : undefined}
                {...stylex.props(s.link, path === page.href && s.here)}
              >
                {page.label}
              </Link>
            ))}
          </nav>
          <a href={source} rel="noreferrer" {...stylex.props(s.link)}>
            github
          </a>
        </div>
      </header>

      <main {...stylex.props(s.main)}>{children}</main>

      <footer {...stylex.props(s.footer)}>
        <div {...stylex.props(s.bar, s.colophonBar)}>
          <span {...stylex.props(s.rot)}>
            Every figure here is a copy of something the program prints, and
            copies rot. <code {...stylex.props(s.code)}>./windsor verify</code>{' '}
            checks them.
          </span>
          <span {...stylex.props(s.colophon)}>
            <span>Oddur Sigurdsson</span>
            <span aria-hidden>·</span>
            <a href={source} rel="noreferrer" {...stylex.props(s.link)}>
              github
            </a>
            <span aria-hidden>·</span>
            <span>MIT</span>
          </span>
        </div>
      </footer>
    </div>
  );
}

const s = stylex.create({
  frame: {
    backgroundColor: color.paper,
    color: color.ink,
    display: 'flex',
    flexDirection: 'column',
    fontFamily: font.text,
    fontSize: size.base,
    lineHeight: leading.prose,
    minHeight: '100vh',   // not dvh: it resizes as mobile toolbars collapse
    '::selection': {
      backgroundColor: color.ink,
      color: color.paper,
    },
  },
  // One column, centred, the same width everywhere. The header sits over the
  // text rather than across the window, so the page reads as a single sheet.
  bar: {
    alignItems: 'baseline',
    columnGap: space.md,
    display: 'flex',
    flexWrap: 'wrap',
    justifyContent: 'space-between',
    marginInline: 'auto',
    maxWidth: layout.column,
    paddingInline: layout.gutter,
    rowGap: space.xs,
    width: '100%',
  },
  // No rule, no ground, no weight. A running head should be findable when it is
  // looked for and absent when it is not, and a line across the top of every
  // page is the loudest thing a site can own without meaning to.
  header: {
    paddingBlockEnd: space.md,
    paddingBlockStart: space.line,
  },
  wordmark: {
    color: color.ink,
    fontSize: size.small,
    fontWeight: 600,
    letterSpacing: '-0.01em',
    textDecoration: 'none',
  },
  nav: {
    columnGap: space.md,
    display: 'flex',
    flexWrap: 'wrap',
    marginInlineEnd: 'auto',
    marginInlineStart: {
      default: space.line,
      '@media (max-width: 34rem)': 0,
    },
    rowGap: space.hair,
  },
  // Colour is the only signal. An underline here would be a second one saying
  // the same thing, and where you are is worth exactly one.
  link: {
    color: { default: color.faint, ':hover': color.ink },
    fontSize: size.tiny,
    textDecorationLine: 'none',
    transitionDuration: '120ms',
    transitionProperty: 'color',
  },
  here: {
    color: color.ink,
  },
  main: {
    flex: 1,
    paddingBlockEnd: space.page,
    paddingBlockStart: space.lg,
  },
  footer: {
    borderTopColor: color.hairline,
    borderTopStyle: 'solid',
    borderTopWidth: 1,
    color: color.faint,
    fontSize: size.micro,
    lineHeight: leading.snug,
    paddingBlock: space.line,
  },
  colophonBar: {
    alignItems: 'baseline',
    rowGap: space.xs,
  },
  rot: {
    maxWidth: '26rem',
  },
  colophon: {
    columnGap: space.xs,
    display: 'flex',
    flexWrap: 'wrap',
  },
  code: {
    fontFamily: font.mono,
    fontSize: '0.95em',
  },
});
