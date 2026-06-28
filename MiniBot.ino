#include <IRremote.hpp>
#include <SoftwareSerial.h>

// ESP32 connection (RX = 12, TX = 13 not used)
SoftwareSerial espSerial(12, 13);

class Motor {
private:
  int in1;
  int in2;
  int en;
  bool invert;
public:
  Motor(int i1, int i2, int enablePin, bool reverse) {
    in1 = i1;
    in2 = i2;
    en = enablePin;
    invert = reverse;
  }

  void begin() {
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(en, OUTPUT);
  }

  void moveRotations(int rotations) {
    double diameter = 2 + (10.0 / 16);  // inches
    double r = diameter / 2;            // find out how many seconds one rotation is
    double area = PI * pow(r, 2);
    double circumfrence = 2 * PI * r;
  }

  void moveTime(int speed, int seconds) {
    speed = constrain(speed, -255, 255);
    if (speed >= 0) {  // move forward
      forward(speed);
      delay(seconds * 1000);
      stop();
    } else {  // move backward
      backward(abs(speed));
      delay(seconds * 1000);
      stop();
    }
  }

  void move(int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {  // move forward
      forward(speed);
    } else if (speed < 0) {  // move backward
      backward(abs(speed));
    } else {
      stop();
    }
  }

  void forward(int speed) {
    if (!invert) {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(en, speed);
    } else {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(en, speed);
    }
  }

  void backward(int speed) {
    if (!invert) {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(en, speed);
    } else {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(en, speed);
    }
  }

  void stop() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(en, 0);
  }
};

class Joystick {
private:
  int pinX, pinY, button, deadzone;

  int centerX = 512;
  int centerY = 512;

public:
  Joystick(int xPin, int yPin, int buttonPin) {
    pinX = xPin;
    pinY = yPin;
    button = buttonPin;
    deadzone = 50;
  }

  Joystick(int xPin, int yPin, int buttonPin, int deadzoneV) {
    pinX = xPin;
    pinY = yPin;
    button = buttonPin;
    deadzone = deadzoneV;
  }

  void begin() {
    pinMode(button, INPUT_PULLUP);
  }

  void calibrate() {
    centerX = analogRead(pinX);
    centerY = analogRead(pinY);
  }

  int getX() {
    int x = analogRead(pinX) - centerX;

    if (abs(x) < deadzone) return 0;
    return x;
  }

  int getY() {
    int y = analogRead(pinY) - centerY;

    if (abs(y) < deadzone) return 0;
    return y;
  }

  bool isPressed() {
    return digitalRead(button) == LOW;
  }

  // Optional: normalized -1 to 1 output
  float getXNorm() {
    return getX() / 512.0;
  }

  float getYNorm() {
    return getY() / 512.0;
  }
};

class IRreceiver {
private:
  int pinNum;
public:
  IRreceiver(int pin) {
    pinNum = pin;
  }

  void begin() {
    IrReceiver.begin(pinNum, ENABLE_LED_FEEDBACK);
  }

  int receive() {
    if (IrReceiver.decode()) {
      int cmd = IrReceiver.decodedIRData.command;
      IrReceiver.resume();
      return cmd;
    }
    return -1;
  }


  String translate(int msg) {
    String button = "";
    switch (msg) {
      case 69: button = "Power"; break;
      case 70: button = "VOL+"; break;
      case 71: button = "FUNC/STOP"; break;
      case 68: button = "|<<"; break;
      case 64: button = ">||"; break;
      case 67: button = ">>|"; break;
      case 7: button = "Down"; break;
      case 21: button = "VOL-"; break;
      case 9: button = "Up"; break;
      case 22: button = "0"; break;
      case 25: button = "EQ"; break;
      case 13: button = "ST/REPT"; break;
      case 12: button = "1"; break;
      case 24: button = "2"; break;
      case 94: button = "3"; break;
      case 8: button = "4"; break;
      case 28: button = "5"; break;
      case 90: button = "6"; break;
      case 66: button = "7"; break;
      case 82: button = "8"; break;
      case 74: button = "9"; break;
      case -1: button = "No input"; break;
      default: button = "?" + String(msg); break;
    }
    return button;
  }
};

// add robot class
Motor left(9, 11, 10, false);  // in 1, int 2, enable A
Joystick one(A0, A1, 2);
Motor right(3, 5, 6, true);  //  in 3, in 4, enable B
IRreceiver ir(4);

String lastCommand = "";

void setup() {
  Serial.begin(9600);
  left.begin();
  right.begin();
  ir.begin();
  one.begin();
  one.calibrate();
  espSerial.begin(9600);
}

void handleCommand(char cmd) {
  Serial.print("Received: ");
  Serial.println(cmd);

  switch (cmd) {

    case 'F': // Forward
      left.move(255);
      right.move(255);
      break;

    case 'B': // Backward
      left.move(-255);
      right.move(-255);
      break;

    case 'L': // Left
      left.move(-255);
      right.move(255);
      break;

    case 'R': // Right
      left.move(255);
      right.move(-255);
      break;

    case 'S': // Stop
      left.stop();
      right.stop();
      break;
  }
}

void loop() {
  if (espSerial.available()) {
    char cmd = espSerial.read();
    handleCommand(cmd);
  }
  // JOYSTICK
  float xNorm = one.getXNorm();
  int x = (int)(xNorm * 255);
  float yNorm = one.getYNorm();
  int y = (int)(yNorm * 255);

  /*
  if (y != 0) {
    if (y > 0) { // turn left
      right.move(y);
      left.move(-y);
    } else { // turn right
      right.move(-y);
      left.move(y);
    }
  } else {
    left.move(x);
    right.move(x);
  }
  */

  // IR RECEIVER
  String button = ir.translate(ir.receive());
  Serial.println(button);
  if (lastCommand.equals(button)) {
    delay(50);
    return;
  }
  if (button.equals("Up") && !lastCommand.equals(button)) {
    right.move(255);
    left.move(255);
  } else if (button.equals("Down") && !lastCommand.equals(button)) {
    right.move(-255);
    left.move(-255);
  } else if (button.equals("|<<") && !lastCommand.equals(button)) {
    left.move(-255);
    right.move(255);
  } else if (button.equals(">>|") && !lastCommand.equals(button)) {
    left.move(255);
    right.move(-255);
  } else if (button.equals("Power") && !lastCommand.equals(button)) {
    left.stop();
    right.stop();
  }
  lastCommand = button;


  delay(50);
}
