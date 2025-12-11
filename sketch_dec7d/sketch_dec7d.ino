#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "MEO-B672F0";
const char* password = "1e251a1c99";

// MUDAR para o IP que a C6 mostrar no Serial
String targetBase = "http://192.168.1.50";  

void enviaFaceOK() {
  HTTPClient http;
  http.begin(targetBase + "/FACE_OK");
  http.GET();
  http.end();
}

void enviaFaceFAIL() {
  HTTPClient http;
  http.begin(targetBase + "/FACE_FAIL");
  http.GET();
  http.end();
}

void setup() {
  // ... o resto do teu setup (camera init, etc.)

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("CAM a ligar ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nCAM ligada ao Wi-Fi");
}

// Dentro do teu loop, na parte onde já tens o resultado:
void trataResultadoFace(bool intruso, bool reconhecido) {
  // Exemplo:
  if (reconhecido && !intruso) {
    Serial.println("FACE_OK");
    enviaFaceOK();      // <--- chama a C6
  } else {
    Serial.println("FACE_FAIL");
    enviaFaceFAIL();    // <--- chama a C6
  }
}