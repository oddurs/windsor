import type { Metadata } from 'next';

import { Output, lines } from '@/components/code';
import { A, Article, Code, Lede, Note, P, Section, Title } from '@/components/prose';
import { Recording } from '@/components/recording';
import { output } from '@/content/output';

export const metadata: Metadata = {
  title: 'The instruments',
  description:
    'Six things bolted to a sealed engine, and what each one prints.',
};

export default function Instruments() {
  return (
    <Article>
      <Title sub="The engine knows nothing about being observed. These are bolted to it afterwards, and none of them may reach into the physics to make its own job easier.">
        Six instruments
      </Title>

      <Section index="1" heading="spec — the shop manual page">
        <Lede>
          What I specified, and then, under a rule drawn across the middle of
          the page, what the program worked out from it. I look at this more
          than anything else in the project.
        </Lede>
        <Output from="./windsor spec">{output.spec}</Output>
        <P>
          The crank drawn in the middle is the view down its nose. Four throws
          at 90°, and the little diagram is why the secondary shaking forces
          cancel — which is the whole reason this forging exists and the reason
          the engine sounds like that is, strictly, a side effect.
        </P>
      </Section>

      <Section index="2" heading="dyno — a water brake on the flywheel">
        <P>
          A load applied to the flywheel, a speed held, torque read off, and
          the same sweep a magazine would have published in 1968. Nothing in
          the model was fitted to Ford&rsquo;s figures, which is the only
          reason it is interesting that they agree.
        </P>
        <Output from="./windsor dyno">{output.dyno}</Output>
        <P>
          The knock index is worst at the bottom of the range, where the end
          gas has the most milliseconds to sit and cook. It reports an index
          and not a verdict, and <Code>knock.hpp</Code> says whose engine the
          threshold of 1.0 belongs to.
        </P>
      </Section>

      <Section index="3" heading="card — it draws its own picture">
        <P>
          James Watt tied a pencil to a piston and let the engine plot its own
          pressure against its own volume. The trace is the work done, and the
          area enclosed is what you get to keep. Two hundred and forty years
          later I am doing it with hashes.
        </P>
        <Output from="./windsor card">{output.card}</Output>
        <P>
          The second trace is the same engine throttled. It is not a smaller
          version of the first one — it is a different shape, and the
          difference between them is the engine spending almost everything it
          makes on suffocating itself.{' '}
          <A href="/evidence">The numbers under that →</A>
        </P>
      </Section>

      <Section index="4" heading="record — two microphones behind two tailpipes">
        <P>
          Nine seconds: idle, a cruise, one pull to the limiter, and back down
          onto the overrun. Both channels hear both pipes, quieter by the extra
          distance and later by the time sound takes to cross the gap. Nothing
          is widened or panned. The image is the geometry.
        </P>
        <Recording />
        <P>
          Switching cranks mid-playback is the entire thesis in one control.
          Nothing else about the engine changes.
        </P>
      </Section>

      <Section index="5" heading="run — a gauge cluster wired to the sensors">
        <P>
          The only instrument I cannot print here, because it is alive: rpm,
          manifold vacuum, cylinder pressure and knock index, redrawn in place
          while the engine idles in your terminal. Digits 0 to 9 open the
          throttle, space puts it on the floor, q gives it back.
        </P>
        <Note>
          It used to draw its interface down the screen a thousand times over
          if you left it running and came back. It no longer does that, and
          fixing it involved learning more about terminal raw mode than I meant
          to.
        </Note>
      </Section>

      <Section index="6" heading="verify — the inspection sheet">
        <P>
          Every number this project quotes, checked against something that
          existed before the code did. A derivative against finite differences,
          a burn rate against its own integral, a cylinder head against a flow
          bench, a firing order against the casting, a shock tube against its
          exact solution, and the thesis against a Fourier transform.
        </P>
        <Output from="./windsor verify">{output.verify}</Output>
        <P>
          The last block is the one I care about. Half-order energy is the
          signature of a pulse train that repeats every two revolutions instead
          of one, and the ratio between the two cranks is 742 to one. That
          number is the entire project, and it came out of four throw angles.
        </P>
      </Section>
    </Article>
  );
}
