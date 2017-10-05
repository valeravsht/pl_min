/*
  FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://esp8266fs.local
  edit the page by going to http://esp8266fs.local/edit
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Wire.h>
#include <ArduinoJson.h>

#include <PubSubClient.h> //MQTT

#include <Ticker.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 //18b20
//// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
//// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
//// arrays to hold device addresses
DeviceAddress thermometer1;

float term;

Ticker flipper;     //таймер взводит переменную flip ЧЕРЕЗ ЗАДАННЫЙ ИНТЕРВАЛ
bool flip =  false;

#define DBG_OUTPUT_PORT Serial

const char* ssid_ = "shintorg1";
const char* password_ = "qwerty123";
const char* host = "esp8266fs";

ESP8266WebServer server(80);
//holds the current upload
File fsUploadFile;


//******* MQTT  ******************

static String  MQTT_TOPIC = "vlrdev/";

static String MqttClName = ""; // имя клинтна MQTT
static String MqttServer = ""; //сервер брокера MQTT
static String MqttPort = ""; // порт брокера MQTT
static String MqttLogin = "";
static String MqttPaswd = "";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
//***********************************


// WiFi
String ssid = ""; //имя точки доступа для подключения (режим клиент)
String password = ""; // пароль для подключения (режим клиент)
boolean ipDCHP = true; // получение IP адреса по DCHP

boolean wifi_AP = true; // режимы wifi  false-клиент, true-точка доступа
String ssidAP = ""; // имя для точки доступа (режим точка доступа)
String passwordAP = ""; // пароль для точки доступа (режим точка доступа)
byte staticIP1 ; // статический IP
byte staticIP2 ; // статический IP
byte staticIP3 ; // статический IP
byte staticIP4 ; // статический IP
byte staticMask1 ; // статический Mask
byte staticMask2; // статический Mask
byte staticMask3 ; // статический Mask
byte staticMask4; // статический Mask
byte staticGw1 ; // статический GW
byte staticGw2; // статический GW
byte staticGw3 ; // статический GW
byte staticGw4 ; // статический GW

byte staticAPIP1 ; // статический IP Точки доступа
byte staticAPIP2; // статический IP Точки доступа
byte staticAPIP3; // статический IP Точки доступа
byte staticAPIP4 ; // статический IP Точки доступа
byte staticAPMask1 ; // статический Mask Точки доступа
byte staticAPMask2 ; // статический Mask Точки доступа
byte staticAPMask3 ; // статический Mask Точки доступа
byte staticAPMask4 ; // статический Mask Точки доступа
byte staticAPGw1 ; // статический GW Точки доступа
byte staticAPGw2; // статический GW Точки доступа
byte staticAPGw3 ; // статический GW Точки доступа
byte staticAPGw4 ; // статический GW Точки доступа

const byte dns1_1 = 8 ; //
const byte dns1_2  = 8; //
const byte dns1_3  = 8 ; //
const byte dns1_4  = 8; //

const byte dns2_1 = 8 ; //
const byte dns2_2  = 8; //
const byte dns2_3  = 8 ; //
const byte dns2_4  = 8; //
/*
bool io5_r = false;
bool io4_r = false;
bool io0_r = false;
bool io2_r = false;
bool io15_r = false;
bool io10_r = false;
bool io13_r = false;
bool io12_r = false;
bool io14_r = false;
bool io16_r = false;
bool adc_r = false;

bool io5_w = false;
bool io4_w = false;
bool io0_w = false;
bool io2_w = false;
bool io15_w = false;
bool io10_w = false;
bool io13_w = false;
bool io12_w = false;
bool io14_w = false;
bool io16_w = false;
bool adc_w = false;

String io5_mqtt =  "";
String io4_mqtt =  "";
String io0_mqtt =  "";
String io2_mqtt =  "";
String io15_mqtt = "";
String io10_mqtt = "";
String io13_mqtt = "";
String io12_mqtt = "";
String io14_mqtt = "";
String io16_mqtt = "";
String adc_mqtt =  "";
*/

struct Set_MQTT{
  String io;
  bool r;
  bool w;
  String mqtt;
}set_mqtt[]={
  {"io5",false,false,""},
  {"io4",false,false,""},
  {"io0",false,false,""},
  {"io2",false,false,""},
  {"io15",false,false,""},
  {"io9",false,false,""},
  {"io10",false,false,""},
  {"io13",false,false,""},
  {"io12",false,false,""},
  {"io14",false,false,""},
  {"io16",false,false,""},
  {"adc",false,false,""}
};

void loop(void) {
  server.handleClient();

  // ********* MQTT **************
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //***************************




  if (flip) { // ЗАДАННЫЙ ИНТЕРВАЛ пришел
    flip = false;
    Serial.println("FLIPPP!!!!!");
    String str = String(term);
    String outTopicMain = MQTT_TOPIC + MqttClName ;
    String outTopic = outTopicMain + "/thermometer";
    client.publish(outTopic.c_str(), str.c_str());
    outTopic = outTopicMain + "/illumination";
    int illum = map(analogRead(A0),0,1024,100,0);
    client.publish(outTopic.c_str(),  String(illum).c_str());
  }


  // термо датчик
  if (sensors.isConversionComplete()) {
    term = sensors.getTempC(thermometer1);
    sensors.requestTemperatures();
  }

}
