/*
 * =============================================================================
 * PORTE — 2 servos + RFID RC522 + tactile TTP223 + LCD I2C 20x4 (optionnel)
 * =============================================================================
 *
 * Scénario combiné (D7 = touch 1, D8 = touch 2 si ENABLE_SECOND_TOUCH) :
 *   • Entrée : carte (PC → PORTE_OPEN) ouvre ; touch 1 ferme.
 *   • Sortie : touch 2 ouvre ; re-passage carte ferme (sans délai 4 s).
 * Touch 1 ne ferme pas si la porte a été ouverte par touch 2 (fermeture par badge).
 * Les commandes série DOOR_RFID_OPENS / DOOR_TOUCH_OPENS ne changent plus le tactile ;
 * elles servent seulement au mode legacy « RFID local ouvre » (doorOnRfidRead + mode 1).
 *
 * LCD I2C 20x4 (module PCF8574) :
 *   Bibliothèque Arduino : « LiquidCrystal I2C » (Frank de Brabander) via Gestionnaire.
 *   Câblage Uno :
 *     LCD SDA → A4        LCD SCL → A5
 *     LCD VCC → 5 V       LCD GND → GND commun Arduino
 *   Adresse I2C souvent 0x27 ou 0x3F (réglage LCD_I2C_ADDR). Écran noir : tester0x3F.
 *   Pas de conflit avec RC522 (SPI sur D10–D13).
 *
 * LCD : écran d’accueil après chaque ouverture/fermeture locale, retrait du badge,
 *   code validé (entrée) ou erreur code (4 s). Qt peut renvoyer L0–L3 juste après.
 *
 * Affichage : Qt envoie après lecture UID (employé en base) :
 *   L0:texte\n L1:...\n  L2:...\n  L3:...\n  (20 caractères max par ligne, ASCII recommandé)
 *   LCDCLR\n → efface l’écran (sans texte) ; utiliser avant un jeu de L0–L3 pour un affichage net
 *   LCLEAR\n → écran d’accueil (idle)
 *
 * Événements porte (pour l’appli PC, Smart Port) :
 *   PORTE:OUVERTE;SRC=P  ouverture commandée par le PC (entrée badge)
 *   PORTE:OUVERTE;SRC=2  ouverture au tactile 2 (sortie : prochain badge ferme)
 *   PORTE:OUVERTE;SRC=R  ouverture locale par lecture RFID (legacy)
 *   PORTE:FERMEE;SRC=1  fermeture au tactile 1 (après entrée carte/PC)
 *   PORTE:FERMEE;SRC=P|R  autres fermetures
 *
 * -----------------------------------------------------------------------------
 * RC522 : SS D10, RST D9, SPI 11–13, 3,3 V + GND
 * Servos signal D5, D6 — alim 5 V externe + GND commun
 * Touch TTP223 signal D7, VCC 5 V, GND
 *
 * Deuxième tactile « sortie » (ouvrir) — optionnel mais requis pour le scénario touch2→carte :
 *   TTP223 ou bouton NO vers GND sur D8 (ENABLE_SECOND_TOUCH / TOUCH2_PIN).
 *
 * Clavier matriciel 4×3 (ENABLE_KEYPAD) — lib « Keypad » (Mark Stanley / Alexander Brevig) :
 *   Lignes → A0, A1, A2, A3   Colonnes → D2, D3, D4 (Uno : broches libres avec ce montage).
 *   Saisie : * démarre (8 chiffres affichés sur LCD), chiffres 0–9, # valide. * en cours efface et recommence.
 *   Ligne série après # : CODE:xxxxxxxx (8 chiffres). Stocker le même code en base (colonne code).
 *   Test Moniteur série (9600) : KEYPAD_DEBUG_SERIAL=1 → « TOUCHE:x » par frappe.
 * =============================================================================
 */

#include <string.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

/** 0 = sans clavier (pas besoin de la lib Keypad). */
#define ENABLE_KEYPAD 1
/** 1 = chaque frappe envoyée sur le port série (Moniteur série 9600) : ligne « TOUCHE:x ». Mettre 0 avec l’appli Qt pour moins de bruit. */
#define KEYPAD_DEBUG_SERIAL 1
#if ENABLE_KEYPAD
#include <Keypad.h>
#endif

/* 0 = compiler sans LCD si la lib n’est pas installée */
#define ENABLE_LCD_I2C 1
#define LCD_I2C_ADDR 0x27  /* essayer 0x3F si rien ne s’affiche */

#if ENABLE_LCD_I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 20, 4);
#endif

