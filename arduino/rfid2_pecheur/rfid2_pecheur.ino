#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 6
#define LED_R 2
#define LED_G 4
#define LED_B 7

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

String serialBuffer;
unsigned long statusUntilMs = 0;

void setRgb(bool rOn, bool gOn, bool bOn) {
  // LED RGB cathode commune: HIGH = allumé
  digitalWrite(LED_R, rOn ? HIGH : LOW);
  digitalWrite(LED_G, gOn ? HIGH : LOW);
  digitalWrite(LED_B, bOn ? HIGH : LOW);
}

void setStatusLed(const String &statutRaw) {
  String s = statutRaw;
  s.trim();
  s.toUpperCase();
  if (s == "ACTIF") {
    // Vert
    setRgb(false, true, false);
  } else if (s == "INACTIF") {
    // Rouge
    setRgb(true, false, false);
  } else {
    // Statut inconnu -> bleu léger (diagnostic visuel)
    setRgb(false, false, true);
  }
}

String fit16(const String &s) {
  String out = s;
  out.replace("\r", " ");
  out.replace("\n", " ");
  out.trim();
  if (out.length() > 16) out = out.substring(0, 16);
  while (out.length() < 16) out += ' ';
  return out;
}

void showIdleScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Projet RFID");
  lcd.setCursor(0, 1);
  lcd.print("Passe Carte...");
}

void showPecheurOnLcd(const String &nom, const String &prenom, const String &role) {
  String line1 = nom;
  if (prenom.length() > 0) {
    if (line1.length() > 0) line1 += " ";
    line1 += prenom;
  }
  String line2 = "Role:" + role;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(fit16(line1));
  lcd.setCursor(0, 1);
  lcd.print(fit16(line2));
  statusUntilMs = millis() + 5000UL;
}

void beepOk() {
  tone(BUZZER, 1100, 120);
}

void beepRead() {
  tone(BUZZER, 1000, 250);
}

void handleSerialCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  // Compatible avec la logique Qt déjà utilisée dans le projet:
  // - LCDCLR
  // - L0:texte / L1:texte  (L2/L3 ignorées sur écran 2x16)
  if (cmd.equalsIgnoreCase("LCDCLR")) {
    lcd.clear();
    statusUntilMs = millis() + 1500UL;
    return;
  }

  if (cmd.startsWith("L") && cmd.length() >= 4 && cmd.charAt(2) == ':') {
    int row = cmd.charAt(1) - '0';
    if (row >= 0 && row <= 1) {
      String txt = cmd.substring(3);
      lcd.setCursor(0, row);
      lcd.print(fit16(txt));
      statusUntilMs = millis() + 5000UL;
    }
    return;
  }

  // Commande directe statut LED:
  // STATUT:ACTIF / STATUT:INACTIF
  if (cmd.startsWith("STATUT:")) {
    String statut = cmd.substring(7);
    setStatusLed(statut);
    return;
  }

  // Commande dédiée pêcheur:
  // PECH:Nom|Prenom|Role|Statut
  // (Statut optionnel pour compatibilité)
  if (cmd.startsWith("PECH:")) {
    String payload = cmd.substring(5);
    int p1 = payload.indexOf('|');
    int p2 = (p1 >= 0) ? payload.indexOf('|', p1 + 1) : -1;
    int p3 = (p2 >= 0) ? payload.indexOf('|', p2 + 1) : -1;
    String nom = (p1 >= 0) ? payload.substring(0, p1) : payload;
    String prenom = (p1 >= 0 && p2 >= 0) ? payload.substring(p1 + 1, p2) : "";
    String role = (p2 >= 0 && p3 >= 0) ? payload.substring(p2 + 1, p3) : ((p2 >= 0) ? payload.substring(p2 + 1) : "");
    String statut = (p3 >= 0) ? payload.substring(p3 + 1) : "";
    showPecheurOnLcd(nom, prenom, role);
    if (statut.length() > 0) setStatusLed(statut);
    beepOk();
    return;
  }
}

void processSerialInput() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        handleSerialCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
      if (serialBuffer.length() > 80) serialBuffer.remove(0, serialBuffer.length() - 80);
    }
  }
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setRgb(false, false, false);

  lcd.init();
  lcd.backlight();
  showIdleScreen();
  
  Serial.println("RFID ready");
}

void loop() {
  processSerialInput();

  if (statusUntilMs != 0 && millis() > statusUntilMs) {
    statusUntilMs = 0;
    showIdleScreen();
  }

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  beepRead();

  // UID normalisé pour Qt: UID:XXXXXXXX (sans espaces)
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    byte b = rfid.uid.uidByte[i];
    if (b < 0x10) uid += "0";
    uid += String(b, HEX);
  }
  uid.toUpperCase();
  Serial.print("UID:");
  Serial.println(uid);

  // Affichage local provisoire
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Badge detecte");
  lcd.setCursor(0, 1);
  lcd.print(fit16(uid));
  statusUntilMs = millis() + 2500UL;

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}