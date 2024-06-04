#include <WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <map>
#include <limits.h>
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;
const int ledPin = 4;
const char *ssid = "****"; // Wifi name
const char *password = "****"; // Wifi passowrd
const char *host = "192.168.0.27"; // Server's ip
const int port = 8085; // Server's port
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -14400, 60000);
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 60000;
String json;
std::map<String, bool> bellStatus;
String previousDay = "";
struct ClocheSlot
{
  String heure_str;
  String heure_str_fin;
  int heure_int;
  int minute_int;
  int creneau;
  int duree;
  int fin;
  int heure_fin;
  int minute_fin;
};

void setup()
{
  Serial.begin(115200);
  matrix.begin();
  byte frame[8][12] = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0},
      {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
      {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
      {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
      {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
      {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  matrix.renderBitmap(frame, 8, 12);

  Serial.print("Connexion au réseau WiFi : ");
  Serial.println(ssid);

  pinMode(ledPin, OUTPUT);
  pinMode(7,INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connecté");
  Serial.print("Adresse IP du Arduino : ");
  Serial.println(WiFi.localIP());

  delay(1000);
  fetchFile();
  timeClient.begin();
  delay(1000);
}

void loop()
{
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  String currentDay = getTheDay();
  Serial.println(currentDay);

  if (currentDay != previousDay) {
    bellStatus.clear();
    Serial.println("ici");
    previousDay = currentDay;
  }


  Serial.print("Heure actuelle : ");
  Serial.print(currentHour);
  Serial.print(":");
  if (currentMinute < 10) {
    Serial.print("0"); 
  }
  Serial.print(currentMinute);
  Serial.print(":");
  if (currentSecond < 10) {
    Serial.print("0");
  }
  Serial.println(currentSecond);
  int etatInterrupteur = digitalRead(7);
  if(etatInterrupteur==HIGH){
    forEachSlot(true);
  }
  else{
    forEachSlot(false);
  }

  if (millis() - lastFetchTime >= fetchInterval)
  {
    fetchFile();
    lastFetchTime = millis();
  }

  delay(1000);
}

void fetchFile()
{
  WiFiClient http_client;
  if (!http_client.connect(host, port))
  {
    return;
  }

  http_client.print(String("GET /api/cloche HTTP/1.1\r\n") +
                    "Host: " + host + ":" + String(port) + "\r\n" +
                    "Connection: close\r\n\r\n");

  delay(1000);

  while (http_client.available())
  {
    String line = http_client.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
  }
  while (http_client.available())
  {
    json = http_client.readStringUntil('\n');
  }
  Serial.println("Je fetch" + json);
  http_client.stop();
}

DynamicJsonDocument jsonParser()
{
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);

  if (error)
  {
    Serial.print("Erreur lors de la conversion JSON : ");
    Serial.println(error.c_str());
    return DynamicJsonDocument(0);
  }
  return doc;
}

void forEachSlot(bool isButtonPressed)
{
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  DynamicJsonDocument doc = jsonParser();
  String day = getTheDay();
  if(day=="NW"){
    return;
  }
  Serial.println(day);
  DynamicJsonDocument docDay(1024); 
  DeserializationError error = deserializeJson(docDay, doc[day].as<String>());
  for (size_t i = 0; i < docDay.size(); i++)
  {
    ClocheSlot slot = createASlot(docDay, i);
    if (checkAutomaticActivation(slot))
    {
      activatingBell(slot.duree);
    }
    else
    {
      if(isButtonPressed && i==getClosestTime(currentHour,currentMinute,docDay)){
        Serial.println(i);
        if(slot.creneau==0){
          activatingBell(5);
          return;
        }
        int lowerBoundary = (slot.heure_int * 60 + slot.minute_int)-slot.creneau;
        int upperBoundary = (slot.heure_int * 60 + slot.minute_int)+slot.creneau;
        int currentTime = currentHour*60 + currentMinute;
        Serial.println(bellStatus[String(lowerBoundary)]);
        Serial.println(lowerBoundary);
        Serial.println(currentTime);
        Serial.println(upperBoundary);
        if(currentTime<=upperBoundary && currentTime>=lowerBoundary && !bellStatus[String(lowerBoundary)])
        {
          activatingBell(slot.duree);
          delay(slot.fin*60000);
          activatingBell(slot.duree);
          bellStatus[String(lowerBoundary)]=true;
        }
        else{
            activatingBell(5);
        }
      }
    }
  }
}

int getClosestTime(int currentHour, int currentMinute, DynamicJsonDocument doc) {
  int currentTimeInMinutes = currentHour * 60 + currentMinute;
  int closestIndex = -1; 
  int minDifference = INT_MAX; 
  for (size_t i = 0; i < doc.size(); i++) {
    ClocheSlot slot = createASlot(doc, i);
    int slotStartTimeInMinutes = (slot.heure_int * 60 + slot.minute_int) - slot.creneau;
    int slotEndTimeInMinutes = (slot.heure_int * 60 + slot.minute_int) + slot.creneau;

    if (slotEndTimeInMinutes < currentTimeInMinutes) {
      continue;
    }

    int differenceStart = abs(slotStartTimeInMinutes - currentTimeInMinutes);
    int differenceEnd = abs(slotEndTimeInMinutes - currentTimeInMinutes);

    if (differenceEnd < minDifference || differenceStart < minDifference ) {
      minDifference= min(differenceEnd,differenceStart);
      closestIndex = i;
    }
  }
  Serial.println(closestIndex);
  return closestIndex;
}




ClocheSlot createASlot(DynamicJsonDocument doc, int i)
{
  ClocheSlot slot;
  slot.heure_str = doc[i]["heure"].as<String>();
  slot.heure_int = atoi(strtok(const_cast<char *>(slot.heure_str.c_str()), ":"));
  slot.minute_int = atoi(strtok(NULL, ":"));
  slot.creneau = atoi(doc[i]["creneau"]);
  slot.duree = atoi(doc[i]["duree"]);
  slot.fin = atoi(doc[i]["fin"]);
  slot.heure_fin = slot.heure_int;
  slot.minute_fin = slot.minute_int;
  if (slot.fin + slot.minute_fin >= 60)
  {
    slot.heure_fin += 1;
    slot.minute_fin = (slot.minute_int + slot.fin) - 60;
  }
  else
  {
    slot.minute_fin = slot.minute_fin + slot.fin;
  }
  slot.heure_str_fin = String(slot.heure_fin) + ":" + String(slot.minute_fin);
  return slot;
}

bool checkAutomaticActivation(ClocheSlot slot)
{
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  if (currentHour == slot.heure_int && currentMinute == slot.minute_int && slot.creneau == 0)
  {
    if (!bellStatus[slot.heure_str])
    {
      bellStatus[slot.heure_str] = true;
      return true;
    }
  }
  if (currentHour == slot.heure_fin && currentMinute == slot.minute_fin && slot.creneau == 0 && slot.heure_fin!=slot.heure_int)
  {
    if (!bellStatus[slot.heure_str_fin])
    {
      bellStatus[slot.heure_str_fin] = true;
      return true;
    }
  }
  return false;
}

String getTheDay(){
  int weekday = timeClient.getDay();
  Serial.println(weekday);
 switch (weekday) {
    case 0:
      return "NW";
    case 1:
      return "lundi";
    case 2:
      return "mardi";
    case 3:
      return "mercredi";
    case 4:
      return "jeudi";
    case 5:
      return "vendredi";
    case 6:
      return "NW";
  } 
}

void activatingBell(int duree)
{
  Serial.println("Activation de la cloche !");
  digitalWrite(ledPin, HIGH);
  delay(duree * 1000);
  digitalWrite(ledPin, LOW);
}
