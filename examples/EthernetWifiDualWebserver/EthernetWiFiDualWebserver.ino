/*
  EthernetWiFiDualWebserver

  Example how 2 webservers can be started over Ethernet AND Wifi

  since there are issues using the stock Ethernet library with Wifi
  https://github.com/espressif/arduino-esp32/pull/3607#issuecomment-576285941


libraries combined:
	Ethernet2 lib V1.0.4  from M5Stack (https://github.com/adafruit/Ethernet2)
	WebServer lib (stock ESP32 V1.0.4)
	WiFi lib	(Stock ESP32 V1.0.4)


 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

Ethernet2:
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

/*** Ethernet includes***/
#include <SPI.h>
#include <Ethernet2.h>
/*** WiFi includes ***/
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

/*** Ethernet ***/
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
EthernetServer serverEth(80);

/*** WiFi ***/
const char* ssid = ".....";
const char* password = ".....";
WebServer serverWifi(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  serverWifi.send(200, "text/plain", "hello from ESP-device (over Wifi)!");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += serverWifi.uri();
  message += "\nMethod: ";
  message += (serverWifi.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverWifi.args();
  message += "\n";
  for (uint8_t i = 0; i < serverWifi.args(); i++) {
    message += " " + serverWifi.argName(i) + ": " + serverWifi.arg(i) + "\n";
  }
  serverWifi.send(404, "text/plain", message);
  digitalWrite(led, 0);
}


void EthernetBegin(){
	  Ethernet.init(5);
	  // start the Ethernet connection and the server:
	  Ethernet.begin(mac);
	  serverEth.begin();
	  Serial.print("server (Ethernet) is at ");
	  Serial.println(Ethernet.localIP());
}

void EthernetRun(){

	  // listen for incoming clients
	  EthernetClient client = serverEth.available();
	  if (client) {
	    Serial.println("Eth::new client");
	    // an http request ends with a blank line
	    boolean currentLineIsBlank = true;
	    while (client.connected()) {
	      if (client.available()) {
	        char c = client.read();
	        Serial.write(c);
	        // if you've gotten to the end of the line (received a newline
	        // character) and the line is blank, the http request has ended,
	        // so you can send a reply
	        if (c == '\n' && currentLineIsBlank) {
	          // send a standard http response header
	          client.println("HTTP/1.1 200 OK");
	          client.println("Content-Type: text/html");
	          client.println("Connection: close");  // the connection will be closed after completion of the response
	          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
	          client.println();
	          client.println("<!DOCTYPE HTML>");
	          client.println("<html>");
	          // output the value of each analog input pin
	          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
	            int sensorReading = analogRead(analogChannel);
	            client.print("analog input ");
	            client.print(analogChannel);
	            client.print(" is ");
	            client.print(sensorReading);
	            client.println("<br />");
	          }
	          client.println("</html>");
	          break;
	        }
	        if (c == '\n') {
	          // you're starting a new line
	          currentLineIsBlank = true;
	        }
	        else if (c != '\r') {
	          // you've gotten a character on the current line
	          currentLineIsBlank = false;
	        }
	      }
	    }
	    // give the web browser time to receive the data
	    delay(1);
	    // close the connection:
	    client.stop();
	    Serial.println("Eth::client disconnected");
	  }
}

void WifiBegin(){
	  pinMode(led, OUTPUT);
	  digitalWrite(led, 0);
	  WiFi.mode(WIFI_STA);
	  WiFi.begin(ssid, password);
	  Serial.println("");

	  // Wait for connection
	  while (WiFi.status() != WL_CONNECTED) {
	    delay(500);
	    Serial.print(".");
	  }
	  Serial.println("");
	  Serial.print("WIFI::Connected to ");
	  Serial.println(ssid);
	  Serial.print("WIFI::IP address: ");
	  Serial.println(WiFi.localIP());

	  serverWifi.on("/", handleRoot);

	  serverWifi.on("/inline", []() {
		  serverWifi.send(200, "text/plain", "this works as well");
	  });

	  serverWifi.onNotFound(handleNotFound);

	  serverWifi.begin();
	  Serial.println("WIFI::HTTP server started");
}

void WifiRun(){
	serverWifi.handleClient();
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  EthernetBegin();	//begin Ethernet-stuff
  WifiBegin();		//begin Wifi-Stuff

}


void loop() {
	EthernetRun();	//handle Ethernet Clients
	WifiRun();		//handle Ethernet Clients
}

