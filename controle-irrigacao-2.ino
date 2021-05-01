#include <PubSubClient.h>
#include <WiFi.h>

//Variavel que armazena a % de umidade atual
float umidade = 0;

// Mude as variáveis a seguir de acordo com a rede que você vai conectar o seu ESP
const char* ssid = "";
const char* password = "";

// Servidor mqtt: utilizaremos um broker do proprio thingspeak
const char* server = "mqtt.thingspeak.com";

//Topic inscricao no canal para leitura do field 1
const char* topic="channels/1377002/subscribe/fields/field1/2P3PDL5JDGWLU0SK";

// Inicializa o ESP como cliente
WiFiClient espClient;
PubSubClient client(server, 1883, espClient);

// Função que conecta o ESP à rede
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin((char*) ssid, (char*) password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connectado - endereço IP do ESP: ");
  Serial.println(WiFi.localIP());
}

// Reconecta o ESP ao servidor MQTT
void reconnect() {
  // Fica em loop até conectar novamente
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    
    if (client.connect("ipufes")) {
      Serial.println("conectado");
      
      client.subscribe(topic); //se inscreve no canal
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Funcao executada quando um dado é publicado no topico que o ESP esta inscrito
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Umidade: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  umidade = messageTemp.toFloat();
}

void setup() {
  Serial.begin(115200); //Iniciando serial
  setup_wifi();  //Conectando Wifi
  client.setCallback(callback); //define a função callback, é ela que recebe os dados do MQTT
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ipufes");

  delay(500); //Espera 0.5 segundo
}
