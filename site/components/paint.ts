// What the instruments print is a user interface, and it was written as one:
// label columns, value columns, units, rules, section headings, plot chrome.
// The program marks a little of that with escape codes — a heading, a passing
// check, which curve is which — and leaves the rest to a terminal's single
// colour, because a terminal has one.
//
// This page does not. So the structure the output already has gets read back
// out of it and shown, under rules that hold everywhere rather than wherever
// they happened to look good:
//
//   a number            amber      the data
//   its unit            fog        never the data
//   a parenthetical     fog        an aside is an aside
//   plot and rule glyph fog        chrome, not content
//   a timing mark       sky        IVO, BDC — the engine's own vocabulary
//   SHOUTED             bright     the output shouts rarely and means it
//   everything else     text
//
// Where the program did choose a colour, its choice stands and none of this
// runs. It knows what it meant; this is only for what it had no way to say.

export type Paint =
  | 'number'
  | 'unit'
  | 'aside'
  | 'chrome'
  | 'timing'
  | 'shout'
  | 'punctuation'
  | 'text';

const units = new Set([
  'bar', 'cc', 'cfm', 'ci', 'cu', 'deg', 'ft', 'g', 'hp', 'hz', 'in', 'k',
  'khz', 'kpa', 'kw', 'l', 'lb', 'lb-ft', 'm', 'm/s', 'mm', 'ms', 'n', 'nm',
  'psi', 'rpm', 's', 'w',
]);

// The four-stroke vocabulary. These are events on a crank, not words, and the
// output uses them as landmarks — so they are given a colour of their own.
const timing = new Set([
  'ABDC', 'ATDC', 'BBDC', 'BDC', 'BTDC', 'EVC', 'EVO', 'ICL', 'IVC', 'IVO',
  'LSA', 'TDC',
]);

const qualifiers = new Set(['rel', 'abs']);

const NUMBER = /^\d(?:[\d.,]|e[+-]?(?=\d)|-(?=\d))*\d|^\d/;
const WORD = /^[A-Za-z][A-Za-z0-9/\-]*/;
const ASIDE = /^\([^)]*\)/;
const CHROME = /^[|+]+|^-{3,}|^={3,}/;
const SPACE = /^\s+/;

export type Piece = { paint: Paint; text: string };

// A row of nothing but words, set in columns, sitting above rows of figures.
// It is a table header, and a table header is a label for the column and not a
// thing in it, so the whole row recedes.
export function isColumnHeader(plain: string) {
  if (/\d/.test(plain) || /[.,;]/.test(plain)) return false;
  const body = plain.replace(/^\s+/, '');
  if (body.length === 0) return false;
  const gaps = body.match(/\s{2,}/g);
  return (gaps?.length ?? 0) >= 2;
}

export function paint(text: string): Piece[] {
  const pieces: Piece[] = [];
  let rest = text;
  let previous: Paint | null = null;

  const add = (p: Paint, t: string) => {
    const last = pieces[pieces.length - 1];
    if (last && last.paint === p) last.text += t;
    else pieces.push({ paint: p, text: t });
    if (p !== 'text' || t.trim().length > 0) previous = p;
  };

  while (rest.length > 0) {
    let hit = SPACE.exec(rest);
    if (hit) {
      const last = pieces[pieces.length - 1];
      if (last) last.text += hit[0];
      else pieces.push({ paint: 'text', text: hit[0] });
      rest = rest.slice(hit[0].length);
      continue;
    }

    hit = ASIDE.exec(rest);
    if (hit) {
      add('aside', hit[0]);
      rest = rest.slice(hit[0].length);
      continue;
    }

    hit = CHROME.exec(rest);
    if (hit) {
      add('chrome', hit[0]);
      rest = rest.slice(hit[0].length);
      continue;
    }

    hit = NUMBER.exec(rest);
    if (hit) {
      add('number', hit[0]);
      rest = rest.slice(hit[0].length);
      continue;
    }

    hit = WORD.exec(rest);
    if (hit) {
      const word = hit[0];
      const lower = word.toLowerCase();
      // A unit is only a unit when it follows a figure, which is what stops
      // `in` the preposition and `N` the letter from going grey in prose.
      const measuring = previous === 'number' || previous === 'unit';
      if (word === 'x' && measuring) add('punctuation', word);
      else if (measuring && units.has(lower)) add('unit', word);
      else if (qualifiers.has(lower)) add('unit', word);
      else if (timing.has(word)) add('timing', word);
      else if (word.length >= 3 && word === word.toUpperCase() && /[A-Z]{3}/.test(word))
        add('shout', word);
      else add('text', word);
      rest = rest.slice(word.length);
      continue;
    }

    add('punctuation', rest.slice(0, 1));
    rest = rest.slice(1);
  }

  return pieces;
}
