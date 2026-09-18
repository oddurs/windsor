import * as stylex from '@stylexjs/stylex';
import type { ReactNode } from 'react';

import { color, font, layout, leading, size, space } from '@/design/tokens.stylex';

// Two readers, and neither of them invents anything.
//
// The first walks C++ and marks what the language already distinguishes. The
// second reads the escape codes the instruments print and maps them onto the
// page's own greys — so the emphasis you see here is the emphasis the program
// chose in a terminal, not a decoration applied afterwards.

type Kind =
  | 'comment'
  | 'string'
  | 'number'
  | 'keyword'
  | 'directive'
  | 'punctuation'
  | 'plain';

const keywords = new Set([
  'alignas', 'auto', 'bool', 'break', 'case', 'char', 'class', 'const',
  'consteval', 'constexpr', 'continue', 'default', 'delete', 'do', 'double',
  'else', 'enum', 'explicit', 'export', 'extern', 'false', 'float', 'for',
  'friend', 'if', 'inline', 'int', 'long', 'namespace', 'noexcept', 'nullptr',
  'operator', 'private', 'protected', 'public', 'return', 'short', 'signed',
  'sizeof', 'static', 'struct', 'switch', 'template', 'this', 'throw', 'true',
  'try', 'typename', 'union', 'unsigned', 'using', 'virtual', 'void', 'while',
]);

const rules: readonly (readonly [Kind, RegExp])[] = [
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

  while (rest.length > 0) {
    let taken = false;
    for (const [kind, pattern] of rules) {
      const hit = pattern.exec(rest);
      if (!hit) continue;
      const word = hit[0];
      add(kind === 'plain' && keywords.has(word) ? 'keyword' : kind, word);
      rest = rest.slice(word.length);
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

// ── What the instruments printed ────────────────────────────────────────────
//
// Primary series in ink, secondary in grey, agreement in patina, scaffolding
// faint. The program said red and yellow for the curve it wants you to read
// first, blue and cyan for the one underneath it, green where they agree.

type Ink = { bold: boolean; dim: boolean; hue: number | null };

const paint = (hue: number | null) => {
  if (hue === 31 || hue === 33) return s.lead;
  if (hue === 34 || hue === 36) return s.second;
  if (hue === 32) return s.agree;
  if (hue === 90) return s.dim;
  return undefined;
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
            {readAnsi(children).map((run, i) => (
              <span
                key={i}
                {...stylex.props(
                  s.token,
                  run.ink.bold && s.bold,
                  run.ink.dim && s.dim,
                  paint(run.ink.hue),
                )}
              >
                {run.text}
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
    marginBlock: space.xl,
    marginInline: 0,
  },
  // Terminal output is eighty columns wide and the prose is sixty-six, so the
  // output steps outside the column — but only once the window is wide enough
  // to lend it the room, and never by a viewport unit. `100vw` includes the
  // scrollbar, so a bleed measured that way is a few pixels wider than the
  // page and every one of these figures pushes the document sideways.
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
    backgroundColor: color.raised,
    borderColor: color.rule,
    borderRadius: 2,
    borderStyle: 'solid',
    borderWidth: 1,
    fontFamily: font.mono,
    fontSize: size.tiny,
    lineHeight: leading.snug,
    margin: 0,
    overflowX: 'auto',
    paddingBlock: space.md,
    paddingInline: space.lg,
  },
  terminal: {
    fontSize: size.micro,
    lineHeight: '1.4',
  },
  token: {
    color: color.ink,
  },
  comment: { color: color.faint },
  string: { color: color.muted },
  number: { color: color.patina },
  keyword: { color: color.ink, fontWeight: 600 },
  directive: { color: color.muted },
  punctuation: { color: color.muted },
  plain: { color: color.ink },
  bold: { color: color.ink, fontWeight: 600 },
  dim: { color: color.faint },
  lead: { color: color.ink },
  second: { color: color.muted },
  agree: { color: color.patina },
  caption: {
    alignItems: 'baseline',
    columnGap: space.md,
    display: 'flex',
    flexWrap: 'wrap',
    paddingBlockStart: space.sm,
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
