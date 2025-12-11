#include <LiquidCrystal.h>

// LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 23, 22, 21, 19);

unsigned long lastUpdate;
bool no_camera = LOW;
bool camera    = LOW;
bool alerta_enviado = false;

// Pinos do LED RGB
const int PIN_GREEN  = 9;
const int PIN_RED    = 6;
const int PIN_BLUE   = 8;

// Buzzer passivo
const int PIN_BUZZER = 7;

// UART da ESP32‑CAM → ESP32‑C6
HardwareSerial CamSerial(2);

bool camera_beep_done = false;
String ultima_msg_app = "";  // Guarda última mensagem da app

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

  // Serial = HC‑05 (Bluetooth) + debug
  Serial.begin(9600);
  delay(2000);
  Serial.println("ESP32 + HC-05 PRONTO");
  Serial.println("Mensagens da app aparecem no LCD!");

  // UART ESP32‑CAM
  CamSerial.begin(115200, SERIAL_8N1, 12, 4);
}

void loop() {
  static String camBuffer = "";

  // 1) LER ESP32‑CAM
  while (CamSerial.available() > 0) {
    char c = CamSerial.read();
    
    if (c == '\n' || c == '\r') {
      camBuffer.trim();
      Serial.print("[CAM=\"");
      Serial.print(camBuffer);
      Serial.println("\"]");

      if (camBuffer == "FACE_OK") {
        camera = HIGH;
        no_camera = LOW;
        alerta_enviado = false;
        Serial.println("=> FACE_OK");
      } else if (camBuffer == "FACE_FAIL") {
        camera = LOW;
        no_camera = HIGH;
        Serial.println("=> FACE_FAIL");

        if (!alerta_enviado) {
          Serial.println("Intruso não reconhecido à porta!");
          alerta_enviado = true;
        }
      }
      camBuffer = "";
    } else {
      camBuffer += c;
    }
  }

  // 2) LER APP BLUETOOTH → MOSTRA NO LCD
  if (Serial.available()) {
    String txt = Serial.readStringUntil('\n');
    txt.trim();
    
    if (txt.length() > 0) {
      ultima_msg_app = txt;  // Guarda para mostrar no LCD
      Serial.print("App→LCD: ");
      Serial.println(txt);
      
      // Pisca azul para confirmar
      digitalWrite(PIN_BLUE, HIGH);
      delay(200);
      digitalWrite(PIN_BLUE, LOW);
    }
  }

  // 3) LÓGICA VISUAL (prioridade: msg app > camera > intruso > default)
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  lcd.clear();

  // PRIORIDADE 1: Mostra mensagem da app
  if (ultima_msg_app.length() > 0) {
    lcd.setCursor(0, 0);
    lcd.print("App:");
    lcd.setCursor(0, 1);
    
    // Limita a 16 chars
    if (ultima_msg_app.length() > 16) {
      lcd.print(ultima_msg_app.substring(0, 16));
    } else {
      lcd.print(ultima_msg_app);
    }
    
    digitalWrite(PIN_BLUE, HIGH);  // Azul = mensagem app
  }
  // PRIORIDADE 2: FACE_OK
  else if (camera == HIGH) {
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
  // PRIORIDADE 3: FACE_FAIL
  else if (no_camera == HIGH) {
    digitalWrite(PIN_RED, HIGH);
    lcd.setCursor(3, 0);
    lcd.print("Tente");
    lcd.setCursor(2, 1);
    lcd.print("novamente");

    tone(PIN_BUZZER, 2000);
    camera_beep_done = false;
  }
  // PRIORIDADE 4: Default
  else {
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