#define MODE_RFID_OPEN_TOUCH_CLOSE  1
#define MODE_TOUCH_OPEN_RFID_CLOSE  2
#define DOOR_DEFAULT_MODE MODE_RFID_OPEN_TOUCH_CLOSE

#define SS_PIN 10
#define RST_PIN 9
#define SERVO1_PIN 5
#define SERVO2_PIN 6
#define TOUCH_PIN 7
/** 1 = lire aussi une 2e broche (autre TTP223 ou bouton poussoir vers GND). Même logique que D7. */
#define ENABLE_SECOND_TOUCH 1
#define TOUCH2_PIN 8

#define TOUCH_ACTIVE_HIGH 0

#define ANGLE_CLOSED_1  0
#define ANGLE_OPEN_1    90
#define ANGLE_CLOSED_2  0
#define ANGLE_OPEN_2    90
#define SERVO2_MIRROR 0

#define TOUCH_DEBOUNCE_MS 320
#define RFID_COOLDOWN_MS  400
#define RFID_MIN_ABSENT_DURATION_MS 600
/** Utilisé seulement pour l’ouverture par carte (mode A), pas pour la fermeture mode B. */
#define MIN_MS_BETWEEN_RFID_SERVO 4000

#define SERIAL_LINE_MAX 72

MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo1;
Servo servo2;

static bool doorOpen = false;
/** Dernière ouverture : 'P' PC, 'R' RFID local, '2' tactile 2 — pour savoir si le badge peut fermer. */
static char doorOpenedBy = 0;
static unsigned long lastTouchMs = 0;
static unsigned long lastRfidMs = 0;
static bool touch1WasPressed = false;
#if ENABLE_SECOND_TOUCH
static bool touch2WasPressed = false;
#endif
static bool rfidWaitingCardRemoval = false;
static unsigned long rfidAbsentSinceMs = 0;
static unsigned long lastRfidServoMoveMs = 0;
static bool doorOnRfidRead = true;
static char serialCmdBuf[SERIAL_LINE_MAX];
static byte serialCmdLen = 0;
static int doorMode = DOOR_DEFAULT_MODE;

static void applyServos(bool open, char src);
static bool openDoor(char src);
static bool closeDoor(char src);

#if ENABLE_LCD_I2C
static void lcdHardwareClear()
{
    lcd.clear();
    delayMicroseconds(4500);
}

static void lcdPrintRowPadded(int row, const char *txt)
{
    char buf[21];
    int i;
    for (i = 0; i < 20 && txt[i] && txt[i] != '\r' && txt[i] != '\n'; ++i)
        buf[i] = txt[i];
    for (; i < 20; ++i)
        buf[i] = ' ';
    buf[20] = '\0';
    lcd.setCursor(0, row);
    lcd.print(buf);
}

static void lcdShowIdleScreen()
{
    lcdHardwareClear();
    /*20 caractères par ligne (padded par lcdPrintRowPadded). */
    lcdPrintRowPadded(0, "   SMART PORT    ");
    lcdPrintRowPadded(1, "====================");
    lcdPrintRowPadded(2, " RFID |  * code ");
    lcdPrintRowPadded(3, "T1 ferme  T2 sortie ");
}

static void lcdShowScanning(const char *uidHex)
{
    lcdHardwareClear();
    lcdPrintRowPadded(0, "  LECTURE BADGE ");
    lcdPrintRowPadded(1, "  ... en cours ... ");
    lcdPrintRowPadded(2, uidHex ? uidHex : "");
    lcdPrintRowPadded(3, " Retirez le badge  ");
}
#else
static void lcdPrintRowPadded(int, const char *) {}
static void lcdShowIdleScreen() {}
static void lcdShowScanning(const char *) {}
#endif

/** Supprime espaces / tab en début et fin (évite échec strcmp si le terminal ajoute des caractères). */
static void trimSerialCmdBuf(char *buf)
{
    if (!buf || !buf[0])
        return;
    char *start = buf;
    while (*start == ' ' || *start == '\t')
        ++start;
    if (start != buf)
        memmove(buf, start, strlen(start) + 1);
    size_t n = strlen(buf);
    while (n > 0 && (buf[n - 1] == ' ' || buf[n - 1] == '\t')) {
        buf[n - 1] = '\0';
        --n;
    }
}

