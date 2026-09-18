import { A, Article, P, Title } from '@/components/prose';

export default function NotFound() {
  return (
    <Article>
      <Title sub="Nothing is fitted here.">404</Title>
      <P>
        The part you asked for is not in the catalogue.{' '}
        <A href="/">Back to the engine →</A>
      </P>
    </Article>
  );
}
