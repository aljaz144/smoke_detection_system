#include "SX1278.h" //LoRa knjižnica
#include <SPI.h>
#include <ArduinoJson.h> //Json format knjižnica
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <xxtea-iot-crypt.h>

//Parametri LoRa
#define LORA_MODE  4
#define LORA_CHANNEL  CH_6_BW_125
#define LORA_ADDRESS  4

int e;
char my_packet[100];
float dim = 0 ;
float hum = 0 ;
float temp = 0 ;
float dim_prejeto = 0 ;
float hum_prejeto = 0 ;
float temp_prejeto = 0 ;
char del1[30];
char del2[30];
char skupaj[60];
int led = 0; //D3
int buzzer = 2; //D4

#define FIREBASE_HOST "nodemcu-8b28d.firebaseio.com" //firebase host
#define FIREBASE_AUTH "qvA1SoHl9NRZIiuBVT06Sgv7jQEKDMmWkE1apcfd" //avtorizacijski ključ
//#define WIFI_SSID "wlnet2"
//#define WIFI_PASSWORD "V1nko2008"
#define WIFI_SSID "huaweitest"
#define WIFI_PASSWORD "feritest"

String key1 = "1sdu7z!9x123"; //kriptirni ključ 1
String key2 = "11_ksL5PA13x"; //kriptirni ključ 2

void setup()
{
  
  //odprtje serijske komunikacije
  Serial.begin(115200);

  pinMode(led, OUTPUT); 
  pinMode(buzzer, OUTPUT);
  
  Serial.println(F("sx1278 konfiguracija"));

  // prižgi modul
  if (sx1278.ON() == 0) {
    Serial.println("Power ON: OK ");
  } else {
    Serial.println("Power ON: ERROR ");
  }

  // nastavi prenosni način
  if (sx1278.setMode(LORA_MODE) == 0) {
    Serial.println("Setting Mode: OK ");
  } else {
    Serial.println("Setting Mode: ERROR");
  }

  // nastavi glavo
  if (sx1278.setHeaderON() == 0) {
    Serial.println("Header ON: OK ");
  } else {
    Serial.println("Header ON: ERROR ");
  }

  // nastavi frekvenčni kanal
  if (sx1278.setChannel(LORA_CHANNEL) == 0) {
    Serial.println("Kanal: OK ");
  } else {
    Serial.println("Kanal: ERROR ");
  }

  // nastavi CRC
  if (sx1278.setCRC_ON() == 0) {
    Serial.println("CRC ON: OK ");
  } else {
    Serial.println("CRC ON: ERROR ");
  }

  // moč (Max, High, Intermediate,Low)
  if (sx1278.setPower('M') == 0) {
    Serial.println("Power: OK ");
  } else {
    Serial.println("Power: ERROR ");
  }

  // nastavimo naslov
  if (sx1278.setNodeAddress(LORA_ADDRESS) == 0) {
    Serial.println("Naslov: OK ");
  } else {
    Serial.println("Naslov: ERROR ");
  }

  // Print a success message
  Serial.println("sx1278 konfiguracija končana");
  Serial.println();
}

void blink_led(){
  //dvakratni utrip LED diode
   for(int i = 0; i <2;i++){
    digitalWrite(led, HIGH);   
    delay(500);              
    digitalWrite(led, LOW);   
    delay(500);
   }
  
}

void alarm1 (){
    //petkratni utrip LED diode
  for(int i = 0 ; i<5;i++){
    digitalWrite(led, HIGH);   
    delay(100);              
    digitalWrite(led, LOW);   
    delay(100);
  }
  
}

void alarm2 (){
    //petkratni pisk piskača
  for(int i = 0 ; i<5;i++){
    digitalWrite(buzzer, HIGH);   
    delay(100);              
    digitalWrite(buzzer, LOW);   
    delay(100);
  }
  
}

void povezi_wifi(){

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //povezava na WiFi
  Serial.print("connecting");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Serial.println("Firebase aventikacija..."); // Firebase avtentikacija
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void poslji(){

  Firebase.setFloat("temp",temp);
  // napaka pri Firebase avtenetikaciji
  if (Firebase.failed()) {
      Serial.print("failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);

  // dodaj novo vrednost v /logs

  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = temp;
  root["smoke"] = dim;
  String name = Firebase.push("logs", root);
  // napak
  if (Firebase.failed()) {
  Serial.print("pošiljanje /logs napaka:");
  Serial.println(Firebase.error());  
  return;
  }
  Serial.print("pošiljanje: /logs/");
  Serial.println(name);
  delay(1000);
}



void loop()
{
  // poslušaj za prejetje sporočila za čas 10s
  e = sx1278.receivePacketTimeout(10000);
  if (e == 0) {
    
    delay(1000);
    Serial.println(F("Paket prejet!"));
    
    blink_led();
    
    for (unsigned int i = 0; i < sx1278.packet_received.length; i++) {
      my_packet[i] = (char)sx1278.packet_received.data[i];
    }
    
    Serial.print("Sporočilo kriptirano: ");
    Serial.println(my_packet);

    // dekripcija---------------------------------------------------------------------------------------------------------

    xxtea.setKey(key2);
    Serial.println(F("1. faza dekripcije (key2)"));
    String result3 = xxtea.decrypt(my_packet);
    Serial.println(result3);

    xxtea.setKey(key1);

    Serial.println(F("2. faza dekripcije (key1)"));
    String result4 = xxtea.decrypt(result3);
    Serial.println(result4);

  
    char * my_packet2 = new char [result4.length()+1];
    strcpy (my_packet2, result4.c_str());
  
    Serial.println("Po pretvorbi v char:\n");
    Serial.println(my_packet2);

    // dekripcija---------------------------------------------------------------------------------------------------------

    if (sscanf(my_packet2, "%f,%f", &temp_prejeto, &dim_prejeto) == 2) {
      //recieved();
      Serial.println("");
      Serial.println("Stevilke:\n");
      temp = temp_prejeto;
      dim = dim_prejeto;
      Serial.println(temp);
      Serial.println(dim);
    }

    if(dim > 1000 || temp > 30){ // v primeru ''ekstremnih'' vrednosti proži alarm
        alarm1();
        alarm2();
    }

    povezi_wifi(); //povezemo se na wifi
    poslji(); // posljemo na Firebase
    delay (500);
    WiFi.disconnect(); //prekinemo povezavo z WiFi
    
  } else {
    Serial.print("Ni paketa...\n");
  }
  delay(1000);
}
