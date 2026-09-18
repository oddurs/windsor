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
  Terms,
  Title,
} from '@/components/prose';

export const metadata: Metadata = {
  title: 'The engine',
  description:
    'Eight cylinders, a cross-plane crank, a cam, two exhaust manifolds — and what each file is arguing with.',
};

const spine = [
  {
    term: 'si.hpp',
    body: 'The one rule everything depends on: inside the engine, everything is SI. Read it first or nothing later makes sense.',
  },
  {
    term: 'geometry.hpp',
    body: 'The crank-slider, which is the entire machine. One equation; every other file is decoration on it.',
  },
  {
    term: 'crankshaft.hpp',
    body: 'The thesis. Why a cross-plane V8 burbles, and why it refuses to let you type in a firing order.',
  },
  {
    term: 'balance.hpp',
    body: 'The other half of the thesis: what the cross-plane crank cost, and why almost nobody buys the alternative.',
  },
];

const cylinder = [
  {
    term: 'charge.hpp',
    body: 'The working fluid. An engine does not burn petrol, it heats air.',
  },
  {
    term: 'wiebe.hpp',
    body: 'How fast the fire spreads — a curve fitted in the 1950s that outlived everything meant to replace it.',
  },
  {
    term: 'woschni.hpp',
    body: 'The third of the fuel that goes into the coolant. Also the one sanctioned exception to the SI rule, and it says so loudly.',
  },
  {
    term: 'camshaft.hpp',
    body: 'The only part of the engine that decides anything. Two engines identical but for this make peak power 2500 rpm apart.',
  },
  {
    term: 'port.hpp',
    body: 'The bottleneck. A naturally aspirated engine is a pump forbidden from using a pump.',
  },
  {
    term: 'cylinder.hpp',
    body: 'All of the above, assembled: five terms of the first law per radian of crank.',
  },
];

const machine = [
  {
    term: 'induction.hpp',
    body: 'The throttle, and the vacuum behind it — the most wasteful control mechanism never replaced.',
  },
  {
    term: 'ignition.hpp',
    body: 'Deciding when to light it, with no way of knowing. Flyweights and a rubber diaphragm.',
  },
  {
    term: 'knock.hpp',
    body: 'The constraint that actually decides compression ratio. Its number is an index, not a verdict, and the file explains why.',
  },
  {
    term: 'friction.hpp',
    body: 'The tax. At idle it is all of it.',
  },
  {
    term: 'exhaust.hpp',
    body: 'Four pipes, a collector, and the noise. Where the project cashes in.',
  },
  {
    term: 'riemann.hpp',
    body: 'Its foil: the nonlinear solver that says what the one above costs. Verified, and not wired in.',
  },
  {
    term: 'engine.hpp',
    body: 'The assembly, and the loop — a running engine is two statements chasing each other.',
  },
  {
    term: 'windsor.hpp',
    body: 'The engine itself, as built. Every figure is Ford’s except the few marked otherwise.',
  },
];

