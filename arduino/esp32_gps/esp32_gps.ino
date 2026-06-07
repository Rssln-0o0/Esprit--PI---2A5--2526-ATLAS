/**
 * ESP32 — GPS (TinyGPS++) vers l’application Qt ATLAS via USB (Serial / COM).
 *
 * Trame vers le PC : ATLAS_GPS,latitude,longitude,cap_deg
 * Branchez l’ESP32 en USB avant d’ouvrir Qt ; l’appli ouvre le port COM (115200 baud).
 *
 * Broches GPS série (UART2) : RX=16, TX=17 (adapter à votre câblage NEO-6M/7M).
 */

#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <math.h>

TinyGPSPlus gps;
HardwareSerial GPSserial(2);

// Démo sans antenne : petite orbite au large de Tunis (1) — mettre 0 pour n’envoyer que le GPS réel (antenne branchée).
#ifndef USE_GPS_DEMO
#define USE_GPS_DEMO 0
#endif

static unsigned long lastSendMs = 0;
static const unsigned kSendIntervalMs = 500;

#if USE_GPS_DEMO
static double demoLat = 36.805;
static double demoLon = 10.175;
static float demoAngle = 0.f;
#endif

static void sendLine(double lat, double lon, double courseDeg) {
  char buf[96];
  snprintf(buf, sizeof(buf), "ATLAS_GPS,%.6f,%.6f,%.1f\n", lat, lon, courseDeg);
  Serial.print(buf);
}

void setup() {
  Serial.begin(115200);
  GPSserial.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("ESP32_GPS_ATLAS — USB 115200 baud, trames ATLAS_GPS");
}

void loop() {
  while (GPSserial.available()) {
    gps.encode(GPSserial.read());
  }

  const unsigned long now = millis();
  if (now - lastSendMs < kSendIntervalMs)
    return;
  lastSendMs = now;

#if USE_GPS_DEMO
  if (!gps.location.isValid() || gps.location.age() > 3000) {
    demoAngle += 0.08f;
    const double r = 0.012;
    const double lat = demoLat + r * cos(demoAngle);
    const double lon = demoLon + r * sin(demoAngle) / cos(demoLat * 0.01745329252);
    sendLine(lat, lon, fmod(demoAngle * 57.2957795f + 90.f, 360.0));
    return;
  }
#endif

  if (gps.location.isUpdated() || (gps.location.isValid() && gps.location.age() < 2000)) {
    const double lat = gps.location.lat();
    const double lon = gps.location.lng();
    double course = 0;
    if (gps.course.isValid())
      course = gps.course.deg();
    else if (gps.speed.isValid() && gps.speed.kmph() > 0.5)
      course = gps.course.deg();
    sendLine(lat, lon, course);
  }
}
