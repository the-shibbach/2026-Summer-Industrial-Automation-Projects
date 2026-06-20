#include <WiFi.h>
#include <PubSubClient.h>

// (my hotspot)
const char* WIFI_SSID = "bryanchan";
const char* WIFI_PASS = "bryanchan";

const char* BROKER = "broker.hivemq.com";        // public test broker
const char* TOPIC  = "bryanchan/esp32/data";     

WiFiClient net;
PubSubClient mqtt(net);

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println(" wifi ok");
}

void connectBroker() {
  mqtt.setServer(BROKER, 1883);

  while (!mqtt.connected()) {
    String id = "esp32-" + String(random(10000));  // unique client ID

    if (mqtt.connect(id.c_str())) {
      Serial.println("broker ok");
    } else {
      delay(1000);
      Serial.print("retry ");
    }
  }
}

void setup() {
  Serial.begin(115200);

  connectWifi();
  connectBroker();
}

void loop() {
  if (!mqtt.connected()) {
    connectBroker();
  }

  mqtt.loop();

  // For example
  int pv  = analogRead(34);  
  int sp  = 3000;
  int out = 119;

  char payload[80];
  sprintf(payload, "{\"pv\":%d,\"sp\":%d,\"out\":%d}", pv, sp, out);

  mqtt.publish(TOPIC, payload);

  Serial.println(payload);

  delay(1000);
}