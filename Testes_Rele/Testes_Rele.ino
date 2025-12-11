// TESTE SIMPLES DO RELÉ COM ESP32-C6

const int PIN_RELE = 5;   // pino ligado a IN do relé

void setup() {
  pinMode(PIN_RELE, OUTPUT);

  // começa com o relé desligado
  digitalWrite(PIN_RELE, LOW);   // se o teu módulo for ativo a LOW, troca para HIGH
  Serial.begin(115200);
  Serial.println("Teste do relé: ON 2s / OFF 2s");
}

void loop() {
  // LIGA o relé 2 segundos
  Serial.println("RELE ON");
  digitalWrite(PIN_RELE, HIGH);  // se for ativo a LOW, troca HIGH<->LOW
  delay(2000);

  // DESLIGA o relé 2 segundos
  Serial.println("RELE OFF");
  digitalWrite(PIN_RELE, LOW);
  delay(2000);
}
