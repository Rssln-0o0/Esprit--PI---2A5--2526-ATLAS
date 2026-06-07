/*
 * Arduino Uno + MFRC522 — même logique que le sketch « qui marche » (SPI, VersionReg).
 * Moniteur série 9600 baud.
 *
 * Affichage carte :
 *   - ligne lisible : UID : AA BB CC DD
 *   - ligne compacte : UID:AABBCCDD (copier-coller formulaires / Qt si vous réintégrez le port série)
 *
 * Si while (!Serial) bloque : remplacez par delay(800); (certains clones CH340).
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

void setup()
{
    Serial.begin(9600);
    while (!Serial) { }

    Serial.println(F("===== TEST RFID RC522 ====="));

    SPI.begin();
    Serial.println(F("SPI initialise"));

    rfid.PCD_Init();
    delay(50);
    Serial.println(F("RFID initialise"));

    byte v = rfid.PCD_ReadRegister(rfid.VersionReg);
    Serial.print(F("Version du module : 0x"));
    Serial.println(v, HEX);

    if (v == 0x00 || v == 0xFF) {
        Serial.println(F("ERREUR : Module RFID NON detecte — verifie cablage (11,12,13,10,9, 3.3V, GND)."));
    } else {
        Serial.println(F("Module RFID detecte."));
        rfid.PCD_AntennaOn();
    }

    Serial.println(F("Approchez une carte RFID (pas de message tant qu'il n'y a pas de carte)."));
    Serial.println();
}

void loop()
{
    if (!rfid.PICC_IsNewCardPresent())
        return;
    if (!rfid.PICC_ReadCardSerial())
        return;

    Serial.println(F("Carte detectee !"));

    /* Format lisible (comme votre code) */
    Serial.print(F("UID : "));
    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10)
            Serial.print('0');
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(' ');
    }
    Serial.println();

    /* Une ligne hex continue pour copier-coller */
    Serial.print(F("UID:"));
    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10)
            Serial.print('0');
        Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();

    Serial.println(F("------------------------"));

    rfid.PICC_HaltA();
    delay(400);
}
