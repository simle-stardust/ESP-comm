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


IPAddress gateway(192, 168, 4, 1);
IPAddress ip(192, 168, 4, 2);
IPAddress antares(192, 168, 4, 3);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);
HTTPClient http;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Welcome to Hernes.");

  connectToAP();

  server.on("/data", handleData);
  server.begin();
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

          sendDataTo(gateway, incomingBuffer);
          
          ndx = 0;
        }
      }

      else if (rc == startMarker)
      {
        recvInProgress = true;
      }
    }
  }
}


bool connectToAP()
{
  Serial.println("Connecting ...");
  //WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to access point
  WiFi.config(ip, gateway, subnet);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
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
