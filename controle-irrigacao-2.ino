#include <WiFi.h>
#include <Servo.h>
#include <PubSubClient.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



// CONFIG SERVO
#define GIRAR 2000
#define PARAR 1500

#define SERVO 16 // Porta Digital 16 PWM

Servo servo;



//CONFIG CONEXAO

#define LEDWIFI 2

//Variavel que armazena a % de umidade atual
float umidade = 100.0;

// Mude as variáveis a seguir de acordo com a rede que você vai conectar o seu ESP
const char* ssid = "VIVOFIBRA-9C51";
const char* pass = "EVLRfYN3XU";

const char* mqtt_server = "mqtt.thingspeak.com";
const char* TOPIC_IRRIGACAO = "channels/1377002/subscribe/fields/field1";

unsigned long humidity_channel = 1377002;
unsigned int humidity = 1;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite (LEDWIFI, HIGH);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "mqtt_user", "30A6ME4COHTR4WZR")) {
      Serial.println("connected");
      client.subscribe(TOPIC_IRRIGACAO, 0);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// CALLBACK MQTT
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

//CONFIG ANIMACAO


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMGOTAS     6 // numero de gotas na animacao

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B01000000,
  B00000000, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B00000111, B11100000,
  B00001111, B11110000,
  B00011111, B11111000,
  B00111111, B11111100,
  B01111111, B11111110,
  B11111111, B11111111,
  B11111111, B11111111,
  B00111111, B11111100,
  B00000111, B11110000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000 };


#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

void irrigacaoanimate(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  display.setRotation(2);
  int8_t f, icons[NUMGOTAS][3];

  // Initialize 'gotas' positions
  for(f=0; f< NUMGOTAS; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(3, 5);
  }

  int starttime = millis();
  int endtime = starttime;
  
  while(1){
    starttime = millis();
    endtime = starttime;
    while ((endtime - starttime) <= 4000){ // do this loop for up to 5000mS{
      display.clearDisplay(); // Clear the display buffer
      
      // desenha cada gota
      for(f=0; f< NUMGOTAS; f++) {
        display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
      }
  
      display.fillRect(0, 24, 150, 150, SSD1306_WHITE);
      
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println(F("    REGANDO..."));
  
      display.display(); // Show the display buffer on the screen
      delay(200);        // Pause for 1/10 second
  
      // atualiza a coordenada de cada gota...
      for(f=0; f< NUMGOTAS; f++) {
        icons[f][YPOS] += icons[f][DELTAY];
      }
      endtime = millis();
    }

    client.loop();
    
    if(umidade >= 50.0){
      break;
    }

    for(f=0; f< NUMGOTAS; f++) {
        icons[f][YPOS] += icons[f][DELTAY];
        // If snowflake is off the bottom of the screen...
        if (icons[f][YPOS] >= display.height()) {
          // Reinitialize to a random position, just off the top
          icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
          icons[f][YPOS]   = -LOGO_HEIGHT;
          icons[f][DELTAY] = random(3, 5);
        }
    }
    
  }
  display.clearDisplay();
  display.display();
}



void setup() {
  //  SERVO
  servo.attach(SERVO);
  
  //  INTERNAL LED
  pinMode(LEDWIFI, OUTPUT);

  // generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();

  Serial.begin(115200); //Iniciando serial
  setup_wifi();  //Conectando Wifi

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void loop() {
  // reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    digitalWrite (LEDWIFI, LOW);
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  if(umidade <= 25.0){
      Serial.println("Irrigando...");
      servo.writeMicroseconds(GIRAR);
      irrigacaoanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT);
      servo.writeMicroseconds(PARAR);
  }
  delay(2000);

}