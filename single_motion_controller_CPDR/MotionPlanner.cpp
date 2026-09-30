#include "MotionPlanner.h"
#include "MotorDriver.h"
#include "Kinematics.h"

static hw_timer_t *motionTimer = NULL;

static volatile long plannedMotorSteps[MOTOR_COUNT] = {0, 0, 0, 0};
static volatile long absoluteMotorSteps[MOTOR_COUNT] = {0, 0, 0, 0};
static volatile long completedMotorSteps[MOTOR_COUNT] = {0, 0, 0, 0};
static volatile long stepErrorAccumulator[MOTOR_COUNT] = {0, 0, 0, 0};
static volatile long signedCompletedMotorSteps[MOTOR_COUNT] = {0, 0, 0, 0};
static volatile long maximumPlannedSteps = 0;
static volatile long motionTickCounter = 0;

static volatile bool isMotionRunning = false;
static volatile bool isMotionCompleted = false;

//update
static bool isSafeMoveActive = false;
static int safeMovePhase = 0;
static Point3D safeMoveTarget;

static int baseFreqHz = TIMER_FREQ_HZ;
static int currentFreqHz = TIMER_FREQ_HZ;

static MotionState motionState = STATE_WAIT_HOME;
static bool isMissionActive = false;
static int missionPhase = 0;

MotionState MotionPlanner_GetState() {
  return motionState;
}

bool MotionPlanner_PlanMoveToPosition(float x, float y, float z) {
  if(motionState==STATE_WAIT_HOME)
  {
    Serial.println("ERROR: HOME FIRST");
    return false;
  }

  if(motionState==STATE_MANUAL_HOME)
  {
    Serial.println("ERROR: HOME FIRST");
    return false;
  }
  if (isMotionRunning) {
    Serial.println("ERROR,BUSY");
    return false;
  }

  if (!Kinematics_IsInsideWorkspace(x, y, z)) {
    motionState = STATE_ERROR;
    Serial.println("ERROR,WORKSPACE");
    return false;
  }

  motionState = STATE_PLANNING;

  Point3D targetPosition = {x, y, z};
  float targetCableLength[MOTOR_COUNT];
  float currentCableLength[MOTOR_COUNT];

  Kinematics_CalculateCableLengths(targetPosition, targetCableLength);
  Kinematics_GetCurrentCableLengths(currentCableLength);

  Serial.println("===== MOTION PLAN =====");

  for (int i = 0; i < MOTOR_COUNT; i++) {
    float cableLengthChange = targetCableLength[i] - currentCableLength[i];
    plannedMotorSteps[i] = Kinematics_ConvertLengthChangeToSteps(cableLengthChange);

    Serial.print("M");
    Serial.print(i + 1);
    Serial.print(" deltaL=");
    Serial.print(cableLengthChange, 2);
    Serial.print(" mm, steps=");
    Serial.println(plannedMotorSteps[i]);
  }

  Kinematics_SetTargetPosition(targetPosition);

  MotionPlanner_StartPlannedMotion();
  return true;
}

void MotionPlanner_StartPlannedMotion() {
  maximumPlannedSteps = 0;
 
  for (int i = 0; i < MOTOR_COUNT; i++) {
    absoluteMotorSteps[i] = abs(plannedMotorSteps[i]);
    completedMotorSteps[i] = 0;
    stepErrorAccumulator[i] = 0;
    signedCompletedMotorSteps[i] = 0;

    MotorDriver_SetDirection(i, plannedMotorSteps[i]);

    if (absoluteMotorSteps[i] > maximumPlannedSteps) {
      maximumPlannedSteps = absoluteMotorSteps[i];
    }
  }

  if (maximumPlannedSteps == 0) {
    isMotionCompleted = true;
    motionState = STATE_DONE;
    return;
  }

  motionTickCounter = 0;
  isMotionCompleted = false;

  delay(5);
  MotorDriver_EnableAll();
  delay(10);

  isMotionRunning = true;
  motionState = STATE_MOVING;

  Serial.println("MOVING");
}

void MotionPlanner_Stop() {
  isMotionRunning = false;
  isMotionCompleted = false;

  float currentCableLength[MOTOR_COUNT];
  Kinematics_GetCurrentCableLengths(currentCableLength);

  for (int i = 0; i < MOTOR_COUNT; i++) {
    float movedLength =
      Kinematics_ConvertStepsToLength(signedCompletedMotorSteps[i]);

    currentCableLength[i] += movedLength;
  }

  Kinematics_SetCurrentCableLengths(currentCableLength);

  //MotorDriver_DisableAll();

  motionState = STATE_STOPPED;

  Serial.println("STOPPED");
}

