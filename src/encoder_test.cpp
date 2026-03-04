#include <Arduino.h>

const int pinA = 2;
const int pinB = 3;
const int pinI = 18;

volatile long encoderCount = 0;

void handleEncoder() {
  bool a = digitalRead(pinA);
  bool b = digitalRead(pinB);

  static bool lastA = false;
  static bool lastB = false;

  if (a != lastA || b != lastB) {
    if (lastA ^ b) {
      encoderCount++;
    } else {
      encoderCount--;
    }
  }
  lastA = a;
  lastB = b;
}

void resetIndex(){
  encoderCount = 0;
}

void setup() {
  Serial.begin(115200); 
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(pinI, INPUT_PULLUP);

  // A相、B相の両方の変化を監視することで精度を最大化（4逓倍）
  attachInterrupt(digitalPinToInterrupt(pinA), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinI), resetIndex, CHANGE);

  Serial.println("AS5047D ABI Encoder Ready");
}

void loop() {
  Serial.print("Count: ");
  Serial.println(encoderCount);
}