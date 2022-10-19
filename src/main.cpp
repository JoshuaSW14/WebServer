// COMP-10184 – Mohawk College 
// WebServer
//  
// This program demonstrates setting up a web server using the esp8266 wifi module. The web server displays
// the temperature captured from the DS18B20 sensor, the running time, ip address and my name and student number.
//
// @author  Joshua Symons-Webb
// @id      000812836
// 
// I, Joshua Symons-Webb, 000812836 certify that this material is my original work. No
// other person's work has been used without due acknowledgement.
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SECS_PER_MIN (60UL)  //Defined seconds per minute
#define SECS_PER_HOUR (3600UL) //Defined seconds per hour
#define SECS_PER_DAY (SECS_PER_HOUR * 24L) //Defined seconds per day 
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN) //Calculates the number of seconds
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) //Calculates the number of minutes
#define numberOfHours(_time_) ((_time_ % SECS_PER_DAY) / SECS_PER_HOUR) //Calculates the number of hours

// access credentials for WiFi network.
const char *ssid = "Hawkins"; //Mohawk-IoT
const char *password = "bobbyorr"; //IoT@MohawK1

const int oneWireBus = D3; //Pin for OneWire Sensor
unsigned long currentTime = millis(); //Running time for arduino

OneWire oneWire(oneWireBus);         // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature DS18B20(&oneWire); // Pass our oneWire reference to Dallas Temperature sensor
WiFiServer server(80); // WiFI server.  Listen on port 80, which is the default TCP port for HTTP

void setup()
{
  Serial.begin(115200);

  DS18B20.begin();
  Serial.println("\nWeb Server Demo");

  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.begin(ssid, password); //Start the connection
  while (WiFi.status() != WL_CONNECTED) //While wifi is not connected
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  server.begin(); //Start the wifi server
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

void loop()
{
  float fTemp;

  WiFiClient client = server.available();

  // wait for a client (web browser) to connect 
  if (client)
  {
    Serial.println("\n>> Client connected");
    Serial.println(">> Client request:");
    while (client.connected()) //While the client is connected
    {
      currentTime = millis(); //Gets the running time in milli seconds 

      DS18B20.requestTemperatures(); //Requests temperature from sensor
      fTemp = DS18B20.getTempCByIndex(0); //Gets the temperature in the first position

      if (client.available()) // read line by line what the client (web browser) is requesting
      {
        String line = client.readStringUntil('\r'); // print information about the request (browser, OS, etc)
        Serial.print(line);

        if (line.length() == 1 && line[0] == '\n') // wait for end of client's request
        {
          client.println("<html>");
          client.println("<h1 style='color: red;'>Joshua Symons-Webb - 000812836</h1>");
          client.println("<h1>Current temperature is: " + String(fTemp) + " deg. Celsius</h1>");
          client.println("<h1>IP Address: " + String(WiFi.localIP().toString().c_str()) + "</h1>"); //Prints IP Address of micro controller
          client.print("<h1>The WeMos D1 Mini has been running for ");

          long val = millis() / 1000;
          int hours = numberOfHours(val); //gets the hours running
          int minutes = numberOfMinutes(val); //gets the minutues running
          int seconds = numberOfSeconds(val); //gets the seconds running
          client.printf("%02i:%02i:%02i\n", hours, minutes, seconds); //Formats output to be HH:MM:SS
          client.println("</h1></html>");

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