static void pollSerialCommands()
{
    while (Serial.available() > 0) {
        const char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (serialCmdLen > 0) {
                serialCmdBuf[serialCmdLen] = '\0';
                trimSerialCmdBuf(serialCmdBuf);
                if (serialCmdBuf[0] == '\0') {
                    serialCmdLen = 0;
                    continue;
                }
                if (strcmp(serialCmdBuf, "RFID_CAPTURE") == 0)
                    doorOnRfidRead = false;
                else if (strcmp(serialCmdBuf, "RFID_DOOR") == 0)
                    doorOnRfidRead = true;
                else if (strcmp(serialCmdBuf, "DOOR_RFID_OPENS") == 0) {
                    doorMode = MODE_RFID_OPEN_TOUCH_CLOSE;
                    Serial.print(F("MODE:"));
                    Serial.println((int)doorMode);
                } else if (strcmp(serialCmdBuf, "DOOR_TOUCH_OPENS") == 0) {
                    doorMode = MODE_TOUCH_OPEN_RFID_CLOSE;
                    Serial.print(F("MODE:"));
                    Serial.println((int)doorMode);
                } else if (strcmp(serialCmdBuf, "PORTE_OPEN") == 0) {
                    (void)openDoor('P');
                } else if (strcmp(serialCmdBuf, "PORTE_CLOSE") == 0) {
                    /* Fermeture PC ou synchro : retour écran d’accueil si déjà fermée (ex. refus accès). */
                    const bool wasOpen = doorOpen;
                    (void)closeDoor('P');
#if ENABLE_LCD_I2C
                    if (!wasOpen)
                        lcdShowIdleScreen();
#endif
                } else if (strcmp(serialCmdBuf, "LCDCLR") == 0) {
#if ENABLE_LCD_I2C
                    lcdHardwareClear();
#endif
                } else if (strcmp(serialCmdBuf, "LCLEAR") == 0) {
                    lcdShowIdleScreen();
                } else if (serialCmdLen >= 4 && serialCmdBuf[0] == 'L' && serialCmdBuf[1] >= '0' && serialCmdBuf[1] <= '3' && serialCmdBuf[2] == ':') {
                    const int row = serialCmdBuf[1] - '0';
                    lcdPrintRowPadded(row, serialCmdBuf + 3);
                }
                serialCmdLen = 0;
            }
        } else if (serialCmdLen < sizeof(serialCmdBuf) - 1) {
            serialCmdBuf[serialCmdLen++] = (byte)c;
        } else {
            serialCmdLen = 0;
        }
    }
}

static bool touchPinPressed(int pin)
{
    int v = digitalRead(pin);
#if TOUCH_ACTIVE_HIGH
    return (v == HIGH);
#else
    return (v == LOW);
#endif
}

static void applyServos(bool open, char src)
{
    int a1 = open ? ANGLE_OPEN_1 : ANGLE_CLOSED_1;
    int a2 = open ? ANGLE_OPEN_2 : ANGLE_CLOSED_2;
#if SERVO2_MIRROR
    a2 = 180 - a2;
#endif
    servo1.write(a1);
    servo2.write(a2);
    doorOpen = open;
    if (open)
        doorOpenedBy = src;
    else
        doorOpenedBy = 0;
    Serial.print(F("PORTE:"));
    Serial.print(open ? F("OUVERTE;SRC=") : F("FERMEE;SRC="));
    Serial.println(src);
}

static bool openDoor(char src)
{
    if (!doorOpen) {
        applyServos(true, src);
#if ENABLE_LCD_I2C
        delayMicroseconds(4500);
        lcdShowIdleScreen();
#endif
        return true;
    }
    return false;
}

static bool closeDoor(char src)
{
    if (doorOpen) {
        applyServos(false, src);
#if ENABLE_LCD_I2C
        delayMicroseconds(4500);
        lcdShowIdleScreen();
#endif
        return true;
    }
    return false;
}

#if ENABLE_KEYPAD
static const byte KP_ROWS = 4;
static const byte KP_COLS = 3;
static char kpKeys[KP_ROWS][KP_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'},
};
static byte kpRowPins[KP_ROWS] = {A0, A1, A2, A3};
static byte kpColPins[KP_COLS] = {2, 3, 4};
static Keypad keypad = Keypad(makeKeymap(kpKeys), kpRowPins, kpColPins, KP_ROWS, KP_COLS);
#define KEYPAD_CODE_DIGITS 8
#define KEYPAD_CODE_BUF (KEYPAD_CODE_DIGITS + 1)
static char kpCodeBuf[KEYPAD_CODE_BUF];
static byte kpCodeLen = 0;
/** Faux tant que l’utilisateur n’a pas appuyé sur * (les chiffres sont ignorés). */
static bool kpEntryActive = false;
#if ENABLE_LCD_I2C
/** Après erreur saisie code : retour accueil automatique (ms). */
static unsigned long kpErrIdleDeadline = 0;
#endif

static void keypadBufferClear(void)
{
    kpCodeLen = 0;
    kpCodeBuf[0] = '\0';
}

