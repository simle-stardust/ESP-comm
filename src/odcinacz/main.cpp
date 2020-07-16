#include <ESP8266WiFi.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

RtcDS3231<TwoWire> Rtc(Wire);

#define LED0 2 // WIFI Module LED

#define CUTTER 6 // Tutaj ustaw pin dla odcinacza.

#define LOOP_DELAY 1000

#define countof(a) (sizeof(a) / sizeof(a[0]))

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

void printDateTime(const RtcDateTime& dt);

void setupRTC();

void establishConnection();

void connectToAP();

void sendData(String *message);

String getData(String uri);

String readSerial(char startMarker, char endMarker);

void setup()
{
  Serial.begin(115200);

	// Setup LED
	pinMode(LED0, OUTPUT);
	digitalWrite(LED0, LOW);


	// Setup cutter
	pinMode(CUTTER, OUTPUT);
	digitalWrite(CUTTER, LOW);

	connectToAP();
}

void loop()
{

	// WIFI
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
	String data = getData("/fallDownToEarth/");
	if (data != "0") {
	  digitalWrite(LED0, LOW);
	  digitalWrite(CUTTER, LOW);
	}  else {
	  digitalWrite(LED0, HIGH);
	  digitalWrite(CUTTER, HIGH);
	}

	delay(LOOP_DELAY);
  }
}


void setupRTC() {
	Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    //printDateTime(compiled);
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

String getData(String uri)
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
	  return line;
	  break;
	}
  }

  client.flush(); // Empty Buffer
  client.stop();
  connectToAP();
}

String readSerial(char startMarker, char endMarker) {
  String message = Serial.readStringUntil(endMarker);
  if (message.indexOf(startMarker) >= 0) {
	message.remove(0, message.indexOf(startMarker) + 1);
	return message;
  }
  return "";
}

void printDateTime(const RtcDateTime& dt)
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
            dt.Second() );
    Serial.print(datestring);
}
