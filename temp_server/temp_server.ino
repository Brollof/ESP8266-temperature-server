#include <Ticker.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebSrv.h>

#include "temperature_html.h"

#define DS18B20_PIN             D1
#define SERIAL_BAUD_RATE        115200
#define WEB_SERVER_PORT         80
#define LED_STEP                1
#define LED_MAX                 260
#define LED_MIN                 245
#define TEMP_READ_PERIOD_MS     1000
#define LED_UPDATE_PERIOD_MS    100

const char *ssid = "";
const char *pass = "";

Ticker blinker;
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(WEB_SERVER_PORT);
String temperature;
uint32_t lastTempRead = 0;

void updateLed()
{
  static bool dir = true;
  static int16_t value = 0;

  value += dir ? LED_STEP : -LED_STEP;

  if (value > LED_MAX)
  {
    value = LED_MAX;
    dir = !dir;
  }
  else if (value < LED_MIN)
  {
    value = LED_MIN;
    dir = !dir;
  }

  analogWrite(LED_BUILTIN, value);
}

String getTemperature()
{
  String ret_temp = "--";

  sensors.requestTemperatures();
  if (sensors.isConversionComplete() == true)
  {
    ret_temp = sensors.getTempCByIndex(0);
  }

  return ret_temp;
}

String processor(const String& var){
  String ret = "";
  if (var == "TEMPERATURE")
  {
    ret = temperature;
  }
  return ret;
}

void setup()
{
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println();
  Serial.println();

  sensors.begin();
  WiFi.begin(ssid, pass);
  
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/c", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperature.c_str());
  });

  server.begin();
  blinker.attach_ms(LED_UPDATE_PERIOD_MS, updateLed);
}


void loop()
{
  if (millis() - lastTempRead >= TEMP_READ_PERIOD_MS)
  {
    temperature = getTemperature();
    lastTempRead = millis();
  }
}
