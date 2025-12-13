#include <WiFi.h>
#include <LiquidCrystal.h>

// ---------- CONFIG WI-FI ----------
const char* WIFI_SSID = "MEO-B672F0";
const char* WIFI_PASS = "1e251a1c99"; // password da tua rede

// Servidor TCP na porta 5000
WiFiServer tcpServer(5000);
WiFiClient camClient;   // cliente atual da ESP32-CAM

// ---------- LCD ----------
LiquidCrystal lcd(2, 3, 23, 22, 21, 19);

// ---------- ESTADOS ----------
bool no_camera = LOW;
bool camera = LOW;
bool alerta_enviado = false;

// ---------- Pinos do LED RGB ----------
const int PIN_GREEN = 9;
const int PIN_RED   = 6;
const int PIN_BLUE  = 8;

// ---------- Buzzer ----------
const int PIN_BUZZER = 7;

// ---------- RELÉ ----------
const int PIN_RELE = 0;

// ---------- FLAGS ----------
bool camera_beep_done = false;

// ---------- DEBOUNCE BLUETOOTH (APP) ----------
unsigned long lastSerialTime = 0;
const unsigned long serialDebounce = 500;

// ---------- DEBUG ----------
#define DEBUG true  // false = terminal silencioso

// Buffer global para dados da CAM
String camBuffer = "";

void conectaWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  if (DEBUG) {
    Serial.print("A ligar ao WiFi: ");
    Serial.println(WIFI_SSID);
  }

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    if (DEBUG) Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    if (DEBUG) {
      Serial.println();
      Serial.print("WiFi ligado. IP ESP32-C6: ");
      Serial.println(WiFi.localIP());
    }
  } else {
    if (DEBUG) Serial.println("\nFalha ao ligar ao WiFi.");
  }
}

void trataMensagemCamera(String msg) {
  msg.trim();

  if (DEBUG) {
    Serial.print("[TCP_CAM_RAW='");
    Serial.print(msg);
    Serial.println("']");
  }

  if (msg == "FACE_OK") {
    camera = HIGH;
    no_camera = LOW;
    alerta_enviado = false;
    camera_beep_done = false;
    if (DEBUG) Serial.println("=> FACE_OK");
  } else if (msg == "FACE_FAIL") {
    camera = LOW;
    no_camera = HIGH;
    if (DEBUG) Serial.println("=> FACE_FAIL");
    if (!alerta_enviado) {
      if (DEBUG) Serial.println("Intruso não reconhecido à porta!");
      alerta_enviado = true;
    }
  } else if (msg == "FACE_ALIGN") {
    camera = LOW;
    no_camera = LOW;
    alerta_enviado = false;
    camera_beep_done = false;
    if (DEBUG) Serial.println("=> FACE_ALIGN (estado neutro)");
  } else {
    if (DEBUG && msg.length() > 0) {
      Serial.println("Mensagem desconhecida da CAM.");
    }
  }
}

void lerDadosDaCam() {
  // aceitar cliente se ainda não houver
  if (!camClient || !camClient.connected()) {
    camClient = tcpServer.available();
    if (camClient && DEBUG) {
      Serial.println("ESP32-CAM conectada ao TCP.");
    }
    camBuffer = "";  // limpa buffer ao trocar de cliente
  }

  if (camClient && camClient.connected()) {
    while (camClient.available() > 0) {
      char c = camClient.read();

      // fim de linha: processa mensagem
      if (c == '\n' || c == '\r') {
        camBuffer.trim();
        if (camBuffer.length() > 0) {
          trataMensagemCamera(camBuffer);
        }
        camBuffer = "";   // limpa sempre após processar
      } else {
        // só acumula caracteres imprimíveis para evitar lixo
        if (c >= 32 && c <= 126) {
          camBuffer += c;
        }
        // proteção: se passar de 32 chars, descarta
        if (camBuffer.length() > 32) {
          if (DEBUG) Serial.println("Buffer da CAM demasiado grande, a limpar.");
          camBuffer = "";
        }
      }
    }
  }
}

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

  if (DEBUG) Serial.println("ESP32-C6 + ESP32-CAM (TCP) + HC-05 + RELE PRONTO");

  conectaWiFi();
  tcpServer.begin();
  if (DEBUG) {
    Serial.println("Servidor TCP iniciado na porta 5000.");
  }

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Aproxime-se");
  lcd.setCursor(4, 1);
  lcd.print("da camera");
}

void loop() {
  // 1) Ler dados da CAM com limpeza de buffer
  lerDadosDaCam();

  // 2) COMANDOS DA APP BLUETOOTH
  if (millis() - lastSerialTime > serialDebounce && Serial.available()) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();

    if (linha.length() > 0 && DEBUG) {
      Serial.print("APP: ");
      Serial.println(linha);
    }

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

    lastSerialTime = millis();
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
        tone(PIN_BUZZER, 4000);
        delay(200);
        noTone(PIN_BUZZER);
        delay(150);
      }
      camera_beep_done = true;
    }
  } else if (no_camera == HIGH) {
    digitalWrite(PIN_RELE, LOW);
    digitalWrite(PIN_RED, HIGH);

    lcd.setCursor(3, 0);
    lcd.print("Tente");
    lcd.setCursor(2, 1);
    lcd.print("novamente");

    tone(PIN_BUZZER, 4000);
    camera_beep_done = false;
  } else {
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

