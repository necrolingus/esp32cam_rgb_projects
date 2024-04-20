#include "esp_camera.h"
#include <stdio.h>
#include <stdlib.h> // For dynamic memory allocation



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



void calculateAverageRGB(uint8_t *rgb_data, int width, int height, int x_start, int y_start, int block_width, int block_height) {
    long sum_r = 0, sum_g = 0, sum_b = 0;
    int count = 0;

    for (int y = y_start; y < y_start + block_height; y++) {
        for (int x = x_start; x < x_start + block_width; x++) {
            int idx = (y * width + x) * 3;

            int r = rgb_data[idx];
            int g = rgb_data[idx + 1];
            int b = rgb_data[idx + 2];

            if (r > 0 || g > 0 || b > 0) {
              sum_r += r;
              sum_g += g;
              sum_b += b;
              count++;
            }
        }
    }
    printf("Block Average RGB: (%ld, %ld, %ld)\n", sum_r / count, sum_g / count, sum_b / count);
}

void processImage(camera_fb_t *fb) {
    int width = fb->width;
    int height = fb->height;
    size_t rgb888_len = width * height * 3; // 3 bytes per pixel in RGB888
    uint8_t *rgb888 = (uint8_t *)malloc(rgb888_len);

    if (rgb888 == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    // Convert frame buffer to RGB888
    if (!fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, rgb888)) {
        printf("Failed to convert image to RGB888\n");
        free(rgb888);
        return;
    }

    int number_of_blocks = 3; //3 horizontal and 3 vertical
    int block_width = width / number_of_blocks;
    int block_height = height / number_of_blocks;

    for (int by = 0; by < number_of_blocks; by++) {
        for (int bx = 0; bx < number_of_blocks; bx++) {
            int x_start = bx * block_width;
            int y_start = by * block_height;
            calculateAverageRGB(rgb888, width, height, x_start, y_start, block_width, block_height);
        }
    }
    Serial.println("");
    free(rgb888);
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
  config.jpeg_quality = 12;
  config.fb_count = 1;
 
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop() {

  //get micros so we can time how long an iteration takes
  unsigned long currentMicros = micros();
  
  //Initialize the camera
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  //Do image stuff
  processImage(fb);

  //Flush fb
  esp_camera_fb_return(fb);

  //print how long it takes to do an iteration then sleep a while
  unsigned long currentMicros2 = micros();
  unsigned long totalTime = currentMicros2 - currentMicros;
  Serial.println("Total Time");
  Serial.println(totalTime);
  Serial.println("-----------------------");
  delay(2000); // Delay for xxxx seconds before capturing the next image

}
