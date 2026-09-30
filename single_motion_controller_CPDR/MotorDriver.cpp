#include "MotorDriver.h"
#include "Config.h"

void MotorDriver_Init() {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    pinMode(STEP_PINS[i], OUTPUT);
    pinMode(DIR_PINS[i], OUTPUT);
    pinMode(EN_PINS[i], OUTPUT);

    digitalWrite(STEP_PINS[i], LOW);
    digitalWrite(DIR_PINS[i], HIGH);

    MotorDriver_Enable(i);
  }
}

void MotorDriver_Enable(int motorIndex) {
  digitalWrite(EN_PINS[motorIndex], DRIVER_ENABLE_ACTIVE);
}

void MotorDriver_Disable(int motorIndex) {
  digitalWrite(EN_PINS[motorIndex], DRIVER_DISABLE_ACTIVE);
}

void MotorDriver_EnableAll() {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    MotorDriver_Enable(i);
  }
}

void MotorDriver_DisableAll() {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    MotorDriver_Disable(i);
  }
}

void MotorDriver_SetDirection(int motorIndex, long stepCount) {
  digitalWrite(DIR_PINS[motorIndex], stepCount >= 0 ? HIGH : LOW);
}

void MotorDriver_GenerateStepPulse(int motorIndex) {
  digitalWrite(STEP_PINS[motorIndex], HIGH);
  delayMicroseconds(5);
  digitalWrite(STEP_PINS[motorIndex], LOW);
}