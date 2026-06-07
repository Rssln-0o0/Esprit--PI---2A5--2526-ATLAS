/*
  Smart Port - Capteurs et actionneurs
  Carte cible: Arduino Uno/Nano (adaptable)

  Scenarios:
  - Gaz detecte -> ventilateur ON + buzzer ON + LED alerte clignote
  - Flamme detectee -> pompe ON + buzzer ON + LED alerte clignote
  - Humidite detectee -> buzzer ON + LED alerte clignote
  - Mouvement detecte -> actif seulement si mode securite ON
      -> buzzer ON + LED alerte ON (fixe)

  Eclairage:
  - Controle via Qt (commande serie LIGHT:1 / LIGHT:0 / LIGHT:TOGGLE)
  - Controle local via bouton tactile (toggle)

  Telemetrie temps reel vers Qt:
  - Envoi periodique d'une ligne "STATE;..."
*/

// ----------------------------- Pins capteurs -------------------------------
const uint8_t PIN_SENSOR_GAS      = 2;   // capteur gaz (DO, actif LOW)
const uint8_t PIN_SENSOR_HUMID    = 3;   // capteur humidite/pluie-sol (DO, actif LOW)
const uint8_t PIN_SENSOR_FLAME    = 4;   // capteur flamme (DO, actif LOW)
const uint8_t PIN_SENSOR_RAIN     = 5;   // capteur pluie (DO, actif LOW)
const uint8_t PIN_SENSOR_MOTION   = 6;   // PIR mouvement (DO, actif HIGH)
const uint8_t PIN_SENSOR_MAGNETIC = 13;  // reed switch (contact magnetique)
const uint8_t PIN_ULTRASON_TRIG   = A4;  // HC-SR04 TRIG
const uint8_t PIN_ULTRASON_ECHO   = A5;  // HC-SR04 ECHO

// ----------------------------- Pins actionneurs ----------------------------
const uint8_t PIN_ACT_FAN         = 8;   // ventilateur via transistor NPN
const uint8_t PIN_ACT_BUZZER      = 9;   // buzzer passif 3 fils: VCC->5V, GND, I/O->cette broche (tone)
const uint8_t PIN_ACT_WATER_PUMP  = 10;  // pompe a eau via transistor NPN
const uint8_t PIN_LED_LIGHT       = 12;  // LED eclairage (Qt ou bouton)
const uint8_t PIN_TOUCH_BUTTON    = 7;   // bouton tactile local (actif LOW)
const uint8_t PIN_LED_GAS         = A0;  // LED capteur gaz
const uint8_t PIN_LED_HUMID       = A1;  // LED capteur humidite
const uint8_t PIN_LED_FLAME       = A2;  // LED capteur flamme
const uint8_t PIN_LED_MOTION      = A3;  // LED commune mouvement + magnetique (si mode securite ON)

// ----------------------------- Parametres ----------------------------------
const unsigned long BLINK_INTERVAL_MS     = 250;
const unsigned long STATUS_INTERVAL_MS    = 300;
const unsigned long TOUCH_DEBOUNCE_MS     = 120;
const unsigned long TOUCH_STARTUP_MS       = 400;  // ignore faux appuis au boot (capteur tactil)
const unsigned long MOTION_WARMUP_MS      = 3000;
const unsigned long SERIAL_BAUD           = 115200;
const float WATER_LEVEL_TIDE_THRESHOLD_CM = 10.0f;
// Polarites capteurs (adapte si besoin selon ton montage reel).
const bool MOTION_ACTIVE_LOW              = true;  // ton capteur: LOW quand detecte
const bool MAGNETIC_ACTIVE_LOW            = false; // ton capteur: HIGH quand detecte
// Buzzer passif (VCC/GND/I/O): utiliser tone() sur la broche I/O. Laisser true.
// Buzzer actif 2 fils seulement SIG+GND: mettre BUZZER_USE_TONE false + regler BUZZER_ACTIVE_LOW.
const bool BUZZER_USE_TONE                = true;
const unsigned int BUZZER_TONE_HZ         = 1500;  // Hz (ajuste 800..2500 selon volume du module)
// Si BUZZER_USE_TONE false (buzzer actif): polarite de la broche signal.
const bool BUZZER_ACTIVE_LOW              = false;
// Si le type de module est inconnu, essaie HIGH/LOW en alternance.
const bool BUZZER_TRY_BOTH_POLARITIES     = false;
const unsigned long BUZZER_POLARITY_SWAP_MS = 180;

