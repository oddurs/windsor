import * as stylex from '@stylexjs/stylex';

import { Cmd, Output, Source, lines } from '@/components/code';
import {
  A,
  Article,
  Code,
  Lede,
  Note,
  P,
  Section,
  Table,
} from '@/components/prose';
import { Recording } from '@/components/recording';
import { output } from '@/content/output';
import { color, leading, size, space } from '@/design/tokens.stylex';

export default function Home() {
  return (
    <Article>
      <header {...stylex.props(s.masthead)}>
        <h1 {...stylex.props(s.name)}>windsor</h1>
        <p {...stylex.props(s.tagline)}>
          A Ford 302, modelled from first principles in C++23, for the sound.
        </p>
        <Cmd>make &amp;&amp; ./windsor spec</Cmd>
        <p {...stylex.props(s.terms)}>
          No dependencies. Nothing to configure. It is an engine, and it runs.
        </p>
      </header>

      <Section index="1" heading="Four numbers on a crankshaft">
        <Lede>
          I wanted to know why an American V8 sounds the way it does. Not well
          enough to describe it — well enough to point at the part.
        </Lede>
        <P>
          A cross-plane V8 burbles. A flat-plane V8 shrieks. Everyone knows
          this, almost nobody can say why, and for a long time I was
          comfortably one of them. The reason is not in the exhaust, the
          camshaft, the displacement or the fuel. It is four numbers on a
          crankshaft.
        </P>
        <P>
          Start with the thing that has to be given up first. A 90° V8 fires
          evenly on <em>either</em> crank — a power stroke every 90° of
          rotation, eight per cycle, perfectly spaced. At the flywheel the two
          engines keep identical time. So every explanation that begins
          &ldquo;because it fires unevenly&rdquo; is wrong about the part it is
          trying to explain, which I mention only because I used to give that
          explanation myself.
        </P>
        <P>
          An engine does not exhale through its flywheel. It exhales through
          two exhaust manifolds, and a manifold is not connected to an engine.
          It is connected to <em>four cylinders</em>, and it only ever hears
          those four.
        </P>
        <Table
          head={['', 'each bank hears']}
          rows={[
            ['cross-plane — throws at 0, 90, 180, 270', '90 · 180 · 270 · 180'],
            ['flat-plane — throws at 0, 180, 0, 180', '180 · 180 · 180 · 180'],
          ]}
          from="./windsor spec"
        />
        <P>
          That is the entire difference. A cross-plane bank coughs twice in
          quick succession, waits three quarters of a turn, and coughs again —
          a limping pulse train beating against the other bank&rsquo;s and
          never resolving. That is a Mustang idling outside a diner. A
          flat-plane bank exhales in even thirds of a revolution, both banks
          lock in phase, and the harmonics stack cleanly at twice the
          frequency. That is a 458 leaving a tunnel.
        </P>
        <P>
          Which gives the whole project a test it cannot argue its way out of.
          Change <em>one part</em>, and the sound should change and nothing
          else.
        </P>
        <Recording />
        <P>
          Press play, then change the crank while it is running. Same block,
          same cam, same nine seconds. The recording does not restart, because
          nothing about the engine restarted either.
        </P>
      </Section>

      <Section index="2" heading="I would not let myself type in the firing order">
        <P>
          1-5-4-2-6-3-7-8 is not a secret. It is cast into the intake manifold
          of every 302 Ford built in 1968. Typing it in would have taken a
          second, and it would have ended the project, because a program that
          has been told the answer cannot surprise you, and a model that cannot
          surprise you is a lookup table with opinions.
        </P>
        <P>
          So the source specifies a forging and nothing downstream of it: a
          bank angle, four journal throws, eight rods hung on them, and which
          revolution each rod fires on. That is what a foundry decides. The
          firing order is what falls out — which is why Ford and Chevrolet got
          different orders out of the same geometry.
        </P>
        <Source>{`Crankshaft{
    90.0_deg,
    { 45.0_deg, 315.0_deg, 135.0_deg, 225.0_deg },
    {{
    //  cylinder    journal   bank          revolution
        /* 1 */     { 0,      Bank::right,  0 },
        /* 2 */     { 1,      Bank::right,  0 },
        /* 3 */     { 2,      Bank::right,  1 },
        ...`}</Source>
        <P>
          It will refuse to build a crank that cannot exist, and then it prints
          a shop manual page with a rule drawn across the middle. Everything
          above the rule I typed. Everything below it, the program worked out.
        </P>
        <Output from="./windsor spec">{lines(output.spec, 21, 38)}</Output>
        <P>
          The first time that line appeared with the right eight numbers on it
          I sat looking at it for a while. I had not put them there.
        </P>
        <Note>
          Fit the 1982 H.O. camshaft to the identical forging — which is
          exactly what Ford did — and the order becomes 1-3-7-2-6-5-4-8, the
          351W order. Different order, <em>identical sound</em>, because the
          sound was never in the cam. <A href="/crankshaft">The argument →</A>
        </Note>
      </Section>

      <Section index="3" heading="What had to be built to hear it">
        <P>
          Between a crankshaft and a sound there is an engine, and it has to be
          honest or the test proves nothing. Each cylinder is an <em>open</em>{' '}
          thermodynamic system, integrated in crank angle, five terms of the
          first law per radian: the piston taking or giving work, the fire, the
          coolant, gas arriving through the valves, and that same gas as mass.
        </P>
        <P>
          Open rather than closed, because everything anybody cares about
          happens in the half of the cycle the air-standard Otto cycle deletes
          — the half where the valves are open and the mass is a variable.
        </P>
        <P>
          The exhaust is a digital waveguide. A pipe is two delay lines, one
          per direction, which is d&rsquo;Alembert&rsquo;s 1747 result sampled
          at 176 kHz instead of in the continuum. Junctions scatter by
          continuity of pressure and conservation of volume flow, and the loop
          is closed, so a header has a length that matters and not merely a
          note.
        </P>
        <P>
          None of which is worth anything unless the engine it adds up to
          behaves like the one Ford sold. So I put it on a brake.
        </P>
        <Output
          from="./windsor dyno"
          note="Ford: 210 hp at 4400, 295 lb-ft at 2400, gross."
        >
          {lines(output.dyno, 18, 44)}
        </Output>
        <P>
          Within two percent on power and four on torque, with nothing fitted
          to either. The curve is too broad — torque peaks 400 rpm low, power
          600 high — and I know exactly which missing part that is, because I
          built it three times.{' '}
          <A href="/evidence">Sixty-three checks, and what they check →</A>
        </P>
      </Section>

      <Section index="4" heading="What the burble cost">
        <P>
          If the flat crank breathes more evenly and sounds better, the obvious
          question is why Detroit forged the other one eight million times. The
          answer is in the same rod table, asked a different question.
        </P>
        <Output from="./windsor spec">{lines(output.spec, 44, 66)}</Output>
        <P>
          Nothing bolted to a shaft turning at ω can oppose a force that goes
          at 2ω. The cross-plane&rsquo;s secondaries cancel outright and its
          primary couple traces a circle, which a counterweight can be drawn to
          oppose. On a flat crank the secondaries trace a line — half a tonne
          of force, a hundred times a second — and a line is not something you
          can chase with a rotating weight.
        </P>
        <P>
          The flat crank sounds better and shakes. That is the bill, and it is
          why almost nobody pays it.
        </P>
      </Section>

      <Section index="5" heading="Four things I built and took out">
        <P>
          A nonlinear gas solver, verified against Sod&rsquo;s shock tube to
          one part in 10⁵, made header length worth 14% of torque — and
          collapsed the difference between the two crankshafts from 118× to 1×.
          An engine that cannot tell the two cranks apart is of no use to me,
          however good its shocks are. Out.
        </P>
        <P>
          Intake runner tuning I built three times and shipped none. The third
          attempt found something better than a feature: a torque-controlled
          brake sitting on a <em>rising</em> torque curve is an unstable
          equilibrium, and ram tuning is precisely what makes a torque curve
          locally rise. The dyno was not broken. It was doing exactly what a
          dyno does, at me.
        </P>
        <P>
          And it still cannot be held between 1000 and 1500 rpm. I do not know
          why. That sentence is in the source, at the bottom of{' '}
          <Code>induction.hpp</Code>, because it is more useful than the
          explanation I would have had to invent in order not to write it.{' '}
          <A href="/evidence">What came out, and what it taught →</A>
        </P>
        <Note>
          A limitation nobody attempted is a guess. A limitation somebody
          built, measured and reverted is a result.
        </Note>
      </Section>

      <Section index="6" heading="Why C++">
        <P>
          Because the domain belongs in the type system. <Code>Bore</Code> and{' '}
          <Code>Stroke</Code> are distinct types, since a bore is not a stroke
          and I should not be able to swap them at four in the afternoon. A{' '}
          <Code>Lobe</Code> knows its own centreline. A <Code>Crankshaft</Code>{' '}
          refuses to be constructed with two rods on one journal. The compiler
          is a machinist checking my work, and it is better at it than I am.
        </P>
        <P>
          Every abstraction here compiles to the arithmetic I would have
          written by hand and no more. Not because the cycles matter at this
          scale — they do not — but because paying for nothing is the habit
          that makes people write C instead.
        </P>
        <P>
          The standard library, and nothing else. Every dependency is a bet
          that something else will still build in fifteen years, and I did not
          want to make eleven of them. The WAV writer is a 44-byte header and
          some samples.{' '}
          <A href="/engine">What is in it, file by file →</A>
        </P>
      </Section>

      <Section index="7" heading="Why">
        <P>
          To find out whether the burble falls out of the geometry or has to be
          put there by hand. It falls out — four numbers on a crankshaft, and
          everything after them is arithmetic.
        </P>
        <P>
          What the answer left behind is an engine that turns over in a
          terminal and cannot move anything, burning fuel that does not exist,
          wasting a third of it out of a pipe into nowhere, and doing all of
          that correctly. I am aware this is not useful. It is the most careful
          thing I have made, and I would do it again.
        </P>
        <P>
          <A href="/instruments">Six instruments are bolted to it →</A>
        </P>
      </Section>
    </Article>
  );
}

const s = stylex.create({
  masthead: { paddingBlockEnd: space.md },
  name: {
    fontSize: size.display,
    fontWeight: 600,
    letterSpacing: '-0.024em',
    lineHeight: leading.flat,
    margin: 0,
  },
  tagline: {
    fontSize: size.lead,
    lineHeight: leading.snug,
    marginBlockEnd: space.md,
    marginBlockStart: space.sm,
    maxWidth: '30rem',
    textWrap: 'pretty',
  },
  terms: {
    color: color.muted,
    fontSize: size.small,
    lineHeight: leading.snug,
    marginBlock: 0,
  },
});
