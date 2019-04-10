#include <ESP8266WiFi.h>

#define LED0 2 // WIFI Module LED
#define LED1 4 // Green LED breadboard

// Variables
char *ssid = "Hermes";     // Wifi Name
char *password = "thereisnospoon"; // Wifi Password
const String Devicename = "Odcinacz";

char serialStart = '@';
char serialEnd = '!';

// WIFI Module Role & Port
IPAddress ip(192, 168, 4, 2);
IPAddress server(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress mask(255, 255, 255, 0);

unsigned int port = 2390;

WiFiClient client;

void setup()
{
  Serial.begin(115200);

  setupWiFiLED();
  setupOdcinacz();
  connectToAP();
}

void loop()
{
  // Check WiFi connectivity;
  if (WiFi.status() != WL_CONNECTED)
  {
    // Reconnect
    Serial.println("Disconnected.");
    connectToAP();
    while (1)
    {
      if (WiFi.status() == WL_CONNECTED)
        break;
    }
  }
  else
  {
    String message = readSerial(serialStart, serialEnd);
    if (message.length() > 0)
    {
      if (message.indexOf("led-on") >= 0) {
        digitalWrite(LED0, !HIGH);
      } else if (message.indexOf("led-off") >= 0) {
        digitalWrite(LED0, !LOW);
      } else {
        getData(message);
      }
    }
  }
}

void sendData(String *message)
{
  // conecting as a client
  client.connect(server, port);
  
  // Send Data
  client.println(*message);

  while (1)
  {
    // Check For Reply
    int len = client.available();
    if (len > 0)
    {
      if (len > 80)
      {
        len = 80;
      }
      String line = client.readStringUntil('\r'); // if '\r' is found
      Serial.print("received: ");                     // print the content
      Serial.println(line);
      break;
    }
  }

  client.flush(); // Empty Buffer
  connectToAP();
}

void getData(String uri)
{

  // conecting as a client
  client.connect(server, port);
    
  // Send Data
  Serial.println("GETting " + uri);

  client.println("GET " + uri);



  while (1)
  {
    // Check For Reply
    int len = client.available();
    if (len > 0)
    {
      if (len > 80)
      {
        len = 80;
      }
      String line = client.readStringUntil('\r'); // if '\r' is found
      Serial.print("received from tcp: ");                     // print the content
      Serial.println(line);
      break;
    }
  }

  client.flush(); // Empty Buffer
  client.stop();
  connectToAP();
}

void connectToAP()
{
  if (WiFi.status() != WL_CONNECTED)
  {

    client.stop(); //Make Sure Everything Is Reset
    WiFi.disconnect();
    Serial.println("Not Connected...trying to connect...");
    delay(50);
    WiFi.mode(WIFI_STA);        // station (Client) Only - to avoid broadcasting an SSID ??
    WiFi.begin(ssid, password); // the SSID that we want to connect to
    WiFi.config(ip, gateway, mask);

    while (WiFi.status() != WL_CONNECTED)
    {
      for (int i = 0; i < 10; i++)
      {
        digitalWrite(LED0, !HIGH);
        delay(250);
        digitalWrite(LED0, !LOW);
        delay(250);
        Serial.print(".");
      }
      Serial.println("");
    }
    // stop blinking to indicate if connected -------------------------------
    digitalWrite(LED0, !HIGH);
    Serial.println("!-- Client Device Connected --!");

    // Printing IP Address --------------------------------------------------
    Serial.println("Connected To      : " + String(WiFi.SSID()));
    Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
    Serial.print("Server IP Address : ");
    Serial.println(server);
    Serial.print("Device IP Address : ");
    Serial.println(WiFi.localIP());

    establishConnection();
  }
}

void establishConnection()
{
  // first make sure you got disconnected
  client.stop();

  // if sucessfully connected send connection message
  if (client.connect(server, port))
  {
    Serial.println("<" + Devicename + "-CONNECTED>");
    client.println("<" + Devicename + "-CONNECTED>");
  }
  client.setNoDelay(1); // allow fast communication?
}

String readSerial(char startMarker, char endMarker) {
  String message = Serial.readStringUntil(endMarker);
  if (message.indexOf(startMarker) >= 0) {
    message.remove(0, message.indexOf(startMarker) + 1);
    return message;
  }
  return "";
}

void setupWiFiLED() {
  pinMode(LED0, OUTPUT);    // WIFI OnBoard LED Light
  digitalWrite(LED0, !LOW); // Turn WiFi LED Off
}

void setupOdcinacz() {
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, !LOW);
}
