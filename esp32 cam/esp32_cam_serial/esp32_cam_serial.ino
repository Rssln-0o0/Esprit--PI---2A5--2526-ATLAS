/*
 * ESP32-CAM - Capture image and send over Serial (USB)
 * Pour ATLAS - Reconnaissance faciale par câble série.
 *
 * Protocole: PC envoie 'C' (0x43) -> ESP32 capture une image JPEG et envoie:
 *   - 4 octets: longueur du JPEG (little-endian, uint32_t)
 *   - N octets: données JPEG
 * LED flash: PC envoie 'L' (0x4C) + 1 octet (0-255) -> réglage luminosité LED (GPIO 4).
 *
 * Câblage: connecter l'ESP32-CAM au PC via USB-Série (CP2102/CH340).
 * Dans l'IDE Arduino: Carte "AI Thinker ESP32-CAM" ou "ESP32 Dev Module"
 *                     Port: COMx (Windows) ou /dev/ttyUSBx (Linux)
 *                     Vitesse: 921600 (doit correspondre au PC).
 */

#include "esp_camera.h"
#include "Arduino.h"

// Pins pour AI Thinker ESP32-CAM (OV2640)
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
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_LED_GPIO    4   // LED flash sur AI Thinker ESP32-CAM
/** Canal LEDC pour la LED (pas le 0 : la caméra utilise LEDC_CHANNEL_0 pour XCLK). */
#define FLASH_LEDC_CHANNEL 1
#define BAUD 921600
// Si la liaison échoue (ex. "Accès refusé" ou pas de réponse), essayez 115200 :
// #define BAUD 115200
// puis retéléversez et choisissez 115200 dans l'application Qt lors de la sélection du port.

void setup() {
  Serial.begin(BAUD);
  delay(500);

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
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Résolution réduite + qualité JPEG plus compressée = transfert plus rapide (temps de réponse réduit)
  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;   // 640x480 (au lieu de SVGA 800x600)
    config.jpeg_quality = 18;            // plus compressé = fichier plus petit
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 18;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // Pas de texte sur Serial : le PC lit un flux binaire (4 octets taille + JPEG).
    Serial.write((uint8_t)0);
    Serial.write((uint8_t)0);
    Serial.write((uint8_t)0);
    Serial.write((uint8_t)0);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
  }
  // LED flash (PWM) — API Arduino-ESP32 2.x (ledcSetup + ledcAttachPin + ledcWrite canal)
  ledcSetup(FLASH_LEDC_CHANNEL, 5000, 8);
  ledcAttachPin(FLASH_LED_GPIO, FLASH_LEDC_CHANNEL);
  ledcWrite(FLASH_LEDC_CHANNEL, 0);
}

void loop() {
  if (Serial.available()) {
    int c = Serial.read();
    if (c == 'C') {
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb) {
        uint32_t zero = 0;
        Serial.write((uint8_t*)&zero, 4);
        return;
      }
      uint32_t len = (uint32_t)fb->len;
      Serial.write((uint8_t*)&len, 4);
      Serial.write(fb->buf, fb->len);
      esp_camera_fb_return(fb);
      delay(5);   // court délai pour laisser le buffer série se vider
    } else if (c == 'L') {
      if (Serial.available()) {
        int v = Serial.read();
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        ledcWrite(FLASH_LEDC_CHANNEL, (uint8_t)v);
      }
    }
  }
  delay(1);   // réaction plus rapide à la commande suivante
}
