import type { Metadata } from 'next';

import {
  A,
  Article,
  Cmd,
  Code,
  Lede,
  Note,
  P,
  Quote,
  Section,
  Table,
  Title,
} from '@/components/prose';

export const metadata: Metadata = {
  title: 'The argument',
  description:
    'Why a cross-plane V8 burbles, and why the program refuses to let you type in a firing order.',
};

export default function Crankshaft() {
  return (
    <Article>
      <Title sub="Why a cross-plane V8 burbles, and why the program refuses to let you type in a firing order.">
        The argument
      </Title>

      <Section index="1" heading="Both cranks fire evenly">
        <Lede>
          The first thing to give up is the intuition that a cross-plane V8
          fires unevenly. It does not. Neither does a flat-plane one.
        </Lede>
        <P>
          Eight cylinders, four strokes, 720° of crankshaft per cycle: 720
          divided by 8 is 90. Both engines put a power stroke on the flywheel
          every 90° of rotation, eight times per cycle, perfectly spaced. Hook
          a torque transducer to the output shaft and the two are
          indistinguishable in rhythm.
        </P>
        <P>
          Which means the burble cannot be coming from the firing interval,
          and every explanation that begins &ldquo;because it fires
          unevenly&rdquo; is wrong about the part it is trying to explain.
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
          Take the even 90° train and throw away the half that belongs to the
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
          thing out of step, and the two pulse trains beat against each other
          and never resolve. A flat-plane bank exhales in even thirds of a
          revolution; both banks lock in phase and the harmonics stack cleanly
          at twice the frequency.
        </P>
        <P>
          Four numbers on a crankshaft. That is the whole of it, and everything
          else in the project exists to make it audible.
        </P>
      </Section>

      <Section index="3" heading="The forging is the input">
        <P>
          A firing order is not a design input. It is a result. You forge a
          crank, you hang eight rods on four journals, you grind a cam that
          decides which cylinder on each pin fires on the first revolution and
          which on the second — and the firing order is whatever falls out.
          Ford and Chevrolet got different orders from identical forgings for
          exactly this reason.
        </P>
        <P>
          So the source specifies the forging, and refuses to accept anything
          downstream of it.
        </P>
        <Cmd>
          {`Crankshaft{
    90.0_deg,
    { 45.0_deg, 315.0_deg, 135.0_deg, 225.0_deg },
    {{
    //  cylinder    journal   bank          revolution
        /* 1 */     { 0,      Bank::right,  0 },
        /* 2 */     { 1,      Bank::right,  0 },
        /* 3 */     { 2,      Bank::right,  1 },
        ...`}
        </Cmd>
        <P>
          It will also refuse to build a crank that cannot exist — two rods on
          one journal, a bank that does not add up. What comes back out is a
          shop manual page with a rule drawn across it:
        </P>
        <Cmd>
          {`  ---- everything above was specified. everything below came out. ----

CRANKSHAFT
  forging              cross-plane
  firing order         1-5-4-2-6-3-7-8
  the engine fires     every 90 deg, eight times per cycle

WHAT EACH BANK HEARS
  right bank   cyl 1 4 2 3   fires at   180 -  90 - 180 - 270
  left  bank   cyl 5 6 7 8   fires at   270 - 180 -  90 - 180

  LOPSIDED. Each bank coughs twice in quick succession, waits three
  quarters of a turn, and coughs again.
  This is the burble.`}
        </Cmd>
        <P>
          1-5-4-2-6-3-7-8 is what is cast into a 1968 302 intake manifold. It
          was never typed in.
        </P>
      </Section>

      <Section index="4" heading="Same forging, different cam">
        <P>
          Fit the 1982 5.0 H.O. camshaft to the identical crankshaft — which is
          exactly what Ford did — and the order becomes 1-3-7-2-6-5-4-8, the
          351W order. The cam moved which cylinder on each pin takes the first
          revolution; it did not move a single throw.
        </P>
        <Quote>
          Same crank, different cam, different firing order, identical sound —
          because the sound was never in the cam.
        </Quote>
        <P>
          This is the sharpest test the model faces, and it is the reason the
          firing order had to be derived. A lookup table would have produced
          the right two orders and learned nothing from either.
        </P>
      </Section>

      <Section index="5" heading="What the burble costs">
        <P>
          If the flat crank breathes more evenly and sounds better, the
          question is why Detroit forged the other one eight million times.
          Ask the same rod table a different question.
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
          Secondary imbalance goes at twice crankshaft speed, and nothing
          bolted to a shaft turning at ω can oppose a force that goes at 2ω.
          The cross-plane&rsquo;s secondaries trace a circle, which
          counterweights can take. The flat-plane&rsquo;s trace a line, which
          nothing can. Half a tonne of force, a hundred times a second, into
          the mounts.
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
        <Note>
          Summing to mono recombines the banks toward an even train and cancels
          much of the unevenness. That is real — it is why people have argued
          about H-pipes for sixty years — but it throws the evidence away.
        </Note>
        <Cmd>
          {`./windsor record          # the factory cross-plane crank
./windsor record --flat   # a billet flat crank, same block, same everything`}
        </Cmd>
        <P>
          One part changed. <A href="/evidence">The measurement →</A>
        </P>
        <P>
          The thesis lives in <Code>crankshaft.hpp</Code>, and the balance
          argument in <Code>balance.hpp</Code>. If changing the crank ever
          stops changing the sound, the model has quietly stopped being true
          and the project is over.
        </P>
      </Section>
    </Article>
  );
}
