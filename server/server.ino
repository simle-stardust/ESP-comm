#include <ESP8266WiFi.h>
#define LED0 2
#define MAXSC 6 // MAXIMUM NUMBER OF CLIENTS

//#define DEBUG

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

const char* const RTDStrings[] =
{ "Upper Sample", "Lower Sample", "Upper Heater", "Lower Heater",
		"Ambient" };

const char* const HeaterStrings[] =
{ "Lower", "Upper" };

static const char main_error_strings[17][17] = {"I2C_PRESSURE_ERR", "I2C_RTC_ERR",
		"POWER_LED_FLAG", "I2C_MAX30205_ERR","I2C_HDC1080_ERR","I2C_INA3221_ERR",
		"I2C_GYRO_ERR", "I2C_ACC_ERR","I2C_BARO_ERR","DS18_ERR","SD_ERR",
		"GPS_ERR","WIFI_ERR","LORA_ERR", "ODCINACZ_FLAG", "RUNNING_FLAG"
};

static const char exp_error_strings[17][17] = {"TEMP1_DN_OK", "TEMP2_DN_OK","TEMP3_DN_OK",
		"TEMP4_DN_OK","TEMP5_DN_OK","TEMP6_DN_OK","TEMP1_UP_OK",
		"TEMP2_UP_OK","TEMP3_UP_OK","TEMP4_UP_OK","TEMP5_UP_OK",
		"TEMP6_UP_OK","RTC_ERR","LTC_ERR","SD_ERR",
		"RUNNING",
};

typedef struct
{
  int16_t hour = 0x00;
  int16_t minute = 0x00;
  int16_t second = 0x00;
  uint16_t humidity = 0x0000;
  uint16_t pressure = 0x0000;
  int32_t lattitude = 0x00000000;
  int32_t longtitude = 0x00000000;
  int32_t altitude = 0x00000000;
  int16_t DS18B20[3] = {0x0000};
  int16_t RTD[30] = {0x0000};
  uint16_t mosfet[12] = {0x00};
  uint16_t flag_main = 0x0000;
  uint16_t flag_antares = 0x0000;
  int16_t fallDownToEarth = 0x0000;
} frame_main;

frame_main Memory;

void setup() {

  Serial.begin(115200);

  pinMode(LED0, OUTPUT);

  setAP(ssid, password);

  // for tests to see if the values are properly received in main gondola
  //memset(&Memory.RTD, '1', sizeof(Memory.RTD));
}

