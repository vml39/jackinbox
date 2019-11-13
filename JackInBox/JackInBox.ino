#include <Stepper.h>
#include <analogWrite.h>
#include "pitches.h"
#include "LiquidCrystal_I2C.h"

extern analog_write_channel_t _analog_write_channels[16];

// Declare constants
const float STEPS_PER_REV = 32;
const float MAX_STEPS = 1500;
const float MIN_STEPS = 1400;
const float MIN_DIST = 25;
const float MAX_DIST = 35;
int pos_step = 24;
int neg_step = -24;

// Initialize stepper
Stepper steppermotor(STEPS_PER_REV, 18, 16, 19, 17);

// Initialize LCD
LiquidCrystal_I2C lcd(0x27,16,2);

// Initialize pins
const int RANGE = 12;
const int speaker = 21;

// Declare vars
int range_val;
float range_volt;
float range_cm;
long dir = 0;
bool step_dir_pos = true;
long num_steps = 0;
int lives = 9;
int pos = 0;

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// tone definition for ESP32
void tone(int pin, int frequency) {
  // 2*N and 2*N+1 channels sahre the same frequency
  int channel = analogWriteChannel(pin);

  if (channel != -1 && channel < 16) {
    _analog_write_channels[channel].frequency = frequency;
    ledcSetup(channel, 
              _analog_write_channels[channel].frequency,
              _analog_write_channels[channel].resolution);
    // set the pluse width to 50%
    analogWrite(pin,2,4);
  }
}

// noTone definition for ESP32
void noTone(int pin) {
  analogWrite(pin, 0);
}

// prints the number of lives remaining to the LCD
void printLives(int l) {
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(String(l)+" lives");
  lcd.setCursor(0,1);
  lcd.print("remaining");
}

void setup() {
  Serial.begin(9600);

  steppermotor.setSpeed(750);
  lcd.init();
  pinMode(speaker, OUTPUT);
}

void loop() {
  printLives(lives);
 
  noTone(speaker);

  if (pos == 0) {
    steppermotor.step(MAX_STEPS);
    pos = MAX_STEPS;
  }

  range_val = analogRead(RANGE);
  range_volt = range_val/1240.9;
  if (range_volt > 0) {
    range_cm = 60.0/range_volt - 6;
  }

  if (pos > MIN_STEPS && pos <= MAX_STEPS && range_cm > MIN_DIST && range_cm < MAX_DIST) {
    lives -= 1;
    printLives(lives);
    tone(speaker, 200);
    delay(1000);
    noTone(speaker);
    steppermotor.step(-pos);
    pos = 0;
  } else if (num_steps < pos_step) {
    dir = random(0,2);
    if (dir == 0 && pos > MAX_STEPS-500) {
      num_steps = random(pos_step, 500);
      step_dir_pos = false;
    } else {
      num_steps = random(pos_step, MAX_STEPS-pos);
      step_dir_pos = true;
    }
  } else {
    if (step_dir_pos) {
      steppermotor.step(pos_step);
      pos += pos_step;
    } else {
      steppermotor.step(neg_step);
      pos += neg_step;
    }
    num_steps -= pos_step;  
  }

  if (lives == 0 && pos == 0) {
    for (int n = 0; n < 8; n++) {
      int dur = 1000/noteDurations[n];
      tone(speaker, melody[n]);
      int pause_bw = dur * 1.30;
      delay(pause_bw);
      noTone(speaker);
    }
    lives = 9;
    printLives(lives);
    delay(5000);
  }
}
