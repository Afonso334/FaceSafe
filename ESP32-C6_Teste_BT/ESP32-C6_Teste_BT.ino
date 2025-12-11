#include <LiquidCrystal.h>

// LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 23, 22, 21, 19);

bool no_camera = LOW;
bool camera    = LOW;
bool alerta_enviado = false;

// Pinos do LED RGB
const int PIN_GREEN  = 9;
const int PIN_RED    = 6;
const int PIN_BLUE   = 8;

// Buzzer passivo
const int PIN_BUZZER = 7;

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

  Serial.begin(9600);
  delay(2000);
  Serial.println("=== TESTE 3 ESTADOS (5s ciclo) ===");
  Serial.println("0=NORMAL | 1=INTRUSO | 2=CAMERA_OK");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo Teste");
  lcd.setCursor(0, 1);
  lcd.print("Aproxime-se");
}

void loop() {
  static unsigned long lastTest = 0;
  static int estado = 0;  // 0=NORMAL, 1=INTRUSO, 2=CAMERA_OK

  // === CICLO AUTOMÁTICO 5s ===
  if (millis() - lastTest > 5000) {
    estado = (estado + 1) % 3;
    
    // RESET estados para cada ciclo
    camera = LOW;
    no_camera = LOW;
    alerta_enviado = false;
    camera_beep_done = false;
    
    switch (estado) {
      case 0:
        Serial.println("ESTADO 0: NORMAL (amarelo, SILÊNCIO)");
        break;
      case 1:
        no_camera = HIGH;
        Serial.println("ESTADO 1: INTRUSO (vermelho, BUZZER, BT alerta)");
        break;
      case 2:
        camera = HIGH;
        Serial.println("ESTADO 2: CAMERA_OK (verde, 2 beeps)");
        break;
    }
    lastTest = millis();
  }

  // === ALERTA BLUETOOTH (só no estado INTRUSO) ===
  static bool alerta_bt_enviado = false;
  if (no_camera == HIGH && !alerta_bt_enviado) {
    Serial.println("🚨 Intruso não reconhecido à porta!");
    alerta_bt_enviado = true;
  }
  if (estado != 1) alerta_bt_enviado = false;  // Reset no próximo ciclo

  // === COMANDOS APP ===
  if (Serial.available()) {
    String txt = Serial.readStringUntil('\n');
    txt.trim();
    if (txt == "0") { estado = 0; lastTest = millis() - 4000; }
    if (txt == "1") { estado = 1; lastTest = millis() - 4000; }
    if (txt == "2") { estado = 2; lastTest = millis() - 4000; }
    Serial.println("APP CMD: " + txt);
  }

  // === LÓGICA VISUAL E SONORA ===
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  lcd.clear();

  if (camera == HIGH) {
    // ESTADO 2: VERDE + 2 BEEPS
    digitalWrite(PIN_GREEN, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Pode");
    lcd.setCursor(4, 1);
    lcd.print("entrar!");
    
    if (!camera_beep_done) {
      for (int i = 0; i < 2; i++) {
        tone(PIN_BUZZER, 2000);
        delay(200);
        noTone(PIN_BUZZER);
        delay(150);
      }
      camera_beep_done = true;
    }
  }
  else if (no_camera == HIGH) {
    // ESTADO 1: VERMELHO + BUZZER CONTÍNUO
    digitalWrite(PIN_RED, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("INTRUSO!");
    lcd.setCursor(1, 1);
    lcd.print("Tente novamente");
    tone(PIN_BUZZER, 2000);
  }
  else {
    // ESTADO 0: AMARELO + SILÊNCIO
    digitalWrite(PIN_RED, HIGH);
    digitalWrite(PIN_GREEN, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Aproxime-se");
    lcd.setCursor(4, 1);
    lcd.print("da camera");
    noTone(PIN_BUZZER);
  }

  delay(100);
}
