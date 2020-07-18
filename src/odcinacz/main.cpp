#include <ESP8266WiFi.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <EEPROM.h>

RtcDS3231<TwoWire> Rtc(Wire);

#define LED0 2 // WIFI Module LED

#define CUTTER 2 // Tutaj ustaw pin dla odcinacza (pin 6 resetuje płytkę, lol)

#define LOOP_DELAY 1000

#define countof(a) (sizeof(a) / sizeof(a[0]))

// Variables
char *ssid = "Hermes";             // Wifi Name
char *password = "thereisnospoon"; // Wifi Password
const String Devicename = "Odcinacz";

RtcDateTime now;

// WIFI Module Role & Port
IPAddress ip(192, 168, 4, 2);
IPAddress server(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress mask(255, 255, 255, 0);

unsigned int port = 2390;

WiFiClient client;

struct MyFlight
{
  bool inFlight = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
};

struct MyFlight flight;

void printDateTime(const RtcDateTime &dt);

void setupRTC();

void establishConnection();

void connectToAP();

String getData(String uri);

void setup()
{
  EEPROM.begin(sizeof(bool) + sizeof(int) * 3);
  Serial.begin(115200);
  setupRTC();
  client.setTimeout(5000);

  Serial.println("\n Welcome to Hermes-Cutter \n The current time is: ");
  printDateTime(Rtc.GetDateTime());

  flight.inFlight = EEPROM.read(0);
  flight.day = EEPROM.read(sizeof(bool));
  flight.hour = EEPROM.read(sizeof(bool) + sizeof(int));
  flight.minute = EEPROM.read(sizeof(bool) + sizeof(int) * 2);

  Serial.print("Liftoff: ");
  Serial.print(flight.inFlight);
  Serial.println(flight.inFlight ? "In The Air" : "On The Ground");
  Serial.print("Liftoff time: \n Day,hour, minute");
  Serial.println(flight.day);
  Serial.println(flight.hour);
  Serial.println(flight.minute);

  // Setup LED
  pinMode(LED0, OUTPUT);
  digitalWrite(LED0, LOW);

  // Setup cutter
  pinMode(CUTTER, OUTPUT);
  digitalWrite(CUTTER, LOW);

  //connectToAP();
}

void loop()
{

  now = Rtc.GetDateTime();

  if (now.Day() >= flight.day && (now.Hour() >= (flight.hour + 4)) && now.Minute() >= flight.minute)
  {
    // Liftoff + 4h
    // Tu powinien być moment zapalenia odcięcia.
    digitalWrite(LED0, HIGH);
    digitalWrite(CUTTER, HIGH);
  }
  // WIFI
  // Check WiFi connectivity;
  if (WiFi.status() != WL_CONNECTED)
  {
    // Reconnect
    Serial.println("Disconnected.");
    connectToAP();
  }
  else
  {
    printDateTime(now);

    Serial.println("\n Getting status from Gondola. \n");

    if (flight.inFlight != true)
    {
      String data = getData("/inFlight/");
      if (data == "4369")
      {
        Serial.println("\n WYKRYTO START \n");
        flight.inFlight = true;
        flight.day = now.Day();
        flight.hour = now.Hour();
        flight.minute = now.Minute();

        EEPROM.write(0, flight.inFlight);
        EEPROM.write(sizeof(bool), flight.day);
        EEPROM.write(sizeof(bool) + sizeof(int), flight.hour);
        EEPROM.write(sizeof(bool) + sizeof(int) * 2, flight.minute);

        Serial.print("Liftoff: ");
        Serial.print(flight.inFlight);
        Serial.println(flight.inFlight ? "In The Air" : "On The Ground");
        Serial.print("Liftoff time: \n Day,hour, minute");
        Serial.println(flight.day);
        Serial.println(flight.hour);
        Serial.println(flight.minute);
      }
    }

    String data1 = getData("/fallDownToEarth/");
    if (data1 == "4369")
    {
      Serial.println("\n WYKRYTO ODCIĘCIE \n");
      digitalWrite(LED0, LOW);
      digitalWrite(CUTTER, LOW);
    }
    else if (data1 == "0")
    {
      digitalWrite(LED0, HIGH);
      digitalWrite(CUTTER, HIGH);
    }
    Serial.println("\n Finished, sleeping for 1000ms");
    delay(LOOP_DELAY);
  }
  delay(100);
}

void printDateTime(const RtcDateTime &dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
}

void setupRTC()
{
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      // Common Causes:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing

      Serial.println("RTC lost confidence in the DateTime!");

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      Rtc.SetDateTime(compiled);
    }
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void establishConnection()
{
  Serial.println("Connecting to server");
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

void connectToAP()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    unsigned long connectionStart = millis();
    client.stop(); //Make Sure Everything Is Reset
    WiFi.disconnect();
    Serial.println("Not Connected...trying to connect...");
    delay(50);
    WiFi.mode(WIFI_STA);        // station (Client) Only - to avoid broadcasting an SSID ??
    WiFi.begin(ssid, password); // the SSID that we want to connect to
    WiFi.config(ip, gateway, mask);

    while (WiFi.status() != WL_CONNECTED && (millis() - connectionStart < 20000))
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

String getData(String uri)
{

  unsigned long requestStart = millis();

  // conecting as a client
  client.connect(server, port);

  // Send Data
  Serial.println("GETting " + uri);

  client.println("GET " + uri);

  while (millis() - requestStart < 5000)
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
      Serial.print("received from tcp: ");        // print the content
      Serial.println(line);
      return line;
      break;
    }
  }

  client.flush(); // Empty Buffer
  client.stop();
  connectToAP();
}
