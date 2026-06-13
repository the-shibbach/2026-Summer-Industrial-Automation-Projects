const int THERMISTOR_PIN = 12;
const float VCC = 3.3;
const float R_fixed = 10000.0;
const float BETA = 2500.0;

void setup(){
  Serial.begin(115200);
  Serial.println("seconds, Celsius");
}

void loop(){
  float raw = analogRead(THERMISTOR_PIN);
  float VOUT = (raw / 4095.0) * 3.3;
  float R_thermistor = (VOUT * R_fixed) / (VCC - VOUT);
  float Temperature = 1.0 / (1.0 / 298 + (1.0 / BETA) * log(R_thermistor/8500.0)) - 273;

  Serial.print(millis() / 1000.0);
  Serial.print(",");
  Serial.println(Temperature);

  delay(1000);
}
