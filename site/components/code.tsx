import * as stylex from '@stylexjs/stylex';
import type { ReactNode } from 'react';

import { color, font, size, space, term } from '@/design/tokens.stylex';

import { isColumnHeader, paint } from './paint';

// Two readers, and neither of them invents anything.
//
// The first walks C++ and marks what the language already distinguishes. The
// second reads the escape codes the instruments print and maps them onto the
// same palette — so the emphasis on this page is the emphasis the program chose
// in a terminal, translated rather than decorated.

type Kind =
  | 'comment'
  | 'string'
  | 'number'
  | 'keyword'
  | 'type'
  | 'scope'
  | 'call'
  | 'directive'
  | 'punctuation'
  | 'plain';

// Control flow, declarators, storage. These are the words that are about the
// program rather than about the engine.
const keywords = new Set([
  'alignas', 'break', 'case', 'catch', 'class', 'const', 'consteval',
  'constexpr', 'continue', 'default', 'delete', 'do', 'else', 'enum',
  'explicit', 'export', 'extern', 'false', 'for', 'friend', 'if', 'inline',
  'namespace', 'new', 'noexcept', 'nullptr', 'operator', 'private',
  'protected', 'public', 'requires', 'return', 'sizeof', 'static',
  'static_assert', 'struct', 'switch', 'template', 'this', 'throw', 'true',
  'try', 'typename', 'union', 'using', 'virtual', 'while',
]);

// The built-in types, which in this project are outnumbered by the ones with
// names — Bore, Stroke, Lobe, Crankshaft — and those are caught by their case.
const builtins = new Set([
  'auto', 'bool', 'char', 'double', 'float', 'int', 'long', 'short',
  'signed', 'size_t', 'unsigned', 'void',
]);

const patterns: readonly (readonly [Kind, RegExp])[] = [
  ['comment', /^\/\/[^\n]*/],
  ['comment', /^\/\*[\s\S]*?\*\//],
  ['string', /^R"\([\s\S]*?\)"/],
  ['string', /^"(?:\\.|[^"\\])*"/],
  ['string', /^'(?:\\.|[^'\\])*'/],
  ['directive', /^#\w+/],
  ['number', /^\d[\w.]*/],
  ['plain', /^[A-Za-z_]\w*/],
  ['plain', /^\s+/],
];

function scan(source: string) {
  const pieces: { kind: Kind; text: string }[] = [];
  let rest = source;

  const add = (kind: Kind, text: string) => {
    const last = pieces[pieces.length - 1];
    if (last && last.kind === kind) last.text += text;
    else pieces.push({ kind, text });
  };

  // What an identifier is depends on what follows it, which is the whole of
  // the grammar this needs: a name before `::` names a scope, a name before `(`
  // is being called, a name in Pascal case is a type because that is the
  // convention the source keeps without exception.
  const classify = (word: string, after: string): Kind => {
    if (keywords.has(word)) return 'keyword';
    if (builtins.has(word)) return 'type';
    if (after.startsWith('::')) return 'scope';
    if (/^\s*\(/.test(after)) return 'call';
    if (/^[A-Z]/.test(word)) return 'type';
    return 'plain';
  };

  while (rest.length > 0) {
    let taken = false;
    for (const [kind, pattern] of patterns) {
      const hit = pattern.exec(rest);
      if (!hit) continue;
      const word = hit[0];
      const next = rest.slice(word.length);
      add(kind === 'plain' && /^[A-Za-z_]/.test(word) ? classify(word, next) : kind, word);
      rest = next;
      taken = true;
      break;
    }
    if (!taken) {
      add('punctuation', rest.slice(0, 1));
      rest = rest.slice(1);
    }
  }
  return pieces;
}

export function Source({ children }: { children: string }) {
  return (
    <figure {...stylex.props(s.figure)}>
      <pre {...stylex.props(s.block)}>
        <code>
          {scan(children).map((piece, i) => (
            <span key={i} {...stylex.props(s.token, s[piece.kind])}>
              {piece.text}
            </span>
          ))}
        </code>
      </pre>
    </figure>
  );
}

// A shell command is a terminal too, so it gets the same panel and none of the
// colouring — nothing in it has been parsed, and pretending otherwise would be
// the first decorative thing on the site.
export function Cmd({ children }: { children: ReactNode }) {
  return (
    <figure {...stylex.props(s.figure)}>
      <pre {...stylex.props(s.block, s.shell)}>{children}</pre>
    </figure>
  );
}

// ── What the instruments printed ────────────────────────────────────────────

type Ink = { bold: boolean; dim: boolean; hue: number | null };

const hues: Record<number, keyof typeof s> = {
  30: 'fog',
  31: 'coral',
  32: 'mint',
  33: 'amber',
  34: 'sky',
  35: 'violet',
  36: 'cyan',
  37: 'plain',
  90: 'fog',
  91: 'coral',
  92: 'mint',
  93: 'amber',
  94: 'sky',
  95: 'violet',
  96: 'cyan',
  97: 'bright',
};

function readAnsi(source: string) {
  const runs: { text: string; ink: Ink }[] = [];
  const escape = /\u001b\[([0-9;]*)m/g;
  let ink: Ink = { bold: false, dim: false, hue: null };
  let at = 0;
  let found: RegExpExecArray | null;

  while ((found = escape.exec(source)) !== null) {
    if (found.index > at) runs.push({ text: source.slice(at, found.index), ink });
    ink = { ...ink };
    for (const part of (found[1] ?? '').split(';')) {
      const code = Number(part.length > 0 ? part : '0');
      if (code === 0) ink = { bold: false, dim: false, hue: null };
      else if (code === 1) ink.bold = true;
      else if (code === 2) ink.dim = true;
      else if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97)) ink.hue = code;
    }
    at = escape.lastIndex;
  }
  if (at < source.length) runs.push({ text: source.slice(at), ink });
  return runs;
}

