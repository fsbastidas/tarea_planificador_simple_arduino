#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define NUM_TASKS 4
#define TICK_INTERVAL 50

Servo motor;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int potValue = 0;
int motorPosition = 0;
bool blinkingEnabled = false;
bool customCharVisible = false;
unsigned long lastCustomCharToggle = 0;
unsigned long customCharInterval = 50;

typedef void (*TaskFunction)();

struct Task {
  TaskFunction func;
  unsigned long interval;
  unsigned long lastExecTime;
};

Task taskList[NUM_TASKS];

byte CustomCharacter[8]= { B11111,
                           B10001,
                           B10001,
                           B10001,
                           B10001,
                           B10001,
                           B10001,
                           B11111};

void readPotentiometer() {
  potValue = analogRead(A0);
}

void updateMotorPosition() {
  motorPosition = map(potValue, 0, 1023, 0, 100);
  motor.write(motorPosition);
}

void toggleBlinking() {
  blinkingEnabled = !blinkingEnabled;
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);

  int numBlocks = (motorPosition * 16) / 100;
  for (int i = 0; i < numBlocks; i++) {
    lcd.write(0xFF);
  }

  lcd.setCursor(0, 1);
  if (customCharVisible) {
    lcd.setCursor(0, 1);
    lcd.write((uint8_t)0);
  } else {
    lcd.setCursor(0, 1);
    lcd.print(" ");
  }
}

void setup() {
  motor.attach(9);
  Wire.begin();
  lcd.init();
  lcd.backlight();

  taskList[0] = {readPotentiometer, 50, 0};
  taskList[1] = {updateMotorPosition, 50, 0};
  taskList[2] = {toggleBlinking, 200, 0};
  taskList[3] = {updateLCD, 200, 0};

  lcd.createChar(0, CustomCharacter);

  customCharInterval = 500;
  lastCustomCharToggle = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  for (int i = 0; i < NUM_TASKS; i++) {
    if (currentMillis - taskList[i].lastExecTime >= taskList[i].interval) {
      taskList[i].lastExecTime = currentMillis;
      taskList[i].func();
    }
  }

  if (blinkingEnabled && currentMillis - lastCustomCharToggle >= customCharInterval) {
    lastCustomCharToggle = currentMillis;
    customCharVisible = !customCharVisible;
  }

  delay(TICK_INTERVAL);
}
