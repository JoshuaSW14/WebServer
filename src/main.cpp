#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SECS_PER_MIN (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY (SECS_PER_HOUR * 24L)

#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) ((_time_ % SECS_PER_DAY) / SECS_PER_HOUR)

// access credentials for WiFi network.
const char *ssid = "HOME";
const char *password = "AkJ197219992001";

const int oneWireBus = D3;

unsigned long currentTime = millis();

OneWire oneWire(oneWireBus);         // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature DS18B20(&oneWire); // Pass our oneWire reference to Dallas Temperature sensor

WiFiServer server(80); // WiFI server.  Listen on port 80, which is the default TCP port for HTTP

void time(long val)
{
}

void setup()
{
  Serial.begin(115200);

  DS18B20.begin();
  Serial.println("\nWeb Server Demo");

  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

void loop()
{
  float fTemp;

  WiFiClient client = server.available();

  if (client)
  {
    Serial.println("\n>> Client connected");
    Serial.println(">> Client request:");
    while (client.connected())
    {
      currentTime = millis();

      DS18B20.requestTemperatures();
      fTemp = DS18B20.getTempCByIndex(0);

      if (client.available()) // read line by line what the client (web browser) is requesting
      {
        String line = client.readStringUntil('\r'); // print information about the request (browser, OS, etc)
        Serial.print(line);

        if (line.length() == 1 && line[0] == '\n') // wait for end of client's request
        {
          client.println("Current temperature is: " + String(fTemp) + " deg. Celsius");
          client.println("IP Address: " + String(WiFi.localIP().toString().c_str()));
          client.print("The WeMos D1 Mini has been running for ");
          long val = millis() / 1000;
          long hours = (numberOfHours(val) == 0) ? 00 : numberOfHours(val);
          long minutes = (numberOfMinutes(val) == 0) ? 00 : numberOfMinutes(val);
          long seconds = (numberOfSeconds(val) == 0) ? 00 : numberOfMinutes(val);

          client.printf("%2d:%2d:%2d\n", hours, minutes, seconds);

          Serial.println(">> Response sent to client");
          break;
        }
      }
    }

    while (client.available()) // keep read client request data (if any).  After that, we can terminate our client connection
    {
      client.read();
    }

    client.stop(); // close the connection:
    Serial.println(">> Client disconnected");
  }
}