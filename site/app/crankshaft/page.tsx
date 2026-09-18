import type { Metadata } from 'next';

import { Output, Source, lines } from '@/components/code';
import {
  A,
  Article,
  Code,
  Lede,
  Note,
  P,
  Quote,
  Section,
  Table,
  Title,
} from '@/components/prose';
import { Recording } from '@/components/recording';
import { output } from '@/content/output';

export const metadata: Metadata = {
  title: 'The argument',
  description:
    'Why a cross-plane V8 burbles, and why I would not let the program be told the firing order.',
};

export default function Crankshaft() {
  return (
    <Article>
      <Title sub="Why a cross-plane V8 burbles, and why I would not let the program be told the firing order.">
        The argument
      </Title>

      <Section index="1" heading="Both cranks fire evenly">
        <Lede>
          The first thing to give up is the intuition that a cross-plane V8
          fires unevenly. It does not. Neither does a flat-plane one.
        </Lede>
        <P>
          Eight cylinders, four strokes, 720° of crankshaft per cycle. 720
          divided by 8 is 90. Both engines put a power stroke on the flywheel
          every 90° of rotation, eight times per cycle, perfectly spaced. Hook
          a torque transducer to the output shaft and there is nothing there to
          tell them apart.
        </P>
        <P>
          Which means the burble is not coming from the firing interval, and
          every explanation that starts &ldquo;because it fires
          unevenly&rdquo; is wrong about the one part it is trying to explain.
          I gave that explanation for years.
        </P>
      </Section>

      <Section index="2" heading="What a bank hears">
        <P>
          An engine does not exhale through its flywheel. It exhales through
          two exhaust manifolds, and a manifold is not bolted to an engine — it
          is bolted to <em>four cylinders</em>, and those four are all it will
          ever hear.
        </P>
        <P>
          So take the even 90° train and throw away the half belonging to the
          other bank. What is left depends entirely on where the throws are.
        </P>
        <Table
          head={['', 'intervals within one bank']}
          rows={[
            ['cross-plane — throws at 0, 90, 180, 270', '90 · 180 · 270 · 180'],
            ['flat-plane — throws at 0, 180, 0, 180', '180 · 180 · 180 · 180'],
          ]}
          from="./windsor spec"
        />
        <P>
          A cross-plane bank coughs twice in quick succession, waits three
          quarters of a turn, and coughs again. The other bank does the same
          thing out of step, the two trains beat against each other, and the
          interference never resolves. A flat-plane bank exhales in even thirds
          of a revolution; both banks lock in phase and the harmonics stack
          cleanly at twice the frequency.
        </P>
        <P>
          Four numbers on a crankshaft. Everything else in the project exists
          to make that audible.
        </P>
      </Section>

      <Section index="3" heading="The forging is the input">
        <P>
          A firing order is not a design input. It is a result. You forge a
          crank, you hang eight rods on four journals, you grind a cam that
          decides which cylinder on each pin fires on the first revolution and
          which on the second — and the order is whatever falls out. Ford and
          Chevrolet got different orders from identical forgings for exactly
          that reason.
        </P>
        <P>
          So this is all the program is given. A bank angle, four throws, eight
          rods, and which revolution each one takes.
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
          It refuses to build a crank that cannot exist — two rods on one
          journal, a bank that does not add up — and then it tells me what I
          have made.
        </P>
        <Output from="./windsor spec">{lines(output.spec, 21, 38)}</Output>
        <P>
          1-5-4-2-6-3-7-8 is what is cast into a 1968 302 intake manifold. It
          was never typed in, and the day it first appeared there I went and
          checked the casting to make sure I had not fooled myself.
        </P>
      </Section>

      <Section index="4" heading="Same forging, different cam">
        <P>
          Fit the 1982 5.0 H.O. camshaft to the identical crankshaft — which is
          exactly what Ford did — and the order becomes 1-3-7-2-6-5-4-8, the
          351W order. The cam moved which cylinder on each pin takes the first
          revolution. It did not move a single throw.
        </P>
        <Quote>
          Same crank, different cam, different firing order, identical sound —
          because the sound was never in the cam.
        </Quote>
        <P>
          This is the sharpest test the model faces and the reason the firing
          order had to be derived. A lookup table would have produced both
          orders correctly and learned nothing from either.
        </P>
      </Section>

      <Section index="5" heading="What the burble cost">
        <P>
          If the flat crank breathes more evenly and sounds better, then why
          did Detroit forge the other one eight million times. Ask the same rod
          table a different question and it tells you.
        </P>
        <Output from="./windsor spec">{lines(output.spec, 44, 66)}</Output>
        <P>
          A piston does not travel sinusoidally, so killing the once-per-turn
          term with a counterweight leaves a twice-per-turn one behind, and
          nothing bolted to a shaft turning at ω can cancel a force at 2ω. On
          the cross-plane crank the secondaries cancel each other outright and
          the primary couple traces a circle, which a counterweight can be
          drawn to oppose. On a flat crank they trace a line. Half a tonne of
          force, a hundred times a second, and a line is not something you can
          chase with a rotating weight.
        </P>
        <P>
          The flat crank sounds better and shakes. That is the bill, and it is
          why almost nobody pays it.
        </P>
      </Section>

      <Section index="6" heading="Two microphones, because there are two pipes">
        <P>
          A V8 does not have an exhaust. It has two, down opposite sides of the
          car, permanently out of step on a cross-plane crank. So the recording
          is made the way it would be made in a car park: two microphones 0.6 m
          apart, 1.5 m behind two tailpipes 1.0 m apart, each microphone
          hearing both pipes — quieter by the extra distance, later by the time
          sound takes to cross the gap.
        </P>
        <P>
          Nothing is widened or panned. The image is the geometry, and moving
          the microphones moves it.
        </P>
        <Recording />
        <Note>
          Summing to mono recombines the banks toward an even train and cancels
          much of the unevenness. That is real — it is why people have argued
          about H-pipes for sixty years — but it throws the evidence away, so
          the file is stereo and stays that way.
        </Note>
        <P>
          The thesis lives in <Code>crankshaft.hpp</Code> and the balance
          argument in <Code>balance.hpp</Code>. If changing the crank ever
          stops changing the sound, the model has quietly stopped being true
          and the project is over.{' '}
          <A href="/evidence">How I check that it has not →</A>
        </P>
      </Section>
    </Article>
  );
}
