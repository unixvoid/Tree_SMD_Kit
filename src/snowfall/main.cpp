#include <Arduino.h>
#include <avr/sleep.h>

// Same tree wiring as demo/pong: 9 charlie-free GPIO LEDs + button on PA2 (Arduino 9).
// Tree order below. Index 4 (PA1) is the apex of the tree; indices 0-3 run
// down one side from it and 5-8 down the other (the same geometry the
// meet-in-the-middle sweep relies on), so a flake falling from the apex walks
// outward along either half. megaTinyCore ATtiny1614 pin mapping:
//   0=PA4  1=PA5  2=PA6  3=PA7  4=PB3  5=PB2  6=PB1  7=PB0  8=PA1  9=PA2
const uint8_t ledPins[] = {5, 2, 7, 0, 8, 1, 6, 3, 4}; // PB2 PA6 PB0 PA4 PA1 PA5 PB1 PA7 PB3
const uint8_t numLEDs = sizeof(ledPins) / sizeof(ledPins[0]);

const unsigned long animationMinutes = 5; // Time in minutes for animation to run
const unsigned long animationDuration = animationMinutes * 60000UL;

// --- Snowfall tuning ---
const uint8_t apex = numLEDs / 2;        // Strand index of the tree top (PA1).
const uint8_t maxFlakes = 2;             // Concurrent falling flakes (heads).
const unsigned int fallStepMs = 130;     // Time for a flake to drop one level.
const unsigned int spawnGapMinMs = 250;  // Shortest gap between flake spawns.
const unsigned int spawnGapMaxMs = 1100; // Longest gap between flake spawns.
// Each flake keeps a one-step trail lit behind it, so at most 2 * 2 = 4 LEDs
// are ever on at once, at full brightness (no PWM).

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

// Flake state.
bool flakeActive[maxFlakes];
int8_t flakePos[maxFlakes];   // Current head position (strand index).
int8_t flakeDir[maxFlakes];   // -1 = falling toward index 0, +1 = toward the last index.
int8_t flakeTrail[maxFlakes]; // Position lit one step behind the head (-1 = none).
unsigned long nextSpawnMs = 0;

// Seed from the button-press moment plus noise read off the floating UPDI
// pin (ADC channel 0, Arduino pin 11) so each run falls differently.
void seedRandom() {
  uint16_t noise = 0;
  for (uint8_t i = 0; i < 4; i++) noise = (noise << 3) ^ (uint16_t)analogRead(11);
  randomSeed(millis() ^ ((unsigned long)noise << 8));
}

// Start a flake at the apex, dropping down a randomly chosen side.
// No-op if every slot already has a flake in it.
void spawnFlake() {
  for (uint8_t f = 0; f < maxFlakes; f++) {
    if (flakeActive[f]) continue;
    flakeActive[f] = true;
    flakePos[f] = apex;
    flakeDir[f] = random(0, 2) ? 1 : -1;
    flakeTrail[f] = -1;
    return;
  }
}

// Advance every flake one level down its side, aging the trails. A flake
// "melts" when it runs off the bottom edge; its trail lingers one extra step.
void ageFlakes() {
  for (uint8_t f = 0; f < maxFlakes; f++) {
    if (!flakeActive[f]) {
      flakeTrail[f] = -1;
      continue;
    }
    flakeTrail[f] = flakePos[f];
    int8_t next = flakePos[f] + flakeDir[f];
    if (next < 0 || next >= (int8_t)numLEDs) {
      flakeActive[f] = false;
    } else {
      flakePos[f] = next;
    }
  }
}

// Redraw the scene: every head and every live trail lit, everything else off.
void drawFlakes() {
  allLEDsOff();
  for (uint8_t f = 0; f < maxFlakes; f++) {
    if (flakeTrail[f] >= 0) digitalWrite(ledPins[flakeTrail[f]], HIGH);
    if (flakeActive[f]) digitalWrite(ledPins[flakePos[f]], HIGH);
  }
}

// Snowfall: spawn flakes at the apex of the tree and let them fall down
// either side at randomized intervals until the button is pressed or the
// animation times out. Leaves all LEDs off when it returns.
void snowfall(unsigned long animStart) {
  seedRandom();
  for (uint8_t f = 0; f < maxFlakes; f++) flakeActive[f] = false;
  nextSpawnMs = animStart; // First flake drops immediately.

  while (true) {
    if (millis() >= nextSpawnMs) {
      spawnFlake();
      nextSpawnMs = millis() + random(spawnGapMinMs, spawnGapMaxMs);
    }
    ageFlakes();
    drawFlakes();
    if (!stepWait(fallStepMs, animStart)) break;
  }
  allLEDsOff();
}

void loop() {
  // A button press (or wake from sleep) starts a fresh snowfall run.
  if (checkButton()) {
    snowfall(millis());
  }

  // Idle: all LEDs off, sleep in POWER_DOWN until the button wakes us.
  allLEDsOff();
  sleepNow();
}
