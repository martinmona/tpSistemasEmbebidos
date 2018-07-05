#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Ticker.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <SPIFFSReadServer.h>
#include <NTPClient.h>


#include <Ticker.h>

Ticker evento;

#include <SoftwareSerial.h>

String ultimo = "";

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 24 * 60 * 60 * 1000    // In miliseconds, once a day
#define NTP_ADDRESS  "0.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
unsigned long epochTime;
String date;
String t;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ; const bool show24hr = true;
// Then convert the UTC UNIX timestamp to local time
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 5 hours - change this as needed
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -240};   //UTC - 6 hours - change this as needed
Timezone usEastern(usEDT, usEST);
static time_t local;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);


//SoftwareSerial mySerial(10, 11); // RX, TX
SoftwareSerial swSer(D2, D0);


File f;


ESP8266WebServer server(80);




String imprimirHora()
{ date = ""; t = "";
  // convert received time stamp to time_t object
  local = usEastern.toLocal(timeClient.getEpochTime());

  // now format the Time variables into strings with proper names for month, day etc
  date += days[weekday(local) - 1];
  date += ", ";
  date += months[month(local) - 1];
  date += " ";
  date += day(local);
  date += ", ";
  date += year(local);

  // format the time to 12-hour format with AM/PM and no seconds
  if (show24hr) {
    t += hour(local);
  } else {
    t += hourFormat12(local);
  }
  // blinking colon
  if (second(local) & 1) {
    t += ":";
  } else {
    t += ":";
  }
  if (minute(local) < 10) // add a zero if minute is under 10
    t += "0";
  t += String(minute(local)) + ":" + String(second(local));

  /*if (!show24hr) {
    t += " " +String(second(local));
    t += ampm[isPM(local)];
    }*/

  return date + " " +  t;
}



String getValue(String data, char separator, int index)
{
  int maxIndex = data.length() - 1;
  int j = 0;
  String chunkVal = "";

  for (int i = 0; i <= maxIndex && j <= index; i++)
  {
    chunkVal.concat(data[i]);

    if (data[i] == separator)
    {
      j++;

      if (j > index)
      {
        chunkVal.trim();
        return chunkVal;
      }

      chunkVal = "";
    }
    else if ((i == maxIndex) && (j < index)) {
      chunkVal = "";
      return chunkVal;
    }
  }
}
const char INDEX_HTML_HEAD[] =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<title>Control de Acceso</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>";

String contenido;
String nombreNuevoUsuario;
String codigoNuevoUsuario;

void agregarUsuario()
{
  nombreNuevoUsuario = server.arg(0); // ?nombre=nicolas&codigo=123
  codigoNuevoUsuario = server.arg(1);
  swSer.println("NEW:asd");
  server.send(200,"text/html","<meta http-equiv='refresh' content='5; url=/'> Insertando usuario nuevo...");
  
  
}

void paginaHistorial() {

  String lectura = "";
  
  contenido = "<body>";
  f = SPIFFS.open("historial.txt" , "r");
  
  if (!f) {
    Serial.println("no existe el archivo que queres abrir");
    f.close();
  } 
  else {
    contenido +="<table>"
    "<tr>"
    "<th>Hora</th>"
    "<th>Tarjeta</th> "
    "<th>Nombre</th> "
    "</tr>";


    do {
      lectura =  f.readStringUntil('\n');
      if (lectura != NULL) {
        String hora =  getValue(lectura, '-', 0);
        hora.remove(hora.length() - 1);
        Serial.println(hora);
    
        String tarjeta = getValue(lectura, '-', 1);
        tarjeta.remove(tarjeta.length() - 1);
        
        String nombre = getValue(lectura, '-', 2);
        nombre.remove(nombre.length() - 1);
        
        contenido +="<tr>"
        "<td>"+hora+"</td>"
        "<td>"+tarjeta+"</td>"
        "<td>"+nombre+"</td>"
        "</tr>";
      } 
      
    } while (lectura != "");
    contenido +="</table>";
    f.close();
  }
  
  contenido += "</body>"
  "</html>";
  server.send(200, "text/html", contenido);
  
}

void paginaUsuarios() {
  String codigo = "";
  String lectura = "";
  
  contenido = "<body>";
  f = SPIFFS.open("baseDeUsuarios.txt" , "r");
  
  if (!f) {
    Serial.println("no existe el archivo que queres abrir");
    contenido+=("no existe el archivo que queres abrir");
    f.close();
  } 
  else 
  {
    contenido +="<table>"
    "<tr>"
    "<th>Codigo</th>"
    "<th>Nombre Usuario</th> "
    "</tr>";


    do {
      lectura =  f.readStringUntil('\n');
      if (lectura != NULL) {
        codigo =  getValue(lectura, '-', 0);
        codigo.remove(codigo.length() - 1);
        Serial.println(codigo);
    
        String nombre = getValue(lectura, '-', 1);
        nombre.remove(nombre.length() - 1);
        contenido +="<tr>"
        "<td>"+codigo+"</td>"
        "<td>"+nombre+"</td>"
        "</tr>";
      } 
      
    } while (lectura != "");
    contenido +="</table>";
    f.close();
  }
  
  contenido += "</body>"
	"</html>";
  server.send(200, "text/html", contenido);
  
}


