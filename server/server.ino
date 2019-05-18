#include <ESP8266WiFi.h>
#define LED0 2
#define MAXSC 6 // MAXIMUM NUMBER OF CLIENTS

char *ssid = "Hermes";
char *password = "thereisnospoon";
unsigned int TCPPort = 2390;

char serialStart = '@';
char serialEnd = '!';

IPAddress antares(192, 168, 4, 2);
IPAddress ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress mask(255, 255, 255, 0);

WiFiServer  server(TCPPort);      // THE SERVER AND THE PORT NUMBER
WiFiClient  client[MAXSC];        // THE SERVER CLIENTS Maximum number

typedef struct
{
  int8_t hour = 0x00;
  int8_t minute = 0x00;
  int8_t second = 0x00;
  int16_t humidity = 0x0000;
  int16_t pressure = 0x0000;
  int32_t lattitude = 0x00000000;
  int32_t longtitude = 0x00000000;
  int32_t altitude = 0x00000000;
  int16_t DS18B20[4] = {0x0000};
  int16_t RTD[30] = {0x0000};
  int16_t mosfet[12] = {0x0000};
  int16_t flag_main = 0x0000;
  int16_t flag_antares = 0x0000;
  int16_t fallDownToEarth = 0x0000;
} frame_main;

frame_main Memory;

void setup() {

  Serial.begin(115200);

  pinMode(LED0, OUTPUT);

  setAP(ssid, password);

}

void loop() {
  String message = readSerial(serialStart, serialEnd);
  if(message.indexOf("MarcinOdcinaj") >= 0) {
    Memory.fallDownToEarth = 0x1111;    
  }
  if(message.indexOf("MarcinSetValues") >= 0) {
    Memory.fallDownToEarth = 0x1111;    
  }
  if(message.indexOf("reset") >= 0) {
    Memory.fallDownToEarth = 0x0000;    
  }
    
  HandleClients();
}

//====================================================================================

void setAP(char* ssid, char* password) {
  // Stop any previous WIFI
  WiFi.disconnect();

  // Setting The Wifi Mode
  WiFi.mode(WIFI_AP_STA);
  Serial.println("WIFI Mode : AccessPoint");

  // Starting the access point
  WiFi.softAPConfig(ip, gateway, mask);                 // softAPConfig (local_ip, gateway, subnet)
  WiFi.softAP(ssid, password, 1, 0, MAXSC);                           // WiFi.softAP(ssid, password, channel, hidden, max_connection)
  Serial.println("WIFI < " + String(ssid) + " > ... Started");

  // wait a bit
  delay(50);

  // getting server IP
  IPAddress IP = WiFi.softAPIP();

  // printing the server IP address
  Serial.print("AccessPoint IP : ");
  Serial.println(IP);

  // starting server
  server.begin();                                                 // which means basically WiFiServer(TCPPort);

  Serial.println("Server Started");
}

void HandleClients() {
  String Message;
  if (server.hasClient()) {
    WiFiClient client = server.available();
    client.setNoDelay(1);                                          // enable fast communication
    while (client.connected()) {
      //---------------------------------------------------------------
      // If clients are connected
      //---------------------------------------------------------------
      if (client.available()) {
        // read the message
        Message = client.readStringUntil('\r');
        
        // print the message on the screen
        Serial.println("Received Message:");

        // print who sent it
        Serial.print("From ");
        Serial.print(client.remoteIP());
        Serial.print(", port ");
        Serial.println(client.remotePort());

        // content
        Serial.print("Content: ");
        Serial.println(Message);

        processKomMessage(Message);

        // reply to the client with a message
        if (Message.indexOf("GET") >= 0) {
          Serial.println("Serving GET request");
          if (Message.indexOf("/memory/") >= 0) {
            client.println(HTMLResponse(CSVmemoryDump()));
          } else if (Message.indexOf("/fallDownToEarth/") >= 0) {
            client.println(Memory.fallDownToEarth);
          } else if (Message.indexOf("/418/") >= 0) {
            client.println("I'm a teapot");
          }
        }
        else {
          client.println("OK");  // important to use println instead of print, as we are looking for a '\r' at the client
        }

        client.println();
        client.flush();
        client.stop();
      }
    }
  }
}

