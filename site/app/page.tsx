import * as stylex from '@stylexjs/stylex';

import {
  A,
  Article,
  Cmd,
  Code,
  Lede,
  Note,
  P,
  Section,
  Table,
} from '@/components/prose';
import { color, font, leading, size, space } from '@/design/tokens.stylex';

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
          A cross-plane V8 burbles. A flat-plane V8 shrieks. Everyone knows
          this and almost nobody can say why, and the reason is not in the
          exhaust, the camshaft, the displacement or the fuel.
        </Lede>
        <P>
          A 90° V8 fires evenly on either crank — a power stroke every 90° of
          rotation, eight per cycle, perfectly spaced. At the flywheel the two
          engines keep identical time. There is no measurement you can take at
          the output shaft that tells them apart.
        </P>
        <P>
          But an engine does not exhale through its flywheel. It exhales
          through two exhaust manifolds, and a manifold is not connected to an
          engine. It is connected to <em>four cylinders</em>, and it only ever
          hears those four.
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
          So the test of the whole project is small and cruel: change{' '}
          <em>one part</em>, and the sound should change and nothing else.
        </P>
        <Cmd>
          {`./windsor record          # the factory cross-plane crank
./windsor record --flat   # a billet flat crank, same block, same everything`}
        </Cmd>
      </Section>

      <Section index="2" heading="A firing order is not an input">
        <P>
          1-5-4-2-6-3-7-8 is not a secret. It is cast into the intake manifold
          of every 302 Ford built in 1968. Typing it in would take a second,
          and it would end the project — because a program told the answer
          cannot be surprised, and a model that cannot surprise you is a lookup
          table with opinions.
        </P>
        <P>
          So the source specifies a forging and nothing else: a bank angle,
          four journal throws, eight rods hung on them, and which revolution
          each rod fires on.
        </P>
        <Cmd>
          {`Crankshaft{
    90.0_deg,
    { 45.0_deg, 315.0_deg, 135.0_deg, 225.0_deg },
    {{
    //  cylinder    journal   bank          revolution
        /* 1 */     { 0,      Bank::right,  0 },
        /* 2 */     { 1,      Bank::right,  0 },
        ...`}
        </Cmd>
        <P>
          Run it, and under a rule that reads{' '}
          <Code>everything above was specified, everything below came out</Code>
          , it prints 1-5-4-2-6-3-7-8. Nobody typed that. Fit the 1982 H.O.
          camshaft to the identical forging — which is exactly what Ford did —
          and the order becomes 1-3-7-2-6-5-4-8, the 351W order. Different
          order, <em>identical sound</em>, because the sound was never in the
          cam.
        </P>
        <Note>
          The same rule runs through everything: if a number is a consequence
          of other numbers, compute it. Ford and Chevrolet got different firing
          orders out of the same forging, and a model that cannot reproduce
          that has not understood either of them.{' '}
          <A href="/crankshaft">The argument, at length →</A>
        </Note>
      </Section>

      <Section index="3" heading="What had to be built to hear it">
        <P>
          Between a crankshaft and a sound there is an engine, and it has to be
          built honestly or the test proves nothing. Each cylinder is an{' '}
          <em>open</em> thermodynamic system, integrated in crank angle, five
          terms of the first law per radian: the piston taking or giving work,
          the fire, the coolant, gas arriving through the valves, and that same
          gas as mass.
        </P>
        <P>
          Open rather than closed, because everything anyone actually cares
          about happens in the half of the cycle the air-standard Otto cycle
          deletes — the half where the valves are open and the mass is a
          variable.
        </P>
        <P>
          The exhaust is a digital waveguide. A pipe is two delay lines, one
          per direction, which is d&rsquo;Alembert&rsquo;s 1747 result sampled
          at 176 kHz instead of in the continuum. Junctions scatter by
          continuity of pressure and conservation of volume flow. The loop is
          closed, so a header has a length that matters and not merely a note.
        </P>
        <P>
          Then you point a microphone at it and take the Fourier transform.
          Half-order energy is the signature of a train that repeats every{' '}
          <em>two</em> revolutions instead of one — which is precisely what a
          lopsided bank does and an even one does not.
        </P>
        <Table
          head={['one bank alone, at idle', 'half-order energy']}
          rows={[
            ['cross-plane', '0.70'],
            ['flat-plane', '0.0009'],
          ]}
          from="./windsor verify"
        />
        <P>
          A ratio of 742 to one. The thesis survives contact with its own
          measurement.{' '}
          <A href="/evidence">
            Sixty-three other checks say the engine underneath it is real →
          </A>
        </P>
      </Section>

      <Section index="4" heading="The bill">
        <P>
          The cross-plane crank is not the obvious way to build a V8. It is
          heavier, it needs counterweights, and it gives up the even breathing
          the flat crank gets for free. It exists because of a second question
          the same rod table can be asked.
        </P>
        <Table
          head={['at 3000 rpm', 'cross-plane', 'flat-plane']}
          rows={[
            ['primary force', '0 N', '0 N'],
            ['secondary force', '0 N', '5000 N'],
          ]}
          from="./windsor spec"
        />
        <P>
          Half a tonne, a hundred times a second. Nothing bolted to a shaft
          turning at ω can oppose a force that goes at 2ω; the cross-plane
          traces a circle, which counterweights can take, and the flat-plane
          traces a line, which nothing can. The flat crank sounds better and
          shakes. That is the price of the noise, and why almost nobody pays
          it.
        </P>
      </Section>

      <Section index="5" heading="Four things that were built and taken out">
        <P>
          A nonlinear gas solver, verified against Sod&rsquo;s shock tube to
          one part in 10⁵, made header length worth 14% of torque — and
          collapsed the difference between the two crankshafts from 118× to 1×.
          An engine that cannot tell the two cranks apart is of no use here,
          however good its shocks are. It was reverted.
        </P>
        <P>
          Intake runner tuning was built three times and shipped none. The
          third attempt found something better than a feature: a
          torque-controlled brake sitting on a <em>rising</em> torque curve is
          an unstable equilibrium, and ram tuning is exactly what makes a
          torque curve locally rise.
        </P>
        <P>
          It still cannot be held between 1000 and 1500 rpm, and I do not know
          why. That sentence is in the source, at the bottom of{' '}
          <Code>induction.hpp</Code>, because saying so is better than the
          explanation one would have to invent in order not to.{' '}
          <A href="/reverted">What was removed, and what it taught →</A>
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
          and you should not be able to swap them by accident. A{' '}
          <Code>Lobe</Code> knows its own centreline. A <Code>Crankshaft</Code>{' '}
          refuses to be constructed with two rods on one journal. The compiler
          is a machinist checking the work, and an error a compiler can catch
          should never reach a test.
        </P>
        <P>
          Every abstraction here compiles to the arithmetic you would have
          written by hand and no more. Not because the cycles matter at this
          scale — they do not — but because paying for nothing is the habit
          that makes people write C instead.
        </P>
        <P>
          The standard library, and nothing else. Every dependency is a bet
          that something else will still build in fifteen years. The WAV writer
          is a 44-byte header and some samples, written here.{' '}
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
          that correctly.
        </P>
        <Cmd>
          {`./windsor spec       what was built, and what falls out of it
./windsor dyno       put it on a water brake and sweep it
./windsor run        watch it idle
./windsor card       let it draw its own indicator diagram
./windsor record     stand behind it with two microphones
./windsor verify     check every number this project quotes`}
        </Cmd>
      </Section>
    </Article>
  );
}

const s = stylex.create({
  masthead: {
    paddingBlockEnd: space.sm,
  },
  name: {
    fontFamily: font.mono,
    fontSize: size.head,
    fontWeight: 500,
    letterSpacing: '-0.01em',
    lineHeight: leading.tight,
    margin: 0,
  },
  tagline: {
    fontSize: size.lead,
    lineHeight: leading.snug,
    marginBlock: space.sm,
    maxWidth: '26rem',
  },
  terms: {
    color: color.muted,
    fontSize: size.small,
    lineHeight: leading.snug,
    marginBlock: 0,
  },
});
