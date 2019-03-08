#include <ESP8266WiFi.h>

const char *ssid = "Hermes";
const char *password = "getmesomemilk";

IPAddress ip(192, 168, 4, 2);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Welcome to Hermes.");

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
  }
}


bool connectToAP()
{
  Serial.println("Connecting ...");
  WiFi.disconnect();
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
