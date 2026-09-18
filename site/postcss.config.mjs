import path from 'node:path';

// Turbopack compiles the StyleX calls; this pass gathers the rules they emitted
// and writes the one stylesheet the page loads. The options below must match
// next.config.ts — the variable names StyleX generates are derived from the
// resolved paths, so two different roots would produce two different names for
// the same token, and the stylesheet would not match the markup.
const root = process.cwd();

const config = {
  plugins: {
    '@stylexswc/postcss-plugin': {
      include: ['app/**/*.{ts,tsx}', 'components/**/*.{ts,tsx}', 'design/**/*.{ts,tsx}'],
      rsOptions: {
        dev: process.env.NODE_ENV === 'development',
        treeshakeCompensation: true,
        aliases: { '@/*': [path.join(root, '*')] },
        unstable_moduleResolution: { type: 'commonJS', rootDir: root },
      },
    },
  },
};

export default config;
