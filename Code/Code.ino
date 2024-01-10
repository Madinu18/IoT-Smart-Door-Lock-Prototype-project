#include <WiFi.h>
#include <Wire.h> 
#include <SPI.h> 
#include <RFID.h>
#include "FirebaseESP32.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>

#define VibSen 25

#define FIREBASE_HOST "https://smart-lock-door-a43d2-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "hYumJZXY0A3Qk7hP8eMQc9vbPz3Dj9sgwGKvBwnn"


RFID rfid(5, 27);
unsigned char str[MAX_LEN];

const int button = 32;

Servo myServo;

const int buzzer = 26;



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char ssid[] = "1";
const char pass[] = "adi123220";

String uidPath= "/";
FirebaseJson json;
FirebaseData firebaseData;
FirebaseJsonArray arr;

unsigned long lastMillis = 0;
String alertMsg;
String device_id;

int nilai;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\n connected!");
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  
  SPI.begin();
  rfid.init();

  pinMode(VibSen,INPUT);
  
  pinMode(button, INPUT);
  
  myServo.attach(13);
  
  pinMode(buzzer, OUTPUT);

  timeClient.begin();
  timeClient.setTimeOffset(25200);
  connect();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void checkAccess (String temp)
{
    json.clear();
    if(Firebase.getInt(firebaseData, uidPath+"/Users/"+temp)){
      bipBenar();
      alertMsg="Pintu Terbuka";
      Serial.println(alertMsg);
      bukaPintu();
      
      unsigned long epochTime = timeClient.getEpochTime();
      struct tm *ptm = gmtime ((time_t *)&epochTime);
      int currentDay = ptm->tm_mday;
      int currentMonth = ptm->tm_mon+1;
      int currentYear = ptm->tm_year+1900;
      String currentDateandTime = String(currentDay) + "-" + String(currentMonth) + "-" + String(currentYear) + " " + String(timeClient.getFormattedTime());
      
      json.add("UID", temp);
      json.add("Date_and_Time", currentDateandTime);
      
      if (Firebase.pushJSON(firebaseData, "/Log", json)) {
          Serial.println(firebaseData.dataPath() + firebaseData.pushName()); 
        } 
      else {
          Serial.println(firebaseData.errorReason());
        }
    }
    else
    {
      bipSalah();
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
    }
    json.clear();
}

void pushButton(int nilai)
{
  if (nilai == HIGH){
    bukaPintu();
    Serial.print("Button");
  }
}

void bukaPintu()
{
  myServo.write(20);
  delay(5000);
  myServo.write(120);
}

void bipBenar()
{
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
}

void bipSalah()
{
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
}

void bipAlarm()
{
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
}

void loop() {
  timeClient.update();
  int vib = digitalRead(VibSen);
  if(vib == HIGH){
    bipAlarm();
    Serial.println("sensor gertar nyala");
  }

  int nilai = digitalRead(button);
  if (nilai == HIGH){
    bukaPintu();
    Serial.print("Button");
  }

  if (rfid.findCard(PICC_REQIDL, str) == MI_OK)
  { 
    Serial.println("Card found"); 
    String temp = "";
    if (rfid.anticoll(str) == MI_OK)
    { 
      Serial.print("The card's ID number is : "); 
      for (int i = 0; i < 4; i++)
      { 
        temp = temp + (0x0F & (str[i] >> 4)); 
        temp = temp + (0x0F & str[i]); 
      } 
      Serial.println(temp);
      json.set("UID", temp);
      Firebase.updateNode(firebaseData, "/RFID_Reader",json);
      checkAccess (temp);
      json.clear();
    } 
    rfid.selectTag(str);
  }
  rfid.halt();
}
