#include <LiquidCrystal.h>

boolean modoTest = false;
boolean modoMantenimiento = false;
boolean modoNuevaTarjeta = false;
boolean puertaAbierta = false;
const int pinPuerta = 21;
const String claveDeTest = "042";
const String claveDeMantenimiento = "043";

void setup() {
  Serial.begin(9600); //Consola
  Serial2.begin(9600);//ESP
  Serial.println("Funcionando");
  setUpPuerta();
  setUpLectorTarjeta();
  setUpTeclado();
  setUpDisplay();
}

void loop() {
  if(modoNuevaTarjeta) {
    procesarNuevaTarjeta();
  } else if(modoTest) {
    correrModoTest();
  } else if(modoMantenimiento) {
    
  } else {
    accesoViaTarjeta();
    accesoViaClaveDeIngreso();
    procesarRespuestaDeESP();
  }
  sonarBuzzer();
}

const int pinBuzzer = 37;
int stepp = -1;
boolean cambioStepp = true;
boolean buzzerTest = LOW;
void correrModoTest() {
  String mensaje;
  char digitoIngresado = leerDigito();
  if(stepp == -1) {
    mensaje = "Modo Test";
  } else if(stepp == 0) {
    char digitoDeEscape = '-';
    int fila = 0;
    int columna = 0;
    while(digitoDeEscape == '-') {
      printEnDisplay("*", columna, fila); 
      digitoDeEscape = leerDigito();
      delay(200);
      columna++;
      if(columna == 17) {
        columna = 0;
        if(fila == 0)
          fila = 1;
        else
          fila = 0;
      }
    }
    digitoIngresado = '#';
  } else if(stepp == 1) {
    mensaje = "Acerque tarjeta";
    String codigoTarjetaTest = leerCodigoDeTarjetaTest();
    if(codigoTarjetaTest != "") {
      printEnDisplay(codigoTarjetaTest, 0, 0); 
    }
  } else if(stepp == 2) {
    mensaje = "*:Sonar Buzzer";
    if(digitoIngresado == '*') {
      if(buzzerTest) {
        buzzerTest = LOW;
      } else {
        buzzerTest = HIGH;
      }
    }
    digitalWrite(pinBuzzer, buzzerTest);
  } else if(stepp == 3) {
    mensaje = "*:Toggle Puerta";
    if(digitoIngresado == '*') {
      Serial.println("Entro");
      cambioEstadoPuerta();
    }
  } else if(stepp == 4) {
    mensaje = "Fin Modo Test";
    modoTest = false;
    stepp = -2;
    cleanUp();
  }
  if(cambioStepp) {
      printEnDisplay(mensaje, 0, 0); 
      cambioStepp = false;
  }
  if(digitoIngresado == '#') {
    stepp++;
    cambioStepp = true;
  }
}

/******* Puerta *******/
unsigned long lastReadPuerta = 0;
const int debouncePuerta = 200;
unsigned long puertaUltimaVezAbierta = 0;
const int timerParaBuzzer = 3000;
boolean sonandoBuzzer = false;

void setUpPuerta() {
  pinMode(pinPuerta, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pinPuerta), cambioEstadoPuerta, CHANGE);
}

void cambioEstadoPuerta() {
  Serial.println("cambio estado");
  if(millis() - lastReadPuerta > debouncePuerta) {
    Serial.println("CAMBIO PUERTA");
    if(puertaAbierta) {
      printEnDisplay("Puerta Cerrada", 0, 0);
    } else {
      printEnDisplay("Puerta Abierta", 0, 0); 
      puertaUltimaVezAbierta = millis();
    }
    
    lastReadPuerta = millis();
    puertaAbierta = !puertaAbierta;
  }
}

void sonarBuzzer() {
  if(puertaAbierta && !sonandoBuzzer && (millis() - puertaUltimaVezAbierta) > timerParaBuzzer) {
    digitalWrite(pinBuzzer, HIGH);
    sonandoBuzzer = true;
  } else if(!puertaAbierta) {
    digitalWrite(pinBuzzer, LOW);
    sonandoBuzzer = false;
  }
}
/******* *******/

/******* Tarjeta *******/  
unsigned long lastRead = 0;
const int debounce = 500;
String codigoLeido = "";

void setUpLectorTarjeta() {
  Serial1.begin(9600);
}

void accesoViaTarjeta() {
  leerCodigoDeTarjeta(false);
}

void procesarNuevaTarjeta() {
  leerCodigoDeTarjeta(true);
  if(codigoLeido != "") {
    modoNuevaTarjeta = false;
    printEnDisplay("Codigo Enviado", 0, 0);
  }
}

void leerCodigoDeTarjeta(boolean nuevaTarjeta) {
  if (Serial1.available() > 0){
    if(millis() - lastRead > debounce) {
      String codigoTarjeta = Serial1.readStringUntil('\n');
      lastRead = millis();
      Serial.println(codigoTarjeta);
      if(nuevaTarjeta) {
        Serial.println("Mando TAR:" +codigoTarjeta);
        Serial2.println("TAR:"+codigoTarjeta);
      } else {
        Serial2.println("LEE:"+codigoTarjeta);
      }
      codigoLeido = codigoTarjeta;
    } else {
      while(Serial1.available()){
        Serial1.read();
        lastRead = millis();
      }
    }
  }
}