// ----------------------------- Etats globaux -------------------------------
bool securityMode = false;
bool lightingOn = false;

// Etat capteurs normalises (true = alarme/evenement detecte)
bool gasDetected = false;
bool humidDetected = false;
bool flameDetected = false;
bool rainDetected = false;
bool motionDetected = false;
bool magneticDetected = false;
float waterLevelCm = 0.0f;

// Etat actionneurs commandes
bool fanOn = false;
bool buzzerOn = false;
bool pumpOn = false;
bool blinkEnabled = false;
bool blinkPhase = false;

// Overrides manuels commandes depuis Qt (AUTO = scenario)
bool fanManualEnabled = false;
bool fanManualValue = false;
bool pumpManualEnabled = false;
bool pumpManualValue = false;
bool buzzerManualEnabled = false;
bool buzzerManualValue = false;
unsigned long lastBlinkTs = 0;
unsigned long lastStatusTs = 0;
unsigned long lastTouchTs = 0;
unsigned long touchArmedAtMs = 0;
unsigned long motionWarmupStartTs = 0;
unsigned long lastBuzzerSwapTs = 0;
bool buzzerPolarityPhase = false;
static bool buzzerTonePlaying = false;

String serialLine;

static bool readActiveLow(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

static bool readDigitalSensor(uint8_t pin, bool activeLow) {
  const int v = digitalRead(pin);
  return activeLow ? (v == LOW) : (v == HIGH);
}

float readUltrasonicCm() {
  digitalWrite(PIN_ULTRASON_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRASON_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASON_TRIG, LOW);
  const unsigned long duration = pulseIn(PIN_ULTRASON_ECHO, HIGH, 30000UL);
  if (duration == 0)
    return waterLevelCm;
  return (float)duration * 0.0343f * 0.5f;
}

void readSensors() {
  gasDetected = readActiveLow(PIN_SENSOR_GAS);
  humidDetected = readActiveLow(PIN_SENSOR_HUMID);
  flameDetected = readActiveLow(PIN_SENSOR_FLAME);
  rainDetected = readActiveLow(PIN_SENSOR_RAIN);
  const bool motionRaw = readDigitalSensor(PIN_SENSOR_MOTION, MOTION_ACTIVE_LOW);
  // Anti faux-positif PIR au demarrage: mouvement force a 0 pendant warmup.
  if ((millis() - motionWarmupStartTs) < MOTION_WARMUP_MS) {
    motionDetected = false;
  } else {
    motionDetected = motionRaw;
  }
  magneticDetected = readDigitalSensor(PIN_SENSOR_MAGNETIC, MAGNETIC_ACTIVE_LOW);
  waterLevelCm = readUltrasonicCm();
}

void applyScenarioLogic() {
  // RAZ sorties calculees
  fanOn = false;
  buzzerOn = false;
  pumpOn = false;
  blinkEnabled = false;
  bool safetyAlert = false;
  // Gaz
  if (gasDetected) {
    fanOn = true;
    buzzerOn = true;
    blinkEnabled = true;
    safetyAlert = true;
  }

  // Flamme
  if (flameDetected) {
    pumpOn = true;
    buzzerOn = true;
    blinkEnabled = true;
    safetyAlert = true;
  }

  // Humidite
  if (humidDetected) {
    buzzerOn = true;
    blinkEnabled = true;
    safetyAlert = true;
  }

  // Mouvement seulement si mode securite ON
  if (securityMode && motionDetected) {
    buzzerOn = true;
    safetyAlert = true;
  }
  // Capteur magnetique: meme logique que mouvement
  if (securityMode && magneticDetected) {
    buzzerOn = true;
    safetyAlert = true;
  }
  // Mouvement/magnetique detectes: LED commune doit clignoter.
  if (securityMode && (motionDetected || magneticDetected)) {
    blinkEnabled = true;
  }

  // Override manuel actionneurs depuis Qt.
  if (fanManualEnabled) {
    fanOn = fanManualValue;
  }
  if (pumpManualEnabled) {
    pumpOn = pumpManualValue;
  }
  // Buzzer:
  // - AUTO: sonne seulement en cas d'alerte capteur
  // - MANUAL OFF: coupe meme en alerte
  // - MANUAL ON: force le buzzer (test materiel / son continu)
  if (buzzerManualEnabled && !buzzerManualValue) {
    buzzerOn = false;
  } else if (buzzerManualEnabled && buzzerManualValue) {
    buzzerOn = true;
  } else {
    buzzerOn = safetyAlert;
  }
}

void applyOutputs() {
  // Clignotement commun pour LED capteurs (gaz/humidite/flamme)
  if (blinkEnabled) {
    const unsigned long now = millis();
    if ((now - lastBlinkTs) >= BLINK_INTERVAL_MS) {
      lastBlinkTs = now;
      blinkPhase = !blinkPhase;
    }
  } else {
    blinkPhase = false;
  }

  const bool gasLedOn = gasDetected && blinkPhase;
  const bool humidLedOn = humidDetected && blinkPhase;
  const bool flameLedOn = flameDetected && blinkPhase;
  // LED commune mouvement/magnetique: clignote en detection.
  const bool motionLedOn = securityMode && (motionDetected || magneticDetected) && blinkPhase;

  digitalWrite(PIN_ACT_FAN, fanOn ? HIGH : LOW);
  if (BUZZER_USE_TONE) {
    if (buzzerOn) {
      if (!buzzerTonePlaying) {
        tone(PIN_ACT_BUZZER, BUZZER_TONE_HZ);
        buzzerTonePlaying = true;
      }
    } else if (buzzerTonePlaying) {
      noTone(PIN_ACT_BUZZER);
      pinMode(PIN_ACT_BUZZER, OUTPUT);
      digitalWrite(PIN_ACT_BUZZER, LOW);
      buzzerTonePlaying = false;
    }
  } else {
    if (buzzerTonePlaying) {
      noTone(PIN_ACT_BUZZER);
      pinMode(PIN_ACT_BUZZER, OUTPUT);
      digitalWrite(PIN_ACT_BUZZER, LOW);
      buzzerTonePlaying = false;
    }
    uint8_t buzzerLevel = BUZZER_ACTIVE_LOW
        ? (buzzerOn ? LOW : HIGH)
        : (buzzerOn ? HIGH : LOW);

    if (BUZZER_TRY_BOTH_POLARITIES && buzzerOn) {
      const unsigned long now = millis();
      if ((now - lastBuzzerSwapTs) >= BUZZER_POLARITY_SWAP_MS) {
        lastBuzzerSwapTs = now;
        buzzerPolarityPhase = !buzzerPolarityPhase;
      }
      buzzerLevel = buzzerPolarityPhase ? HIGH : LOW;
    }
    digitalWrite(PIN_ACT_BUZZER, buzzerLevel);
  }
  digitalWrite(PIN_ACT_WATER_PUMP, pumpOn ? HIGH : LOW);
  digitalWrite(PIN_LED_LIGHT, lightingOn ? HIGH : LOW);
  digitalWrite(PIN_LED_GAS, gasLedOn ? HIGH : LOW);
  digitalWrite(PIN_LED_HUMID, humidLedOn ? HIGH : LOW);
  digitalWrite(PIN_LED_FLAME, flameLedOn ? HIGH : LOW);
  digitalWrite(PIN_LED_MOTION, motionLedOn ? HIGH : LOW);
}

void sendStateLine() {
  // Format simple a parser dans Qt
  // Exemple:
  // STATE;SEC=1;GAS=0;HUM=1;FLAME=0;RAIN=0;MOTION=1;MAG=0;FAN=1;BUZZER=1;PUMP=0;LED_GAS=0;LED_HUM=1;LED_FLAME=0;LED_MOTION=1;LIGHT=0
  const bool ledGas = gasDetected && blinkPhase;
  const bool ledHum = humidDetected && blinkPhase;
  const bool ledFlame = flameDetected && blinkPhase;
  const bool ledMotion = securityMode && (motionDetected || magneticDetected);

  Serial.print(F("STATE;SEC="));
  Serial.print(securityMode ? 1 : 0);
  Serial.print(F(";GAS="));
  Serial.print(gasDetected ? 1 : 0);
  Serial.print(F(";HUM="));
  Serial.print(humidDetected ? 1 : 0);
  Serial.print(F(";FLAME="));
  Serial.print(flameDetected ? 1 : 0);
  Serial.print(F(";RAIN="));
  Serial.print(rainDetected ? 1 : 0);
  Serial.print(F(";MOTION="));
  Serial.print(motionDetected ? 1 : 0);
  Serial.print(F(";MAG="));
  Serial.print(magneticDetected ? 1 : 0);
  Serial.print(F(";FAN="));
  Serial.print(fanOn ? 1 : 0);
  Serial.print(F(";FAN_MODE="));
  Serial.print(fanManualEnabled ? F("MANUAL") : F("AUTO"));
  Serial.print(F(";BUZZER="));
  Serial.print(buzzerOn ? 1 : 0);
  Serial.print(F(";BUZZER_MODE="));
  Serial.print(buzzerManualEnabled ? F("MANUAL") : F("AUTO"));
  Serial.print(F(";PUMP="));
  Serial.print(pumpOn ? 1 : 0);
  Serial.print(F(";PUMP_MODE="));
  Serial.print(pumpManualEnabled ? F("MANUAL") : F("AUTO"));
  Serial.print(F(";LED_GAS="));
  Serial.print(ledGas ? 1 : 0);
  Serial.print(F(";LED_HUM="));
  Serial.print(ledHum ? 1 : 0);
  Serial.print(F(";LED_FLAME="));
  Serial.print(ledFlame ? 1 : 0);
  Serial.print(F(";LED_MOTION="));
  Serial.print(ledMotion ? 1 : 0);
  Serial.print(F(";WATER_CM="));
  Serial.print(waterLevelCm, 1);
  Serial.print(F(";TIDE="));
  Serial.print((waterLevelCm > WATER_LEVEL_TIDE_THRESHOLD_CM) ? F("LOW") : F("HIGH"));
  Serial.print(F(";LIGHT="));
  Serial.println(lightingOn ? 1 : 0);
}

void handleCommand(const String &cmdRaw) {
  String cmd = cmdRaw;
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "PING") {
    Serial.println(F("PONG"));
    return;
  }
  if (cmd == "STATUS?") {
    sendStateLine();
    return;
  }

  if (cmd.startsWith("MODE:")) {
    const String v = cmd.substring(5);
    if (v == "1" || v == "ON") securityMode = true;
    if (v == "0" || v == "OFF") securityMode = false;
    Serial.print(F("ACK;MODE="));
    Serial.println(securityMode ? 1 : 0);
    return;
  }

  if (cmd.startsWith("LIGHT:")) {
    const String v = cmd.substring(6);
    if (v == "1" || v == "ON") lightingOn = true;
    else if (v == "0" || v == "OFF") lightingOn = false;
    else if (v == "TOGGLE") lightingOn = !lightingOn;
    Serial.print(F("ACK;LIGHT="));
    Serial.println(lightingOn ? 1 : 0);
    return;
  }

  if (cmd.startsWith("ACT:FAN:")) {
    const String v = cmd.substring(8);
    if (v == "AUTO") {
      fanManualEnabled = false;
    } else if (v == "ON" || v == "1") {
      fanManualEnabled = true;
      fanManualValue = true;
    } else if (v == "OFF" || v == "0") {
      fanManualEnabled = true;
      fanManualValue = false;
    }
    Serial.print(F("ACK;ACT:FAN:"));
    if (!fanManualEnabled) Serial.println(F("AUTO"));
    else Serial.println(fanManualValue ? F("ON") : F("OFF"));
    return;
  }

  if (cmd.startsWith("ACT:PUMP:")) {
    const String v = cmd.substring(9);
    if (v == "AUTO") {
      pumpManualEnabled = false;
    } else if (v == "ON" || v == "1") {
      pumpManualEnabled = true;
      pumpManualValue = true;
    } else if (v == "OFF" || v == "0") {
      pumpManualEnabled = true;
      pumpManualValue = false;
    }
    Serial.print(F("ACK;ACT:PUMP:"));
    if (!pumpManualEnabled) Serial.println(F("AUTO"));
    else Serial.println(pumpManualValue ? F("ON") : F("OFF"));
    return;
  }

  if (cmd.startsWith("ACT:BUZZER:")) {
    const String v = cmd.substring(11);
    if (v == "AUTO") {
      buzzerManualEnabled = false;
    } else if (v == "ON" || v == "1") {
      buzzerManualEnabled = true;
      buzzerManualValue = true;
    } else if (v == "OFF" || v == "0") {
      buzzerManualEnabled = true;
      buzzerManualValue = false;
    }
    Serial.print(F("ACK;ACT:BUZZER:"));
    if (!buzzerManualEnabled) Serial.println(F("AUTO"));
    else Serial.println(buzzerManualValue ? F("ON") : F("OFF"));
    return;
  }

  if (cmd == "ACT:ALL:AUTO") {
    fanManualEnabled = false;
    pumpManualEnabled = false;
    buzzerManualEnabled = false;
    Serial.println(F("ACK;ACT:ALL:AUTO"));
    return;
  }

  if (cmd == "RESET") {
    securityMode = false;
    lightingOn = false;
    fanManualEnabled = false;
    pumpManualEnabled = false;
    buzzerManualEnabled = false;
    Serial.println(F("ACK;RESET=1"));
    return;
  }

  Serial.print(F("ERR;UNKNOWN_CMD="));
  Serial.println(cmd);
}

