#include <LiquidCrystal.h>

// Cria o objeto LCD e define os pinos de conexão (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 23, 22, 21, 19);

unsigned long lastUpdate;
bool no_camera = LOW;
bool camera = HIGH;

// Pinos do LED RGB
const int PIN_GREEN  = 9;
const int PIN_RED    = 6;
const int PIN_BLUE   = 8;

// Buzzer passivo (pino S do módulo)
const int PIN_BUZZER = 7;

// Estado para saber se já fizemos os 2 bips
bool camera_beep_done = false;

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  noTone(PIN_BUZZER);

  lcd.begin(16, 2);
}

void loop() {
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

    // Dois bips apenas uma vez quando camera fica HIGH
    if (!camera_beep_done) {
      for (int i = 0; i < 2; i++) {
        tone(PIN_BUZZER, 2000);  // 2 kHz
        delay(200);              // 200 ms ligado
        noTone(PIN_BUZZER);
        delay(150);              // 150 ms pausa entre bips
      }
      camera_beep_done = true;
    }
  }
  else if (no_camera == HIGH) {
    // VERMELHO + "Tenta novamente" + apito contínuo
    digitalWrite(PIN_RED, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Tente");
    lcd.setCursor(2, 1);
    lcd.print("novamente");

    tone(PIN_BUZZER, 2000);     // apito contínuo
    camera_beep_done = false;   // para voltar a fazer 2 bips quando tiver camera HIGH outra vez
  }
  else {
    // AMARELO + "Aproxime-se da camera" + buzzer desligado
    digitalWrite(PIN_RED, HIGH);
    digitalWrite(PIN_GREEN, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Aproxime-se");
    lcd.setCursor(4, 1);
    lcd.print("da camera");

    noTone(PIN_BUZZER);
    camera_beep_done = false;
  }

  delay(100);
}
