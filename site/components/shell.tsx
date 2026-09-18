'use client';

import * as stylex from '@stylexjs/stylex';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import type { ReactNode } from 'react';

import { color, font, layout, leading, size, space } from '@/design/tokens.stylex';

// The frame every page hangs in: a name, four ways out of here, and a rule at
// the bottom. A site about a program that has one job should not need a menu.
const pages = [
  { href: '/crankshaft', label: 'the argument' },
  { href: '/engine', label: 'the engine' },
  { href: '/instruments', label: 'the instruments' },
  { href: '/evidence', label: 'the evidence' },
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
        </div>
      </header>

      <main {...stylex.props(s.main)}>{children}</main>

      <footer {...stylex.props(s.footer)}>
        <div {...stylex.props(s.bar)}>
          <span>
            Every figure here is a copy of something the program prints, and
            copies rot. <code {...stylex.props(s.code)}>./windsor verify</code>{' '}
            checks them.
          </span>
          <span {...stylex.props(s.colophon)}>
            <span>Oddur Sigurdsson</span>
            <span aria-hidden>·</span>
            <a href={source} {...stylex.props(s.link)}>
              source
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
  },
  // One column, centred, the same width everywhere. The header sits over the
  // text rather than across the window, so the page reads as a single sheet.
  bar: {
    alignItems: 'baseline',
    columnGap: space.lg,
    display: 'flex',
    flexWrap: 'wrap',
    justifyContent: 'space-between',
    marginInline: 'auto',
    maxWidth: layout.column,
    paddingInline: layout.gutter,
    rowGap: space.sm,
    width: '100%',
  },
  header: {
    borderBottomColor: color.rule,
    borderBottomStyle: 'solid',
    borderBottomWidth: 1,
    paddingBlock: space.lg,
  },
  wordmark: {
    color: color.ink,
    fontSize: size.small,
    fontWeight: 500,
    letterSpacing: '0.01em',
    textDecoration: 'none',
  },
  nav: {
    columnGap: space.lg,
    display: 'flex',
    flexWrap: 'wrap',
    rowGap: space.xs,
  },
  link: {
    color: { default: color.muted, ':hover': color.ink },
    fontSize: size.tiny,
    textDecorationColor: { default: color.rule, ':hover': color.muted },
    textDecorationLine: 'underline',
    textDecorationThickness: '1px',
    textUnderlineOffset: '0.2em',
    transitionDuration: '120ms',
    transitionProperty: 'color',
  },
  here: {
    color: color.ink,
    textDecorationColor: color.faint,
  },
  main: {
    flex: 1,
    paddingBlockEnd: space.page,
    paddingBlockStart: space.xxl,
  },
  footer: {
    borderTopColor: color.rule,
    borderTopStyle: 'solid',
    borderTopWidth: 1,
    color: color.faint,
    fontSize: size.micro,
    lineHeight: leading.snug,
    paddingBlock: space.xl,
  },
  colophon: {
    columnGap: space.sm,
    display: 'flex',
  },
  code: {
    fontFamily: font.mono,
    fontSize: '0.95em',
  },
});
