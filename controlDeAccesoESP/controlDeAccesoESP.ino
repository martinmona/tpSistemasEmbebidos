

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
#include <ESP8266mDNS.h>
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




String nombreNuevoUsuario;
String codigoNuevoUsuario;

void agregarUsuario() {
  String contenido = leer("header.txt");
  String lectura = "";

  contenido += "<br>"
               "<br>"
               "<form action=\"/agregando\" method=\"get\">"
               "<div class=\"form-group row\">"
               "<label for=\"inputNombre\" class=\"col-sm-2 col-form-label\">Nombre</label>"
               "<div class=\"col-sm-10\">"
               "  <input type=\"text\" class=\"form-control\" id=\"inputNombre\" name=\"Nombre\">"
               " </div>"
               "</div>"
               "<div class=\"form-group row\">"
               "<label for=\"inputCodigo\" class=\"col-sm-2 col-form-label\">Codigo</label>"
               "<div class=\"col-sm-10\">"
               "  <input type=\"text\" class=\"form-control\" id=\"inputCodigo\" name=\"Codigo\">"
               " </div>"
               "</div>"


               "<div class=\"form-group row\">"
               "<div class=\"col-sm-10\">"
               "  <button type=\"submit\" class=\"btn btn-primary\">Agregar Usuario</button>"
               " </div>"
               " </div>"
               "</form>";



  contenido += leer("footer.txt");
  server.send(200, "text/html", contenido);

}

void agregando()
{
  nombreNuevoUsuario = server.arg(0); // ?nombre=nicolas&codigo=123
  codigoNuevoUsuario = server.arg(1);
  swSer.println("NEW:asd");
  server.send(200, "text/html", "<meta http-equiv='refresh' content='5; url=/'> Insertando usuario nuevo...");


}

void guardar(String archivo, String valor) {
  File f = SPIFFS.open(archivo, "w");
  if (!f) {
    Serial.println("file creation failed");
  } else {
    if (valor != "") {
      f.print(valor);
    }
    f.flush();
  }
  f.close();
}


String  leer(String archivo) {
  f = SPIFFS.open("/" + archivo, "r");
  if (!f) {
    Serial.println("no existe el archivo que queres abrir" + archivo);
    f.close();
    return "";
  } else {
    String temp;
    while (f.available()) {
      temp += char(f.read());
    }

    return temp;


  }
}
String  leerSinLinea(String archivo, String codTarjeta) {
  f = SPIFFS.open(archivo, "r");
  if (!f) {
    Serial.println("no existe el archivo que queres abrir" + archivo);
    f.close();
    return "";
  } else {

    String texto = "";
    while (f.available()) {
      String temp;
      char caracter ;
      do {
        caracter = char(f.read());
        temp += caracter;
      } while (caracter != '\n');

      Serial.println("Lei: " + temp);

      String tarjeta =  getValue(temp, '-', 0);
      tarjeta.remove(tarjeta.length() - 3);

      Serial.println(tarjeta);
      Serial.println(codTarjeta);

      if (!tarjeta.equals(codTarjeta)) {
        texto += temp;
        Serial.println("IMP " + temp);
      } else {
        Serial.println("Son iguales");
      }

    }

    return texto;


  }
}
void borrarHistorial() {
  SPIFFS.remove("historial.txt");
  guardar("historial.txt", "");
  server.send(200, "text/html", "<meta http-equiv='refresh' content='5; url=/historial.html'> Borrando historial");


}
void borrarUsuarios() {
  SPIFFS.remove("baseDeUsuarios.txt");
  guardar("baseDeUsuarios.txt", "");
  server.send(200, "text/html", "<meta http-equiv='refresh' content='5; url=/usuarios.html'> Borrando usuarios");


}
void paginaBorrar() {
  String codigoTarjeta = server.arg(0);
  String newUsuarios = leerSinLinea("baseDeUsuarios.txt", codigoTarjeta);
  Serial.print(newUsuarios);
  guardar("baseDeUsuarios.txt", newUsuarios);
  server.send(200, "text/html", "<meta http-equiv='refresh' content='5; url=/usuarios.html'> Borrando usuario");

}

void paginaHistorial() {
  String contenido = leer("header.txt");
  String lectura = "";

  f = SPIFFS.open("historial.txt" , "r");

  if (!f) {
    Serial.println("historial no existe el archivo que queres abrir hisotrial");
    f.close();
  }
  else {
    contenido += "<h2>Historial de acceso</h2>"
                 "<div class=\"table-responsive\">"
                 "<table class=\"table table-striped\">"
                 "<thead>"
                 "<tr>"
                 "<th>Hora</th>"
                 "<th>Tarjeta</th> "
                 "<th>Nombre</th> "
                 "</tr>"
                 "</thead>"
                 "<tbody>";


    do {
      lectura =  f.readStringUntil('\n');
      if (lectura != NULL) {
        String hora =  getValue(lectura, '-', 0);
        hora.remove(hora.length() - 1);
        //Serial.println(hora);

        String tarjeta = getValue(lectura, '-', 1);
        tarjeta.remove(tarjeta.length() - 1);

        String nombre = getValue(lectura, '-', 2);
        nombre.remove(nombre.length() - 1);

        contenido += "<tr>"
                     "<td>" + hora + "</td>"
                     "<td>" + tarjeta + "</td>"
                     "<td>" + nombre + "</td>"
                     "</tr>";
      }

    } while (lectura != "");
    contenido += "</tbody>"
                 "</table>"
                 "</div>";
    f.close();
  }
  contenido += leer("footer.txt");
  server.send(200, "text/html", contenido);

}