export default function Engine() {
  return (
    <Article>
      <Title sub="Eight cylinders, a cross-plane crank, a cam, two exhaust manifolds — and what each file is arguing with.">
        The engine
      </Title>

      <Section index="1" heading="One cylinder, as an open system">
        <Lede>
          The physics has to be honest or the thesis proves nothing. So each
          cylinder is integrated in crank angle, five terms of the first law
          per radian.
        </Lede>
        <Cmd>
          {`m·cᵥ·dT/dθ  =  −p·dV/dθ           the piston, taking or giving work
               + dQ_burn/dθ       the fire            (Wiebe)
               − dQ_wall/dθ       the coolant         (Woschni)
               + Σ (dmᵢ/dθ)·hᵢ    gas arriving        (compressible orifice flow)
               − u·dm/dθ          the same, as mass`}
        </Cmd>
        <P>
          Open and not closed, because everything anyone cares about happens in
          the half of the cycle the air-standard Otto cycle deletes — the half
          where the valves are open and the mass is a variable. Pumping loss,
          volumetric efficiency, cam timing, reversion, the entire reason a
          throttle costs you anything: all of it lives there.
        </P>
        <P>
          γ varies with temperature, falling from 1.400 in a cold intake charge
          to 1.246 in combustion products. A cycle computed at a constant 1.4
          will promise you a fifth more thermal efficiency than any engine can
          deliver, which is why the textbook number and the dynamometer have
          never agreed.
        </P>
      </Section>

      <Section index="2" heading="Two delay lines make a pipe">
        <P>
          The exhaust is a digital waveguide. A pipe is two delay lines, one
          per direction, which is d&rsquo;Alembert&rsquo;s 1747 result sampled
          at 176 kHz rather than in the continuum. Junctions scatter by
          continuity of pressure and conservation of volume flow.
        </P>
        <P>
          The loop is closed: each cylinder&rsquo;s exhaust boundary is the
          pressure standing in <em>its own primary pipe</em>, so a header has a
          length that matters and not merely a note. Only by about two percent
          here, and the reason why is a measurement rather than an excuse.
        </P>
        <Note>
          A delay line has <em>no</em> numerical dissipation. It is the exact
          solution to the linear problem, not an approximation to it — which is
          why the cruder-looking model turned out to be the more faithful one.{' '}
          <A href="/reverted">The solver that proved it →</A>
        </Note>
      </Section>

      <Section index="3" heading="What is in it">
        <P>
          Every header opens with prose explaining why the part exists and what
          it is arguing with — not what the code does, because the code does
          that. The register is a service manual written by someone who likes
          the machine. They were written to be read in this order, each one
          assuming the last.
        </P>
        <Terms items={spine} />
        <P>
          Stop there and you have the argument. Everything below is the machine
          that makes it audible.
        </P>
        <Terms items={cylinder} />
        <Terms items={machine} />
        <Note>
          Files named after people — Wiebe, Woschni — are named that way on
          purpose. Those are empirical curves somebody measured, not laws, and
          the credit is also a warning.
        </Note>
      </Section>

      <Section index="4" heading="The instruments">
        <P>
          The engine is a sealed mechanism that turns and gets hot and knows
          nothing about being observed. Six instruments are bolted to it
          afterward, and none of them may reach into the physics to make its
          own job easier.
        </P>
        <Cmd>
          {`./windsor spec       the shop manual page
./windsor dyno       a water brake on the flywheel
./windsor run        a gauge cluster, wired to the sensors
./windsor card       let it draw its own indicator diagram, as Watt did
./windsor record     three feet behind the tailpipe, in stereo
./windsor verify     the inspection sheet`}
        </Cmd>
        <P>
          Read <Code>verify.cpp</Code> last. Its entry point is the contents
          page of everything the project claims, and every claim there is
          checked against something outside it.{' '}
          <A href="/evidence">What it checks →</A>
        </P>
      </Section>

      <Section index="5" heading="What it does not model">
        <P>
          Stated plainly, because an unstated simplification is a lie and a
          stated one is a design decision. Each is named in the file where it
          bites.
        </P>
        <Terms
          items={[
            {
              term: 'wrist-pin offset',
              body: 'Real pistons carry 0.5–1.5 mm toward the thrust side. It buys a rattle you cannot hear and costs a closed-form solution.',
            },
            {
              term: 'runner tuning',
              body: 'Built three times, shipped none. Length mattered more than the junction, and a rising torque curve turned out to be an unstable equilibrium for the brake.',
            },
            {
              term: 'nonlinear gas',
              body: 'The solver exists, is verified, and is not wired in. A real blowdown front clocks at 1409 m/s — Mach 2.4 — and the shipped model cannot make one. It says so with a number instead of an apology.',
            },
            {
              term: 'and the rest',
              body: 'Blow-by, oil temperature, crankshaft torsion, dissociation above 2000 K. None of these would change the shape of the project.',
            },
          ]}
        />
      </Section>

      <Section index="6" heading="Building a different one">
        <P>
          Everything the engine is made of is in <Code>windsor.hpp</Code>, and
          nothing else needs touching to build another. The source marks the
          distinction that matters: a <em>specification</em> is Ford&rsquo;s —
          a bore, a rod length, a cam card, a carburettor, measured off the
          engine. A <em>calibration</em> is a number fitted because nothing
          else could supply it. Every calibration is marked in the source and
          says what it was fitted to. There are five. Anything unmarked is
          Ford&rsquo;s.
        </P>
        <Cmd>
          {`short_block()          4.000 × 3.000, 5.090 in rods, 9.5:1
stock_cam()            266°/256°, 0.426/0.425 in lift, 117° ICL, 110.5° LSA

cross_plane_crank()    the factory forging  →  1-5-4-2-6-3-7-8
cross_plane_crank_ho() the same forging, 1982 cam  →  1-3-7-2-6-5-4-8
flat_plane_crank()     a billet flat crank, same block, same rods`}
        </Cmd>
        <P>
          Grind the throws into one plane and every bank interval goes to 180°.
          Hand <Code>Camshaft::from_card</Code> what a catalogue prints and it
          converts the centreline at its own boundary, because cam cards quote
          centrelines about the gas-exchange top dead centre, 360° from the
          firing one everything else here uses.
        </P>
        <P>
          Then run <Code>./windsor verify</Code>. Several of its checks belong
          to <em>this</em> engine — the flow bench, the cam card, the firing
          orders — and will fail honestly if you have built a different one.
          That is the check doing its job.
        </P>
      </Section>
    </Article>
  );
}
