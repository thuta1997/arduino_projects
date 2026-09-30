#include <Arduino.h>

#include "MotorDriver.h"
#include "Kinematics.h"
#include "MotionPlanner.h"
#include "SerialProtocol.h"

void setup() {
  Serial.begin(115200);

  MotorDriver_Init();
  Kinematics_Init();
  MotionPlanner_InitTimer();

  Serial.println("ESP32 Motion Controller Ready");
  Serial.println("Commands:");
  Serial.println("MOVE,800,610,686");
  Serial.println("STATUS");
  Serial.println("START_MISSION");
  Serial.println("STOP");
  //Serial.println("SET_HOME");
  delay(2000);
  MotionPlanner_StartAutoHome();
  //Serial.println("Auto Homing done");
}

void loop() {
  SerialProtocol_Update();
  MotionPlanner_Update();
}