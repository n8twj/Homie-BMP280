
#include <Homie.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;

const int sleepTimeS = 300;
bool sentOnce = false;

HomieNode temperatureNode("temperature", "temperature");
HomieNode pressureNode("pressure", "pressure");

void setupHandler() {
  Homie.setNodeProperty(temperatureNode, "unit").setRetained(true).send("F");
  Homie.setNodeProperty(pressureNode, "unit").setRetained(true).send("inches");
}

void loopHandler() {
  if (!sentOnce) {
    sentOnce = true;
    float temperature = bmp.readTemperature();
    float tempF = temperature * 1.8 + 32;
    float pressure = bmp.readPressure();
    float mbarToInches = pressure / 100 * 0.0295301;

    Serial.print("temperature F: ");
    Serial.println(tempF);
    Serial.print("temperature C: ");
    Serial.println(temperature);
    Serial.print("pressure: ");
    Serial.println(mbarToInches);
    if (Homie.isConnected()) {
      Homie.setNodeProperty(temperatureNode, "F").send(String(tempF));
      Homie.setNodeProperty(pressureNode, "inches").send(String(mbarToInches));
    }
  }
}

void onHomieEvent(HomieEvent event) {
  switch(event) {
    case HomieEvent::MQTT_CONNECTED:
      sentOnce = false;
      Homie.prepareForSleep();
      break;
    case HomieEvent::READY_FOR_SLEEP:
      ESP.deepSleep(sleepTimeS * 1000000, RF_NO_CAL);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  bmp.begin(0x76);  // XXX pull i2c address from config?
  Homie_setFirmware("bmp280", "1.0.0");
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);
  Homie.disableLogging();
  Homie.disableLedFeedback();

  temperatureNode.advertise("unit");
  temperatureNode.advertise("temperature");
  pressureNode.advertise("unit");
  pressureNode.advertise("pressure");

  Homie.setup();
}

void loop() {
  Homie.loop();
}
