import { defineConfig } from 'vite';

export default defineConfig({
  root: '.',
  base: './',
  // Two real pages (flasher + assembly guide) with no client-side routing, so
  // declare an MPA: unknown paths 404 instead of serving the flasher as a
  // fallback. Vite still resolves /build -> /build/index.html in dev.
  appType: 'mpa',
  build: {
    outDir: 'dist',
    emptyOutDir: true,
    sourcemap: false,
    minify: false,
    rollupOptions: {
      input: {
        main: 'index.html',
        // Assembly guide -> dist/build/index.html -> /Tree_SMD_Kit/build
        build: 'build/index.html'
      }
    }
  },
  server: {
    open: true,
    port: 3000,
  },
});
