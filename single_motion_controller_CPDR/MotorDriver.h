#pragma once
#include <Arduino.h>

void MotorDriver_Init();

void MotorDriver_Enable(int motorIndex);
void MotorDriver_Disable(int motorIndex);
void MotorDriver_EnableAll();
void MotorDriver_DisableAll();

void MotorDriver_SetDirection(int motorIndex, long stepCount);
void MotorDriver_GenerateStepPulse(int motorIndex);