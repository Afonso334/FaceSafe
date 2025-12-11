#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal.h>

// === LCD paralelo (RS, E, D4, D5, D6, D7) ===
LiquidCrystal lcd(2, 3, 23, 22, 21, 19);

// === LED RGB ===
const int PIN_GREEN  = 9;
const int PIN_RED    = 6;
const int PIN_BLUE   = 8;

// === Buzzer ===
const int PIN_BUZZER = 7;

// === Wi-Fi (MESMA rede da câmara) ===
const char* ssid = "MEO-B672F0";
const char* password = "1e251a1c99";

WebServer server(80);

// --- Funções auxiliares ---
void ledsOff() {
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
}

void beepDuploOK() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(120);
    digitalWrite(PIN_BUZZER, LOW);
    delay(120);
  }
}

void beepIntruso() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(80);
    digitalWrite(PIN_BUZZER, LOW);
    delay(80);
  }
}

void mostraLCD(const char* linha1, const char* linha2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
}

// --- Handlers HTTP (chamados pela CÂMARA) ---
void handleRoot() {
  String html =
    "<h1>ESP32-C6 CONTROLO</h1>"
    "<p>/FACE_OK  -> Pode entrar</p>"
    "<p>/FACE_FAIL -> Acesso negado</p>";
  server.send(200, "text/html", html);
}

// FACE_OK = cara reconhecida -> Pode entrar
void handleFaceOk() {
  ledsOff();
  digitalWrite(PIN_GREEN, HIGH);      // verde ligado

  mostraLCD("Pode entrar", "Face valida");
  beepDuploOK();

  server.send(200, "text/plain", "FACE_OK recebido");
}

// FACE_FAIL = intruso / falha
void handleFaceFail() {
  ledsOff();
  digitalWrite(PIN_RED, HIGH);        // vermelho ligado

  mostraLCD("Acesso negado", "Intruso / falha");
  beepIntruso();

  server.send(200, "text/plain", "FACE_FAIL recebido");
}

void handleNotFound() {
  server.send(404, "text/plain", "Nao encontrado");
}

void setup() {
  // Pinos
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  ledsOff();
  digitalWrite(PIN_BUZZER, LOW);

  // LCD
  lcd.begin(16, 2);
  mostraLCD("A iniciar...", "");

  // Serial para debug
  Serial.begin(115200);
  delay(200);

  // Wi-Fi
  Serial.println("A ligar ao Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("A ligar ao WiFi: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Ligado. IP: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());  // vai aparecer o IP no LCD
  // HTTP routes
  server.on("/", handleRoot);
  server.on("/FACE_OK", handleFaceOk);
  server.on("/FACE_FAIL", handleFaceFail);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}