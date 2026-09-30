#pragma once
#include <Arduino.h>
#include <math.h>

#define MOTOR_COUNT 4

const int STEP_PINS[MOTOR_COUNT] = {23, 27, 33, 22};
const int DIR_PINS[MOTOR_COUNT]  = {15, 26, 32, 19};
const int EN_PINS[MOTOR_COUNT]   = {4, 25, 18, 21};

const int DRIVER_ENABLE_ACTIVE  = LOW;
const int DRIVER_DISABLE_ACTIVE = HIGH;

//normal speed
const int TIMER_FREQ_HZ = 1000;

const float FRAME_X_MM = 1448.0; //4'9"
const float FRAME_Y_MM = 1219.0;//4'
const float FRAME_Z_MM = 1524.0;//5'

const float PULLEY_DIAMETER_MM = 40.0;
const float PULLEY_CIRCUMFERENCE_MM = PI * PULLEY_DIAMETER_MM;

const int STEPS_PER_REV = 3200;

struct Point3D {
  float x;
  float y;
  float z;
};

// z-value at ground level
const float GROUND_Z_MM = 190.0; 

// height of xy movement
const float TRANSIT_Z_MM = 700.0; 

// landing coordinate
const float MISSION_WAYPOINTS[1][2] = {
  {1200.0, 500.0},   // Point 1 (X, Y)
};

enum MotionState
{
    STATE_WAIT_HOME,

    STATE_MANUAL_HOME,

    STATE_IDLE,

    STATE_PLANNING,

    STATE_MOVING,

    STATE_DONE,

    STATE_STOPPED,

    STATE_ERROR
};