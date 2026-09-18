import type { Metadata } from 'next';

import { Output, lines } from '@/components/code';
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
import { LivengoodWu } from '@/components/math';
import { output } from '@/content/output';

export const metadata: Metadata = {
  title: 'The evidence',
  description:
    'Sixty-three checks against things that existed before the code did, and the four pieces of work I measured and took out.',
};

export default function Evidence() {
  return (
    <Article>
      <Title sub="Sixty-three checks against things that existed before the code did — and the two largest pieces of work I did on this project, neither of which is in it.">
        The evidence
      </Title>

      <Section index="1" heading="What a check has to be">
        <Lede>
          A test that compares a program against itself proves the program is
          consistent, which is not the claim I am making.
        </Lede>
        <P>
          So: a derivative against finite differences. A burn rate against its
          own integral. A cylinder head against a flow bench. A firing order
          against the casting. A shock tube against its exact solution. And the
          thesis against a Fourier transform. Nothing in the model was fitted
          to any of it, which is the only reason agreement means anything.
        </P>
        <Output from="./windsor verify">{lines(output.verify, 1, 20)}</Output>
      </Section>

      <Section index="2" heading="The engine, against Ford">
        <Table
          head={['', 'model', 'Ford, 1968 302-2V (gross)']}
          rows={[
            ['peak torque', '307 lb-ft @ 1985', '295 lb-ft @ 2400'],
            ['peak power', '206 hp @ 4994', '210 hp @ 4400'],
          ]}
          from="./windsor dyno"
        />
        <P>
          Within four percent and two, with nothing fitted. The magnitudes are
          close and the curve is too broad — torque peaks 400 rpm low, power
          600 high — which is the intake runner tuning that is still missing,
          and which I will come back to at the bottom of this page.
        </P>
        <P>
          It was ten percent optimistic with the power peak a thousand rpm high
          until the carburettor&rsquo;s venturis went in. A two-barrel engine
          breathes through a hole that cannot be opened, and that is most of
          why the 2V made its power at 4400 where the 4V made more at 4800. I
          had been modelling a throttle. The throttle was never the limit.
        </P>
      </Section>

      <Section index="3" heading="The cylinder head, against a flow bench">
        <P>
          Curtain area, a discharge coefficient, a throat cap. How fast that
          coefficient sags past its peak is the only number in the port model
          fitted to anything at all, and this is the thing it is fitted to.
        </P>
        <Table
          head={['lift', 'model', 'published, stock C8OE-F']}
          rows={[
            ['0.400 in', '153.5 cfm', '~150 cfm'],
            ['0.426 in — the cam’s peak', '149.9 cfm', '—'],
          ]}
          from="./windsor verify"
        />
        <P>
          The head is already past its best at the lift the camshaft actually
          reaches. That is not a defect in the model. It is what a stock 302
          head does, and it is why peak effective area lands exactly where Ford
          put the lobe.
        </P>
      </Section>

      <Section index="4" heading="The indicator card">
        <P>
          Watt&rsquo;s diagram, wide open and throttled, drawn to the same
          scale so the two can be compared honestly. Pressure is logarithmic
          here, so the area enclosed is not the work — the work is printed
          underneath it.
        </P>
        <Output from="./windsor card">{lines(output.card, 29, 59)}</Output>
        <P>
          Ninety-six percent of everything it makes, spent on suffocating
          itself. The throttle is the most wasteful control mechanism nobody
          has replaced, and this is the number that says so.
        </P>
      </Section>

      <Section index="5" heading="The fuel, which is what stops the engine">
        <P>
          Livengood–Wu over Douaud–Eyzat: the end gas spends <Code>dt/τ</Code>{' '}
          of its patience each instant and goes off when the account reaches
          one.
        </P>
        <LivengoodWu />
        <P>
          It reports an index and not a verdict — the threshold belongs to the
          CFR engine Douaud and Eyzat fitted the delay on, and a value of 1
          means autoignition in <em>that</em> chamber, not in a Ford wedge.
          Dividing by five to make the stock engine read 1.0 would be fitting
          the model to flatter itself, so the constant stays as published and
          the number is reported raw. What survives the recalibration is every
          comparison, and the comparisons are what compression ratio and
          advance are actually chosen by.
        </P>
        <Output from="./windsor verify">{lines(output.verify, 91, 99)}</Output>
        <P>
          Which makes 9.5:1, 34° of advance and whatever was in the tank in
          1968 one decision instead of three.
        </P>
      </Section>

      <Section index="6" heading="The thesis, against a Fourier transform">
        <P>
          Half-order energy is the signature of a train that repeats every{' '}
          <em>two</em> revolutions instead of one, which is exactly what a
          lopsided bank does and an even one does not. I stated the thesis
          before I wrote the transform. The transform did not care, and agreed.
        </P>
        <Output from="./windsor verify">{lines(output.verify, 115, 123)}</Output>
        <P>
          Ninety-nine percent of the energy sits below 90 Hz, where the burble
          lives, and there is nothing measurable at all above 1.4 kHz.
        </P>
      </Section>

      <Section index="7" heading="The solver I built, measured and took out">
        <P>
          The exhaust is two delay lines per pipe, which is exact for a linear
          wave and wrong for a blowdown, so its source is clamped at Mach 1.
          Rather than leave that as an apology in a comment, I wrote the solver
          that would have fixed it: a second-order MUSCL-Hancock finite-volume
          Euler solver with an HLL flux, verified against Sod&rsquo;s shock
          tube to one part in 10⁵.
        </P>
        <P>
          Wired in place of the delay lines it worked. No clamp anywhere, pipe
          pressures physically correct, and header length finally became worth
          14% of torque with a peak at 1.2 m — where the old builder&rsquo;s
          rule predicts 1.10 m. It agreed with sixty years of exhaust shops.
        </P>
        <P>
          Then I took it out. It cost seven times the runtime, and it collapsed
          the one measurement this project exists to make: the ratio between
          the two crankshafts went from 118× to 1×. An engine that cannot tell
          the two cranks apart is of no use to me, however good its shocks are.
        </P>
        <Quote>
          A delay line has no numerical dissipation. It is the exact solution
          to the linear problem, not an approximation to it, while any
          finite-volume scheme is diffusive everywhere. For a problem that is
          mostly linear propagation with occasional violence, the
          cruder-looking model is the more faithful one.
        </Quote>
        <P>
          What survives is a measurement. <Code>riemann.hpp</Code> is still in
          the tree, still verified, and its job now is to say precisely what
          the shipped model cannot do.
        </P>
        <Output from="./windsor verify">{lines(output.verify, 100, 113)}</Output>
      </Section>

      <Section index="8" heading="The runners, three times">
        <P>
          Intake runner tuning I built three times and shipped none. As a
          waveguide it tuned correctly and put peak torque within 90 rpm of
          Ford&rsquo;s, and could not be held below 2000 rpm at any damping. As
          a Helmholtz resonator it was perfectly stable and moved the torque
          peak the wrong way. As eight runners sharing one junction it found
          two things worth more than the feature.
        </P>
        <P>
          <em>The junction was not the blocker.</em> Sharing redistributes
          energy between runners; it does not dissipate any. Eight low-loss
          pipes passing a wave around still have it afterwards.
        </P>
        <P>
          <em>Length mattered more.</em> At 0.30 m the ram gain came out at 34%
          where a stock log manifold is worth a few percent — and that is what
          made the engine unholdable, for a reason I had not expected to learn
          from a program I wrote myself.
        </P>
        <Quote>
          A torque-controlled brake sitting on a <em>rising</em> torque curve
          is an unstable equilibrium. Speed up a little, make more torque,
          speed up more. Ram tuning is precisely the thing that makes a torque
          curve locally rise, so the dyno slid into the resonance instead of
          holding short of it.
        </Quote>
        <P>
          That is real engine-and-dynamometer physics and not a defect in
          either, which is why modern cells are speed-controlled. The dyno was
          not broken. It was doing exactly what a dyno does, at me.
        </P>
        <P>
          Shortened to 0.18 m the gain falls to 4% and the curve comes out
          smooth — and it still could not be held between 1000 and 1500 rpm.
          Not at any controller gain from 0.6 to 12 N·m per rpm, not with a
          fresh engine per point, not with the load lagged.{' '}
          <strong>I do not know why.</strong> It is written at the bottom of{' '}
          <Code>induction.hpp</Code> as the next thing to find out, because it
          is more useful than the explanation I would have had to invent in
          order not to write it down.
        </P>
        <Note>
          A limitation nobody attempted is a guess. A limitation somebody
          built, measured and reverted is a result — which is the only reason
          these two are on the evidence page and not in a footnote.
        </Note>
      </Section>

      <Section index="9" heading="What a failing check means">
        <P>
          Several of the sixty-three belong to <em>this</em> engine — the flow
          bench, the cam card, the firing orders. Build a different one and
          they will fail honestly. That is the check working, not breaking.{' '}
          <A href="/engine">How to build a different one →</A>
        </P>
        <Note>
          Every figure quoted outside the code is a copy, and copies rot. Mine
          have drifted three times, always the same way: an edit anchored on a
          string that had since been reformatted, applied without checking,
          which silently did nothing. An edit that silently does nothing is
          worse than one that fails, because you will believe it worked. Every
          figure on this site is captured from a real run for that reason.
        </Note>
      </Section>
    </Article>
  );
}
