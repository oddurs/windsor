import path from 'node:path';

import withStylexTurbopack from '@stylexswc/nextjs-plugin/turbopack';
import type { NextConfig } from 'next';

// StyleX is compiled by SWC, not Babel: adding a styling system should not cost
// you the fast pipeline, and here it does not. Turbopack will not run webpack
// plugins, so the transform happens in a loader and the stylesheet is gathered
// afterwards by PostCSS — see postcss.config.mjs, which must agree with this.
//
// The root is read from the working directory, not from this file's own path.
// Next compiles this config into .next/build/chunks before running it, so by
// the time the code executes it no longer knows where it was written.
const root = process.cwd();

const config: NextConfig = {
  reactStrictMode: true,
  poweredByHeader: false,
};

export default withStylexTurbopack({
  rsOptions: {
    dev: process.env.NODE_ENV === 'development',
    treeshakeCompensation: true,
    aliases: { '@/*': [path.join(root, '*')] },
    unstable_moduleResolution: { type: 'commonJS', rootDir: root },
  },
})(config);
