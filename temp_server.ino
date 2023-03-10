#include <Ticker.h> 
#include <OneWire.h>
#include <DallasTemperature.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#include "temperature_html.h"

#define DS18B20_PIN     D1

const char *ssid = "";
const char *pass = "";

Ticker blinker;
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(80);
String temperature;
unsigned long lastTempRead = 0;

void changeLedState()
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  
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
  Serial.begin(115200);
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
  blinker.attach_ms(1000, changeLedState);
}


void loop()
{
  if (millis() - lastTempRead >= 1000)
  {
    temperature = getTemperature();
    lastTempRead = millis();
  }
}