void handleSerial() {
  while (Serial.available() > 0) {
    const char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialLine.length() > 0) {
        handleCommand(serialLine);
        serialLine = "";
      }
    } else {
      serialLine += c;
      if (serialLine.length() > 64) {
        serialLine = ""; // anti-debordement
      }
    }
  }
}

void handleTouchButton() {
  const unsigned long now = millis();
  static bool touchPrevReleased = true;

  const bool pressed = (digitalRead(PIN_TOUCH_BUTTON) == LOW);

  if (now < touchArmedAtMs) {
    touchPrevReleased = !pressed;
    return;
  }

  if (touchPrevReleased && pressed) {
    if ((now - lastTouchTs) < TOUCH_DEBOUNCE_MS) return;
    lightingOn = !lightingOn;
    lastTouchTs = now;
    Serial.print(F("EVENT;LIGHT_TOGGLE="));
    Serial.println(lightingOn ? 1 : 0);
  }
  touchPrevReleased = !pressed;
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  pinMode(PIN_SENSOR_GAS, INPUT_PULLUP);
  pinMode(PIN_SENSOR_HUMID, INPUT_PULLUP);
  pinMode(PIN_SENSOR_FLAME, INPUT_PULLUP);
  pinMode(PIN_SENSOR_RAIN, INPUT_PULLUP);
  pinMode(PIN_SENSOR_MOTION, INPUT);        // la plupart des PIR sortent HIGH/LOW franc
  pinMode(PIN_SENSOR_MAGNETIC, INPUT_PULLUP);
  pinMode(PIN_ULTRASON_TRIG, OUTPUT);
  pinMode(PIN_ULTRASON_ECHO, INPUT);

  pinMode(PIN_TOUCH_BUTTON, INPUT_PULLUP);  // touche capacitive ou bouton tact

  pinMode(PIN_ACT_FAN, OUTPUT);
  pinMode(PIN_ACT_BUZZER, OUTPUT);
  noTone(PIN_ACT_BUZZER);
  digitalWrite(PIN_ACT_BUZZER, LOW);
  pinMode(PIN_ACT_WATER_PUMP, OUTPUT);
  pinMode(PIN_LED_LIGHT, OUTPUT);
  digitalWrite(PIN_LED_LIGHT, LOW);
  pinMode(PIN_LED_GAS, OUTPUT);
  pinMode(PIN_LED_HUMID, OUTPUT);
  pinMode(PIN_LED_FLAME, OUTPUT);
  pinMode(PIN_LED_MOTION, OUTPUT);

  motionWarmupStartTs = millis();
  touchArmedAtMs = millis() + TOUCH_STARTUP_MS;
  applyOutputs();
  Serial.println(F("SMART_PORT_READY"));
}

void loop() {
  handleSerial();
  handleTouchButton();

  readSensors();
  applyScenarioLogic();
  applyOutputs();

  const unsigned long now = millis();
  if ((now - lastStatusTs) >= STATUS_INTERVAL_MS) {
    lastStatusTs = now;
    sendStateLine();
  }
}

