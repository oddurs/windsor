import type { Metadata } from 'next';

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

export const metadata: Metadata = {
  title: 'Reverted',
  description:
    'The two largest pieces of work in this project are not in it. Both were finished, measured, and removed.',
};

export default function Reverted() {
  return (
    <Article>
      <Title sub="The two largest pieces of work in this project are not in it. Both were finished, measured, and removed because the measurement said so.">
        Four things that were built and taken out
      </Title>

      <Section index="1" heading="Why write them down">
        <Lede>
          A limitation nobody attempted is a guess. A limitation somebody
          built, measured and reverted is a result.
        </Lede>
        <P>
          The list of things a model does not do is usually a list of things
          its author did not get to. These four were finished. What follows is
          what they measured, and why the measurement was an argument for
          taking them out.
        </P>
      </Section>

      <Section index="2" heading="The nonlinear gas solver">
        <P>
          <Code>riemann.hpp</Code> — still in the tree, still verified, no
          longer wired to the engine. A second-order MUSCL-Hancock
          finite-volume Euler solver with an HLL flux, verified against
          Sod&rsquo;s shock tube to one part in 10⁵ and conserving mass and
          energy to 1.5 × 10⁻¹⁴.
        </P>
        <P>
          It was built to answer the one limitation <Code>exhaust.hpp</Code>{' '}
          had always admitted: its delay lines are linear, and a blowdown is
          not, so the source has to be clamped at Mach 1. Wired in place of the
          delay lines it worked. No clamp anywhere, pipe pressures physically
          correct, and header length finally became worth something.
        </P>
        <Table
          head={['', 'with delay lines', 'with the solver']}
          rows={[
            [
              'header length worth',
              '1.8%, no peak',
              '14% of torque, peaking at 1.2 m',
            ],
          ]}
          from="./windsor dyno"
        />
        <P>
          Where the old builder&rsquo;s rule — L = 850·ED/rpm − 3 — predicts
          1.10 m. It agreed with sixty years of exhaust shops.
        </P>
        <P>
          It was reverted. It cost seven times the runtime, and it collapsed
          the one measurement the project exists to make: the ratio between the
          two crankshafts went from 118× to 1×. An engine that cannot tell the
          two cranks apart is of no use here, however good its shocks are.
        </P>
        <Quote>
          A delay line has no numerical dissipation. It is the exact solution
          to the linear problem, not an approximation to it, while any
          finite-volume scheme is diffusive everywhere. For a problem that is
          mostly linear propagation with occasional violence, the
          cruder-looking model is the more faithful one.
        </Quote>
        <P>
          What survives is a measurement. <Code>./windsor verify</Code> sends a
          real blowdown front down a primary with no clamp and clocks it at
          1409 m/s — Mach 2.4, against 586 for sound in the gas ahead of it. It
          is a shock. The shipped model cannot make one, and now says so with a
          number instead of an apology.
        </P>
      </Section>

      <Section index="3" heading="Intake runner tuning, three times">
        <P>
          Each attempt left the next one better posed, which is the most a
          failure can do.
        </P>
        <P>
          <strong>As a waveguide</strong>, each runner ending at a fixed
          reflection of its own. It tuned correctly and put peak torque within
          90 rpm of Ford&rsquo;s. It could not be held below 2000 rpm at any
          damping: torque swung between 137 and 465 N·m in a limit cycle
          seventeen cycles long.
        </P>
        <P>
          <strong>As an inertance and a port compliance</strong> — a Helmholtz
          resonator, which is what a runner is. Written the obvious way it is
          an algebraic loop, because the flow being differentiated is the one
          the cylinder computes <em>from</em> the pressure being solved for; it
          produced NaN in under a second. Written properly, with the runner
          flow as a state, it was perfectly stable — 0% cycle-to-cycle
          variation at 1500 and 4000 rpm — and it moved the torque peak the
          wrong way.
        </P>
        <P>
          <strong>As eight runners meeting at one junction</strong>, which was
          the hypothesis the second attempt ended on: they had each been ending
          at a private wall, as though the other seven were not there. They
          were given the scattering node <Code>exhaust.hpp</Code> already uses
          for its collector, widened from five ports to nine.
        </P>
      </Section>

      <Section index="4" heading="What the third attempt found instead">
        <P>
          <em>The junction was not the blocker.</em> Sharing redistributes
          energy between runners; it does not dissipate any. Eight low-loss
          pipes passing a wave around still have it afterwards.
        </P>
        <P>
          <em>Length mattered more.</em> At 0.30 m the ram gain came out at 34%
          where a stock log manifold is worth a few percent — and that is what
          made the engine unholdable, for a reason worth stating plainly.
        </P>
        <Quote>
          A torque-controlled brake sitting on a <em>rising</em> torque curve
          is an unstable equilibrium. Speed up a little, make more torque,
          speed up more. Ram tuning is precisely the thing that makes a torque
          curve locally rise, so the dyno slid into the resonance instead of
          holding short of it.
        </Quote>
        <P>
          That is real engine-and-dynamometer physics, not a defect in either,
          and it is why modern cells are speed-controlled. Shortened to 0.18 m
          the gain falls to 4% and the curve comes out smooth.
        </P>
      </Section>

      <Section index="5" heading="And then it stops">
        <P>
          It still could not be held between 1000 and 1500 rpm. Not at any
          controller gain from 0.6 to 12 N·m per rpm, not with a fresh engine
          per point, not with the load lagged.
        </P>
        <P>
          <strong>I do not know why.</strong> It is written at the bottom of{' '}
          <Code>induction.hpp</Code> as the next thing to find out, because
          saying so is better than the explanation one would have to invent in
          order not to.
        </P>
        <Note>
          This is also the missing 400 rpm in the torque peak and the extra 600
          in the power peak. The model is broad where a real 302 is peaky, and
          the reason is named rather than absorbed.{' '}
          <A href="/evidence">Where that shows up →</A>
        </Note>
      </Section>
    </Article>
  );
}