static void keypadBufferPush(char d)
{
    if (kpCodeLen >= KEYPAD_CODE_DIGITS)
        return;
    kpCodeBuf[kpCodeLen++] = d;
    kpCodeBuf[kpCodeLen] = '\0';
}

#if ENABLE_LCD_I2C
static void keypadRefreshEntryLcd(void)
{
    if (!kpEntryActive)
        return;
    lcdHardwareClear();
    lcdPrintRowPadded(0, "   CODE A SAISIR ");
    lcdPrintRowPadded(1, "* = effacer      ");
    char line[21];
    byte i;
    for (i = 0; i < KEYPAD_CODE_DIGITS && i < 20; ++i)
        line[i] = (i < kpCodeLen) ? kpCodeBuf[i] : '_';
    line[i] = '\0';
    lcdPrintRowPadded(2, line);
    lcdPrintRowPadded(3, "# = valider (8 ch)");
}
#else
static void keypadRefreshEntryLcd(void) {}
#endif

static void keypadSubmitCode(void)
{
    if (kpCodeLen != KEYPAD_CODE_DIGITS)
        return;
    Serial.print(F("CODE:"));
    Serial.println(kpCodeBuf);
    /* Sortie (porte ouverte par tactile 2) : comme RFID, fermeture locale puis traitement PC. */
    if (doorOpen && doorOpenedBy == '2') {
        (void)closeDoor('R');
    } else {
#if ENABLE_LCD_I2C
        delayMicroseconds(4500);
        lcdShowIdleScreen();
#endif
    }
#if ENABLE_LCD_I2C
        kpErrIdleDeadline = 0;
#endif
    keypadBufferClear();
}

static void pollKeypad(void)
{
    char key = keypad.getKey();
    if (!key)
        return;
#if KEYPAD_DEBUG_SERIAL
    Serial.print(F("TOUCHE:"));
    Serial.println(key);
#endif
    if (key == '*') {
        keypadBufferClear();
        kpEntryActive = true;
#if ENABLE_LCD_I2C
        kpErrIdleDeadline = 0;
#endif
        keypadRefreshEntryLcd();
        return;
    }
    if (key == '#') {
        if (!kpEntryActive)
            return;
        if (kpCodeLen != KEYPAD_CODE_DIGITS) {
#if ENABLE_LCD_I2C
            lcdHardwareClear();
            lcdPrintRowPadded(0, " CODE INCOMPLET ");
            lcdPrintRowPadded(1, "  8 chiffres requis");
            lcdPrintRowPadded(2, "  * recommencer  ");
            lcdPrintRowPadded(3, " ou attente accueil");
            kpErrIdleDeadline = millis() + 4000UL;
#endif
            keypadBufferClear();
            kpEntryActive = false;
            return;
        }
        keypadSubmitCode();
        kpEntryActive = false;
        return;
    }
    if (key >= '0' && key <= '9') {
        if (!kpEntryActive)
            return;
        keypadBufferPush(key);
        keypadRefreshEntryLcd();
    }
}
#endif /* ENABLE_KEYPAD */

static void printUidLines(char *uidHexOut, size_t uidHexCap)
{
    if (uidHexOut && uidHexCap >= 17) {
        uidHexOut[0] = '\0';
        size_t p = 0;
        for (byte i = 0; i < rfid.uid.size && p + 2 < uidHexCap; i++) {
            byte b = rfid.uid.uidByte[i];
            static const char xd[] = "0123456789ABCDEF";
            uidHexOut[p++] = xd[b >> 4];
            uidHexOut[p++] = xd[b & 0x0F];
        }
        uidHexOut[p] = '\0';
    }

    Serial.println(F("Carte detectee !"));
    /* « HEX : » seulement : la ligne « UID: » suivante est la seule parsée par l’appli (évite double événement). */
    Serial.print(F("HEX : "));
    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10)
            Serial.print('0');
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
    Serial.print(F("UID:"));
    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10)
            Serial.print('0');
        Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();
    Serial.println(F("------------------------"));
}

