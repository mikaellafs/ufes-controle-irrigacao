//#include <PubSubClient.h>
#include <WiFi.h>
#include <Servo.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macro

// CONFIG ANIMACAO

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
#define LEDSERVER 17

//Variavel que armazena a % de umidade atual
float umidade = 0;

// Mude as variáveis a seguir de acordo com a rede que você vai conectar o seu ESP
const char* ssid = "VIVOFIBRA-9C51";
const char* pass = "EVLRfYN3XU";
WiFiClient  client;

unsigned long humidity_channel = 1377002;
unsigned int humidity = 1;

//// Função que conecta o ESP à rede
//void setup_wifi() {
//  delay(10);
//  Serial.println();
//  Serial.print("Conectando-se a ");
//  Serial.println(ssid);
//  WiFi.begin((char*) ssid, (char*) password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
//  Serial.print("WiFi connectado - endereço IP do ESP: ");
//  Serial.println(WiFi.localIP());
//
//}

//CONFIG ANIMACAO


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     6 // Number of snowflakes in the animation example

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

void regacaoanimate(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  display.setRotation(2);
  int8_t f, icons[NUMFLAKES][3];

  // Initialize 'snowflake' positions
  for(f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(3, 5);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }

  int starttime = millis();
  int endtime = starttime;

  float umidade = 20.0;
  int statusCode = 0;
  
  while(1){
    starttime = millis();
    endtime = starttime;
    while ((endtime - starttime) <= 4000) // do this loop for up to 5000mS
    {
  //  for(;;) { // Loop forever...
      display.clearDisplay(); // Clear the display buffer
      
      // Draw each snowflake:
      for(f=0; f< NUMFLAKES; f++) {
        display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
      }
  
      display.fillRect(0, 24, 150, 150, SSD1306_WHITE);
      
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println(F("    REGANDO..."));
  
      display.display(); // Show the display buffer on the screen
      delay(200);        // Pause for 1/10 second
  
      // Then update coordinates of each flake...
      for(f=0; f< NUMFLAKES; f++) {
  
        icons[f][YPOS] += icons[f][DELTAY];
        // If snowflake is off the bottom of the screen...
//        if (icons[f][YPOS] >= display.height()) {
//          // Reinitialize to a random position, just off the top
//          icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
//          icons[f][YPOS]   = -LOGO_HEIGHT;
//          icons[f][DELTAY] = random(1, 6);
//        }
      }
      endtime = millis();
    }

    
    
    // Read in field 4 of the public channel recording the temperature
    umidade = ThingSpeak.readFloatField(humidity_channel, humidity); 
  
    // Check the status of the read operation to see if it was successful
    statusCode = ThingSpeak.getLastReadStatus();
    if(statusCode == 200){
      Serial.println("Humidade: " + String(umidade) + "%");
      if(umidade > 50.0){
        break;
      }
    }

    for(f=0; f< NUMFLAKES; f++) {
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
  
  //  LED
  pinMode(LEDWIFI, OUTPUT);
  pinMode(LEDSERVER, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();

  Serial.begin(115200); //Iniciando serial
//  setup_wifi();  //Conectando Wifi

  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}


void loop() {
  int statusCode = 0;
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    digitalWrite (LEDWIFI, LOW);
    digitalWrite (LEDSERVER, LOW);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    digitalWrite (LEDWIFI, HIGH);
    Serial.println("\nConnected");
  }

  // Read in field 4 of the public channel recording the temperature
  umidade = ThingSpeak.readFloatField(humidity_channel, humidity); 

  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    digitalWrite (LEDSERVER, HIGH);
    Serial.println("Humidade: " + String(umidade) + "%");

    if(umidade <= 25.0){
      Serial.println("Regando...");
      servo.writeMicroseconds(GIRAR);
      regacaoanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT);
      servo.writeMicroseconds(PARAR);
    }
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
    digitalWrite (LEDSERVER, LOW);
  }

  delay(4000);

}
