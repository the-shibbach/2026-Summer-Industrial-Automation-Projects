#include <Arduino.h>
#include <math.h>

// ---------------- Pins ----------------

const int LDR_PIN = 34;
const int LED_PIN = 25;

// ---------------- PID settings ----------------

float Kp = 0.30;
float Ki = 0.10;
float Kd = 0.00;

const int OUT_MIN = 0;
const int OUT_MAX = 255;

const unsigned long INTERVAL = 50;  // PID interval, ms
const float ALPHA = 0.05;           // LDR smoothing factor

float SP = 0.0;          // Setpoint must be changeable
float pv = 0.0;
float integral = 0.0;
float lastError = 0.0;

unsigned long lastStep = 0;

// ---------------- Batch settings ----------------

enum Phase {
  RAMP,
  HOLD,
  RETURN,
  DONE
};

Phase phase = DONE;

const float TARGET = 3500.0;        // Desired LDR reading
const float TOL = 60.0;             // Target tolerance
const float COOLED = 1200.0;        // Return considered complete
const unsigned long HOLD_MS = 60000; // Hold for 60 seconds

unsigned long batchStart = 0;
unsigned long phaseStart = 0;

// ---------------- Event logging ----------------

void logEvent(const char *message) {
  Serial.print("# ");
  Serial.print((millis() - batchStart) / 1000.0);
  Serial.print(" s, ");
  Serial.println(message);
}

// ---------------- Start a batch ----------------

void startBatch() {
  batchStart = millis();
  phaseStart = millis();

  phase = RAMP;
  SP = TARGET;

  integral = 0.0;
  lastError = SP - pv;

  logEvent("BATCH START");
  logEvent("PHASE RAMP");
}

// ---------------- Batch state machine ----------------

void stepBatch() {
  switch (phase) {
    case RAMP:
      if (fabsf(pv - TARGET) < TOL) {
        phase = HOLD;
        phaseStart = millis();

        logEvent("PHASE HOLD");
      }
      break;

    case HOLD:
      if (millis() - phaseStart >= HOLD_MS) {
        phase = RETURN;
        phaseStart = millis();

        SP = 0.0;

        // Clear accumulated PID action when returning.
        integral = 0.0;
        lastError = SP - pv;

        logEvent("PHASE RETURN");
      }
      break;

    case RETURN:
      if (pv < COOLED) {
        phase = DONE;
        SP = 0.0;
        integral = 0.0;

        analogWrite(LED_PIN, 0);

        logEvent("BATCH COMPLETE");
      }
      break;

    case DONE:
      SP = 0.0;
      break;
  }
}

// ---------------- Arduino setup ----------------

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  analogWriteResolution(LED_PIN, 8);
  analogWrite(LED_PIN, 0);

  Serial.println("seconds,phase,sp,pv,output");

  startBatch();
}

// ---------------- Main control loop ----------------

void loop() {
  unsigned long now = millis();

  if (now - lastStep < INTERVAL) {
    return;
  }

  float dt = (now - lastStep) / 1000.0;
  lastStep = now;

  // Read and smooth the LDR measurement.
  int raw = analogRead(LDR_PIN);
  pv = (1.0 - ALPHA) * pv + ALPHA * raw;

  // Update the batch phase before calculating PID output.
  stepBatch();

  float error = SP - pv;
  float derivative = (error - lastError) / dt;

  float output =
      Kp * error +
      Ki * integral +
      Kd * derivative;

  float outClamped = constrain(
      output,
      (float)OUT_MIN,
      (float)OUT_MAX
  );

  // Basic integral anti-windup.
  if (output >= OUT_MIN && output <= OUT_MAX) {
    integral += error * dt;
  }

  if (phase == DONE) {
    outClamped = 0.0;
  }

  analogWrite(LED_PIN, (int)outClamped);

  lastError = error;

  // CSV output for Serial Monitor or Serial Plotter.
  Serial.print((millis() - batchStart) / 1000.0);
  Serial.print(",");

  Serial.print((int)phase);
  Serial.print(",");

  Serial.print(SP);
  Serial.print(",");

  Serial.print(pv);
  Serial.print(",");

  Serial.println(outClamped);
}