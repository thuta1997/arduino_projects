#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <HTTPClient.h>

// hotspot
const char* ssid = "TT-payload";
const char* password = "!@#$%^&*";

// Laptop ၏ IP Address  
const char* serverName = "http://172.19.254.1:5000/upload"; 

int unsentImagesCount = 0; 

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
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

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
    config.frame_size = FRAMESIZE_SVGA; 
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    return;
  }

  if(!SD_MMC.begin()){
    return;
  }
  
  for(int i=1; i<=20; i++) { 
    String checkPath = "/payload_image_" + String(i) + ".jpg";
    if (SD_MMC.exists(checkPath.c_str())) {
      unsentImagesCount = i;
    }
  }
}

void loop() {
  if (Serial.available()) {
    String cmdStr = Serial.readStringUntil('\n'); 
    cmdStr.trim();
    
    if (cmdStr.length() > 0) {
      char cmd = cmdStr.charAt(0);
      if (cmd == 'C' || cmd == 'c') {
        int count = 1; 
        int delaySec = 0; 
        
        if (cmdStr.length() > 1) {
          String params = cmdStr.substring(1);
          int commaIdx = params.indexOf(','); 
          
          if (commaIdx != -1) {
            count = params.substring(0, commaIdx).toInt();
            delaySec = params.substring(commaIdx + 1).toInt();
          } else {
            count = params.toInt(); 
          }
          
          if (count <= 0) count = 1;
          if (delaySec < 0) delaySec = 0;
        }
        captureAndSaveImage(count, delaySec);
      } 
      else if (cmd == 'S' || cmd == 's') {
        sendImageToLaptop();
      }
    }
  }
}

void captureAndSaveImage(int count, int delaySec) {
  if (delaySec > 0) {
    for(int i = delaySec; i > 0; i--) {
      delay(1000); 
    }
  }

  for (int i = 0; i < count; i++) {
    camera_fb_t * fb = esp_camera_fb_get();  
    if (!fb) continue; 

    unsentImagesCount++;
    String path = "/payload_image_" + String(unsentImagesCount) + ".jpg";
    File file = SD_MMC.open(path.c_str(), FILE_WRITE);
    
    if (file) {
      file.write(fb->buf, fb->len);
    }
    file.close();
    esp_camera_fb_return(fb); 
    delay(500);
  }
}

void sendImageToLaptop() {
  if (unsentImagesCount == 0) return;

  if(WiFi.status() == WL_CONNECTED) {
    for (int i = 1; i <= unsentImagesCount; i++) {
      String path = "/payload_image_" + String(i) + ".jpg";
      File file = SD_MMC.open(path.c_str(), FILE_READ);
      
      if(!file) continue;

      size_t size = file.size();
      uint8_t *buffer = (uint8_t *)malloc(size); 
      if (buffer == NULL) {
        file.close();
        continue;
      }
      
      file.read(buffer, size);
      file.close();

      HTTPClient http;
      http.begin(serverName); 
      http.addHeader("Content-Type", "image/jpeg");
      int httpResponseCode = http.POST(buffer, size); 

      if(httpResponseCode == 200 || httpResponseCode == 201) {
        SD_MMC.remove(path.c_str());
      }
      free(buffer); 
      http.end();
    }
    unsentImagesCount = 0;
  }
}