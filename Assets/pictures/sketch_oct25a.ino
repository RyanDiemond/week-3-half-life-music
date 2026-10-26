#include <Adafruit_NeoPixel.h>
#include "USB.h"
#include "USBMIDI.h"

USBMIDI MIDI("MIDI Keypad");

#define LED_PIN 9
#define NUM_LEDS 4

Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int touchPins[] = {1, 2, 3, 4, 5};
int buttonPins[] = {8, 43};
int notes[] = {60, 62, 64, 65, 67, 69, 72};

bool touchState[5] = {};
bool buttonState[2] = {};

int sliderPin = 6;
int encoderA = 44;
int encoderB = 7;

int octave = 0;
int lastA = HIGH;

void setup() {
  Serial.begin(115200);

  for (int pin : buttonPins) {
    pinMode(pin, INPUT_PULLUP);
  }

  pinMode(encoderA, INPUT_PULLUP);
  pinMode(encoderB, INPUT_PULLUP);

  analogReadResolution(12);

  leds.begin();
  leds.setBrightness(40);
  leds.show();

  MIDI.begin();
  USB.begin();
}

void loop() {
  // Touch pads: MIDI notes only, no LEDs
  for (int i = 0; i < 5; i++) {
    int value = touchRead(touchPins[i]);
    bool pressed = value < 20; // Adjust if needed

    if (pressed != touchState[i]) {
      int note = notes[i] + octave * 12;

      if (pressed) {
        MIDI.noteOn(note, 100, 1);
      } else {
        MIDI.noteOff(note, 0, 1);
      }

      touchState[i] = pressed;
    }
  }

  // Buttons: play notes and light LEDs 0 and 1
  for (int i = 0; i < 2; i++) {
    bool pressed = digitalRead(buttonPins[i]) == LOW;
    int note = notes[i + 5] + octave * 12;

    if (pressed != buttonState[i]) {
      if (pressed) {
        MIDI.noteOn(note, 100, 1);
      } else {
        MIDI.noteOff(note, 0, 1);
      }

      buttonState[i] = pressed;
    }

    if (pressed) {
      leds.setPixelColor(i, leds.Color(255, 0, 0));
    } else {
      leds.setPixelColor(i, 0);
    }
  }

  // Slider: LED 2 turns on above halfway
  bool sliderActive = analogRead(sliderPin) > 2048;

  if (sliderActive) {
    leds.setPixelColor(2, leds.Color(0, 255, 0));
  } else {
    leds.setPixelColor(2, 0);
  }

  // Rotary encoder: change octave
  int a = digitalRead(encoderA);

  if (lastA == HIGH && a == LOW) {
    if (digitalRead(encoderB) == HIGH) {
      octave++;
    } else {
      octave--;
    }

    octave = constrain(octave, -3, 3);
  }

  lastA = a;

  // LED 3 indicates the octave setting
  if (octave < 0) {
    leds.setPixelColor(3, leds.Color(0, 0, 255));
  } else if (octave > 0) {
    leds.setPixelColor(3, leds.Color(150, 0, 255));
  } else {
    leds.setPixelColor(3, 0);
  }

  leds.show();
  delay(10);
}