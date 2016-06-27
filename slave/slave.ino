#include <SimpleTimer.h>
#include <Wire.h>

SimpleTimer timer;
// pin assignments [RED, GREEN, BLUE, AMBER, YELLOW]
int buzzerLEDPins[] = {2, 3, 4, 5, 6};
int rankIndicatorPins[][5] = {{7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}}; // {pin, state} where state can be 0 = OFF, 1 = ON, n > 1 = blinking
unsigned long lastCheck[] = {0, 0, 0, 0, 0};

void setup() {
  for (int i = 0; i < 5; i++) {
    pinMode(buzzerLEDPins[i], OUTPUT);
    pinMode(rankIndicatorPins[i][0], OUTPUT);
  }
  
  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  timer.setInterval(1, manageLEDState);
}

void loop() {
  timer.run();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int bytesReceived) {
  int c = Wire.read(); // receive first byte
  int v = Wire.read(); // receive second byte

  parseTransmission(c, v);
}

void parseTransmission(int c, int v) {
  if (c != 8) {
    // data transmitted is rank
    activateLights(c, v);
  }
  else {
    // data transmitted is reset
    deactivateLights();
  }
}

void activateLights(int c, int v) {
  // activate buzzer leds
  if (v == 1) {
    // turn on the buzzer led
    digitalWrite(buzzerLEDPins[c], HIGH);
  }

  // turn on rank indicator leds
  activateIndicatorLED(c, v);
}

void activateIndicatorLED(int c, int v) {
  switch(v) {
    case 1:
      // solid
      digitalWrite(rankIndicatorPins[c][0], HIGH);
      rankIndicatorPins[c][1] = 1;
      break;
    case 2:
      // very fast blink
      rankIndicatorPins[c][1] = 125;
      break;
    case 3:
      // fast blink
      rankIndicatorPins[c][1] = 250;
      break;
    case 4:
      // slow blink
      rankIndicatorPins[c][1] = 500;
      break;
    case 5:
      // off
      break;
    default:
      break;
  }
}

void deactivateLights() {
  for (int i = 0; i < 5; i++) {
    // turn off buzzer LED
    digitalWrite(buzzerLEDPins[i], LOW);
    // turn off rank indicator LED
    rankIndicatorPins[i][1] = 0;
    rankIndicatorPins[i][2] = 0;
    digitalWrite(rankIndicatorPins[i][0], LOW);
  }
}

void manageLEDState() {
  for (int i = 0; i < 5; i++) {
    if (rankIndicatorPins[i][1] > 1 && (millis() - lastCheck[i]) > rankIndicatorPins[i][1]) {
      digitalWrite(rankIndicatorPins[i][0], !digitalRead(rankIndicatorPins[i][0])); // flip pin state
      lastCheck[i] = millis();
    }
  }
}