String leerCodigoDeTarjetaTest() {
  if (Serial1.available() > 0){
    if(millis() - lastRead > debounce) {
      String codigoTarjeta = Serial1.readStringUntil('\n');
      lastRead = millis();
      return codigoTarjeta;
    } else {
      while(Serial1.available()){
        Serial1.read();
        lastRead = millis();
      }
    }
  }
  return "";
}

/******* *******/

/******* Clave Manual *******/  
boolean procesarClave = false;
boolean esperandoRespuestaCodigo = false;
String claveLeida = "";

void accesoViaClaveDeIngreso() {
  String claveIngresada = leerClave();
  if(claveIngresada != claveLeida) {
    claveLeida = claveIngresada;
    printEnDisplay(claveLeida, 0, 0);
  }
  if(procesarClave){
    procesarClaveIngresada(claveLeida);
    procesarClave = false;
  }
}

void procesarClaveIngresada(String clave) {
    if(!modoMantenimiento && (clave == claveDeTest)) {
      modoTest = true;
    } else if(clave == claveDeMantenimiento) {
      modoMantenimiento = !modoMantenimiento;
    } else {
      Serial2.println("COD:"+clave);
      if(modoMantenimiento) {
        printEnDisplay("Envia al ESP", 0, 0);
        printEnDisplay(clave, 0, 1);
      } else {
        esperandoRespuestaCodigo = true;
      }
    }
}

/******* *******/

/******* ESP *******/
void procesarRespuestaDeESP() {
  if (Serial2.available() > 0){
    String lectura = Serial2.readStringUntil('\n');
    lectura[sizeof(lectura)-1]=0;
    
    Serial.println("Recibido: " + lectura);
    if(lectura=="AUH:1"){
      Serial.println("todo ok");
      printEnDisplay("Abre Puerta", 0, 0);
      cleanUp();
    }else if(lectura=="AUH:0"){
      printEnDisplay("No Autorizado", 0, 0);
      cleanUp();
    } else if(lectura.equals("NEW:a")) {     
      printEnDisplay("Acerce su tarjeta", 0, 0);
      Serial.println("Pedido");
      Serial.println("Recibido pedido de nueva tarjeta");
      modoNuevaTarjeta = true;
    }else {
      Serial2.println("RTY:");
      Serial.println("Pidiendo retry...");
    }
  }
}

boolean isValidMessage(String message) {
  return message=="AUH:0" || message=="AUH:1" || message=="NEW:a";
}

void cleanUp() {
  codigoLeido = "";
  claveLeida = "";
  limpiarClaveIngresada();
  esperandoRespuestaCodigo = false;
}

/******* *******/

/******* Teclado *******/
const int tecladoFilas[4] = {23, 25, 27, 29};
const int tecladoColumnas[3] = {31, 33, 35};
const char tecladoLayout[4][3] = {{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};
String claveIngresada = "";

void setUpTeclado() {
   for(int i=0; i <= 2; i++){
    pinMode(tecladoColumnas[i], INPUT_PULLUP);
  }
 
  for(int i=0; i <= 3; i++){
    pinMode(tecladoFilas[i], OUTPUT);
  }
}

String leerClave() {
  for (int f = 0; f < 4; f++) {
    fijarFila(f);
    int columnaLeida = buscarColumnaActiva();
    if(columnaLeida != -1) {
      char caracterLeido = tecladoLayout[f][columnaLeida];
      
      if(caracterLeido == '#'){
        if(claveIngresada != ""){
          procesarClave = true;
        }
      } else {
        claveIngresada += caracterLeido;
        Serial.print(caracterLeido);
      }
      
      desactivarColumna(columnaLeida);
    }
  }

  return claveIngresada;
}

char leerDigito() {  
  for (int f = 0; f < 4; f++) {
    fijarFila(f);
    int columnaLeida = buscarColumnaActiva();
    if(columnaLeida != -1) {
      char caracterLeido = tecladoLayout[f][columnaLeida];      
      desactivarColumna(columnaLeida);
      return caracterLeido;
    }
  }

  return '-';
}

void limpiarClaveIngresada() {
  claveIngresada = "";
}

void fijarFila(int fila) {
   for(int i=0; i <= 3; i++){
    digitalWrite(tecladoFilas[i], HIGH);
  }
  
  digitalWrite(tecladoFilas[fila], LOW);
}

int buscarColumnaActiva() {
   for(int c = 0; c < 3; c++) {
      if(digitalRead(tecladoColumnas[c]) == LOW){
        return c;
     }
   }
   return -1;
}

void desactivarColumna(int columna) {
    delay(100);
    while(digitalRead(tecladoColumnas[columna]) == LOW){}
    delay(100);
}
/******* *******/

/******* Display *******/
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //rs, en, d4, d5, d6, d7
void setUpDisplay() {
  lcd.begin(16, 2); //Numero de columnas y filas
  lcd.blink();
}

void printEnDisplay(String text, int atCol, int atRow){
  lcd.clear();
  lcd.setCursor(atCol, atRow);
  lcd.print(text);
}
