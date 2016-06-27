#include <SimpleTimer.h>
#include <Wire.h>

SimpleTimer timer;
// pin assignments [RED, GREEN, BLUE, AMBER, YELLOW]
int buttonPins[][5] = {{10, 0, 0}, {11, 0, 0}, {12, 0, 0}, {13, 0, 0}, {14, 0, 0}}; // {pin, currBtnState, lastBtnState}
unsigned long btnLastDebounce[] = {0, 0, 0, 0, 0};
int buzzerPins[] = {5, 6, 7, 8, 9};
int resetPin[] = {4, 0, 0}; // {pin, currBtnState, lastBtnState}
unsigned long rstLastDebounce = 0;
int debounceDelay = 10;
int ranks[] = {0, 0, 0, 0, 0};

void setup() {
  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i][0], INPUT);
  }
  pinMode(4, INPUT);
  Wire.begin(); // join i2c bus (address optional for master)

  timer.setInterval(1, checkInput);
}

void loop() {
  timer.run();
}

void checkInput() {
  checkResetInput();
  checkButtonInput();
}

void checkResetInput() {
  int reading = 0;
  // check the reset button
  reading = digitalRead(resetPin[0]);
  if (reading != resetPin[2]) {
    rstLastDebounce = millis();
  }
  if ((millis() - rstLastDebounce) > debounceDelay) {
    if (reading != resetPin[1]) {
      resetPin[1] = reading;
      if (resetPin[1] == HIGH) {
        reset();
      }
    }
  }
  resetPin[2] = reading;
}

void checkButtonInput() {
  int reading = 0;
  for (int i = 0; i < 5; i++) {
    if (ranks[i] != 0) {
      // ignore ranked buzzers until game is reset
      continue;
    }
    reading = digitalRead(buttonPins[i][0]);
    if (reading != buttonPins[i][2]) {
      btnLastDebounce[i] = millis();
    }
    if ((millis() - btnLastDebounce[i]) > debounceDelay) {
      if (reading != buttonPins[i][1]) {
        buttonPins[i][1] = reading;
        if (buttonPins[i][1] == HIGH) {
          registerBuzz(i);
        }
      }
    }
    buttonPins[i][2] = reading;
  }
}

void registerBuzz(int buzzer) {
  // check ranks array. zero-value indices indicate no-input buzzers
  int unbuzzed = 0;
  for (int i = 0; i < 5; i++) {
    if (ranks[i] == 0) {
      unbuzzed++;
    }
  }

  if (unbuzzed == 0) {
    // everyone have pressed the buzzer, so ignore subsequent input until reset
    return;
  }
  
  // save rank to array
  ranks[buzzer] = 6 - unbuzzed;

  if (ranks[buzzer] == 1) {
    // if this is the first one, activate its buzzer & lights
    activateBuzzer(buzzer);
  }

  // activate the rank indicator light on the master box as well as the buzzer lights
  activateLights(buzzer);
}

void activateBuzzer(int buzzer) {
  tone(buzzerPins[buzzer], 300 + (buzzer*50), 1000); // play different tones
  timer.setTimeout(1500, deactivateBuzzers);
}

void deactivateBuzzers() {
  for (int i = 0; i < 5; i++) {
    noTone(buzzerPins[i]);
  }
}

void activateLights(int buzzer) {
  Wire.beginTransmission(8);
  Wire.write(buzzer);         // send buzzer index
  Wire.write(ranks[buzzer]);  // send buzzer rank
  Wire.endTransmission();
}

void deactivateLights() {
  Wire.beginTransmission(8);
  Wire.write(8);              // int 8 sends a deactivate command
  Wire.write(8);              // filler value
  Wire.endTransmission();
}

void reset() {
  deactivateBuzzers();
  deactivateLights();

  // reset ranks
  for (int i = 0; i < 5; i++) {
    ranks[i] = 0;
  }
}

