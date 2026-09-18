'use client';

import * as stylex from '@stylexjs/stylex';
import { useEffect, useRef, useState } from 'react';

import { color, font, leading, size, space } from '@/design/tokens.stylex';

// The claim is that one part decides the sound, so the control that matters is
// the crank and not the transport. Switching cranks keeps your place in the
// recording: the same nine seconds, the same engine, one part changed.

type Crank = 'cross' | 'flat';

const takes = {
  cross: {
    src: '/windsor.wav',
    label: 'cross-plane',
    command: './windsor record',
  },
  flat: {
    src: '/windsor-flat.wav',
    label: 'flat-plane',
    command: './windsor record --flat',
  },
} as const;

const clock = (seconds: number) => {
  const whole = Math.max(0, Math.floor(seconds));
  return `${Math.floor(whole / 60)}:${String(whole % 60).padStart(2, '0')}`;
};

export function Recording() {
  const cross = useRef<HTMLAudioElement>(null);
  const flat = useRef<HTMLAudioElement>(null);
  const [crank, setCrank] = useState<Crank>('cross');
  const [playing, setPlaying] = useState(false);
  const [at, setAt] = useState(0);
  const [length, setLength] = useState(0);

  const pick = (which: Crank) =>
    which === 'cross' ? cross.current : flat.current;

  // The transport position is read from the element rather than pushed to it,
  // because the element is the one that knows.
  useEffect(() => {
    if (!playing) return;
    let frame = 0;
    const tick = () => {
      const element = crank === 'cross' ? cross.current : flat.current;
      if (element) setAt(element.currentTime);
      frame = requestAnimationFrame(tick);
    };
    frame = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(frame);
  }, [playing, crank]);

  // Nothing is fetched until it is asked for. Once one take is playing the
  // other is worth having ready, because the whole point is switching between
  // them without a gap.
  useEffect(() => {
    if (!playing) return;
    const other = crank === 'cross' ? flat.current : cross.current;
    if (other && other.preload !== 'auto') {
      other.preload = 'auto';
      other.load();
    }
  }, [playing, crank]);

  const toggle = () => {
    const element = pick(crank);
    if (!element) return;
    if (playing) {
      element.pause();
      setPlaying(false);
    } else {
      void element.play();
      setPlaying(true);
    }
  };

  const fit = (next: Crank) => {
    if (next === crank) return;
    const from = pick(crank);
    const to = pick(next);
    setCrank(next);
    if (!from || !to) return;
    to.currentTime = from.currentTime;
    from.pause();
    if (playing) void to.play();
  };

  const ended = () => {
    setPlaying(false);
    setAt(0);
    const element = pick(crank);
    if (element) element.currentTime = 0;
  };

  const progress = length > 0 ? Math.min(1, at / length) : 0;

  return (
    <figure {...stylex.props(s.figure)}>
      <div {...stylex.props(s.panel)}>
        <div {...stylex.props(s.row)}>
          <button
            type="button"
            onClick={toggle}
            aria-label={playing ? 'Pause' : 'Play'}
            {...stylex.props(s.transport)}
          >
            {playing ? (
              <svg viewBox="0 0 10 10" width="9" height="9" aria-hidden>
                <path d="M1 0h3v10H1zM6 0h3v10H6z" fill="currentColor" />
              </svg>
            ) : (
              <svg viewBox="0 0 10 10" width="9" height="9" aria-hidden>
                <path d="M1 0l9 5-9 5z" fill="currentColor" />
              </svg>
            )}
          </button>

          <span {...stylex.props(s.time)}>
            {clock(at)} / {clock(length)}
          </span>

          <div {...stylex.props(s.cranks)}>
            {(Object.keys(takes) as Crank[]).map((which) => (
              <button
                key={which}
                type="button"
                onClick={() => fit(which)}
                aria-pressed={crank === which}
                {...stylex.props(s.crank, crank === which && s.fitted)}
              >
                {takes[which].label}
              </button>
            ))}
          </div>
        </div>

        <div {...stylex.props(s.track)}>
          <div
            {...stylex.props(s.run)}
            style={{ width: `${progress * 100}%` }}
          />
        </div>

        <audio
          ref={cross}
          src={takes.cross.src}
          preload="metadata"
          onEnded={ended}
          onLoadedMetadata={(event) =>
            setLength(event.currentTarget.duration || 0)
          }
        />
        <audio ref={flat} src={takes.flat.src} preload="metadata" onEnded={ended} />
      </div>
      <figcaption {...stylex.props(s.caption)}>
        {takes[crank].command}
      </figcaption>
    </figure>
  );
}

const s = stylex.create({
  figure: {
    marginBlock: space.xl,
    marginInline: 0,
  },
  panel: {
    backgroundColor: color.raised,
    borderColor: color.rule,
    borderRadius: 2,
    borderStyle: 'solid',
    borderWidth: 1,
    paddingBlock: space.md,
    paddingInline: space.lg,
  },
  row: {
    alignItems: 'center',
    columnGap: space.lg,
    display: 'flex',
    flexWrap: 'wrap',
    rowGap: space.sm,
  },
  transport: {
    alignItems: 'center',
    backgroundColor: { default: color.paper, ':hover': color.paper },
    borderColor: { default: color.rule, ':hover': color.ink },
    borderRadius: 2,
    borderStyle: 'solid',
    borderWidth: 1,
    color: color.ink,
    cursor: 'pointer',
    display: 'flex',
    height: '1.9rem',
    justifyContent: 'center',
    padding: 0,
    transitionDuration: '120ms',
    transitionProperty: 'border-color',
    width: '1.9rem',
  },
  time: {
    color: color.muted,
    fontSize: size.tiny,
    fontVariantNumeric: 'tabular-nums',
    lineHeight: leading.snug,
  },
  cranks: {
    columnGap: space.lg,
    display: 'flex',
    marginInlineStart: 'auto',
  },
  crank: {
    backgroundColor: 'transparent',
    borderStyle: 'none',
    color: { default: color.muted, ':hover': color.ink },
    cursor: 'pointer',
    fontFamily: 'inherit',
    fontSize: size.tiny,
    lineHeight: leading.snug,
    padding: 0,
    textDecorationColor: 'transparent',
    textDecorationLine: 'underline',
    textDecorationThickness: '1px',
    textUnderlineOffset: '0.25em',
    transitionDuration: '120ms',
    transitionProperty: 'color',
  },
  fitted: {
    color: color.ink,
    textDecorationColor: color.faint,
  },
  track: {
    backgroundColor: color.rule,
    height: 1,
    marginBlockStart: space.md,
    overflow: 'hidden',
  },
  run: {
    backgroundColor: color.ink,
    height: 1,
  },
  caption: {
    color: color.faint,
    fontFamily: font.mono,
    fontSize: size.micro,
    paddingBlockStart: space.sm,
  },
});
