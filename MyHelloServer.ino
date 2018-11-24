#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <AutoConnect.h>

#define AVERAGE_CNT 3
#define LED_BUILTIN 2
// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

#define ANALOG_SENSOR_PIN 36 // not all pins are working for whatever reason, but seems 36 is

const char* ssid = "........";
const char* password = "........";

WebServer server(80);
AutoConnect Portal(server);
int SensorValue = 0; // holds the sensor value
int SensorValueAvg = 0; 
int SensorValueCounter = 0; // Used for counting the number of values read from the anaolog pin, after a ceratain number of values, the average is built and set to Sensor Value

const int led = 13;

void handleRoot() {
  String message = "Sensor Value = ";
  message += String(SensorValueAvg);
  server.send(200, "text/plain", message);
  
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void ReadAndCalc(void) {
int CurrentValue;
  CurrentValue = analogRead(ANALOG_SENSOR_PIN);
  
  Serial.print("Analog Value (");
  Serial.print(ANALOG_SENSOR_PIN);
  Serial.print(")= ");
  Serial.println(CurrentValue);

  SensorValueCounter++;
  SensorValue += CurrentValue;
  if (SensorValueCounter == AVERAGE_CNT) {
    SensorValueAvg = SensorValue / (AVERAGE_CNT + 1);
    SensorValue = CurrentValue;
    SensorValueCounter = 0;
  }
  Serial.print("SensorValueCounter=");
  Serial.print(SensorValueCounter);
  Serial.print(" SensorValue=");
  Serial.print(SensorValue);
  Serial.print(" SensorValueAvg=");
  Serial.println(SensorValueAvg);
  Serial.println("---------------------");

  ledcWrite(LEDC_CHANNEL_0, SensorValueAvg);
  
  delay(500);
}

void setup(void) {
  Serial.println("setup started");
  
  
  Serial.begin(115200);

    // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_BUILTIN, LEDC_CHANNEL_0);
  
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  //Serial.println("");

  // Wait for connection
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("");
  //Serial.print("Connected to ");
  //Serial.println(ssid);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  //server.begin();
  Portal.begin();
  Serial.println("HTTP server started");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(void) {
  //server.handleClient();
  ReadAndCalc();
  
  Portal.handleClient();
}
