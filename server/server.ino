#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

char startMarker = '@';
char endMarker = '!';


#define bufferSize 8192
const byte bufferCharSize = 255;
char incomingBuffer[bufferCharSize];
char outgoingBuffer[bufferSize];
static boolean recvInProgress = false;
static byte ndx = 0;
char rc;

const char *ssid = "Hermes";
const char *password = "getmesomemilk";


IPAddress ip(192, 168, 4, 1);
IPAddress odcinacz(192, 168, 4, 2);
IPAddress antares(192, 168, 4, 3);
IPAddress subnet(255, 255, 255, 0);

/* Pamięć: */
// Gondola Główna
typedef struct {
  int8_t hour = 0x00;
  int8_t minute = 0x00;
  int8_t second = 0x00;
  int16_t DS18B20[4] = {0x0000};
  int16_t humidity = 0x0000;
  int16_t pressure = 0x0000;
  int32_t lattitude = 0x00000000;
  int32_t longtitude = 0x00000000;
  int16_t RTD[30] = {0x0000};
  int16_t mosfet = 0x0000;
  int16_t flag_main = 0x0000;
  int16_t flag_antares = 0x0000;
} frame_main;

/*
  // Gondola Experymentalna
  typedef struct {
  int16_t RTD[30] = {0x0000};
  int16_t mosfet = 0x0000;
  int16_t flag = 0x0000;

  } frame_antares;
*/

//frame_main Memory;

ESP8266WebServer server(80);
HTTPClient http;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Hermes reporting for duty");

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);


  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  server.handleClient();

  if (Serial.available() > 0)
  {
    rc = Serial.read();

    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        incomingBuffer[ndx] = rc;
        ndx++;
        if (ndx >= bufferCharSize)
        {
          ndx = bufferCharSize - 1;
        }
      }
      else
      {
        incomingBuffer[ndx] = '\0'; // terminate the string
        recvInProgress = false;

        Serial.print("Received From Serial: ");
        Serial.println(incomingBuffer);

        sendDataTo(odcinacz, incomingBuffer);

        ndx = 0;
      }
    }

    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}

void handleRoot() {
  server.send(200, "text/html", "<h1>Hermes reporting for duty</h1>");
}

void handleData() {
  server.send(200, "text/html", "OK");
  Serial.print("Received From " + server.client().remoteIP().toString() + ": ");
  Serial.println(server.arg("payload"));
}

void sendDataTo(const IPAddress& ip, String message) {
  http.begin("http://" + Ip2Str(ip) + ":80/data");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST("payload=" + message);
  String payload = http.getString();
  String response = String(httpCode) + " " + String(payload);
  http.end();
  Serial.println(response);
}

String Ip2Str(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3]);
}
