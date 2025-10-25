#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>

/*

NOME: DAVI MUNHOZ     RM: 566223
NOME: GABRIEL CIRIACO RM: 564880
NOME: VINICIUS MAFRA  RM: 565916
NOME: MARIANA FRANÇA  RM: 562353
NOME: LARISSA SHIBA   RM: 560462

*/

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int trigA = 25;
const int echoA = 26;
const int trigB = 19;
const int echoB = 18;
const int botaoAnulaGol = 15;

int placarA = 0;
int placarB = 0;
char ultimoTimeGol = ' ';

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "54.146.161.199"; // IP EC2
const int mqtt_port = 1883;
const char* mqtt_topic_publish = "jogo/placar";
const char* mqtt_topic_subscribe = "jogo/comandos";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Tentando conectar MQTT...");
    if (client.connect("ESP32_Placar")) {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic_subscribe);
      Serial.println("Inscrito no tópico: " + String(mqtt_topic_subscribe));
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 2s");
      delay(2000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico ");
  Serial.print(topic);
  Serial.print(": ");
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println(msg);
  if (msg == "golA") {
    placarA++;
    ultimoTimeGol = 'A';
    Serial.println("🏆 Gol do Time A!");
    atualizarPlacar();
  } 
  else if (msg == "golB") {
    placarB++;
    ultimoTimeGol = 'B';
    Serial.println("⚽ Gol do Time B!");
    atualizarPlacar();
  }
  else if (msg == "anular") {
    if (ultimoTimeGol == 'A' && placarA > 0) {
      placarA--;
      
    } 
    else if (ultimoTimeGol == 'B' && placarB > 0) {
      placarB--;
      Serial.println("↩️ Gol anulado do Time B!");
    } 
    else {
      Serial.println("⚠️ Nenhum gol para anular!");
    }
    ultimoTimeGol = ' ';
    atualizarPlacar();
  }
  else if (msg == "reset") {
    placarA = 0;
    placarB = 0;
    ultimoTimeGol = ' ';
    Serial.println("🔄 Placar resetado via MQTT");
    atualizarPlacar();
  }
  else {
    Serial.println("❌ Comando desconhecido");
  }
}

void publicarPlacar() {
  String msg = "{\"TimeA\":" + String(placarA) + ",\"TimeB\":" + String(placarB) + "}";
  client.publish(mqtt_topic_publish, msg.c_str());
  Serial.print("Placar enviado MQTT: ");
  Serial.println(msg);
}

long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 20000); 
  long distancia = duracao * 0.034 / 2; 
  return distancia;
}

void atualizarPlacar() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time A: ");
  lcd.print(placarA);
  lcd.setCursor(0, 1);
  lcd.print("Time B: ");
  lcd.print(placarB);

  publicarPlacar();
}

void anulaGol() {
  if (digitalRead(botaoAnulaGol) == LOW) { 
    if (ultimoTimeGol == 'A' && placarA > 0) {
      placarA--;
      atualizarPlacar();
      delay(500);
    } else if (ultimoTimeGol == 'B' && placarB > 0) {
      placarB--;
      atualizarPlacar();
      delay(500);
    }
    ultimoTimeGol = ' ';
  }
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(trigA, OUTPUT);
  pinMode(echoA, INPUT);
  pinMode(trigB, OUTPUT);
  pinMode(echoB, INPUT);
  pinMode(botaoAnulaGol, INPUT_PULLUP);

  lcd.setCursor(0, 0);
  lcd.print("Placar Iniciado");
  delay(2000);
  lcd.clear();

  atualizarPlacar();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  long distanciaA = medirDistancia(trigA, echoA);
  long distanciaB = medirDistancia(trigB, echoB);

  if (distanciaA > 0 && distanciaA < 10) {
    placarA++;
    ultimoTimeGol = 'A';
    atualizarPlacar();
    delay(1000);
  }

  if (distanciaB > 0 && distanciaB < 10) {
    placarB++;
    ultimoTimeGol = 'B';
    atualizarPlacar();
    delay(1000);
  }

  anulaGol();
}
