<div align="center">

# Christmas Tree Ornament SMD Soldering Kit

[![Firmware Build](https://github.com/unixvoid/Tree_SMD_Kit/actions/workflows/build.yml/badge.svg)](https://github.com/unixvoid/Tree_SMD_Kit/actions/workflows/build.yml)
[![Firmware flasher](https://img.shields.io/badge/%F0%9F%94%A5%20Flasher-unixvoid.github.io%2FTree__SMD__Kit-green.svg)](https://unixvoid.github.io/Tree_SMD_Kit/)
[![Build guide](https://img.shields.io/badge/%F0%9F%94%A7%20Build%20guide-unixvoid.github.io%2FTree__SMD__Kit%2Fbuild-blue.svg)](https://unixvoid.github.io/Tree_SMD_Kit/build)
[![ATtiny1614](https://img.shields.io/badge/ATtiny1614-tinyAVR-0FA0CE?labelColor=333&logo=microchip&logoColor=white)]()

**[Flash an animation directly from your browser → unixvoid.github.io/Tree_SMD_Kit](https://unixvoid.github.io/Tree_SMD_Kit/)**

</div>

## Overview

Coin-cell powered Christmas tree ornament based on the ATtiny1614.
Press the button to start/stop the animation; the animation auto-stops after
5 minutes, and the chip sleeps in `POWER_DOWN` (wake on button) when idle to
save the coin cell. Need to load a new animation? Use the hosted
[web flasher](https://unixvoid.github.io/Tree_SMD_Kit/) — it talks to your
ornament right from the browser, no PlatformIO install needed. Soldering the kit up for the first
time? The [assembly guide](https://unixvoid.github.io/Tree_SMD_Kit/build) walks through all
thirteen steps.

---

## Web flasher

The flasher lives in [`docs/`](docs/) and is deployed to
**[unixvoid.github.io/Tree_SMD_Kit](https://unixvoid.github.io/Tree_SMD_Kit/)**
via GitHub Pages (`.github/workflows/pages.yml` builds the Vite site in
`docs/` and publishes `docs/dist`).

How it works:

1. Connect a serialUPDI programmer, open the flasher in **Chrome, Edge, or
   Opera** (Web Serial support required), and click **Connect**.
2. The flasher auto-detects the ATtiny1614 over UPDI and lists the animations
   published by CI at `s3://unixvoid-builds/ornament/<name>/firmware.hex`
   (pick `demo`, `pong`, … from the dropdown).
3. Click **Program Device** — the `.hex` is fetched, flashed over UPDI,
   verified, and the BOD fuse is programmed to keep the coin cell safe.

The animation list is generated, not hardcoded: every `[env:<name>]` in
`platformio.ini` builds to `.pio/build/<name>/firmware.hex`, CI uploads each
one to S3 on pushes to `main`, and the flasher enumerates that S3 prefix at
connect time — so adding an animation (see below) automatically adds it to the
flasher.

## Assembly guide

The soldering guide lives at
**[unixvoid.github.io/Tree_SMD_Kit/build](https://unixvoid.github.io/Tree_SMD_Kit/build)** — thirteen
steps, three soldering methods, and the LED/cathode orientation that's easiest to get wrong.

It's a second Vite page (`docs/build/index.html`) built by the same
`.github/workflows/pages.yml` run as the flasher, so `docs/dist/build/index.html` ships alongside
`docs/dist/index.html` and there's nothing extra to deploy:

```sh
cd docs
npm install
npm run dev     # flasher at /  , guide at /build/
npm run build   # -> dist/index.html + dist/build/index.html
```

The guide resolves at `/build` and `/build/` on Pages — GitHub Pages 301-redirects a directory
path to its trailing-slash form, the same way `unixvoid.github.io/Tree_SMD_Kit` redirects today.
When testing locally, use the trailing slash (`npm run preview` then `/build/`); the local dev and
preview servers serve the directory index only with it.

### Photos

Guide photos live in `docs/public/img/` and are referenced from the page as `/img/<name>.jpg`.
With `base: './'`, Vite rewrites those to `../img/...` for the nested page, so they resolve
correctly under the `/Tree_SMD_Kit/` prefix with no configuration. They're copied verbatim, so
the URLs are stable and unhashed.

To replace one, drop a new file over the old name in `docs/public/img/`. To add one, add a
`<figure class="shot">` to `docs/build/index.html` — keep the `width`/`height` attributes, which
reserve the right space before the file loads, and a descriptive `alt`, since the alt text is what a
visitor sees if the photo fails to load.

Aim for 1600 px on the long edge at JPEG quality ~80 (roughly 200–400 KB each). The current set went
in straight off a camera at ~16 MB in total — nothing breaks, but it's slow on a phone at the bench,
and resizing to 1600 px would bring the page down to around 2 MB.
**[`docs/PHOTOS.md`](docs/PHOTOS.md) lists what each photo is and where it appears.**

### Editing the guide

- **Steps** are `<li class="step" id="step-NN">` blocks inside `<ol class="steps">` in
  `docs/build/index.html`, in the order they appear. Add one and add a matching entry to the step
  rail at the top of the file.
- **Styles** are in `docs/build.css`, layered on top of `docs/styles.css`. The palette, fonts and
  the `.shell`/`.hero`/`.panel` primitives come from `styles.css`; `build.css` only adds
  guide-specific layout, so the flasher page can't regress. The guide also prints cleanly (one step
  per page) via a `@media print` block at the bottom of `build.css`.

## Animations (`src/<name>/main.cpp`)

| Animation | What it does | Power notes |
|-----------|--------------|-------------|
| `demo` | Zigzag snake, sequential sweep, random sparkle | Up to 3 LEDs on at once |
| `pong` | Fast ping-pong chase alternating with "meet in the middle" sweeps (both ends, then 1 in from each side, ... to the single middle LED, then back out) | Up to 2 LEDs at once, full brightness (no PWM) |
| `snowfall` | Flakes drop from the tree apex and fall down either side with a one-step trail, at randomized intervals | Up to 4 LEDs at once (2 flakes + trails), full brightness (no PWM) |

Tune the pattern in `src/pong/main.cpp`: `pingStepMs` (chase speed),
`pingPasses` (traversals per ping-pong phase), `meetStepMs` /
`meetRepeats` (meet-in-the-middle speed and repeats), and
`animationMinutes` (how long it runs before sleeping).

## Build & flash

One PlatformIO environment per animation — the env name **is** the animation name:

```ini
[env:demo]
build_src_filter = +<demo/>

[env:pong]
build_src_filter = +<pong/>
```

```sh
pio run              # build all animations
pio run -e pong      # build just pong
pio run -e pong -t upload   # flash via serialupdi
```

Adding an animation: copy `src/demo/` to `src/<name>/`, edit it, and add a
matching `[env:<name>]` block with `build_src_filter = +<<name>/>` to
`platformio.ini`. CI (`.github/workflows/build.yml`) builds every env and
publishes `.pio/build/<name>/firmware.hex` to
`s3://unixvoid-builds/ornament/<name>/`, which the browser flasher in `docs/`
lists.

## Hardware

- MCU: ATtiny1614 (Arduino/megaTinyCore, `board = ATtiny1614`)
- 9 LEDs on Arduino pins `{8, 0, 1, 7, 6, 2, 3, 5, 4}`, button on pin `9` (PA2, falling-edge wake)
