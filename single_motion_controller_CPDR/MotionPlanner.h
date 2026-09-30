#pragma once
#include "Config.h"

void MotionPlanner_InitTimer();

bool MotionPlanner_PlanMoveToPosition(float x, float y, float z);
void MotionPlanner_StartPlannedMotion();
void MotionPlanner_Stop();

void MotionPlanner_Update();

MotionState MotionPlanner_GetState();

void IRAM_ATTR MotionPlanner_TimerISR();
void MotionPlanner_StartManualHome();
void MotionPlanner_FinishManualHome();

//for mission 
void MotionPlanner_StartMission();
void MotionPlanner_AdvanceMission();

// update
void MotionPlanner_StartSafeMove(float x, float y, float z);
void MotionPlanner_AdvanceSafeMove();

//auto home
void MotionPlanner_StartAutoHome();

//jogging
void MotionPlanner_JogCartesian(float dx, float dy, float dz);

//speed_control
void MotionPlanner_SetSpeed(int freqHz);