#include <Arduino.h>
#include <avr/sleep.h>

// Same tree wiring as demo: 9 charlie-free GPIO LEDs + button on PA2 (Arduino 9).
// Scan order below is the physical tree order. The pattern alternates between
// a ping-pong chase along that order and "meet in the middle" sweeps.
// megaTinyCore ATtiny1614 pin mapping:
//   0=PA4  1=PA5  2=PA6  3=PA7  4=PB3  5=PB2  6=PB1  7=PB0  8=PA1  9=PA2
const uint8_t ledPins[] = {5, 2, 7, 0, 8, 1, 6, 3, 4}; // PB2 PA6 PB0 PA4 PA1 PA5 PB1 PA7 PB3
const uint8_t numLEDs = sizeof(ledPins) / sizeof(ledPins[0]);

const unsigned long animationMinutes = 5; // Time in minutes for animation to run
const unsigned long animationDuration = animationMinutes * 60000UL;

// --- Pattern tuning ---
// LEDs run at full brightness (no PWM). At most TWO are lit at once (the
// meet-in-the-middle rows), which is within the demo's 3-at-once coin-cell
// budget.
const unsigned int pingStepMs = 70;  // Per-LED hold in the ping-pong chase.
const uint8_t pingPasses = 6;        // End-to-end traversals per ping-pong phase.
const unsigned int meetStepMs = 110; // How long each meeting row stays lit.
const uint8_t meetRepeats = 3;       // Meet-in-the-middle repeats per phase.

volatile bool buttonPressed = false;

ISR(PORTA_PORT_vect) {
  PORTA.INTFLAGS = PIN2_bm;
  buttonPressed = true;
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  PORTA.INTFLAGS = PIN2_bm;
  sei();
  sleep_cpu();
  sleep_disable();
}

void setup() {
  PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN2CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;

  for (uint8_t i = 0; i < numLEDs; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
}

void allLEDsOff() {
  for (uint8_t i = 0; i < numLEDs; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

bool checkButton() {
  if (buttonPressed) {
    buttonPressed = false;
    delay(50);
    if (digitalRead(9) == LOW) {
      while (digitalRead(9) == LOW);
      delay(50);
      return true;
    }
  }
  return false;
}

// Delay `ms` while watching for the button and the animation timeout every
// couple of ms, so a press stops the pattern promptly. Returns false to stop.
bool stepWait(unsigned int ms, unsigned long animStart) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (checkButton()) return false;
    if (millis() - animStart >= animationDuration) return false;
    delay(2);
  }
  return true;
}

// Light LEDs one at a time from `from` to `to` inclusive, each held for
// pingStepMs. Returns false if the animation should stop (button or timeout);
// the caller turns everything off.
bool sweep(uint8_t from, uint8_t to, unsigned long animStart) {
  int8_t dir = (to > from) ? 1 : -1;
  for (int8_t i = from; i != to + dir; i += dir) {
    digitalWrite(ledPins[i], HIGH);
    if (!stepWait(pingStepMs, animStart)) {
      digitalWrite(ledPins[i], LOW);
      return false;
    }
    digitalWrite(ledPins[i], LOW);
  }
  return true;
}

// Phase 1: ping-pong the scan point back and forth along the tree, one LED
// at a time, for pingPasses end-to-end traversals.
bool pingPongPhase(unsigned long animStart) {
  for (uint8_t pass = 0; pass < pingPasses; pass++) {
    // pass 0 goes 0 -> last, pass 1 returns, pass 2 goes out again, ...
    if (pass & 1) {
      if (!sweep(numLEDs - 1, 0, animStart)) return false;
    } else {
      if (!sweep(0, numLEDs - 1, animStart)) return false;
    }
  }
  return true;
}

// Light the pair `off` positions in from each end (just the single middle LED
// when off is numLEDs/2) for meetStepMs. Returns false if the animation should
// stop (button or timeout); the caller turns everything off.
bool meetRow(uint8_t off, unsigned long animStart) {
  uint8_t a = off;
  uint8_t b = numLEDs - 1 - off;
  digitalWrite(ledPins[a], HIGH);
  if (b != a) digitalWrite(ledPins[b], HIGH);
  if (!stepWait(meetStepMs, animStart)) return false;
  digitalWrite(ledPins[a], LOW);
  if (b != a) digitalWrite(ledPins[b], LOW);
  return true;
}

// Phase 2: "meet in the middle" - light the two ends together, then one in
// from each side, then two in from each side, ... until only the single
// middle LED is lit, then run back out the same way. The tree appears to run
// up both sides, converge, and come back down.
bool meetPhase(unsigned long animStart) {
  for (uint8_t rep = 0; rep < meetRepeats; rep++) {
    // Converge up: ends, 1 in from each side, ..., middle single.
    for (uint8_t off = 0; off <= numLEDs / 2; off++) {
      if (!meetRow(off, animStart)) return false;
    }
    // Expand back down: out one step at a time to the ends again. The middle
    // was the previous row, so start one out from it (no repeat).
    for (int8_t off = numLEDs / 2 - 1; off >= 0; off--) {
      if (!meetRow((uint8_t)off, animStart)) return false;
    }
  }
  return true;
}

// Alternate ping-pong and meet-in-the-middle phases until the button is
// pressed or the animation times out. Leaves all LEDs off when it returns.
void scanAnimation(unsigned long animStart) {
  while (true) {
    if (!pingPongPhase(animStart)) break;
    if (!meetPhase(animStart)) break;
  }
  allLEDsOff();
}

void loop() {
  // A button press (or wake from sleep) starts a fresh animation run.
  if (checkButton()) {
    scanAnimation(millis());
  }

  // Idle: all LEDs off, sleep in POWER_DOWN until the button wakes us.
  allLEDsOff();
  sleepNow();
}
