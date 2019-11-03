#include <SPI.h>
#include <OSCMessage.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>

// constants won't change. They're used here to set pin numbers:
int status = WL_IDLE_STATUS;
#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

const int buttonPin =  2; 
const int potentiometerPin = A7;


//destination IP
IPAddress outIp(192, 168, 1, 2);

WiFiUDP Udp;

void setup() {
  // initialize the pushbutton pin as an input:
  Serial.begin(9600);  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  connectToWifi();
  pinMode(buttonPin, INPUT);

  Udp.begin(8888);
}



void loop() {

  sendSensorsOverUDP();

  sendSensorsOverOSC();
}

void sendSensorsOverUDP() {
  // read the state of the pushbutton value:
  int buttonState = digitalRead(buttonPin);
  int potentiometerValue = analogRead(potentiometerPin);

  Udp.beginPacket(outIp, 9999);
  Udp.write(buttonState);
  Udp.write(uint8_t(map(potentiometerValue, 0, 1024, 0, 255)));
  Udp.endPacket();
}

int lastButtonState = 0;

void sendSensorsOverOSC() {
  // read the state of the pushbutton and potentioemeter
  int buttonState = digitalRead(buttonPin);
  int potentiometerValue = analogRead(potentiometerPin);

  Udp.beginPacket(outIp, 5005);

  // if the button state has changed
  if (buttonState != lastButtonState) {
    // send an osc message with the new button state
    OSCMessage msg("/toggleD_1");
    msg.add((int32_t)buttonState);
  
    msg.send(Udp);
    msg.empty();
  }

  lastButtonState = buttonState;

  // convert the potentiometer value from 1-1028 to 0.0 to 0.1
  float faderValue = potentiometerValue / 1028.0;

  // send the potentiometer message as the fader value.
  OSCMessage faderMsg("/1/faderA");
  faderMsg.add(faderValue);
  faderMsg.send(Udp);
  faderMsg.empty();

  Udp.endPacket();

  delay(10);
}

void serialPrintSensorValues() {
  int buttonState = digitalRead(buttonPin);
  int potentiometerValue = analogRead(potentiometerPin);

  Serial.print(buttonState);
  Serial.print(" ");
  Serial.println(potentiometerValue);  
}


void connectToWifi() {
  Serial.println("Connecting to wifi...");
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to wifi");
  printWiFiStatus();
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
