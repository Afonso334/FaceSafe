#include <LiquidCrystal.h>

// Cria o objeto LCD e define os pinos de conexão (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 23, 22, 21, 19);
unsigned long lastUpdate;
bool no_camera = LOW;
bool camera = HIGH;


void setup() {
  // Especifica o número de colunas e linhas do LCD
  //pinMode(4, OUTPUT);   // Define pino 4 como saída
  //digitalWrite(4, HIGH); // Coloca o pino 4 em nível alto
   
}

void loop() {
  
  if (no_camera) {
     lcd.begin(16, 2);
     lcd.setCursor(3, 0);
     lcd.print("Aproxime-se");
     lcd.setCursor(4, 1);
     lcd.print("da camera");
  }
  else if (camera) {
    lcd.begin(16, 2);
    lcd.setCursor(3, 0);
    lcd.print("Pode");
    lcd.setCursor(4, 1);
    lcd.print("entrar!");
    delay(10);
  } 
  else {
    lcd.begin(16, 2);
    lcd.setCursor(3, 0);
    lcd.print("Tente");
    lcd.setCursor(2, 1);
    lcd.print("novamente");
  }
  delay(100);

}
