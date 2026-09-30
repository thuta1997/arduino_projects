#include <ESPmDNS.h>
#include <WiFi.h>
#include <WebServer.h>

//hospot
const char* ssid = "TT-payload";
const char* password = "!@#$%^&*";

WebServer server(80);

// UART Pins
#define RXp2 16
#define TXp2 17



void handleCapture() {
  String countStr = "1";
  String delayStr = "0"; 
  
  // Dashboard Parameter 
  if (server.hasArg("count")) {
    countStr = server.arg("count");
  }
  if (server.hasArg("delay")) {
    delayStr = server.arg("delay"); 
  }

  Serial.println("[WIFI Command] -> RECEIVED: CAPTURE " + countStr + " IMAGES WITH " + delayStr + "s DELAY");
  
  // ESP-CAM သို့ 'C3,10' form command ပေးပို့ခြင်း
  Serial2.println("C" + countStr + "," + delayStr); 
  
  server.send(200, "text/plain", "Command Sent: CAPTURE " + countStr + " DELAY " + delayStr + "s");
}

void handleSend() {
  Serial.println("[WIFI Command] -> RECEIVED: SEND IMAGE");
  Serial2.println("S"); 
  server.send(200, "text/plain", "Command Sent: SEND IMAGE");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2);
  
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 Main Board IP Address: ");
  Serial.println(WiFi.localIP()); 

  server.on("/capture", handleCapture);
  server.on("/send", handleSend);
  
  server.begin();
  Serial.println("Web Server Started.");
  if (!MDNS.begin("esp32cam")) { 
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started. Access via http://esp32cam.local");
}

void loop() {
  server.handleClient();

  // ESP-CAM reply show on Serial Monitor
  if (Serial2.available()) {
    String response = Serial2.readStringUntil('\n');
    Serial.print("[ESP-CAM Reply] : ");
    Serial.println(response);
  }
}