void setup()
{
    Serial.begin(9600);
    while (!Serial) { }

#if ENABLE_LCD_I2C
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcdShowIdleScreen();
#endif

#if TOUCH_ACTIVE_HIGH
    pinMode(TOUCH_PIN, INPUT);
#if ENABLE_SECOND_TOUCH
    pinMode(TOUCH2_PIN, INPUT);
#endif
#else
    pinMode(TOUCH_PIN, INPUT_PULLUP);
#if ENABLE_SECOND_TOUCH
    pinMode(TOUCH2_PIN, INPUT_PULLUP);
#endif
#endif

    doorMode = DOOR_DEFAULT_MODE;
    Serial.println(F("=== Porte 2 servos + RFID + Touch + LCD ==="));
    Serial.print(F("DOOR_MODE="));
    Serial.println((int)doorMode);

    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);
    applyServos(false, 'P');

    SPI.begin();
    rfid.PCD_Init();
    delay(50);
    byte v = rfid.PCD_ReadRegister(rfid.VersionReg);
    Serial.print(F("RC522 version 0x"));
    Serial.println(v, HEX);
    if (v != 0x00 && v != 0xFF) {
        rfid.PCD_AntennaOn();
        Serial.println(F("RFID OK."));
    } else {
        Serial.println(F("RFID non detecte."));
    }

    Serial.print(F("TOUCH_ACTIVE_HIGH="));
    Serial.print(TOUCH_ACTIVE_HIGH);
    Serial.print(F("  D7="));
    Serial.print(digitalRead(TOUCH_PIN));
#if ENABLE_SECOND_TOUCH
    Serial.print(F("  D"));
    Serial.print(TOUCH2_PIN);
    Serial.print(F("="));
    Serial.print(digitalRead(TOUCH2_PIN));
#endif
    Serial.println();
    Serial.println(F("Pret."));
#if ENABLE_KEYPAD
    Serial.println(F("KEYPAD: * puis 8 chiffres puis # | * = effacer/recommencer"));
#endif
}

void loop()
{
    pollSerialCommands();

    unsigned long now = millis();

#if ENABLE_KEYPAD
    pollKeypad();
#endif
#if ENABLE_LCD_I2C && ENABLE_KEYPAD
    if (kpErrIdleDeadline != 0 && (long)(now - kpErrIdleDeadline) >= 0) {
        kpErrIdleDeadline = 0;
        if (!kpEntryActive)
            lcdShowIdleScreen();
    }
#endif

    const bool t1 = touchPinPressed(TOUCH_PIN);
#if ENABLE_SECOND_TOUCH
    const bool t2 = touchPinPressed(TOUCH2_PIN);
#endif
    if (t1 && !touch1WasPressed) {
        if (lastTouchMs == 0 || (now - lastTouchMs) > TOUCH_DEBOUNCE_MS) {
            lastTouchMs = now;
            /* Fermeture entrée : pas après ouverture tactile 2 (sortie = badge). */
            if (doorOpen && doorOpenedBy != '2')
                (void)closeDoor('1');
        }
    }
#if ENABLE_SECOND_TOUCH
    if (t2 && !touch2WasPressed) {
        if (lastTouchMs == 0 || (now - lastTouchMs) > TOUCH_DEBOUNCE_MS) {
            lastTouchMs = now;
            if (!doorOpen)
                (void)openDoor('2');
        }
    }
    touch2WasPressed = t2;
#endif
    touch1WasPressed = t1;

    if (rfidWaitingCardRemoval) {
        if (!rfid.PICC_IsNewCardPresent()) {
            if (rfidAbsentSinceMs == 0)
                rfidAbsentSinceMs = now;
            if ((now - rfidAbsentSinceMs) >= RFID_MIN_ABSENT_DURATION_MS) {
                rfidWaitingCardRemoval = false;
                rfidAbsentSinceMs = 0;
#if ENABLE_LCD_I2C
#  if ENABLE_KEYPAD
                if (!kpEntryActive)
#  endif
                    lcdShowIdleScreen();
#endif
            }
        } else {
            rfidAbsentSinceMs = 0;
        }
        return;
    }

    if ((now - lastRfidMs) < RFID_COOLDOWN_MS)
        return;
    if (!rfid.PICC_IsNewCardPresent())
        return;
    if (!rfid.PICC_ReadCardSerial())
        return;

    lastRfidMs = now;

    char uidHex[17];
    printUidLines(uidHex, sizeof(uidHex));
    lcdShowScanning(uidHex);

    const bool allowRfidOpen = (lastRfidServoMoveMs == 0)
        || ((now - lastRfidServoMoveMs) >= MIN_MS_BETWEEN_RFID_SERVO);

    bool moved = false;
    if (doorOpen && doorOpenedBy == '2') {
        /* Sortie : porte ouverte par tactile 2 → badge ferme. */
        moved = closeDoor('R');
    } else if (doorOnRfidRead && allowRfidOpen) {
        if (doorMode == MODE_RFID_OPEN_TOUCH_CLOSE)
            moved = openDoor('R');
    }
    if (moved)
        lastRfidServoMoveMs = millis();

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    rfidWaitingCardRemoval = true;
    rfidAbsentSinceMs = 0;
}
