
#include <LiquidCrystal.h>

boolean modoMantenimiento = false;

void setup() {
  Serial.begin(9600); //Consola
  Serial2.begin(9600);//ESP
  Serial.println("Funcionando");
  setUpLectorTarjeta();
  setUpTeclado();
  setUpDisplay();
}

void loop() {
  accesoViaTarjeta();
  accesoViaClaveDeIngreso();
}

/******* Tarjeta *******/  
unsigned long lastRead = 0;
const int debounce = 500;
String codigoLeido = "";

void setUpLectorTarjeta() {
  Serial1.begin(9600);
}

void accesoViaTarjeta() {
  leerCodigoDeTarjeta();
  if(codigoLeido != "") {
    procesarRespuestaDeESP();
  }
}

void leerCodigoDeTarjeta() {
  if (Serial1.available() > 0){
    if(millis() - lastRead > debounce) {
      String codigoTarjeta = Serial1.readStringUntil('\n');
      lastRead = millis();
      Serial.println(codigoTarjeta);
      Serial2.println("LEE:"+codigoTarjeta);
      codigoLeido = codigoTarjeta;
    } else {
      while(Serial1.available()){
        Serial1.read();
        lastRead = millis();
      }
    }
  }
}

/******* *******/

/******* Clave Manual *******/  
boolean procesarClave = false;
boolean esperandoRespuestaCodigo = false;
const String claveDeMantenimiento = "123123";
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
  if(esperandoRespuestaCodigo) {
    procesarRespuestaDeESP();
  }
}

void procesarClaveIngresada(String clave) {
    if(clave == claveDeMantenimiento) {
      modoMantenimiento = true;
    } else {
      Serial2.println("COD:"+clave);
      esperandoRespuestaCodigo = true;
    }
}

/******* *******/

/******* ESP *******/
void procesarRespuestaDeESP() {
  Serial.println("Procesando");
  if (Serial2.available() > 0){
    String lectura = Serial2.readStringUntil('\n');
    lectura[sizeof(lectura)-1]=0;
    
    Serial.println("Recibido: " + lectura);
    if(lectura=="AUH:1"){
      printEnDisplay("Abre Puerta", 0, 0);
      cleanUp();
    }else if(lectura=="AUH:0"){
      printEnDisplay("No Autorizado", 0, 0);
      cleanUp();
    } else {
      Serial2.println("RTY:");
    }
  }
}

void cleanUp() {
  codigoLeido = "";
  claveLeida = "";
  limpiarClaveIngresada();
  esperandoRespuestaCodigo = false;
}

/******* *******/

/******* Teclado *******/
const int tecladoFilas[4] = {41, 43, 45, 47};
const int tecladoColumnas[3] = {49, 51, 53};
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
        procesarClave = true;
      } else {
        claveIngresada += caracterLeido;
        Serial.print(caracterLeido);
      }
      
      desactivarColumna(columnaLeida);
    }
  }

  return claveIngresada;
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
