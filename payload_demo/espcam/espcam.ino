#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <HTTPClient.h>

// 🔴 သင့် Laptop ကနေ လွှင့်ထားတဲ့ Hotspot နာမည်နဲ့ Password ကို ဒီမှာ ပြင်ထည့်ပါ 🔴
const char* ssid = "TT-payload";
const char* password = "!@#$%^&*";

// 🟢 Laptop ၏ IP Address သစ်ကို ထည့်သွင်းထားပါသည် 🟢
const char* serverName = "http://192.168.23.111:5000/upload"; 

// AI-Thinker Camera Pinout Configuration
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM     25
#define HREF_GPIO_NUM      23
#define PCLK_GPIO_NUM      22

void setup() {
  // Main Board နှင့် ဆက်သွယ်ရန် Serial (Pins 1 & 3) ကို ဖွင့်ခြင်း
  Serial.begin(115200);

  // WiFi (Laptop Hotspot) သို့ ချိတ်ဆက်ခြင်း
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Camera Configuration (ပုံထွက်အရည်အသွေး သတ်မှတ်ခြင်း)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 

  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA; // 800x600 Resolution 
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed");
    return;
  }

  // SD Card စတင်ခြင်း
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  Serial.println("ESP32-CAM Payload Ready. Connected to Hotspot!");
}

void loop() {
  // Main Board (ESP32) မှ Command လာမလာ စောင့်ဖတ်ခြင်း
  if (Serial.available()) {
    char cmd = Serial.read();
    
    // Command 'C' ဆိုလျှင် ဓာတ်ပုံရိုက်မည်
    if (cmd == 'C' || cmd == 'c') {
      captureAndSaveImage();
    } 
    // Command 'S' ဆိုလျှင် ဓာတ်ပုံကို Laptop သို့ ပို့မည်
    else if (cmd == 'S' || cmd == 's') {
      sendImageToLaptop();
    }
  }
}

// ---------------------------------------------------------
// ဓာတ်ပုံရိုက်ပြီး SD Card ထဲ သိမ်းမည့် လုပ်ငန်းစဉ်
// ---------------------------------------------------------
void captureAndSaveImage() {
  camera_fb_t * fb = esp_camera_fb_get();  
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  String path = "/payload_image.jpg";
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len);
    Serial.println("ACK: Image saved to SD Card.");
  }
  
  file.close();
  esp_camera_fb_return(fb); 
}

// ---------------------------------------------------------
// SD Card မှ ပုံကိုပြန်ဖတ်ပြီး Laptop သို့ POST ဖြင့် ပို့မည့် လုပ်ငန်းစဉ်
// ---------------------------------------------------------
void sendImageToLaptop() {
  if(WiFi.status() == WL_CONNECTED) {
    String path = "/payload_image.jpg";
    File file = SD_MMC.open(path.c_str(), FILE_READ);
    
    if(!file) {
      Serial.println("Error: No image found in SD Card!");
      return;
    }

    size_t size = file.size();
    uint8_t *buffer = (uint8_t *)malloc(size); 
    if (buffer == NULL) {
      Serial.println("Error: Not enough memory");
      file.close();
      return;
    }
    
    file.read(buffer, size);
    file.close();

    HTTPClient http;
    http.begin(serverName); 
    http.addHeader("Content-Type", "image/jpeg");

    Serial.println("Uploading image to Laptop...");
    int httpResponseCode = http.POST(buffer, size); 

    if(httpResponseCode > 0) {
      Serial.printf("ACK: Upload Success. Code: %d\n", httpResponseCode);
    } else {
      Serial.printf("Error: Upload Failed. Code: %d\n", httpResponseCode);
    }
    
    free(buffer); 
    http.end();
  } else {
    Serial.println("Error: WiFi Disconnected");
  }
}