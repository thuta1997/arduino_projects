#pragma once
#include "Config.h"

void Kinematics_Init();

float Kinematics_CalculateCableLength(Point3D anchor, Point3D position);
void Kinematics_CalculateCableLengths(Point3D position, float cableLengths[MOTOR_COUNT]);
float Kinematics_ConvertStepsToLength(long steps);

Point3D Kinematics_GetTargetPosition();
void Kinematics_SetTargetPosition(Point3D position);

long Kinematics_ConvertLengthChangeToSteps(float lengthChangeMm);

bool Kinematics_IsInsideWorkspace(float x, float y, float z);

Point3D Kinematics_GetCurrentPosition();
void Kinematics_SetCurrentPosition(Point3D position);

void Kinematics_GetCurrentCableLengths(float cableLengths[MOTOR_COUNT]);
void Kinematics_SetCurrentCableLengths(float cableLengths[MOTOR_COUNT]);