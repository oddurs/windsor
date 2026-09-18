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

// The site is served from a project page, which lives in a subdirectory. Next
// rewrites its own asset URLs for that, but a path written by hand — the two
// recordings — it cannot see, so the same value is published to the client and
// used there. Empty locally, so `pnpm dev` still serves from the root.
const base = process.env.NEXT_PUBLIC_BASE_PATH ?? '';

const config: NextConfig = {
  reactStrictMode: true,
  poweredByHeader: false,

  // A static export, because the site is five pages that were prerendered
  // anyway and a server would be a thing to keep alive for no reason.
  output: 'export',
  images: { unoptimized: true },

  // Pages serves directories, not extensionless files: /engine/index.html is
  // found at /engine and /engine.html is not.
  trailingSlash: true,

  basePath: base,
  assetPrefix: base.length > 0 ? base : undefined,
};

export default withStylexTurbopack({
  rsOptions: {
    dev: process.env.NODE_ENV === 'development',
    treeshakeCompensation: true,
    aliases: { '@/*': [path.join(root, '*')] },
    unstable_moduleResolution: { type: 'commonJS', rootDir: root },
  },
})(config);
