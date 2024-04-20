#include <JPEGDecoder.h>
#include "esp_camera.h"
#include <base64.h>
#include <vector>


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


// Data structure to hold RGB values
struct RGB {
    uint32_t r, g, b;
};

// Function to send average RGB values over serial
void sendAverageRGB(const RGB* averages, int count) {
  for (int i = 0; i < count; i++) {
    Serial.printf("Block %d: R=%d, G=%d, B=%d\n", i, averages[i].r, averages[i].g, averages[i].b);
    
    // Serial.print(averages[i].r);
    // Serial.print(" ");
    // Serial.print(averages[i].g);
    // Serial.print(" ");
    // Serial.print(averages[i].b);
    // Serial.print(" "); // Separate blocks by spaces
  

  }
  //Serial.println(); // Newline to end the set of readings
}


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
   
  //for the image
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; //320 x 240
  config.jpeg_quality = 10;
  config.fb_count = 1;
 
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    //Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop() {
  
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    //Serial.println("Camera capture failed");
    return;
  }

  
  // Decode the JPEG
  JpegDec.decodeArray(fb->buf, fb->len);
  int width = JpegDec.width;
  int height = JpegDec.height;


  unsigned long currentMicros2 = micros();
  Serial.println("before calculate the sums for each block");
  Serial.println(currentMicros2);
  Serial.println("-----------------------");
 
  // Prepare to calculate averages for 9 blocks
  int blocksPerRow = 3;
  int blocksPerCol = 3;
  int blockWidth = width / blocksPerRow;
  int blockHeight = height / blocksPerCol;
  RGB avgRGB[9] = {};
  RGB sumRGB[9] = {};
  uint32_t countRGB[9] = {};
  int pixelsPerBlock = blockWidth * blockHeight;


  int index = 0;
  for (uint16_t y = 0; y < height; y++) {
      int blockRow = (y / blockHeight) * blocksPerRow;
      
      for (uint16_t x = 0; x < width; x++, index++) {
          uint16_t pixel = JpegDec.pImage[index];
          int blockCol = x / blockWidth;
          int blockIndex = blockRow + blockCol;

          // Perform RGB conversion
          uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
          uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
          uint8_t b = (pixel & 0x1F) * 255 / 31;

          // Amplify higher values by squaring them
          // Check if RGB values are not zero
          if (r > 50 || g > 50 || b > 50) {
              // Amplify higher values by squaring them
              sumRGB[blockIndex].r += r * r;
              sumRGB[blockIndex].g += g * g;
              sumRGB[blockIndex].b += b * b;
              countRGB[blockIndex]++;  // Count how many times we add to the sum for each block
          }


          // Debug print before averaging to check raw values
          //  if ((x % 40 == 0) && (y % 40 == 0)) { // Print every 40th pixel to avoid too much data
          //      //Serial.printf("Pixel (%d,%d): R=%d, G=%d, B=%d\n", x, y, r, g, b);
          //      Serial.print(r);
          //      Serial.print(" ");
          //      Serial.print(g);
          //     Serial.print(" ");
          //      Serial.print(g);
          //      Serial.print(" ");
          //      Serial.println(); // Newline to end the set of readings
          // }
      }
  }

  // Calculate the weighted average for each block and apply the square root to scale back
  for (int i = 0; i < 9; i++) {
      double scale = 255.0 / sqrt(65025);
      avgRGB[i].r = sqrt(sumRGB[i].r / countRGB[i]);
      avgRGB[i].g = sqrt(sumRGB[i].g / countRGB[i]);
      avgRGB[i].b = sqrt(sumRGB[i].b / countRGB[i]);
  }


  sendAverageRGB(avgRGB, 9);
  

  // Encode image to base64 and transmit
  //String base64Image = base64::encode(fb->buf, fb->len);
  //Serial.println(base64Image); // Use println to ensure each image ends with a newline
  esp_camera_fb_return(fb);
  delay(2000); // Delay for 5 seconds before capturing the next image
}
