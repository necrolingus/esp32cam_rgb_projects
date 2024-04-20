#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include "esp_camera.h"
#include <base64.h>
#include <WiFi.h>
#include <HTTPClient.h>


//Wifi Config
const char* ssid = "xxx";
const char* password = "xxx";

// PIN Configuration (for AI-Thinker ESP32-CAM module)
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

//JPEG to Base64 conversion
String getImageBase64() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "";
  }
  
  String imageBase64;
  if(fb->format == PIXFORMAT_JPEG){
    imageBase64 = base64::encode(fb->buf, fb->len);
  }

  esp_camera_fb_return(fb);
  return imageBase64;
}


//Send image to webserver
void sendImage(String imageBase64) {
  if (imageBase64.length() > 0) {
    HTTPClient http;
    http.begin("http://192.168.2.105:35201/processimage");
    http.addHeader("Content-Type", "text/plain");
    int httpResponseCode = http.POST(imageBase64);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server response: ");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    Serial.printf("[HTTP]... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
  }
}

void doSomeLogging(String message) {
  unsigned long currentMicros = micros();
  Serial.println(message);
  Serial.println(currentMicros);
}


void printSignalStrength() {
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


//All our setup variables
void setup() {
  Serial.begin(115200);

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
    config.jpeg_quality = 12;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(5000);
  printSignalStrength();
}

//Lets do it
void loop() {
  
  doSomeLogging("Start Take Image");
  String base64Image = getImageBase64();

  doSomeLogging("Start Send Image");
  sendImage(base64Image);

  //just some logging
  doSomeLogging("Get Response");
  Serial.println("");
  Serial.println("");

  delay(5000); // Delay for demonstration purposes

}
