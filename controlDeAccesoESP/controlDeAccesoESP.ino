#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Ticker.h>

#include <SoftwareSerial.h>

//SoftwareSerial mySerial(10, 11); // RX, TX
SoftwareSerial swSer(D2, D0);



ESP8266WebServer server(80);

void setup() {
  // put your setup code here, to run once:
  swSer.begin(9600);
   Serial.begin(9600); delay(200);
  
  
  WiFi.mode(WIFI_STA);
  WiFi.begin("DisiLabHotSpot", "AccessAir");
  Serial.println("");
  Serial.println("Conectando");

  while (WiFi.status() != WL_CONNECTED) {
    //digitalWrite(LED, !digitalRead(LED));
    // delay(200);
    //  digitalWrite(LED, !digitalRead(LED));
    delay(200);
    Serial.print(".");
    
  } 
  
    Serial.println("");
    Serial.println("Conectado!");
}

void loop() {

     while (swSer.available() > 0) {
Serial.println(swSer.readStringUntil('\n'));
swSer.println("AUH:1");
//delay(1000);
Serial.println("envia");
  }
  
  // put your main code here, to run repeatedly:

}




/* CODIGO PARA EL ARDUINO COM SERIAL
 * 
 * 
 * 
 
  
  
  
  
  unsigned long lastRead = 0;
const int debounce = 500;
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
}
 
void loop() {
  if (Serial1.available() > 0){
    if(millis() - lastRead > debounce) {
      String codigo = Serial1.readStringUntil('\n');
      lastRead = millis();
      Serial.println(codigo);
      Serial2.println("LEE:"+codigo);
    } else {
      while(Serial1.available()){
        Serial1.read();
        lastRead = millis();
      }
    }
  }
 
 
  //ver de hacerlo mas lindo con  substring y startwith ( con este no me hace falta trimear para comparar
if (Serial2.available() > 0){
   String lectura = Serial2.readStringUntil('\n');
   lectura[sizeof(lectura)-1]=0;
 
  if(lectura=="AUH:1"){
   Serial.println("acceso concedido");
  }
      if(lectura=="AUH:0"){
   Serial.println("acceso negado");
  }
     
   
 
  }
}
