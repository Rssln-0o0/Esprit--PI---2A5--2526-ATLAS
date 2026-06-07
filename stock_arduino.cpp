#include <Arduino.h>
#include <HX711_ADC.h>
#include <math.h>

// Wiring:
// HX711 DT  -> D4
// HX711 SCK -> D5
// Red LED   -> D6
// Green LED -> D7
// IMPORTANT: D1 is reserved by USB serial on the Uno, so do not use it for the LED.

static const byte HX711_DT_PIN = 4;
static const byte HX711_SCK_PIN = 5;
static const byte RED_LED_PIN = 12;
static const byte GREEN_LED_PIN = 7;

// Replace this with the calibration factor you obtain from stock_calibration.cpp.
// If the measured value is inverted, you can either keep the absolute value below
// or flip the sign of this factor.
static float calibrationFactor = -7050.0f;

static const float WEIGHT_PRESENT_THRESHOLD_KG = 0.02f;
static const float WEIGHT_CLEAR_THRESHOLD_KG = 0.01f;
static const unsigned long SENSOR_REFRESH_MS = 80;
static const unsigned long COMMAND_TIMEOUT_MS = 2500;
static const unsigned long SETUP_TIMEOUT_MS = 4000;

static HX711_ADC LoadCell(HX711_DT_PIN, HX711_SCK_PIN);

static bool sensorReady = false;
static bool loadDetected = false;
static bool tareInProgress = false;
static float latestWeightKg = 0.0f;
static unsigned long lastSensorRefreshMs = 0;
static char commandBuffer[48];
static byte commandLength = 0;

static void setLeds(bool weightPresent)
{
	digitalWrite(RED_LED_PIN, weightPresent ? LOW : HIGH);
	digitalWrite(GREEN_LED_PIN, weightPresent ? HIGH : LOW);
}

static void setNoWeightState()
{
	loadDetected = false;
	setLeds(false);
}

static void printWeightLine(float weightKg)
{
	Serial.print(F("WEIGHT: "));
	Serial.print(weightKg, 2);
	Serial.println(F(" kg"));
}

static void printStatusLine()
{
	Serial.print(F("STATUS: "));
	Serial.print(sensorReady ? F("READY") : F("NOT_READY"));
	Serial.print(F(", LOAD="));
	Serial.print(loadDetected ? F("YES") : F("NO"));
	Serial.print(F(", VALUE="));
	Serial.println(latestWeightKg, 2);
}

static void updateLedStateFromWeight(float weightKg)
{
	if (weightKg >= WEIGHT_PRESENT_THRESHOLD_KG)
		loadDetected = true;
	else if (weightKg < WEIGHT_CLEAR_THRESHOLD_KG)
		loadDetected = false;

	setLeds(loadDetected);
}

static void processTareStatus()
{
	if (!tareInProgress)
		return;

	if (LoadCell.getTareStatus()) {
		tareInProgress = false;
		latestWeightKg = 0.0f;
		sensorReady = true;
		setNoWeightState();
		Serial.println(F("OK: TARED"));
	}
}

static void serviceSensor()
{
	processTareStatus();

	if (LoadCell.update()) {
		float weightKg = LoadCell.getData();
		if (isnan(weightKg) || isinf(weightKg))
			return;

		latestWeightKg = weightKg;
		sensorReady = true;
		updateLedStateFromWeight(weightKg);
	}
	else if (!sensorReady) {
		setNoWeightState();
	}
}

static void handleCommand(const String &command)
{
	String cmd = command;
	cmd.trim();
	cmd.toUpperCase();

	if (cmd.isEmpty())
		return;

	if (cmd == "REQUEST_WEIGHT") {
		if (!sensorReady) {
			Serial.println(F("ERROR: HX711_NOT_READY"));
			return;
		}

		if (latestWeightKg < WEIGHT_PRESENT_THRESHOLD_KG) {
			Serial.println(F("NO_WEIGHT"));
			return;
		}

		printWeightLine(latestWeightKg);
		return;
	}

	if (cmd == "STATUS") {
		printStatusLine();
		return;
	}

	if (cmd == "RAW") {
		if (!sensorReady) {
			Serial.println(F("ERROR: HX711_NOT_READY"));
			return;
		}

		Serial.print(F("RAW: "));
		Serial.println(latestWeightKg, 2);
		return;
	}

	if (cmd == "TARE") {
		if (!sensorReady) {
			Serial.println(F("ERROR: HX711_NOT_READY"));
			return;
		}

		tareInProgress = true;
		LoadCell.tareNoDelay();
		Serial.println(F("OK: TARE_STARTED"));
		return;
	}

	if (cmd.startsWith("CAL:")) {
		const float newFactor = cmd.substring(4).toFloat();
		if (newFactor == 0.0f) {
			Serial.println(F("ERROR: INVALID_CAL_FACTOR"));
			return;
		}

		calibrationFactor = newFactor;
		LoadCell.setCalFactor(calibrationFactor);
		Serial.print(F("OK: CAL="));
		Serial.println(calibrationFactor, 4);
		return;
	}

	Serial.println(F("ERROR: UNKNOWN_COMMAND"));
}

static void processSerialInput()
{
	while (Serial.available() > 0) {
		const char incoming = static_cast<char>(Serial.read());

		if (incoming == '\r')
			continue;

		if (incoming == '\n') {
			commandBuffer[commandLength] = '\0';
			handleCommand(String(commandBuffer));
			commandLength = 0;
			continue;
		}

		if (commandLength < sizeof(commandBuffer) - 1)
			commandBuffer[commandLength++] = incoming;
	}
}

static void waitForFirstReading(unsigned long timeoutMs)
{
	const unsigned long start = millis();
	while (millis() - start < timeoutMs) {
		if (LoadCell.update()) {
			float weightKg = LoadCell.getData();
			if (isnan(weightKg) || isinf(weightKg))
				continue;
			
			latestWeightKg = weightKg;
			sensorReady = true;
			updateLedStateFromWeight(weightKg);
			return;
		}
	}
}

void setup()
{
	pinMode(RED_LED_PIN, OUTPUT);
	pinMode(GREEN_LED_PIN, OUTPUT);
	setNoWeightState();

	Serial.begin(9600);

	LoadCell.begin();
	LoadCell.start(2000, true);
	LoadCell.setCalFactor(calibrationFactor);

	if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
		sensorReady = false;
		setNoWeightState();
		Serial.println(F("ERROR: HX711_NOT_READY"));
		return;
	}

	waitForFirstReading(SETUP_TIMEOUT_MS);
	if (!sensorReady) {
		Serial.println(F("ERROR: HX711_NO_STABLE_READING"));
	}
	else {
		Serial.println(F("OK: HX711_READY"));
		Serial.print(F("OK: CAL="));
		Serial.println(calibrationFactor, 4);
	}
}

void loop()
{
	processSerialInput();

	const unsigned long now = millis();
	if (now - lastSensorRefreshMs >= SENSOR_REFRESH_MS) {
		lastSensorRefreshMs = now;
		serviceSensor();
	}
}