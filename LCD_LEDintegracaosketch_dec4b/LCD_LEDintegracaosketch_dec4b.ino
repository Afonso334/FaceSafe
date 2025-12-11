#include <LiquidCrystal.h>

// Cria o objeto LCD e define os pinos de conexão (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 23, 22, 21, 19);

unsigned long lastUpdate;
bool no_camera = LOW;
bool camera = HIGH;

// Pinos do LED RGB
const int PIN_GREEN  = 9;
const int PIN_RED = 6;
const int PIN_BLUE  = 8;

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  // LED começa apagado
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  
  // Inicializa LCD só uma vez aqui
  lcd.begin(16, 2);
}

void loop() {
  // Apaga todos os LEDs primeiro
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  
  lcd.clear();

  if (camera == HIGH) {
    // VERDE + "Pode entrar!"
    digitalWrite(PIN_GREEN, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Pode");
    lcd.setCursor(4, 1);
    lcd.print("entrar!");
  }
  else if (no_camera == HIGH) {
    // VERMELHO + "Tenta novamente"
    digitalWrite(PIN_RED, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Tente");
    lcd.setCursor(2, 1);
    lcd.print("novamente");
  }
  else {
    // AMARELO + "Aproxime-se da camera"
    digitalWrite(PIN_RED, HIGH);
    digitalWrite(PIN_GREEN, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Aproxime-se");
    lcd.setCursor(4, 1);
    lcd.print("da camera");
  }
  
  delay(100);
}

