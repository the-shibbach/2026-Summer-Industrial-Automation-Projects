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

const unsigned long INTERVAL = 50;
const float ALPHA = 0.05;

float SP = 0.0;
float pv = 0.0;
float integral = 0.0;
float lastError = 0.0;

unsigned long lastStep = 0;

// ---------------- LDR interlock settings ----------------

// Thresholds are slightly inside 0 and 4095 because ADC readings
// may not reach the exact end values.
const int SENSOR_LOW_LIMIT = 20;
const int SENSOR_HIGH_LIMIT = 4075;

// Fault must remain present for this long before tripping.
const unsigned long SENSOR_FAULT_DELAY_MS = 300;

bool sensorFaultPending = false;
unsigned long sensorFaultStart = 0;

// ---------------- Batch settings ----------------

enum Phase {
  RAMP,
  HOLD,
  RETURN,
  DONE,
  INTERLOCK
};

Phase phase = DONE;

const float TARGET = 3500.0;
const float TOL = 60.0;
const float COOLED = 1200.0;

const unsigned long HOLD_MS = 60000;

unsigned long batchStart = 0;
unsigned long phaseStart = 0;

// ---------------- Event logging ----------------

void logEvent(const char *message) {
  Serial.print("# ");
  Serial.print((millis() - batchStart) / 1000.0);
  Serial.print(" s, ");
  Serial.println(message);
}

// ---------------- Interlock ----------------

void triggerInterlock(const char *message, int raw) {
  // Do not repeatedly trigger an already-latched interlock.
  if (phase == INTERLOCK) {
    return;
  }

  phase = INTERLOCK;

  SP = 0.0;
  integral = 0.0;
  lastError = 0.0;

  // Turn the LED off immediately.
  analogWrite(LED_PIN, 0);

  Serial.println();
  Serial.println("****************************************");
  Serial.println("ERROR: LDR SENSOR INTERLOCK TRIPPED");
  Serial.print("Cause: ");
  Serial.println(message);
  Serial.print("Raw ADC reading: ");
  Serial.println(raw);
  Serial.println("LED OUTPUT FORCED OFF");
  Serial.println("Reset the ESP32 to clear the interlock.");
  Serial.println("****************************************");
  Serial.println();
}

void checkSensorInterlock(int raw) {
  bool sensorOutOfRange =
      raw <= SENSOR_LOW_LIMIT ||
      raw >= SENSOR_HIGH_LIMIT;

  if (!sensorOutOfRange) {
    sensorFaultPending = false;
    return;
  }

  if (!sensorFaultPending) {
    sensorFaultPending = true;
    sensorFaultStart = millis();
    return;
  }

  if (millis() - sensorFaultStart < SENSOR_FAULT_DELAY_MS) {
    return;
  }

  if (raw <= SENSOR_LOW_LIMIT) {
    triggerInterlock(
        "ADC reading is at or near 0 counts",
        raw
    );
  } else {
    triggerInterlock(
        "ADC reading is at or near 4095 counts",
        raw
    );
  }
}

// ---------------- Start a batch ----------------

void startBatch() {
  batchStart = millis();
  phaseStart = millis();

  phase = RAMP;
  SP = TARGET;

  integral = 0.0;
  lastError = SP - pv;

  sensorFaultPending = false;

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

    case INTERLOCK:
      SP = 0.0;
      analogWrite(LED_PIN, 0);
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

  // Initialise the filter from the real sensor reading.
  pv = analogRead(LDR_PIN);

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

  // Read the raw sensor value.
  int raw = analogRead(LDR_PIN);

  // Interlock uses the raw value so filtering cannot hide a fault.
  checkSensorInterlock(raw);

  // Smooth the process measurement.
  pv = (1.0 - ALPHA) * pv + ALPHA * raw;

  // Do not continue the batch after an interlock.
  if (phase != INTERLOCK) {
    stepBatch();
  }

  float outClamped = 0.0;

  if (phase != DONE && phase != INTERLOCK) {
    float error = SP - pv;
    float derivative = (error - lastError) / dt;

    float output =
        Kp * error +
        Ki * integral +
        Kd * derivative;

    outClamped = constrain(
        output,
        (float)OUT_MIN,
        (float)OUT_MAX
    );

    // Integral anti-windup.
    if (output >= OUT_MIN && output <= OUT_MAX) {
      integral += error * dt;
    }

    lastError = error;
  }

  // Final safety override.
  if (phase == DONE || phase == INTERLOCK) {
    outClamped = 0.0;
  }

  analogWrite(LED_PIN, (int)outClamped);

  // CSV output.
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