void IRAM_ATTR MotionPlanner_TimerISR() {
  if (!isMotionRunning) return;

  if (motionTickCounter >= maximumPlannedSteps) {
    isMotionRunning = false;
    isMotionCompleted = true;
    return;
  }

  for (int motor = 0; motor < MOTOR_COUNT; motor++) {
    if (completedMotorSteps[motor] < absoluteMotorSteps[motor]) {
      stepErrorAccumulator[motor] += absoluteMotorSteps[motor];

      if (stepErrorAccumulator[motor] >= maximumPlannedSteps) {
        MotorDriver_GenerateStepPulse(motor);
        completedMotorSteps[motor]++;

        if (plannedMotorSteps[motor] >= 0) {
          signedCompletedMotorSteps[motor]++;
        } else {
          signedCompletedMotorSteps[motor]--;
        }

        stepErrorAccumulator[motor] -= maximumPlannedSteps;
      }
    }
  }

  motionTickCounter++;
}

void MotionPlanner_InitTimer() {
  // Timer ကို 1 MHz (1 tick = 1 microsecond) ဖြင့် အသေထားမည် (Timer Bug ကို ဖြေရှင်းရန်)
  motionTimer = timerBegin(1000000);
  timerAttachInterrupt(motionTimer, &MotionPlanner_TimerISR);
  timerAlarm(motionTimer, 1000000 / TIMER_FREQ_HZ, true, 0);
}

void MotionPlanner_Update() {
  if (isMotionCompleted) {
    isMotionCompleted = false;

   // MotorDriver_DisableAll();

    motionState = STATE_DONE;
    Point3D target = Kinematics_GetTargetPosition();

    Kinematics_SetCurrentPosition(target);

    float targetCableLength[MOTOR_COUNT];
    Kinematics_CalculateCableLengths(target, targetCableLength);
    Kinematics_SetCurrentCableLengths(targetCableLength);

    Serial.println("DONE");

    for (int i = 0; i < MOTOR_COUNT; i++) {
      Serial.print("M");
      Serial.print(i + 1);
      Serial.print(" = ");
      Serial.println(completedMotorSteps[i]);
    }

    motionState = STATE_IDLE;
    if (isMissionActive) {
      delay(1000); // ဆင်းသက်ပြီး/ရောက်ရှိပြီးတိုင်း ၁ စက္ကန့် ရပ်နားမည်
      MotionPlanner_AdvanceMission();
    }
    // --- Safe Move အတွက် ထပ်ထည့်သော Code ---
    else if (isSafeMoveActive) {
      delay(500); // အဆင့်တစ်ခုနှင့် တစ်ခုကြား 0.5 စက္ကန့် ရပ်
      MotionPlanner_AdvanceSafeMove();
    }
  }
}

void MotionPlanner_StartManualHome()
{
    motionState = STATE_MANUAL_HOME;
    MotorDriver_DisableAll();
    Serial.println("MANUAL_HOME_MODE");
}

void MotionPlanner_FinishManualHome()
{
   Point3D home = {
        FRAME_X_MM / 2.0f,
        FRAME_Y_MM / 2.0f,
        GROUND_Z_MM // မြေပြင်အမြင့်ကို Home အဖြစ် သတ်မှတ်သည်
    };

    float cable[MOTOR_COUNT];

    Kinematics_CalculateCableLengths(home, cable);

    Kinematics_SetCurrentPosition(home);

    Kinematics_SetTargetPosition(home);

    Kinematics_SetCurrentCableLengths(cable);

    motionState = STATE_IDLE;
    MotorDriver_EnableAll();
    Serial.println("HOME_SUCCESS");
}


//mission 
void MotionPlanner_StartMission() {
  if (motionState != STATE_IDLE) {
    Serial.println("ERROR: SYSTEM MUST BE IDLE (HOME FIRST)");
    return;
  }
  isMissionActive = true;
  missionPhase = 0;
  Serial.println("MISSION STARTED: PREPARING TO LAUNCH");
  
  MotionPlanner_AdvanceMission(); // ပထမဆုံးအဆင့်ကို စတင်မည်
}

void MotionPlanner_StartSafeMove(float x, float y, float z) {
  if (motionState != STATE_IDLE) {
    Serial.println("ERROR: SYSTEM MUST BE IDLE");
    return;
  }
  
  // သွားရမည့် နောက်ဆုံး Target ကို မှတ်ထားမည်
  safeMoveTarget = {x, y, z};
  isSafeMoveActive = true;
  safeMovePhase = 0;
  
  Serial.println("SAFE MOVE STARTED");
  MotionPlanner_AdvanceSafeMove();
}

