/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <WiFi.h>
#include <ArduinoJson.h>       // Install through the IDE
#include <WebSocketsClient.h>  // download and install from https://github.com/Links2004/arduinoWebSockets
#include <SocketIOclient.h>    // Install through the IDE
#include <Keypad.h>
#include <Wire.h>               //This library allows you to communicate with I2C devices
#include <LiquidCrystal_I2C.h>  //  This library is for character LCDs based on the HD44780 controller



#define STASSID "Maxime"
#define STAPSK "maxou1234"

// Server connection var declarations
const char* SSID = STASSID;
const char* PASSWORD = STAPSK;

const char* host = "192.168.47.250";
const uint16_t port = 3000;
SocketIOclient socketIO;

// LCD var declarations
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad var declarations
const byte rows = 4;
const byte cols = 4;
char keys[rows][cols] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[rows] = { 15, 14, 13, 12 };  //connect to the row pinouts of the keypad
byte colPins[cols] = { 11, 10, 9, 8 };    //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

// Domain
std::array<String, 2> getTextMessage(char inputKey) {
  switch (inputKey) {
    case '1':
      return { "I love you" };

    case '2':
      return { "You are the most", "beautiful woman!" };

    case '3':
      return { "You are the most", "handsome man!" };

    case '4':
      return { "I don't have any", "idea..." };

    case '5':
      return { "..." };

    case '6':
      return { "... ... ..." };

    case '7':
      return { "1234567" };

    case '8':
      return { "I can't live", "without you!" };

    case '9':
      return { "I want to breath", "with you..." };

    case 'A':
      return { "I'm your father" };

    case 'B':
      return { "I'm Terminator" };

    case 'C':
      return { "... ... ... ..." };

    case 'D':
      return { "Tralala" };

    case '*':
      return { "You're my star" };
    default:
      return { "Unknown message" };
  }
}

void sendMessage(char inputKey) {

  std::array<String, 2> message = getTextMessage(inputKey);

  // display message
  lcd.clear();
  for (int i = 0; i < message.size(); i++) {
    lcd.setCursor(0, i);
    lcd.print(message[i]);
    Serial.println(message[i]);
  }

  Serial.println("Send message");

  // Send message
  String payload = String(inputKey);
  socketIO.send(sIOtype_EVENT, "[\"newMessage\", \"" + payload + "\"]");
}

void messageHandler(uint8_t* payload) {
  StaticJsonDocument<64> doc;

  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println(error.f_str());
    return;
  }

  String messageKey = doc[0];
  String value = doc[1];

  if (messageKey == "lastMessage") {
    char inputKey = value.c_str()[0];
    std::array<String, 2> message = getTextMessage(inputKey);
    Serial.println(messageKey);

    // display message
    lcd.clear();
    for (int i = 0; i < message.size(); i++) {
      lcd.setCursor(0, i);
      lcd.print(message[i]);
      Serial.println(message[i]);
    }
  }
}

void socketIOEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      Serial.println("Disconnected!");
      break;

    case sIOtype_CONNECT:
      Serial.printf("Connected to url: %s%s\n", host, payload);

      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      break;

    case sIOtype_EVENT:
      messageHandler(payload);
      break;
  }
}

void setupWiFi() {
  Serial.println("\nConnecting...");

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnected : ");
  Serial.println(WiFi.localIP());
}

void setupLcd() {
  Wire.setSDA(4);
  Wire.setSCL(5);
  lcd.init();
  lcd.backlight();  // turn on the backlight
  lcd.setCursor(0, 0);
  lcd.print("   Welcome to  ");
}

void setup() {
  Serial.begin(9600);

  // We start by connecting to a WiFi network
  setupWiFi();
  setupLcd();

  socketIO.begin(host, 3000, "/socket.io/?EIO=4");

  socketIO.onEvent(socketIOEvent);
}

void loop() {
  // Socket loop
  socketIO.loop();

  // Keypad loop
  char customKey = keypad.getKey();

  if (customKey) {
    Serial.println(customKey);
    sendMessage(customKey);
  }
}
