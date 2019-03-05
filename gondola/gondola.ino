#include <ESP8266WiFi.h>

const char *ssid = "AntaresSkyNet";
const char *password = "nimbus2000";

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Welcome to SkyNet.");

  connectToAP();
}

void loop()
{
  // Check WiFi connectivity;
  if (WiFi.status() != WL_CONNECTED)
  {
    // Reconnect
    Serial.println("Disconnected.");
    connectToAP()
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

  //Start UDP
  Serial.println("Starting UDP");
  udp.begin(udpPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}