void processKomMessage(String Message) {
  String command = "MarcinSetValuesKom:";
  if(Message.indexOf(command) >= 0) {
      Message.remove(0, Message.indexOf(command) + command.length());
      saveToMemoryKom(Message);
   }
}

void processMessage(String Message) {
  String command = "MarcinSetValues:";
  if(Message.indexOf(command) >= 0) {
      Message.remove(0, Message.indexOf(command) + command.length());
      saveToMemory(Message);
   }
}

void saveToMemoryKom(String data) {
  char temp[1024];
  data.toCharArray(temp, data.length()+1);
  sscanf(temp, "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd", &Memory.RTD[0],&Memory.RTD[1],&Memory.RTD[2],&Memory.RTD[3],&Memory.RTD[4],&Memory.RTD[5],&Memory.RTD[6],&Memory.RTD[7],&Memory.RTD[8],&Memory.RTD[9],&Memory.RTD[10],&Memory.RTD[11],&Memory.RTD[12],&Memory.RTD[13],&Memory.RTD[14],&Memory.RTD[15],&Memory.RTD[16],&Memory.RTD[17],&Memory.RTD[18],&Memory.RTD[19],&Memory.RTD[20],&Memory.RTD[21],&Memory.RTD[22],&Memory.RTD[23],&Memory.RTD[24],&Memory.RTD[25],&Memory.RTD[26],&Memory.RTD[27],&Memory.RTD[28],&Memory.RTD[29], &Memory.mosfet[0],&Memory.mosfet[1],&Memory.mosfet[2],&Memory.mosfet[3],&Memory.mosfet[4],&Memory.mosfet[5],&Memory.mosfet[6],&Memory.mosfet[7],&Memory.mosfet[8],&Memory.mosfet[9],&Memory.mosfet[10],&Memory.mosfet[11], &Memory.flag_antares);
}

void saveToMemory(String data) {
  char temp[1024];
  data.toCharArray(temp, data.length()+1);
  sscanf(temp, "%c,%c,%c,%hd,%hd,%hd,%hd,%hd,%hd,%d,%d,%d,%hd", &Memory.hour,&Memory.minute,&Memory.second,&Memory.DS18B20[1],&Memory.DS18B20[2],&Memory.DS18B20[3],&Memory.DS18B20[4],&Memory.humidity,&Memory.pressure,&Memory.lattitude,&Memory.longtitude,&Memory.altitude,&Memory.flag_main);
}

String readSerial(char startMarker, char endMarker) {
  String message = Serial.readStringUntil(endMarker);
  if (message.indexOf(startMarker) >= 0) {
    message.remove(0, message.indexOf(startMarker) + 1);
    return message;
  }
  return "";
}

String HTMLResponse(String body)
{

  String content  = String("<!DOCTYPE HTML>") +
                    "<html>" +
                    body +
                    "</html>" +
                    "\r\n\r\n";
  String htmlPage =
    String("HTTP/1.1 200 OK\r\n") +
    "Content-Type: text/html\r\n" +
    "Content-Length: " + content.length() + "\r\n" +
    "Connection: close\r\n" +  // the connection will be closed after completion of the response
    "Refresh: 1\r\n" +  // refresh the page automatically every 5 sec
    "\r\n" + content;
  return htmlPage;
}


String CSVmemoryDump() {
  //CSV: hour,minute,second,humidity,pressure,lattitude,longitude,4xDS18B20,30xRTD,12xmosfet,flag_main,flag_antares,flag_odcinacz
  String CSV = String(Memory.hour) + "," + Memory.minute + "," + Memory.second + "," + Memory.humidity + "," + Memory.pressure + "," + Memory.lattitude + "," + Memory.longtitude + ",";
  for (int i = 0; i < 4; ++i) {
    CSV += String(Memory.DS18B20[i]) + ",";
  }

  for (int i = 0; i < 30; ++i) {
    CSV += String(Memory.RTD[i]) + ",";
  }

  for (int i = 0; i < 12; ++i) {
    CSV += String(Memory.mosfet[i]) + ",";
  }
  CSV += String(Memory.flag_main, BIN) + "," + String(Memory.flag_antares, BIN) + "," + String(Memory.fallDownToEarth, BIN);
  return CSV;
}
