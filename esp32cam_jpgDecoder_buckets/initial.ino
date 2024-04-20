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

// Define a struct to hold RGB count for each range
struct RGBBucket {
    int count = 0;
};

RGBBucket buckets[9];  // 9 buckets for each range of 30 units


void processImage(camera_fb_t *fb) {
    JpegDec.decodeArray(fb->buf, fb->len);
    int width = JpegDec.width;
    int height = JpegDec.height;

    RGB totalRGB = {0, 0, 0}; // Structure to store total sums of RGB values
    int sampledCount = 0;     // Counter for the number of pixels sampled


    // Reset bucket counts
    for (int i = 0; i < 9; i++) {
        buckets[i].count = 0;
    }

    int index = 0;
    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++, index++) {
            if ((y * width + x) % 10 == 0) {  // Check every 10th pixel

                uint16_t pixel = JpegDec.pImage[index];

                // Perform RGB conversion
                uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
                uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
                uint8_t b = (pixel & 0x1F) * 255 / 31;

                //int bucketIndex = max(max(r / 30, g / 30), b / 30); // Find the maximum range index
                int bucketIndex = (r / 30); // Find the maximum range index

                if (bucketIndex < 9) {
                    buckets[bucketIndex].count++;
                }

                //Serial.printf("RGB Values: R=%d, G=%d, B=%d\n", r, g, b);
                // Accumulate the RGB values if above 50
                if (r > 10 && g > 10 && b > 10) {
                  totalRGB.r += r;
                  totalRGB.g += g;
                  totalRGB.b += b;

                  // Increment the count of sampled pixels
                  sampledCount++;
                }
            }
        }
    }

    // Calculate the average RGB values
    if (sampledCount > 0) { // Check to prevent division by zero
        uint32_t avgR = totalRGB.r / sampledCount;
        uint32_t avgG = totalRGB.g / sampledCount;
        uint32_t avgB = totalRGB.b / sampledCount;

        Serial.printf("Average RGB of Sampled Pixels: R=%d, G=%d, B=%d\n", avgR, avgG, avgB);
    }

    // Print the bucket counts
    for (int i = 0; i < 9; i++) {
        Serial.printf("Bucket %d (Range %d-%d): %d\n", i, i*30, (i+1)*30-1, buckets[i].count);
    }
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
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  processImage(fb);
  esp_camera_fb_return(fb);
  delay(2000); // Delay for 5 seconds before capturing the next image
}
