#include "SerialProtocol.h"
#include "MotionPlanner.h"
#include "Kinematics.h"

static String commandBuffer = "";

static void processCommand(String command) {
  command.trim();

  Serial.print("RX: ");
  Serial.println(command);

  if (command.startsWith("MOVE")) {
    int c1 = command.indexOf(',');
    int c2 = command.indexOf(',', c1 + 1);
    int c3 = command.indexOf(',', c2 + 1);

    if (c1 < 0 || c2 < 0 || c3 < 0) {
      Serial.println("ERROR,FORMAT");
      return;
    }

    float x = command.substring(c1 + 1, c2).toFloat();
    float y = command.substring(c2 + 1, c3).toFloat();
    float z = command.substring(c3 + 1).toFloat();
    MotionPlanner_SetSpeed(TIMER_FREQ_HZ); 
    //MotionPlanner_PlanMoveToPosition(x, y, z);
    MotionPlanner_StartSafeMove(x, y, z);
  }
  else if (command == "HOME"){
    Serial.println("RETURNING TO HOME");
    MotionPlanner_SetSpeed(TIMER_FREQ_HZ); 
    MotionPlanner_StartSafeMove(FRAME_X_MM/2, FRAME_Y_MM/2, GROUND_Z_MM);
  }
  else if (command == "READY") {
    Serial.println("GOING TO READY POSITION ...");
    MotionPlanner_SetSpeed(TIMER_FREQ_HZ); 
    MotionPlanner_StartSafeMove(250.0, 250.0, TRANSIT_Z_MM);
  }
  else if (command == "STATUS") {
    SerialProtocol_PrintStatus();
  }

  else if (command == "STOP") {
    MotionPlanner_Stop();
  }
 
  else if(command == "START_MISSION")
  {
    MotionPlanner_StartMission();
  }
  else if (command == "SET_HOME") {
    Serial.println("RECEIVED: SET_HOME command");
    MotionPlanner_StartAutoHome();
  }
  else if (command.startsWith("JOG_X")) {
    int c1 = command.indexOf(',');
    if (c1 > 0) {
      float dist = command.substring(c1 + 1).toFloat();
      MotionPlanner_JogCartesian(dist, 0, 0);
    }
  }
  else if (command.startsWith("JOG_Y")) {
    int c1 = command.indexOf(',');
    if (c1 > 0) {
      float dist = command.substring(c1 + 1).toFloat();
      MotionPlanner_JogCartesian(0, dist, 0);
    }
  }
  else if (command.startsWith("JOG_Z")) {
    int c1 = command.indexOf(',');
    if (c1 > 0) {
      float dist = command.substring(c1 + 1).toFloat();
      MotionPlanner_JogCartesian(0, 0, dist);
    }
  }
  else {
    Serial.println("ERROR,UNKNOWN_COMMAND");
  }
}

void SerialProtocol_Update() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      processCommand(commandBuffer);
      commandBuffer = "";
    } else if (c != '\r') {
      commandBuffer += c;
    }
  }
}

void SerialProtocol_PrintStatus() {
  Point3D p = Kinematics_GetCurrentPosition();

  Serial.print("STATUS,");

  switch (MotionPlanner_GetState()) {
    case STATE_IDLE:     Serial.print("IDLE"); break;
    case STATE_PLANNING: Serial.print("PLANNING"); break;
    case STATE_MOVING:   Serial.print("MOVING"); break;
    case STATE_DONE:     Serial.print("DONE"); break;
    case STATE_STOPPED:  Serial.print("STOPPED"); break;
    case STATE_WAIT_HOME: Serial.print("WAIT_HOME"); break;
    case STATE_MANUAL_HOME: Serial.print("MANUAL_HOME"); break;
    case STATE_ERROR:    Serial.print("ERROR"); break;
  }

  Serial.print(",POS,");
  Serial.print(p.x);
  Serial.print(",");
  Serial.print(p.y);
  Serial.print(",");
  Serial.println(p.z);
}