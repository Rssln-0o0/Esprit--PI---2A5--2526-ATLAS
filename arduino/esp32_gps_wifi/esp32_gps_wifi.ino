/**
 * ESP32 — même logique GPS que esp32_gps.ino, envoi vers le PC en **Wi‑Fi TCP** (port 3333).
 * Connexion **STA** au réseau domestique (même SSID que le PC pour l’appli Qt).
 *
 * Dépendances : TinyGPS++ ; carte ESP32.
 * UART GPS : RX=16, TX=17 (adapter si besoin).
 */

#include <TinyGPS++.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <math.h>

TinyGPSPlus gps;
HardwareSerial GPSserial(2);

#ifndef USE_GPS_DEMO
#define USE_GPS_DEMO 0
#endif

static const char *kWifiSsid = "TT-ALHN-8E64-2.4";
static const char *kWifiPass = "azMQ9JysJe";

/** Optionnel : IP fixe pour que le PC pointe toujours la même adresse (désactiver si conflit DHCP). */
#ifndef ESP32_USE_STATIC_IP
#define ESP32_USE_STATIC_IP 1
#endif

#if ESP32_USE_STATIC_IP
static const IPAddress kLocalIp(192, 168, 1, 188);
static const IPAddress kGateway(192, 168, 1, 1);
static const IPAddress kSubnet(255, 255, 255, 0);
#endif

static WiFiServer tcpServer(3333);
static WiFiClient tcpClient;

static unsigned long lastSendMs = 0;
static const unsigned kSendIntervalMs = 500;

// LED RGB (etat Wi-Fi): rouge = deconnecte, violet = connecte.
// Choix par defaut: LED RGB cathode commune.
static const int kLedRPin = 25;
static const int kLedGPin = 26;
static const int kLedBPin = 27;
static const uint8_t kLedBrightness = 36; // 0..255 (36 ~= 14%): luminosite reduite.
static const int kLedPwmFreq = 5000;
static const int kLedPwmResolution = 8;
static const int kLedRChannel = 0;
static const int kLedGChannel = 1;
static const int kLedBChannel = 2;
static bool g_hasSatelliteFix = false;
static unsigned long g_ledBlinkMs = 0;
static bool g_ledBlinkOn = false;

#ifndef RGB_ACTIVE_LOW
#define RGB_ACTIVE_LOW 0
#endif

#if USE_GPS_DEMO
static double demoLat = 36.805;
static double demoLon = 10.175;
static float demoAngle = 0.f;
#endif

static inline uint8_t pwmFor(bool on)
{
#if RGB_ACTIVE_LOW
    return on ? uint8_t(255 - kLedBrightness) : uint8_t(255);
#else
    return on ? kLedBrightness : uint8_t(0);
#endif
}

static void setRgb(bool red, bool green, bool blue)
{
    ledcWrite(kLedRChannel, pwmFor(red));
    ledcWrite(kLedGChannel, pwmFor(green));
    ledcWrite(kLedBChannel, pwmFor(blue));
}

static bool hasGpsFixLikeModuleLed()
{
    // Critere principal: position valide et recente.
    if (gps.location.isValid() && gps.location.age() < 3000)
        return true;

    // Secours: certains modules indiquent le fix avant maj location exploitable.
    if (gps.satellites.isValid() && gps.satellites.value() >= 4 && gps.hdop.isValid() && gps.hdop.value() > 0
        && gps.hdop.value() < 500) {
        return true;
    }
    return false;
}

static void updateWifiLed()
{
    if (WiFi.status() != WL_CONNECTED) {
        // Rouge fixe si Wi-Fi non connecte.
        setRgb(true, false, false);
        return;
    }

    // Violet clignotant des qu'un fix GPS est detecte (independant du TCP client).
    if (g_hasSatelliteFix) {
        const unsigned long now = millis();
        if (now - g_ledBlinkMs >= 350UL) {
            g_ledBlinkMs = now;
            g_ledBlinkOn = !g_ledBlinkOn;
        }
        if (g_ledBlinkOn)
            setRgb(true, false, true); // violet
        else
            setRgb(false, false, false); // eteint entre les clignotements
        return;
    }

    // Wi-Fi connecte mais pas de fix satellite exploitable: vert fixe.
    setRgb(false, true, false);
}

static bool sendLine(WiFiClient &cl, double lat, double lon, double courseDeg)
{
    if (!cl || !cl.connected())
        return false;
    char buf[96];
    snprintf(buf, sizeof(buf), "ATLAS_GPS,%.6f,%.6f,%.1f\n", lat, lon, courseDeg);
    cl.print(buf);
    return true;
}

void setup()
{
    GPSserial.begin(9600, SERIAL_8N1, 16, 17);

    Serial.begin(115200);
    Serial.println();
    Serial.println("ESP32 ATLAS — Wi-Fi STA + TCP 3333 (ATLAS_GPS)");

    ledcSetup(kLedRChannel, kLedPwmFreq, kLedPwmResolution);
    ledcSetup(kLedGChannel, kLedPwmFreq, kLedPwmResolution);
    ledcSetup(kLedBChannel, kLedPwmFreq, kLedPwmResolution);
    ledcAttachPin(kLedRPin, kLedRChannel);
    ledcAttachPin(kLedGPin, kLedGChannel);
    ledcAttachPin(kLedBPin, kLedBChannel);
    updateWifiLed();

    WiFi.mode(WIFI_STA);
#if ESP32_USE_STATIC_IP
    if (!WiFi.config(kLocalIp, kGateway, kSubnet)) {
        Serial.println("WiFi.config() a echoue — desactivez ESP32_USE_STATIC_IP si besoin.");
    }
#endif
    WiFi.begin(kWifiSsid, kWifiPass);

    const unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 45000UL) {
        delay(400);
        Serial.print('.');
    }
    Serial.println();
    updateWifiLed();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP ESP32: ");
        Serial.println(WiFi.localIP());
        tcpServer.begin();
    } else {
        Serial.println("ECHEC connexion Wi-Fi — verifier SSID / mot de passe / portee.");
    }
}

void loop()
{
    g_hasSatelliteFix = hasGpsFixLikeModuleLed();
    updateWifiLed();

    if (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        return;
    }

    if (!tcpClient || !tcpClient.connected()) {
        tcpClient = tcpServer.available();
        if (tcpClient)
            Serial.println("Client TCP connecte");
    }

    while (GPSserial.available())
        gps.encode(GPSserial.read());

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
        sendLine(tcpClient, lat, lon, fmod(demoAngle * 57.2957795f + 90.f, 360.0));
        g_hasSatelliteFix = true;
        updateWifiLed();
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
        sendLine(tcpClient, lat, lon, course);
    }
    g_hasSatelliteFix = hasGpsFixLikeModuleLed();
    updateWifiLed();
}