const PAINT = {
  number: 'amber',
  unit: 'fog',
  aside: 'fog',
  chrome: 'fog',
  timing: 'sky',
  shout: 'shout',
  punctuation: 'punctuation',
  text: 'plain',
} as const;

const ANSI = /\u001b\[[0-9;]*m/;

function Row({ row }: { row: string }) {
  // A header row is a label for a column and not a thing in it, so it recedes
  // whole rather than word by word.
  if (!ANSI.test(row) && isColumnHeader(row)) {
    return <span {...stylex.props(s.token, s.fog)}>{row}</span>;
  }
  return (
    <>
      {readAnsi(row).map((run, i) => {
        const chosen = run.ink.hue !== null || run.ink.bold || run.ink.dim;
        if (chosen) {
          return (
            <span
              key={i}
              {...stylex.props(
                s.token,
                run.ink.hue !== null ? s[hues[run.ink.hue] ?? 'plain'] : undefined,
                run.ink.dim && s.fog,
                run.ink.bold && s.bold,
              )}
            >
              {run.text}
            </span>
          );
        }
        return paint(run.text).map((piece, j) => (
          <span key={`${i}-${j}`} {...stylex.props(s.token, s[PAINT[piece.paint]])}>
            {piece.text}
          </span>
        ));
      })}
    </>
  );
}

// Pull a window out of a captured run, so a page can quote the four lines that
// matter without reprinting a hundred and twenty.
export function lines(source: string, from: number, to: number) {
  return source.split('\n').slice(from - 1, to).join('\n');
}

export function Output({
  children,
  from,
  note,
}: {
  children: string;
  from: string;
  note?: ReactNode;
}) {
  return (
    <figure {...stylex.props(s.figure, s.bleed)}>
      <div {...stylex.props(s.centre)}>
        <pre {...stylex.props(s.block, s.terminal)}>
          <code>
            {children.split('\n').map((row, i, all) => (
              <span key={i}>
                <Row row={row} />
                {i < all.length - 1 ? '\n' : ''}
              </span>
            ))}
          </code>
        </pre>
        <figcaption {...stylex.props(s.caption)}>
          <span {...stylex.props(s.command)}>{from}</span>
          {note ? <span {...stylex.props(s.note)}>{note}</span> : null}
        </figcaption>
      </div>
    </figure>
  );
}

const s = stylex.create({
  figure: {
    marginBlock: space.lg,
    marginInline: 0,
  },
  // Terminal output is eighty columns wide and the prose is sixty-eight, so the
  // output steps outside the column — but only once the window is wide enough
  // to lend it the room, and never by a viewport unit. `100vw` includes the
  // scrollbar, so a bleed measured that way is a few pixels wider than the page
  // and every one of these figures pushes the document sideways.
  bleed: {
    marginInline: {
      default: 0,
      '@media (min-width: 60rem)': '-5.5rem',
    },
  },
  centre: {
    marginInline: 'auto',
    maxWidth: '47rem',
  },
  block: {
    backgroundColor: term.ground,
    borderColor: term.border,
    borderRadius: 4,
    borderStyle: 'solid',
    borderWidth: 1,
    color: term.text,
    fontFamily: font.mono,
    fontSize: size.tiny,
    lineHeight: '1.55',
    margin: 0,
    overflowX: 'auto',
    paddingBlock: space.sm,
    paddingInline: space.md,
    scrollbarColor: `${term.border} transparent`,
    scrollbarWidth: 'thin',
    '::selection': {
      backgroundColor: term.glow,
      color: term.bright,
    },
  },
  shell: {
    color: term.text,
  },
  terminal: {
    fontSize: size.micro,
    lineHeight: '1.4',
  },
  token: {
    color: term.text,
  },

  // C++
  comment: { color: term.fog },
  string: { color: term.mint },
  number: { color: term.amber },
  keyword: { color: term.violet },
  type: { color: term.cyan },
  scope: { color: term.cyan },
  call: { color: term.sky },
  directive: { color: term.coral },
  punctuation: { color: term.punctuation },

  // ANSI
  plain: { color: term.text },
  bright: { color: term.bright },
  bold: { color: term.bright, fontWeight: 600 },
  shout: { color: term.bright, fontWeight: 600 },
  fog: { color: term.fog },
  coral: { color: term.coral },
  mint: { color: term.mint },
  amber: { color: term.amber },
  sky: { color: term.sky },
  violet: { color: term.violet },
  cyan: { color: term.cyan },
  rose: { color: term.rose },

  caption: {
    alignItems: 'baseline',
    columnGap: space.sm,
    display: 'flex',
    flexWrap: 'wrap',
    paddingBlockStart: space.xs,
  },
  command: {
    color: color.faint,
    fontFamily: font.mono,
    fontSize: size.micro,
  },
  note: {
    color: color.faint,
    fontSize: size.micro,
  },
});
