#include <Homie.h>
#include <Wire.h>
#include <BMP280.h>

extern "C" {
  #include "user_interface.h"
}

// ADC Adjustment -  Measure actual VCC voltage with an accurate voltage meter. 
// [measured vcc voltage] / (ESP.getVcc() / 1000) 
#define ADC_ADJ 1.02774274905422;  // perahps an adjustment ratio
ADC_MODE(ADC_VCC);

BMP280 bmp;

const int sleepTimeS = 3600;
bool sentOnce = false;

HomieNode temperatureNode("temperature", "temperature");
HomieNode pressureNode("pressure", "pressure");
HomieNode voltageNode("battery", "battery");

float readVcc() {
  int vccMv = ESP.getVcc();
  float vcc = vccMv / 1000.0;
  return vcc * ADC_ADJ;
}

void loopHandler() {
   if (Homie.isConnected()) {
    if (!sentOnce) {
      double temperature, pressure;
      char result = bmp.startMeasurment();
      if (result != 0) {
        delay(result);
        result = bmp.getTemperatureAndPressure(temperature, pressure);
        if (result != 0) {
          float tempF = temperature * 1.8 + 32;
          float mbarToInches = pressure * 0.0295301;
          float vcc = readVcc();
          temperatureNode.setProperty("Fahrenheit").send(String(tempF));
          temperatureNode.setProperty("Celsius").send(String(temperature));
          pressureNode.setProperty("inches").send(String(mbarToInches));
          pressureNode.setProperty("mBar").send(String(pressure));
          voltageNode.setProperty("volts").send(String(vcc));
          sentOnce = true;
        }
      }
    }
  }
}

void onHomieEvent(HomieEvent event) {
  switch (event.type) {
    case HomieEventType::MQTT_CONNECTED:
      sentOnce = false;
      Homie.prepareToSleep();
      break;
    case HomieEventType::READY_TO_SLEEP:
      ESP.deepSleep(sleepTimeS * 1000000);
      break;
  }
}

void setup() {
  Serial.begin(74880);
  // Serial.setDebugOutput(true);
  bmp.begin();
  Homie_setFirmware("bmp280", "1.0.0");
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);
  Homie.disableLedFeedback();
  bmp.setOversampling(4);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