void MotionPlanner_AdvanceSafeMove() {
  if (!isSafeMoveActive) return;

  Point3D current = Kinematics_GetCurrentPosition();
  float targetX, targetY, targetZ;

  switch (safeMovePhase) {
    case 0: // အဆင့် ၁: လက်ရှိနေရာမှ အပေါ် Transit Height သို့ အရင်တက်မည်
      targetX = current.x; targetY = current.y; targetZ = TRANSIT_Z_MM;
      Serial.println("SAFE MOVE: ASCENDING...");
      break;

    case 1: // အဆင့် ၂: လိုချင်သော X, Y နေရာသို့ လေထဲမှ ရွှေ့မည်
      targetX = safeMoveTarget.x; targetY = safeMoveTarget.y; targetZ = TRANSIT_Z_MM;
      Serial.println("SAFE MOVE: MOVING XY...");
      break;

    case 2: // အဆင့် ၃: လိုချင်သော Z အမြင့်သို့ အောက်ဆင်းမည်
      targetX = safeMoveTarget.x; targetY = safeMoveTarget.y; targetZ = safeMoveTarget.z;
      Serial.println("SAFE MOVE: DESCENDING...");
      break;

    default: // ပြီးဆုံးသွားပြီ
      isSafeMoveActive = false;
      Serial.println("SAFE MOVE COMPLETE");
      return;
  }

  safeMovePhase++;
  // တွက်ချက်ထားသော လမ်းကြောင်းအတိုင်း သွားရန် Command ပေးမည်
  MotionPlanner_PlanMoveToPosition(targetX, targetY, targetZ);
}

// -----------------------------------------------------------------
// Power-ON Auto Homing စနစ်
// -----------------------------------------------------------------
void MotionPlanner_StartAutoHome() {
  
  // ၁။ စမှတ်အလယ်အောက်
  Point3D startPos = {FRAME_X_MM / 2.0f, FRAME_Y_MM / 2.0f, 180.0};
  float startCable[MOTOR_COUNT];

  Kinematics_CalculateCableLengths(startPos, startCable);
  Kinematics_SetCurrentPosition(startPos);
  Kinematics_SetCurrentCableLengths(startCable);

  // ၂။ မော်တာများကို ရွှေ့ရန်အတွက် စက်ကို IDLE အနေအထားသို့ အရင်ပြောင်းသည်
  motionState = STATE_IDLE;

  Serial.println("SET_HOME: Ascending and moving to Standby Point (250, 250)...");
  MotionPlanner_StartSafeMove(250, 250, TRANSIT_Z_MM);
}

// -----------------------------------------------------------------
// Cartesian Jogging (X, Y, Z ဝင်ရိုးအတိုင်း အနည်းငယ်စီ ရွှေ့ခြင်း)
// -----------------------------------------------------------------
void MotionPlanner_JogCartesian(float dx, float dy, float dz) {
  if (motionState != STATE_IDLE) {
    Serial.println("ERROR: SYSTEM MUST BE IDLE TO JOG");
    return;
  }

  Point3D current = Kinematics_GetCurrentPosition();
  float targetX = current.x + dx;
  float targetY = current.y + dy;
  float targetZ = current.z + dz;

  Serial.print("JOGGING TO -> X:"); Serial.print(targetX);
  Serial.print(" Y:"); Serial.print(targetY);
  Serial.print(" Z:"); Serial.println(targetZ);

  // move to the target 
  MotionPlanner_PlanMoveToPosition(targetX, targetY, targetZ);
}

void MotionPlanner_SetSpeed(int freqHz) {
  baseFreqHz = freqHz;
  currentFreqHz = freqHz;
  if (motionTimer == NULL) {
    motionTimer = timerBegin(1000000); // 1MHz base clock (1 tick = 1 us)
    timerAttachInterrupt(motionTimer, &MotionPlanner_TimerISR);
  }
  //set the timer counter to 0
  timerWrite(motionTimer, 0);
  // change the new frequency
  timerAlarm(motionTimer, 1000000 / currentFreqHz, true, 0);
}


void MotionPlanner_AdvanceMission() {
  if (!isMissionActive) return;

  float targetX, targetY, targetZ;
  Point3D current = Kinematics_GetCurrentPosition();

  switch (missionPhase) {
    case 0: 
      // XY movement
      targetX = MISSION_WAYPOINTS[0][0]; // 1050.0
      targetY = MISSION_WAYPOINTS[0][1]; // 975.0
      targetZ = TRANSIT_Z_MM;
      
      // for 15 sec with normal Hz
      MotionPlanner_SetSpeed(TIMER_FREQ_HZ); 
      Serial.println("MISSION: Moving XY Diagonally (Takes ~15s)...");
      break;

    case 1: 
      // Landing
      targetX = MISSION_WAYPOINTS[0][0]; 
      targetY = MISSION_WAYPOINTS[0][1]; 
      targetZ = GROUND_Z_MM;
      
      // reduce Hz for 45 sec
      MotionPlanner_SetSpeed(270); 
      Serial.println("MISSION: Landing Slowly (Takes ~45s)...");
      break;


    default:
      // Mission complete
      isMissionActive = false;
      Serial.println("MISSION COMPLETE: TOUCHDOWN CONFIRMED");
      return;
  }

  Serial.print("MISSION PHASE ");
  Serial.print(missionPhase);
  Serial.println(" INITIATED");
  
  missionPhase++; 
  
  // goto target point
  MotionPlanner_PlanMoveToPosition(targetX, targetY, targetZ);
}