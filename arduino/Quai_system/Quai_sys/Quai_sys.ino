/*
 * Quai_sys — deux capteurs :
 *   - pin 8 (sensor1) ENTRÉE → SENSOR:TRIG (PC : quai actif → Inactif puis PORTE_OPEN)
 *   - pin 6 (sensor2) SORTIE → SENSOR:LEAVE (PC : quai inactif → Actif puis PORTE_OPEN)
 * Entrée : n’envoie SENSOR:TRIG que si DB_ALLOW_OPEN. Sortie : SENSOR:LEAVE si politique série reçue (pour ouvrir même si aucun quai « libre »).
 */
#include <Servo.h>
#include <string.h>

const int sensorEnter = 8;  // entrée
const int sensorLeave = 6;  // sortie

const int servoPin = 9;

const int redLED = 5;   // port fermé
const int greenLED = 4; // port ouvert

#define GATE_HOLD_MS 4000

Servo gate;
bool serialDbPolicySeen = false;
bool dbAllowsOpen = false;
char serialBuf[32];
byte serialLen = 0;

bool gateHeldOpen = false;
unsigned long gateOpenSince = 0;
bool sensorEnterLatched = false;
bool sensorLeaveLatched = false;

void setGateOpen(bool open) {
  gate.write(open ? 90 : 0);
  digitalWrite(redLED, open ? LOW : HIGH);
  digitalWrite(greenLED, open ? HIGH : LOW);
}

void beginGateOpenCycle() {
  setGateOpen(true);
  gateHeldOpen = true;
  gateOpenSince = millis();
  Serial.println(F("GATE:OPEN"));
}

void endGateHold() {
  gateHeldOpen = false;
  setGateOpen(false);
}

void serviceGateHold() {
  if (!gateHeldOpen)
    return;
  const unsigned long now = millis();
  if ((unsigned long)(now - gateOpenSince) >= (unsigned long)GATE_HOLD_MS)
    endGateHold();
}

void handleSerialCommand(const char *cmd) {
  if (strcmp(cmd, "DB_ALLOW_OPEN") == 0) {
    serialDbPolicySeen = true;
    dbAllowsOpen = true;
    Serial.println(F("DB:ALLOW=1"));
  } else if (strcmp(cmd, "DB_BLOCK_OPEN") == 0) {
    serialDbPolicySeen = true;
    dbAllowsOpen = false;
    gateHeldOpen = false;
    setGateOpen(false);
    Serial.println(F("DB:ALLOW=0"));
  } else if (strcmp(cmd, "PORTE_OPEN") == 0) {
    beginGateOpenCycle();
    Serial.println(F("PORTE:OUVERTE;SRC=P"));
  } else if (strcmp(cmd, "PORTE_CLOSE") == 0) {
    gateHeldOpen = false;
    setGateOpen(false);
    Serial.println(F("PORTE:FERMEE;SRC=P"));
  }
}

void pollSerial() {
  while (Serial.available() > 0) {
    const char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialLen > 0) {
        serialBuf[serialLen] = '\0';
        handleSerialCommand(serialBuf);
        serialLen = 0;
      }
    } else if (serialLen < sizeof(serialBuf) - 1) {
      serialBuf[serialLen++] = c;
    } else {
      serialLen = 0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(sensorEnter, INPUT_PULLUP);
  pinMode(sensorLeave, INPUT_PULLUP);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  gate.attach(servoPin);

  gateHeldOpen = false;
  sensorEnterLatched = false;
  sensorLeaveLatched = false;
  setGateOpen(false);
  Serial.println(F("Quai_sys: pin8=ENTREE(TRIG) pin6=SORTIE(LEAVE)"));
}

void loop() {
  pollSerial();
  serviceGateHold();

  const int sIn = digitalRead(sensorEnter);
  const int sOut = digitalRead(sensorLeave);
  const bool enterTrig = (sIn == LOW);
  const bool leaveTrig = (sOut == LOW);
  const bool dbOkEnter = serialDbPolicySeen && dbAllowsOpen;

  if (!enterTrig)
    sensorEnterLatched = false;
  else if (enterTrig && !sensorEnterLatched && dbOkEnter && !gateHeldOpen) {
    Serial.println(F("SENSOR:TRIG"));
    sensorEnterLatched = true;
  }

  if (!leaveTrig)
    sensorLeaveLatched = false;
  else if (leaveTrig && !sensorLeaveLatched && serialDbPolicySeen && !gateHeldOpen) {
    Serial.println(F("SENSOR:LEAVE"));
    sensorLeaveLatched = true;
  }

  if (!gateHeldOpen)
    setGateOpen(false);

  delay(100);
}
