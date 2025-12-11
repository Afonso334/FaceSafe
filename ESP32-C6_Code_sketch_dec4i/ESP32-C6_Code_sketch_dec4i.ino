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

// Buzzer
const int PIN_BUZZER = 7;

// RELÉ
const int PIN_RELE   = 0;

// UART da ESP32‑CAM → ESP32‑C6
HardwareSerial CamSerial(2);

bool camera_beep_done = false;

// DEBOUNCE BLUETOOTH (GLOBAL)
unsigned long lastSerialTime = 0;
const unsigned long serialDebounce = 500;

// DEBUG CONTROL
#define DEBUG true  // false = terminal silencioso

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RELE, OUTPUT);

  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  noTone(PIN_BUZZER);
  digitalWrite(PIN_RELE, LOW);

  lcd.begin(16, 2);

  Serial.begin(9600);
  delay(2000);
  if (DEBUG) Serial.println("ESP32-C6 + ESP32-CAM + HC-05 + RELE PRONTO");

  CamSerial.begin(115200, SERIAL_8N1, 4, 5);

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Aproxime-se");
  lcd.setCursor(4, 1);
  lcd.print("da camera");
}

void loop() {
  static String camBuffer = "";

  // 1) RECEBER LINHAS DA ESP32‑CAM
  while (CamSerial.available() > 0) {
    char c = CamSerial.read();

    if (c == '\n' || c == '\r') {
      camBuffer.trim();
      if (camBuffer.length() > 0 && DEBUG) {
        Serial.print("[CAM=");
        Serial.print(camBuffer);
        Serial.println("]");
      }

      if (camBuffer == "FACE_OK") {
        camera = HIGH;
        no_camera = LOW;
        alerta_enviado = false;
        camera_beep_done = false;
        if (DEBUG) Serial.println("=> FACE_OK");
      } else if (camBuffer == "FACE_FAIL") {
        camera = LOW;
        no_camera = HIGH;
        if (DEBUG) Serial.println("=> FACE_FAIL");
        if (!alerta_enviado) {
          if (DEBUG) Serial.println("Intruso não reconhecido à porta!");
          alerta_enviado = true;
        }
      }
      camBuffer = "";
    } else {
      camBuffer += c;
    }
  }

  // 2) COMANDOS DA APP BLUETOOTH (CORRIGIDO)
  if (millis() - lastSerialTime > serialDebounce && Serial.available()) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();
    
    if (linha.length() > 0 && DEBUG) {
      Serial.print("APP: ");
      Serial.println(linha);
      
      if (linha.equalsIgnoreCase("OK")) {
        camera = HIGH;
        no_camera = LOW;
        alerta_enviado = false;
        camera_beep_done = false;
        Serial.println("APP FORCOU: FACE_OK");
      } else if (linha.equalsIgnoreCase("FECHA")) {
        camera = LOW;
        no_camera = HIGH;
        alerta_enviado = true;
        camera_beep_done = false;
        Serial.println("APP FORCOU: FACE_FAIL");
      }
    }
    lastSerialTime = millis();  // ATUALIZA SEMPRE
    
    while (Serial.available()) Serial.read();
  }

  // 3) LÓGICA VISUAL/SONORA + RELÉ
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  lcd.clear();

  if (camera == HIGH) {
    digitalWrite(PIN_RELE, HIGH);
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
    digitalWrite(PIN_RELE, LOW);
    digitalWrite(PIN_RED, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Tente");
    lcd.setCursor(2, 1);
    lcd.print("novamente");
    tone(PIN_BUZZER, 2000);
    camera_beep_done = false;
  }
  else {
    digitalWrite(PIN_RELE, LOW);
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

