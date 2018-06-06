
#include <LiquidCrystal.h>

boolean procesarClave = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Funcionando");
  setUpTeclado();
  setUpDisplay();
}

void loop() {
  String claveIngresada = leerClave();
  printDisplay(claveIngresada, 0, 0);
  if(procesarClave){
    procesarClaveIngresada(claveIngresada);
  }
  delay(100);
}

void procesarClaveIngresada(String clave) {
    if(clave == "1234") {
      clave = "Clave correcta!";
    } else {
      clave = "Calve incorrecta";
      printDisplay(clave, 0, 0);
      delay(1000);
      limpiarClaveIngresada();
    }
    procesarClave = false;
    return;
}

/******* *******/
/******* Teclado *******/
const int tecladoFilas[4] = {22, 23, 24, 25};
const int tecladoColumnas[3] = {26, 27, 28};
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
}
void printDisplay(String text, int atCol, int atRow){
  lcd.setCursor(atCol, atRow);
  lcd.clear();
  lcd.print(text);
}
