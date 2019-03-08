#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

const char *ssid = "Hermes";
const char *password = "getmesomemilk";

IPAddress ip(192, 168, 4, 3);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

HTTPClient http;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Welcome to Hernes.");

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
    while(1)
    {
      if (WiFi.status() == WL_CONNECTED)
        break;
    }
  }
  else
  {
    http.begin("http://" + Ip2Str(gateway) + ":80/data");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST("data=Hello From ESP8266");  
   String payload = http.getString();                  
 
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
 
   http.end();
   delay(1000);
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


String Ip2Str(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}