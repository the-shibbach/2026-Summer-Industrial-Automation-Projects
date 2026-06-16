const int LDR_PIN = 34;
const int LED_PIN = 25;

float Kp = 0.30;
float Ki = 0.10;
float Kd = 0.00;

const float SP = 3500.0;
const int OUT_MIN = 0;
const int OUT_MAX = 255;
const int INTERVAL = 50;
const float ALPHA = 0.05;

float pv = 0;
float integral = 0;
float lastError = 0;
int lastStep = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("seconds,sp,pv,output");
}

void loop() { 
  // put your main code here, to run repeatedly:
  if(millis() - lastStep < INTERVAL) return;
  float dt = (millis() - lastStep) / 1000.0;
  lastStep = millis();

  int raw = analogRead(LDR_PIN);
  pv = (1-ALPHA)*pv + ALPHA*raw;

  float error = SP - pv;
  float derivative = (error - lastError) / dt;
  float output = Kp*error + Ki*integral + Kd*derivative;

  float outClamped = constrain(output, OUT_MIN, OUT_MAX);

  if(output == outClamped){
    integral = integral + dt*error;
  }

  analogWrite(LED_PIN, (int)outClamped);
  lastError = error;

  Serial.print(millis()/1000.0); Serial.print(",");
  Serial.print(SP);  Serial.print(",");
  Serial.print(pv);  Serial.print(",");
  Serial.println(outClamped);
}
