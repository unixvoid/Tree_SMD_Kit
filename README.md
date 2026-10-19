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
[web flasher](https://unixvoid.github.io/Tree_SMD_Kit/) it talks to your
ornament right from the browser, no PlatformIO install needed. Soldering the kit up for the first
time? The [assembly guide](https://unixvoid.github.io/Tree_SMD_Kit/build) walks through all
thirteen steps.

---

## Web flasher

Load a new animation onto your ornament straight from your browser no
toolchain or PlatformIO install needed. Open the flasher at
**[unixvoid.github.io/Tree_SMD_Kit](https://unixvoid.github.io/Tree_SMD_Kit/)**
in **Chrome, Edge, or Opera** (Web Serial support required):

1. Connect a serialUPDI programmer and click **Connect**. The flasher
   auto-detects the ATtiny1614 over UPDI and lists the available animations
   (pick `demo`, `pong`, `snowfall`, … from the dropdown).
2. Click **Program Device** the firmware is flashed over UPDI and verified,
   and the BOD fuse is programmed to keep the coin cell safe.

## Assembly guide

The soldering guide lives at
**[unixvoid.github.io/Tree_SMD_Kit/build](https://unixvoid.github.io/Tree_SMD_Kit/build)** 
It walks through placing every part, the three ways to solder it (hand, hot plate, hot air),
flashing the firmware, and fitting the coin cell with a photo for each step.

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

## Building from source

The firmware is a [PlatformIO](https://platformio.org/) project with one
environment per animation, the env name **is** the animation name:

```sh
pio run                     # build all animations
pio run -e pong             # build just pong
pio run -e pong -t upload   # flash via serialupdi
```

To add an animation, copy `src/demo/` to `src/<name>/`, edit it, and add a
matching `[env:<name>]` block with `build_src_filter = +<<name>/>` to
`platformio.ini`. Pushing to `main` builds every environment and publishes the
firmware, so new animations show up in the web flasher automatically.

## Hardware

- MCU: ATtiny1614 (Arduino/megaTinyCore, `board = ATtiny1614`)
- 9 LEDs on Arduino pins `{8, 0, 1, 7, 6, 2, 3, 5, 4}`, button on pin `9` (PA2, falling-edge wake)