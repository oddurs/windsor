import type { Metadata } from 'next';

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
  Title,
} from '@/components/prose';

export const metadata: Metadata = {
  title: 'Evidence',
  description:
    'Sixty-three checks in a couple of seconds, every one against something outside the project.',
};

export default function Evidence() {
  return (
    <Article>
      <Title sub="Sixty-three checks in a couple of seconds, every one against something outside the project.">
        Evidence
      </Title>

      <Section index="1" heading="What a check has to be">
        <Lede>
          A test that compares a program against itself proves the program is
          consistent, which is not the claim. Every check here is against
          something that existed before the code did.
        </Lede>
        <P>
          A derivative against finite differences. A burn rate against its own
          integral. A cylinder head against a flow bench. A firing order
          against the casting. A shock tube against its exact solution. And the
          thesis against a Fourier transform. Nothing in the model was fitted
          to any of it.
        </P>
        <Cmd>./windsor verify</Cmd>
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
          600 high — which is the intake runner tuning still missing.
        </P>
        <P>
          It was ten percent optimistic with the power peak a thousand rpm high
          until the carburettor&rsquo;s venturis went in. A two-barrel engine
          breathes through a hole that cannot be opened, and that is most of
          why the 2V made its power at 4400 where the 4V made more at 4800.
        </P>
      </Section>

      <Section index="3" heading="The cylinder head, against a flow bench">
        <P>
          Curtain area, a discharge coefficient, a throat cap. How fast that
          coefficient sags past its peak is the only number in the port model
          fitted to anything, and this is what it is fitted to.
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
          reaches. That is not a defect in the model; it is what a stock 302
          head does, and it is why the peak effective area lands where Ford put
          the lobe.
        </P>
      </Section>

      <Section index="4" heading="The indicator card">
        <P>
          <Code>./windsor card</Code> draws Watt&rsquo;s diagram — wide open and
          throttled, to the same scale. At 3000 rpm the trace peaks at 64 bar,
          9.5° after top dead centre: late enough to use the crank&rsquo;s
          leverage, early enough that the end gas has not gone off by itself.
        </P>
        <Table
          head={['at 2000 rpm', 'gross', 'pumping', 'net']}
          rows={[
            ['wide open', '11.99 bar', '−0.08 bar', '11.90 bar'],
            ['throttled', '0.97 bar', '−0.93 bar', '0.03 bar'],
          ]}
          from="./windsor card"
        />
        <P>
          Ninety-six percent of everything it makes, spent on suffocating
          itself. The throttle is the most wasteful control mechanism nobody
          has replaced, and this is the number that says so.
        </P>
      </Section>

      <Section index="5" heading="The fuel, which is what stops the engine">
        <P>
          Livengood–Wu over Douaud–Eyzat: the end gas spends{' '}
          <Code>dt/τ</Code> of its patience each instant and goes off when the
          account reaches one. It reports an index and not a verdict — the
          threshold belongs to the CFR engine the correlation was fitted on —
          but the comparisons are what compression ratio and advance are
          actually chosen by, and they all come out right.
        </P>
        <P>
          Knock worsens at low rpm, with advance, with compression, with
          cheaper fuel, and eases on a rich mixture. Which makes 9.5:1, 34° of
          advance and what was in the tank in 1968 one decision instead of
          three.
        </P>
      </Section>

      <Section index="6" heading="The balance, which is why the crank exists">
        <Table
          head={['at 3000 rpm', 'cross-plane', 'flat-plane']}
          rows={[
            ['primary force', '0 N', '0 N'],
            ['secondary force', '0 N', '5000 N'],
            ['what it traces', 'a circle', 'a line'],
          ]}
          from="./windsor spec"
        />
        <P>
          A circle counterweights can take. A line nothing can.{' '}
          <A href="/crankshaft">The argument this belongs to →</A>
        </P>
      </Section>

      <Section index="7" heading="The sound, against itself">
        <P>
          Half-order energy against whole-order energy at idle. Half-orders are
          the signature of a train that repeats every <em>two</em> revolutions
          instead of one, which is exactly what a lopsided bank does and an
          even one does not.
        </P>
        <Table
          head={['one bank alone', 'cross-plane', 'flat-plane', '']}
          rows={[['half-order energy', '0.70', '0.0009', '742×']]}
          from="./windsor verify"
        />
        <P>
          Ninety-nine percent of the energy sits below 90 Hz, where the burble
          lives, and there is nothing measurable at all above 1.4 kHz. The
          thesis was stated before the transform was written, and the transform
          agreed with it.
        </P>
      </Section>

      <Section index="8" heading="What a failing check means">
        <P>
          Several of the sixty-three belong to <em>this</em> engine — the flow
          bench, the cam card, the firing orders. Build a different one and
          they will fail honestly. That is the check working, not breaking.
        </P>
        <Note>
          Every figure quoted outside the code is a copy, and copies rot. They
          have drifted three times here, always the same way: an edit anchored
          on a string that had since been reformatted, applied without
          checking, which silently did nothing. An edit that silently does
          nothing is worse than one that fails, because you will believe it
          worked.
        </Note>
      </Section>
    </Article>
  );
}