void imprimirHoraSerial() {
  Serial.println(imprimirHora());
}
void setup() {

  //  evento.attach(10, imprimirHoraSerial);


  // put your setup code here, to run once:
  swSer.begin(9600);
  Serial.begin(9600); delay(200);
  bool result = SPIFFS.begin();

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

  server.serveStatic("/", SPIFFS, "historial.txt");
  server.on("/usuarios.html", paginaUsuarios);
  server.on("/historial.html", paginaHistorial);
  server.on("/agregarUsuario", agregarUsuario);  
  server.begin();
  

  Serial.println("");
  Serial.println("Conectado!");

  timeClient.update();  //Actualizo la hora interna
  delay(2000);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  //insertarUsuario("5361F5CC","sebastian","123456");
  Serial.println(validarCodigo("123456"));
  Serial.println(validarTarjeta("AAAAA"));
}





void loop() {

  while (swSer.available() > 0) {

    String lectura = swSer.readStringUntil('\n');
    String codigo  = getValue(lectura, ':', 0);
    codigo.remove(codigo.length() - 1);




    if (codigo == "LEE") {

      String tarjeta  = getValue(lectura, ':', 1);
      tarjeta.remove(tarjeta.length() - 2);
      String lee = validarTarjeta(tarjeta);
      if (lectura !="") {
        swSer.println("AUH:1");
        ultimo = "AUH:1";
        Serial.println("Autorizo");
        insertarHistorial(tarjeta,retornarNombre(lee),imprimirHora());     
      } else {
        swSer.println("AUH:0");
        ultimo = "AUH:0";
        Serial.println("NO Autorizo");
      }
      Serial.println(tarjeta);



    } else if (codigo == "RTY") {
      swSer.println(ultimo);

    }else if (codigo=="TAR"){
      String tarjeta = getValue(lectura, ':', 1);
      Serial.println("Recibi nueva tarjeta");
      Serial.println(tarjeta);
      insertarUsuario(tarjeta,nombreNuevoUsuario,codigoNuevoUsuario);
      


    } else if (codigo == "COD") {

      String codigo  = getValue(lectura, ':', 1);
      codigo.remove(codigo.length() - 1);
      String lee2 = validarCodigo(codigo);
      if (lee2!="") {
        swSer.println("AUH:1");
        ultimo = "AUH:1";
        insertarHistorial(retornarTarjeta(lee2),retornarNombre(lee2),imprimirHora());     
        Serial.println("Autorizo");
      } else {
        swSer.println("AUH:0");
        ultimo = "AUH:0";
        Serial.println("No Autorizo");
      }
      Serial.println(codigo);



    } else {
      Serial.println("no entendi codigo");
    }


  }
  // put your main code here, to run repeatedly:

  server.handleClient();
}




void insertarUsuario(String tarjeta, String nombre, String codigo) {
  File f = SPIFFS.open("baseDeUsuarios.txt", "a");
  if (!f) {
    Serial.println("file creation failed");
  } else {
    f.println(tarjeta + "-" + nombre + "-" + codigo);
    f.flush();
  }
  f.close();
}


void insertarHistorial(String tarjeta, String nombre, String hora) {
  File f = SPIFFS.open("historial.txt", "a");
  if (!f) {
    Serial.println("file creation failed");
  } else {
    f.println(hora + "-" + tarjeta + "-" + nombre);
    f.flush();
  }
  f.close();
}



String validarTarjeta(String tarjeta) {

  String codigo = "";
  String lectura = "";
  f = SPIFFS.open("baseDeUsuarios.txt" , "r");

  if (!f) {
    Serial.println("no existe el archivo que queres abrir");
    f.close();
  } else {

    do {
      lectura =  f.readStringUntil('\n');
      if (lectura != NULL) {
        codigo =  getValue(lectura, '-', 0);
        codigo.remove(codigo.length() - 1);
        Serial.println(codigo);
        if (codigo == tarjeta) {
          f.close();
          String nombre = getValue(lectura, '-', 1);
          nombre.remove(nombre.length() - 1);
          insertarHistorial(codigo, nombre, imprimirHora());
          return lectura;
        }
      }

    } while (lectura != "");

    f.close();
    return "";
  }
}


String validarCodigo(String codigoAVerificar) {

  String codigo = "";
  String lectura = "";
  f = SPIFFS.open("baseDeUsuarios.txt" , "r");

  if (!f) {
    Serial.println("no existe el archivo que queres abrir");
    f.close();
  } else {

    do {
      lectura =  f.readStringUntil('\n');
      if (lectura != NULL) {
        codigo =  getValue(lectura, '-', 2);
        codigo.remove(codigo.length() - 1);
        Serial.println(codigo);
        if (codigo == codigoAVerificar) {
          f.close();

    
          insertarHistorial(retornarTarjeta(lectura), retornarNombre(lectura), imprimirHora());
          return lectura;
        }
      }

    } while (lectura != "");

    f.close();
    return "";
  }
}


String retornarTarjeta(String lectura){
          String codigo =  getValue(lectura, '-', 0);
          codigo.remove(codigo.length() - 1);
          return codigo;  
}


String retornarNombre(String lectura){
          String nombre = getValue(lectura, '-', 1);
          nombre.remove(nombre.length() - 1);
          return lectura;

  }

/* CODIGO PARA EL ARDUINO COM SERIAL




  //ind1 = readString.indexOf(',');
  // angle = readString.substring(0, ind1);





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
*/