void loop() {
  digitalWrite(LED0, !LOW);
  String message = readSerial(serialStart, serialEnd);
  if(message.indexOf("MarcinOdcinaj") >= 0) {
    Serial.println("@MarcinOK");
    Memory.fallDownToEarth = 0x1111;    
  }
  if(message.indexOf("MarcinSetValues") >= 0) {
#ifdef DEBUG
    Serial.println("SERIAL SET VALUES: ");
    Serial.println(message);
#endif
    processMessage(message);
  }
  if(message.indexOf("MarcinGetValues") >= 0) {
#ifdef DEBUG
    Serial.print("SERIAL GET VALUES");
#endif
    Serial.print("@MarcinOK:"); 
    RawMemoryDump();
    Serial.println("!"); 
  }
  if(message.indexOf("reset") >= 0) {
#ifdef DEBUG
    Serial.println("ODCIECIE RESET");
#endif
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

#ifdef DEBUG  
  Serial.println("WIFI Mode : AccessPoint");
#endif

  // Starting the access point
  WiFi.softAPConfig(ip, gateway, mask);                 // softAPConfig (local_ip, gateway, subnet)
  WiFi.softAP(ssid, password, 1, 0, MAXSC);                           // WiFi.softAP(ssid, password, channel, hidden, max_connection)

#ifdef DEBUG
  Serial.println("WIFI < " + String(ssid) + " > ... Started");
#endif

  // wait a bit
  delay(50);

  // getting server IP
  IPAddress IP = WiFi.softAPIP();

  // printing the server IP address
#ifdef DEBUG
  Serial.print("AccessPoint IP : ");
  Serial.println(IP);
#endif

  // starting server
  server.begin();                                                          // which means basically WiFiServer(TCPPort);
#ifdef DEBUG
  Serial.println("Server Started");
#endif
}

void HandleClients() {
  String Message;
  if (server.hasClient()) {
    WiFiClient client = server.available();
    //client.setNoDelay(1);                                          // enable fast communication
    while (client.connected()) {
      //---------------------------------------------------------------
      // If clients are connected
      //---------------------------------------------------------------
      if (client.available()) {
        // read the message
        Message = client.readStringUntil('\r');
        
#ifdef DEBUG
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
#endif
        processMessage(Message);

        // reply to the client with a message
        if (Message.indexOf("GET") >= 0) {
          
          digitalWrite(LED0, !HIGH);
#ifdef DEBUG
          Serial.println("Serving GET request");
#endif
          if (Message.indexOf("/memory/") >= 0) {
            String resp = HTMLResponse(CSVmemoryDump());
            uint8_t i = 0;
            for (; i < resp.length()/200; i++)
            {
              client.print(resp.substring(i * 200, (i+1) * 200));
            }
            client.println(resp.substring(i * 200, resp.length()));
          } else if (Message.indexOf("/fallDownToEarth/") >= 0) {
            client.println(Memory.fallDownToEarth);
          } else if (Message.indexOf("/418/") >= 0) {
            client.println("I'm a teapot");
          }
        }
        else if (Message.indexOf("MarcinSetValuesKom") >= 0) {
          client.println("@MarcinOK!");
        }
        else if (Message.indexOf("MarcinGetWysokosc") >= 0) {
          client.write((uint8_t)((uint32_t)Memory.altitude >> 24 ));
          client.write((uint8_t)((uint32_t)Memory.altitude >> 16 ));
          client.write((uint8_t)((uint32_t)Memory.altitude >> 8 ));
          client.write((uint8_t)Memory.altitude);
          client.println("");
        } else {
          client.println("OK");  // important to use println instead of print, as we are looking for a '\r' at the client
        }

        client.println();
        client.flush();
        client.stop();
      }
    }
  }
}

void processMessage(String Message) {
  char temp[1024];
  String command = "MarcinSetValuesKom:";
  
  if(Message.indexOf(command) >= 0) {
      Message.remove(0, Message.indexOf(command) + command.length());
      Message.toCharArray(temp, Message.length()+1);
      sscanf(temp, "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd", 
        &Memory.RTD[0],&Memory.RTD[1],&Memory.RTD[2],&Memory.RTD[3],&Memory.RTD[4],&Memory.RTD[5],
        &Memory.RTD[6],&Memory.RTD[7],&Memory.RTD[8],&Memory.RTD[9],&Memory.RTD[10],&Memory.RTD[11],
        &Memory.RTD[12],&Memory.RTD[13],&Memory.RTD[14],&Memory.RTD[15],&Memory.RTD[16],&Memory.RTD[17],
        &Memory.RTD[18],&Memory.RTD[19],&Memory.RTD[20],&Memory.RTD[21],&Memory.RTD[22],&Memory.RTD[23],
        &Memory.RTD[24],&Memory.RTD[25],&Memory.RTD[26],&Memory.RTD[27],&Memory.RTD[28],&Memory.RTD[29], 
        &Memory.mosfet[0],&Memory.mosfet[1],&Memory.mosfet[2],&Memory.mosfet[3],&Memory.mosfet[4],
        &Memory.mosfet[5],&Memory.mosfet[6],&Memory.mosfet[7],&Memory.mosfet[8],&Memory.mosfet[9],
        &Memory.mosfet[10],&Memory.mosfet[11], &Memory.flag_antares);
  }
  
  command = "MarcinSetValues:";
  if(Message.indexOf(command) >= 0) {
      Message.remove(0, Message.indexOf(command) + command.length());
      Message.toCharArray(temp, Message.length()+1);

      sscanf(temp, "%hd,%hd,%hd,%hd,%hd,%hd,%hu,%hu,%d,%d,%d,%hu", 
      &Memory.hour,&Memory.minute,&Memory.second,&Memory.DS18B20[0],&Memory.DS18B20[1],
      &Memory.DS18B20[2],&Memory.humidity,&Memory.pressure,&Memory.lattitude,
      &Memory.longtitude,&Memory.altitude,&Memory.flag_main);
  }
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
    "Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
    "\r\n" + content;
  return htmlPage;
}


String CSVmemoryDump() {
  //CSV: hour,minute,second,humidity,pressure,lattitude,longitude,4xDS18B20,30xRTD,12xmosfet,flag_main,flag_antares,flag_odcinacz
  String CSV = "Timestamp: " + String(Memory.hour) + ":" + String(Memory.minute) + ":" + String(Memory.second) + "<br/>";
  CSV += "Humidity: " + String(Memory.humidity) + "<br/>";
  CSV += "Pressure: " + String(Memory.pressure) + "<br/>";
  CSV += "Lattitude: " + String(Memory.lattitude) + "<br/>";
  CSV += "Longtitude: " + String(Memory.longtitude) + "<br/>";
  CSV += "Altitude: " + String(Memory.altitude) + "<br/>";

  for (int i = 0; i < 3; ++i) {
    CSV += "DS18B20[" + String(i) + "]: " + String(Memory.DS18B20[i]) + "<br />";
  }

  CSV += "<table><tr><th>RTD</th><th>LTC1</th><th>LTC2</th><th>LTC3</th><th>LTC4</th><th>LTC5</th><th>LTC6</th></tr>";

  for (int i = 0; i < 30; i += 6) {
    CSV += "<tr><td>" + String(RTDStrings[i / 6]) + "</td>";
    CSV += "<td>" + String(Memory.RTD[i]) + "</td><td>" + String(Memory.RTD[i+1]) + "</td><td>" + String(Memory.RTD[i+2]) + "</td>";
    CSV += "<td>" + String(Memory.RTD[i+3]) + "</td><td>" + String(Memory.RTD[i+4]) + "</td><td>" + String(Memory.RTD[i+5]) + "</td></tr>";
  }

  CSV += "</table><br/><table><tr><th>Level</th><th>Heater 1</th><th>Heater 2</th><th>Heater 3</th><th>Heater 4</th><th>Heater 5</th><th>Heater6</th></tr>";
  for (int i = 0; i < 12; i += 6) {
    CSV += "<tr><td>" + String(HeaterStrings[i / 6]) + "</td>";
    CSV += "<td>" + String(Memory.mosfet[i]) + "</td><td>" + String(Memory.mosfet[i+1]) + "</td><td>" + String(Memory.mosfet[i+2]) + "</td>";
    CSV += "<td>" + String(Memory.mosfet[i+3]) + "</td><td>" + String(Memory.mosfet[i+4]) + "</td><td>" + String(Memory.mosfet[i+5]) + "</td></tr>";
  }
  CSV += "</table><br/>";
  
  CSV += "Flag raw main: " + String(Memory.flag_main, BIN) + "<br/>";
  CSV += "Flag raw experiment: " + String(Memory.flag_antares, BIN) + "<br/>";
  CSV += "Falldown: " + String(Memory.fallDownToEarth, BIN) + "<br/><br/>";

  CSV += "Flags main: ";

  for (uint8_t i = 0; i < 15; i++)
  {
    if ((Memory.flag_main & (1 << i)) != 0)
    {
      CSV += "<b>" + String(main_error_strings[i]) + "</b>, ";
    }
    else
    {
      CSV += String(main_error_strings[i]) + ", ";
    }
  }

  CSV += "<br/>Flags experiment: ";

  for (uint8_t i = 0; i < 15; i++)
  {
    if ((Memory.flag_antares & (1 << i)) != 0)
    {
      CSV += "<b>" + String(exp_error_strings[i]) + "</b>, ";
    }
    else
    {
      CSV += String(exp_error_strings[i]) + ", ";
    }
  }
  return CSV;
}

void RawMemoryDump()
{
  for (uint8_t i = 0; i < 30; i++)
  {
    Serial.write((uint8_t)((uint16_t)Memory.RTD[i] >> 8 ));
    Serial.write((uint8_t)(Memory.RTD[i]));
  }
  Serial.write((uint8_t)(Memory.flag_antares >> 8));
  Serial.write((uint8_t)Memory.flag_antares);
}
