const int RELAY_PIN = 13;
const int LDR_PIN = 34;
const float sp = 3800.0;
const float deadband = 50.0;
float pv = 3700.0;

void setup(){
  Serial.begin(115200);
  Serial.println("seconds,light intensity");
  pinMode(RELAY_PIN, OUTPUT);
}

void loop(){
  int raw = analogRead(LDR_PIN);
  pv = raw * 0.1 + pv * 0.9;

  if(pv < 5){
    digitalWrite(RELAY_PIN, LOW);
    return;
  }
  if(pv > 4090){
    digitalWrite(RELAY_PIN, LOW);
    return;
  }

  if(pv < (sp-deadband)){
    digitalWrite(RELAY_PIN, HIGH);
  }

  else if(pv > (sp+deadband)){
    digitalWrite(RELAY_PIN, LOW);
  }

  float seconds = millis() / 1000.0;
  Serial.print(seconds);
  Serial.print(",");
  Serial.println(pv);
  delay(200);
}