#include <PubSubClient.h>
#include <WiFi.h>

// Mude as variáveis a seguir de acordo com a rede que você vai conectar o seu ESP
const char* ssid = "";
const char* password = "";

// Servidor mqtt: utilizaremos um broker do proprio thingspeak
const char* server = "mqtt.thingspeak.com";

//Topic para envio de dados para um canal e multiplos fields de forma geral
//topic = “channels/<channelID>/publish/<writeAPIKey
const char* topic="channels/1377002/publish/IIKNQ8UH34M6ZAEK";

// Inicializa o ESP32 como cliente
WiFiClient espClient;
PubSubClient client(server, 1883, espClient);

//Pino do higrometro
const int higPin = 32;

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
  Serial.print("WiFi connectado - endereço IP do ESP8266: ");
  Serial.println(WiFi.localIP());
}

// Reconecta o ESP ao servidor MQTT
void reconnect() {
  // Fica em loop até conectar novamente
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    
    if (client.connect("ip1ufes")) {
      Serial.println("conectado");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Tente novamente em 5 segundos");
      // Espera 5 segundos para tentar novamente
      delay(5000);
    }
  }
}

// Envia o dado para o thingspeak através do protocolo MQTT
void publishData(String payload){
  if (client.connected()){
    Serial.print("Enviando payload: ");
    Serial.println(payload);
    
    if (client.publish(topic, (char*) payload.c_str())) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }
  }
}

// Função que lê os dados do higrometro e retorna % de umidade
float readHIG(){
  float nvlUmidade;
  int value= analogRead(higPin);// le faixa de valores de 0 - 4096 (0 extremamente umido- 4096 extremamente seco)
  nvlUmidade = ( 100.00 - ( (value/4095.00) * 100.00 ) ); //nivel de umidade em porcentagem

  Serial.print("Umidade lida: ");
  Serial.print(nvlUmidade);
  Serial.println("%");
 
  return nvlUmidade;
}

String geraPayload(){
  String payload = "field1=" + String(readHIG()) + "&"; //Dado do higrometro
    
  payload += "status=MQTTPUBLISH";  //Forma padrao de envio para multiplos fields

  return payload;
}


void setup() {
  Serial.begin(115200); //Iniciando serial
  setup_wifi();  //Conectando Wifi
  pinMode(higPin, INPUT); //Inicia higrometro
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ip1ufes");

  //Envia a umidade lida

  publishData(geraPayload());

  delay(4000); //Espera 5 segundos para proxima leitura
}
