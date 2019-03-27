#include <ESP8266WiFi.h>

#define LED0 2 // WIFI Module LED

// Variables
char *ssid = "Hermes"; // Wifi Name
char *password = "thereisnospoon";   // Wifi Password
const String Devicename = "Antares";

char serialStart = '@';
char serialEnd = '!';

// WIFI Module Role & Port
IPAddress ip(192, 168, 4, 3);
IPAddress server(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress mask(255, 255, 255, 0);

unsigned int port = 2390;

WiFiClient client;

void setup()
{
  Serial.begin(115200);

  pinMode(LED0, OUTPUT);    // WIFI OnBoard LED Light
  digitalWrite(LED0, !LOW); // Turn WiFi LED Off

  connectToAP();
}

void loop()
{

    char *message = readSerial(serialStart, serialEnd);
    if (message != "0")
    {

      Serial.print("Received From Serial: ");
      Serial.println(message);
      sendData(message);
    }
  
}

void sendData(char *message)
{
  // Send Data
  client.println(message);

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
      Serial.print("received: ");                 // print the content
      Serial.println(line);
      break;
    }
  }

  client.flush(); // Empty Buffer
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

    // conecting as a client -------------------------------------
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

char *readSerial(char startMarker, char endMarker)
{
  char incomingBuffer[1024];
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char rc;

  while (Serial.available() > 0)
  {
    rc = Serial.read();

    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        incomingBuffer[ndx] = rc;
        ndx++;
        if (ndx >= sizeof incomingBuffer)
        {
          ndx = sizeof incomingBuffer - 1;
        }
      }
      else
      {
        incomingBuffer[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        Serial.println(incomingBuffer);
        return incomingBuffer;
      }
    }

    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
  return "0";
}