void paginaUsuarios() {

  String lectura = "";
  String contenido = leer("header.txt");

  f = SPIFFS.open("baseDeUsuarios.txt" , "r");

  if (!f) {
    Serial.println("rompe no existe el archivo que queres abrir base usuarios");
    //contenido+=("no existe el archivo que queres abrir");
    f.close();
  }
  else
  {
    contenido += "<h2>Usuarios</h2>"
                 "<div class=\"table-responsive\">"
                 "<table class=\"table table-striped\">"
                 "<thead>"
                 "<tr>"
                 "<th>Tarjeta</th>"
                 "<th>Nombre Usuario</th> "
                 "<th>Codigo</th> "
                 "</tr>"
                 "</thead>"
                 "<tbody>";


    do {
      lectura =  f.readStringUntil('\n');
      if (lectura != NULL) {
        String tarjeta =  getValue(lectura, '-', 0);
        tarjeta.remove(tarjeta.length() - 1);
        Serial.println(tarjeta);

        String nombre = getValue(lectura, '-', 1);
        nombre.remove(nombre.length() - 1);

        String codigo = getValue(lectura, '-', 2);
        codigo.remove(codigo.length() - 1);
        contenido += "<tr>"
                     "<td>" + tarjeta + "</td>"
                     "<td>" + nombre + "</td>"
                     "<td>" + codigo + "</td>"
                     "<td><a style='color:black' type=\"button\" href=\"borrar.html?id=" + tarjeta + "\" class=\"btn btn-primary btn-xs\"><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" class=\"feather feather-trash\"><polyline points=\"3 6 5 6 21 6\"></polyline><path d=\"M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2\"></path></svg></a></td>"
                     "</tr>";
      }

    } while (lectura != "");
    contenido += "</tbody>"
                 "</table>"
                 "</div>";
    f.close();
  }

  contenido += leer("footer.txt");
  server.send(200, "text/html", contenido);

}


void imprimirHoraSerial() {
  //Serial.println(imprimirHora());
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


  if (!MDNS.begin("controlacceso")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }


  server.serveStatic("/log", SPIFFS, "baseDeUsuarios.txt");
  server.serveStatic("/log2", SPIFFS, "historial.txt");
  server.on("/", paginaUsuarios);
  server.on("/usuarios.html", paginaUsuarios);
  server.on("/borrar.html", paginaBorrar);
  server.on("/historial.html", paginaHistorial);
  server.on("/agregarUsuario", agregarUsuario);
  server.on("/agregando", agregando);
  server.on("/borrarUsuarios", borrarUsuarios);
  server.on("/borrarHistorial", borrarHistorial);
  server.begin();


  Serial.println("");
  Serial.println("Conectado!");

  timeClient.update();  //Actualizo la hora interna
  delay(2000);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  //insertarUsuario("5361F5CC","sebastian","123456");
  //insertarUsuario("242ASDA2","pepe","988733");
  //Serial.println(validarCodigo("123456"));
  //Serial.println(validarTarjeta("AAAAA"));
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
      if (lee != "") {
        swSer.println("AUH:1");
        ultimo = "AUH:1";
        Serial.println("Autorizo");
        //  insertarHistorial(tarjeta,retornarNombre(lee),imprimirHora());
      } else {
        swSer.println("AUH:0");
        ultimo = "AUH:0";
        Serial.println("NO Autorizo");
      }
      Serial.println(tarjeta);



    } else if (codigo == "RTY") {
      swSer.println(ultimo);

    } else if (codigo == "TAR") {
      String tarjeta = getValue(lectura, ':', 1);
      Serial.println("Recibi nueva tarjeta");
      Serial.println(tarjeta);
      insertarUsuario(tarjeta, nombreNuevoUsuario, codigoNuevoUsuario);



    } else if (codigo == "COD") {

      String codigo  = getValue(lectura, ':', 1);
      codigo.remove(codigo.length() - 1);
      String lee2 = validarCodigo(codigo);
      if (lee2 != "") {
        swSer.println("AUH:1");
        ultimo = "AUH:1";
        //insertarHistorial(retornarTarjeta(lee2),retornarNombre(lee2),imprimirHora());
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
  Serial.println("Insertando usuario" + tarjeta + " " + nombre + " " + codigo);
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
    Serial.println("no existe el archivo que queres abrir validar");
    f.close();
  } else {

    do {
      lectura =  f.readStringUntil('\n');
      if (lectura != NULL) {
        codigo =  getValue(lectura, '-', 0);
        codigo.remove(codigo.length() - 2);
        Serial.println(codigo.substring(0, 7) + " * " + tarjeta.substring(0, 7));
        if (codigo.substring(0, 7).equals(tarjeta.substring(0, 7))) {
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
    Serial.println("no existe el archivo que queres abrir usuarios");
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


String retornarTarjeta(String lectura) {
  String codigo =  getValue(lectura, '-', 0);
  codigo.remove(codigo.length() - 1);
  return codigo;
}


String retornarNombre(String lectura) {
  String nombre = getValue(lectura, '-', 1);
  nombre.remove(nombre.length() - 1);
  return nombre;

}
