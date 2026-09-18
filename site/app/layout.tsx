import type { Metadata } from 'next';
import type { ReactNode } from 'react';

import { Shell } from '@/components/shell';

import './globals.css';

export const metadata: Metadata = {
  title: {
    default: 'windsor',
    template: '%s — windsor',
  },
  description:
    'A Ford 302, modelled from first principles in C++23, for no reason.',
  openGraph: {
    title: 'windsor',
    description:
      'A Ford 302, modelled from first principles in C++23, for no reason.',
    type: 'website',
  },
};

export default function RootLayout({ children }: { children: ReactNode }) {
  return (
    <html lang="en">
      <body>
        <Shell>{children}</Shell>
      </body>
    </html>
  );
}
