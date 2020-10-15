#include "SX1278.h" //LoRa knjižnica
#include <SPI.h>
#include <ArduinoJson.h> //Json format knjižnica
#include <ESP8266WiFi.h>
#include <xxtea-iot-crypt.h>
#include "DHT.h"        // knjižnica za temperaturni senzor
#define DHTTYPE DHT11   // DHT 11
#define dht_dpin 2

//Parametri LoRa
#define LORA_MODE  4
#define LORA_CHANNEL  CH_6_BW_125
#define LORA_ADDRESS  2
#define LORA_SEND_TO_ADDRESS  4
#define LED     D0

DHT dht(dht_dpin, DHTTYPE); 
float dim = 0 ;
float hum = 0 ;
float temp = 0 ;

char del1[30];
char del2[30];
char skupaj[60];

const int analog_pin = A0; 
uint8_t MQ2_Pin = D3;
int e;

char charVal[10]; 

String key1 = "1sdu7z!9x123"; //kriptirni ključ 1
String key2 = "11_ksL5PA13x"; //kriptirni ključ 2

void izmeri() {
  
    temp = dht.readTemperature();
    delay(500);
    
    Serial.print("Temperatura = ");
    Serial.print(temp); 
    Serial.print(" C  ");
    delay(500);
    dim = analogRead (analog_pin); 
    delay(500);
    Serial.print("Dim = "); 
    Serial.print(dim); 
    Serial.print(" ppm");
    
    if (isnan(temp)) {temp=0; }
    
}

void poslji_lora(){
   
}

void setup(){
  WiFi.mode( WIFI_OFF ); //izklop WiFi
  WiFi.forceSleepBegin(); 

  pinMode(MQ2_Pin, OUTPUT);//vklop MQ-2
  digitalWrite(MQ2_Pin,LOW);
  
  dht.begin();
  
  //odprtje serijske komunikacije
  Serial.begin(115200);
  delay(2000);
  
  izmeri();

 
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
    Serial.println("Setting Mode: ERROR ");
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
    Serial.println(F("Power: OK "));
  } else {
    Serial.println(F("Power: ERROR "));
  }

  // nastavimo naslov
  if (sx1278.setNodeAddress(LORA_ADDRESS) == 0) {
    Serial.println(F("Naslov: OK "));
  } else {
    Serial.println(F("Naslov: ERROR "));
  }

  Serial.println(F("sx1278 konfiguracija končana"));
  Serial.println();

  sprintf(del1, "%.2f", temp);
  Serial.printf(del1);
  Serial.printf("\n");
  sprintf(del2, "%.2f", dim);
  Serial.printf(del2);
  Serial.printf("\n");
  sprintf(skupaj, "%s,%s", del1, del2); 
  Serial.printf(skupaj);

//enkripcija-------------------------------------------------------------------------
  
  xxtea.setKey(key1);
  Serial.print(F(" Encrypted Data: "));
  String result1 = xxtea.encrypt(skupaj);
  result1.toLowerCase(); 
  Serial.println(result1);

  xxtea.setKey(key2);
  String result2 = xxtea.encrypt(result1);
  result2.toLowerCase(); 
  Serial.println(result2);

  char * skupaj2 = new char [result2.length()+1];
  strcpy (skupaj2, result2.c_str());
  
  Serial.println("Po pretvorbi v char:/n");
  Serial.println(skupaj2);
//enkripcija-------------------------------------------------------------------------

  // pošiljanje paketa
  e = sx1278.sendPacketTimeout(LORA_SEND_TO_ADDRESS, skupaj2);
  Serial.print("Paket poslan");
  Serial.println(e, DEC);

  if (e == 0) {
      
      delay(500);
      
  }

  delay(4000);  

  delay(4000); 

  digitalWrite(MQ2_Pin,HIGH); //izklop MQ-2
  
  //začetek globokega spanca
  Serial.println("Globok spanec 30s ...");
  ESP.deepSleep(30e6); 
  delay(500);
  
}

void loop(void){}
