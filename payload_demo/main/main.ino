#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "TT-payload";
const char* password = "!@#$%^&*";

WebServer server(80);

// UART2 Pin 
#define RXp2 16
#define TXp2 17

// Laptop "/capture" Function
void handleCapture() {
  Serial.println("[WIFI Command] -> RECEIVED: CAPTURE");
  Serial2.println("C"); // sending command to ESP-CAM 
  server.send(200, "text/plain", "Command Sent: CAPTURE");
}

// Laptop "/send" Function
void handleSend() {
  Serial.println("[WIFI Command] -> RECEIVED: SEND IMAGE");
  Serial2.println("S"); // sending command to ESP-CAM 
  server.send(200, "text/plain", "Command Sent: SEND IMAGE");
}

void setup() {
  Serial.begin(115200);
  // innitiate UART2
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

  // Server Route 
  server.on("/capture", handleCapture);
  server.on("/send", handleSend);
  
  server.begin();
  Serial.println("Web Server Started.");
}

void loop() {
  server.handleClient(); // reading Incoming HTTP Requests 

  
  if (Serial2.available()) {
    String response = Serial2.readStringUntil('\n');
    Serial.print("[ESP-CAM Reply] : ");
    Serial.println(response);
  }
}