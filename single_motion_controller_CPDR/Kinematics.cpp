#include "Kinematics.h"

static Point3D anchorPoints[MOTOR_COUNT] = {
  {0,          FRAME_Y_MM, FRAME_Z_MM},
  {FRAME_X_MM, FRAME_Y_MM, FRAME_Z_MM},
  {FRAME_X_MM, 0,          FRAME_Z_MM},
  {0,          0,          FRAME_Z_MM}
};

static Point3D currentPosition = {762.0, 610.0, 686.0};
static Point3D targetPosition  = {762.0, 610.0, 686.0};

static float currentCableLength[MOTOR_COUNT];

void Kinematics_Init() {
  Kinematics_CalculateCableLengths(currentPosition, currentCableLength);
}

float Kinematics_CalculateCableLength(Point3D anchor, Point3D position) {
  float dx = anchor.x - position.x;
  float dy = anchor.y - position.y;
  float dz = anchor.z - position.z;

  return sqrt(dx * dx + dy * dy + dz * dz);
}

void Kinematics_CalculateCableLengths(Point3D position, float cableLengths[MOTOR_COUNT]) {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    cableLengths[i] = Kinematics_CalculateCableLength(anchorPoints[i], position);
  }
}

long Kinematics_ConvertLengthChangeToSteps(float lengthChangeMm) {
  float revolutions = lengthChangeMm / PULLEY_CIRCUMFERENCE_MM;
  return round(revolutions * STEPS_PER_REV);
}

bool Kinematics_IsInsideWorkspace(float x, float y, float z) {
  if (x < 0 || x > FRAME_X_MM) return false;
  if (y < 0 || y > FRAME_Y_MM) return false;
  if (z < 0 || z > FRAME_Z_MM) return false;
  return true;
}

Point3D Kinematics_GetCurrentPosition() {
  return currentPosition;
}

void Kinematics_SetCurrentPosition(Point3D position) {
  currentPosition = position;
}

void Kinematics_GetCurrentCableLengths(float cableLengths[MOTOR_COUNT]) {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    cableLengths[i] = currentCableLength[i];
  }
}

void Kinematics_SetCurrentCableLengths(float cableLengths[MOTOR_COUNT]) {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    currentCableLength[i] = cableLengths[i];
  }
}
float Kinematics_ConvertStepsToLength(long steps)
{
  float revolutions = (float)steps / STEPS_PER_REV;
  return revolutions * PULLEY_CIRCUMFERENCE_MM;
}

Point3D Kinematics_GetTargetPosition()
{
  return targetPosition;
}

void Kinematics_SetTargetPosition(Point3D position)
{
  targetPosition = position;
}