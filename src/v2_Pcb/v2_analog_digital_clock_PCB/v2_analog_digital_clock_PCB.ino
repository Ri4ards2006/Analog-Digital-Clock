/*
 * Project: 12x12 LED Matrix Time Display with DS1307 RTC and Button Controls
 * Author: Richard Zuikov
 * Date: December 8, 2025
 * Version: 1.0
 * Description: Arduino code for a 12x12 LED matrix that displays time (hours, minutes, seconds) using DS1307 RTC for accurate timekeeping.
 *              Buttons adjust hours/minutes manually (enter setup mode via "set" button). Uses multiplexing for matrix scanning.
 * Hardware Requirements:
 * - ATmega8535 Microcontroller
 * - DS1307 RTC Module (I2C-compatible)
 * - 12x12 LED Matrix (common cathode rows, common anode columns)
 * - 5 Mechanical Buttons (for H+, H-, Set, M+, M-)
 * Pins (Adjust based on your schematics):
 * - I2C (RTC): SDA=A4, SCL=A5 (default ATmega8535 I2C pins)
 * - Matrix Rows (Cathodes - LOW=activate): A0, A1, A2, A3, A4, A5, C0, C1, C2, C3, C4, C5 (ROW_PINS)
 * - Matrix Columns (Anodes - HIGH=activate): D0, D1, D2, D3, D4, D5, B0, B1, B2, B3, B4, B5 (COL_PINS)
 * - Buttons (INPUT_PULLUP):
 *   Hplus: A13 (Hour increment)
 *   Hminus: A12 (Hour decrement)
 *   setButton: A11 (Enter/exit setup mode)
 *   Mplus: A10 (Minute increment)
 *   Mminus: A9 (Minute decrement)
 */

#include <Wire.h>    // Für I2C-Kommunikation mit der DS1307
#include <RTClib.h>  // Bibliothek für RTC-Funktionen

// --- Matrix-Ansteuerung - Zeilen und Spalten --- //
// Definiere die Pins für die 12 Zeilen der LED-Matrix (Kathoden)
const int ROW_PINS[12] = {A0, A1, A2, A3, A4, A5, C0, C1, C2, C3, C4, C5}; 
// Definiere die Pins für die 12 Spalten der LED-Matrix (Anoden)
const int COL_PINS[12] = {D0, D1, D2, D3, D4, D5, B0, B1, B2, B3, B4, B5}; 

// --- Tasten --- //
const int Hplus = A13;     // Stunde inkrementieren (Taste +)
const int Hminus = A12;    // Stunde dekrementieren (Taste -)
const int setButton = A11; // Setup/Tastenmodus (Taste Set)
const int Mplus = A10;     // Minute inkrementieren (Taste +)
const int Mminus = A9;     // Minute dekrementieren (Taste -)

// --- RTC Initialisierung --- //
RTC_DS1307 rtc;            // Instanz der RTC-Bibliothek

// --- Anzeigebuffer für die Matrix (12x12) --- //
// buffer[row][col] = true → LED an; false → LED aus
bool matrixBuffer[12][12]; 

// --- Zustandsvariablen --- //
bool inSetupMode = false;  // Flag: ob der Setup-Modus aktiv ist
unsigned long setupStart = 0; // Zeitstempel für Setup-Modus-Timeout
const unsigned long setupTimeout = 5000; // Setup-Modus beenden nach 5 Sekunden Inaktivität (ms)

// --- Zeitvariablen (Aktuelle Uhrzeit) --- //
int currentHour = 0;       // Aktuelle Stunde (12h-Format, 0-11)
int currentMinute = 0;    // Aktuelle Minute (0-59)
int currentSecond = 0;    // Aktuelle Sekunde (0-59)

// --- Timing-Controlling --- //
unsigned long lastTimeUpdate = 0; // Letzter Zeitaktualisierungszeitpunkt (ms)
const unsigned long updateInterval = 1000; // Zeitupdate alle 1 Sekunde (ms)

// --- Hilfsvariablen für Multiplexing --- //
static int currentRow = 0; // Aktuell scannende Zeile
static unsigned long lastScan = 0; // Letzter Scan-Zeitpunkt (ms)
const unsigned long scanInterval = 500; // Jede Zeile scannt alle 500µs (Anpassung an Helligkeit möglich)

void setup() {
    // Serial-Komunikation initialisieren (für Debugging)
    Serial.begin(9600);
    // I2C-Bus starten (für RTC-Kommunikation)
    Wire.begin();
    // RTC initialisieren
    rtc.begin();

    // --- RTC-Zeit prüfen und ggf. setzen --- //
    if (!rtc.isrunning()) {
        Serial.println("RTC is not running! Setting default time (2025-12-08 00:00:00).");
        // RTC mit Standardzeit initialisieren (Datum und Zeit des Codes)
        rtc.adjust(DateTime(2025, 12, 8, 0, 0, 0));
    }

    // --- Matrix-Pins initialisieren --- //
    for (int i = 0; i < 12; i++) {
        pinMode(ROW_PINS[i], OUTPUT); // Zeilen als Ausgänge konfigurieren
        pinMode(COL_PINS[i], OUTPUT);  // Spalten als Ausgänge konfigurieren
        // LEDs standardmäßig ausschalten:
        digitalWrite(ROW_PINS[i], HIGH); // Zeile inaktiv (kein GND)
        digitalWrite(COL_PINS[i], LOW);  // Spalte inaktiv (keine Versorgung)
    }

    // --- Tasten-Pins initialisieren (INPUT_PULLUP) --- //
    pinMode(Hplus, INPUT_PULLUP);
    pinMode(Hminus, INPUT_PULLUP);
    pinMode(setButton, INPUT_PULLUP);
    pinMode(Mplus, INPUT_PULLUP);
    pinMode(Mminus, INPUT_PULLUP);

    // --- Anfangszeit laden und Buffer initialisieren --- //
    updateTimeVariables(); // Aktuelle Zeit von RTC abfragen
    updateMatrixBuffer();  // Buffer mit Zeitdaten füllen
}

void loop() {
    // --- Multiplexing (Matrix-Scannen) --- //
    // Nur scannen, wenn genug Zeit seit letztem Scan vergangen ist
    unsigned long currentMillis = millis();
    if (currentMillis - lastScan >= scanInterval) {
        lastScan = currentMillis;

        // 1. Vorherige Zeile deaktivieren (LEDs ausschalten)
        if (currentRow < 12) {
            digitalWrite(ROW_PINS[currentRow], HIGH); // Zeile inaktiv (GND abgeschaltet)
        }

        // 2. Nächste Zeile auswählen (Wrap-around)
        currentRow = (currentRow + 1) % 12;

        // 3. Spalten für aktuelle Zeile basierend auf Buffer einstellen
        for (int col = 0; col < 12; col++) {
            if (matrixBuffer[currentRow][col]) {
                digitalWrite(COL_PINS[col], HIGH); // Spalte aktiv (Versorgung an)
            } else {
                digitalWrite(COL_PINS[col], LOW);  // Spalte inaktiv (Versorgung aus)
            }
        }

        // 4. Aktuelle Zeile aktivieren (LEDs in dieser Zeile leuchten, wenn Spalten angeschaltet)
        digitalWrite(ROW_PINS[currentRow], LOW); // Zeile aktiv (GND angeschaltet)
    }

    // --- Setup-Modus und Tastenbehandlung --- //
    handleSetupMode(currentMillis);

    // --- Zeitaktualisierung (nur auf Nicht-Setup-Modus) --- //
    if (currentMillis - lastTimeUpdate >= updateInterval && !inSetupMode) {
        lastTimeUpdate = currentMillis;
        updateTimeVariables(); // Fresh Time von RTC abfragen
        updateMatrixBuffer();  // Buffer aktualisieren
    }

    // --- Kleinere Verzögerung, um CPU-Belastung zu reduzieren --- //
    delayMicroseconds(10);
}

// --- Zeitdaten von RTC abfragen und in Variablen speichern --- //
void updateTimeVariables() {
    DateTime now = rtc.now(); // Holen der aktuellen Uhrzeit von RTC
    currentHour = now.hour() % 12;   // Konvertieren zu 12h-Format (0-11)
    currentMinute = now.minute();
    currentSecond = now.second();
}

// --- Anzeigebuffer basierend auf Zeitdaten aktualisieren --- //
void updateMatrixBuffer() {
    // 1. Alle LEDs im Buffer ausschalten (zuvor geleuchtete löschen)
    for (int row = 0; row < 12; row++) {
        for (int col = 0; col < 12; col++) {
            matrixBuffer[row][col] = false;
        }
    }

    // 2. Stundanzeige (Zeilen 0-11, Spalte 0)
    // Jede Stunde (0-11) leuchtet eine Zeile in Spalte 0
    if (currentHour >= 0 && currentHour < 12) {
        matrixBuffer[currentHour][0] = true; // Zeile currentHour, Spalte 0 → LED an
    }

    // 3. Minutenanzeige (Spalten 1-11, Zeilen 0-4)
    // Spalte: currentMinute / 5 (0-11, 5 Minuten pro Spalte)
    // Zeile: currentMinute % 5 (0-4, exakte Minute im 5-Minuten-Block)
    int minuteColumn = currentMinute / 5;   // 0-11 (passend zu Spalten 1-11)
    int minuteRow = currentMinute % 5;     // 0-4 (passend zu Zeilen 0-4)
    if (minuteColumn >= 0 && minuteColumn < 12 && minuteRow >= 0) {
        // Spalte 1-11 verwenden (Spalte 0 reserviert für Stunden)
        matrixBuffer[minuteRow][minuteColumn + 1] = true; 
    }

    // 4. Sekundenanzeige (Spalte 11, Blink-Effekt)
    // Jede Sekunde wechselt die LED in Spalte 11, Zeile 0 zwischen an/aus
    matrixBuffer[0][11] = (currentSecond % 2 == 0) ? true : false; 
}

// --- Handling des Setup-Modus (Zeitmanipulation via Tasten) --- //
void handleSetupMode(unsigned long currentMillis) {
    // --- Setup-Taste prüfen (Einfach/Exit) --- //
    if (digitalRead(setButton) == LOW) { // Taste pressed (active low)
        if (!inSetupMode) {
            // Modus betreten: Flag setzen und Timer starten
            inSetupMode = true;
            setupStart = currentMillis;
            Serial.println("Entering setup mode...");
        }
    } else { // Taste not pressed
        if (inSetupMode) {
            // Modus beenden, wenn Timeout überschritten
            if (currentMillis - setupStart >= setupTimeout) {
                inSetupMode = false;
                // Geräumte Zeit in RTC speichern (Stunden/Minuten)
                DateTime now = rtc.now();
                rtc.adjust(DateTime(
                    now.year(), now.month(), now.day(),
                    currentHour, currentMinute, now.second() // Sekunden unverändert behalten
                ));
                Serial.print("Exiting setup mode. Saved time: ");
                Serial.print(currentHour); Serial.print(":");
                Serial.print(currentMinute); Serial.println();
            }
        }
    }

    // --- Nur Tastenbehandlung im Setup-Modus --- //
    if (inSetupMode) {
        // Debounce-Intervall (50ms) zur Vermeidung von Fälschtriggern
        const unsigned long debounceDelay = 50;

        // --- Hour+ Taste (Stunde inkrementieren) --- //
        static unsigned long lastHplusCheck = 0;
        if (currentMillis - lastHplusCheck >= debounceDelay) {
            if (digitalRead(Hplus) == LOW) { // Taste pressed
                lastHplusCheck = currentMillis;
                // Stunde inkrementieren (wrap-around bei 11→0)
                currentHour = (currentHour == 11) ? 0 : currentHour + 1;
                Serial.print("Hour adjusted to: "); Serial.println(currentHour);
                updateMatrixBuffer(); // Buffer aktualisieren
            }
        }

        // --- Hour- Taste (Stunde dekrementieren) --- //
        static unsigned long lastHminusCheck = 0;
        if (currentMillis - lastHminusCheck >= debounceDelay) {
            if (digitalRead(Hminus) == LOW) { // Taste pressed
                lastHminusCheck = currentMillis;
                // Stunde dekrementieren (wrap-around bei 0→11)
                currentHour = (currentHour == 0) ? 11 : currentHour - 1;
                Serial.print("Hour adjusted to: "); Serial.println(currentHour);
                updateMatrixBuffer(); // Buffer aktualisieren
            }
        }

        // --- Minute+ Taste (Minute inkrementieren) --- //
        static unsigned long lastMplusCheck = 0;
        if (currentMillis - lastMplusCheck >= debounceDelay) {
            if (digitalRead(Mplus) == LOW) { // Taste pressed
                lastMplusCheck = currentMillis;
                // Minute inkrementieren (wrap-around bei 59→0)
                currentMinute = (currentMinute == 59) ? 0 : currentMinute + 1;
                Serial.print("Minute adjusted to: "); Serial.println(currentMinute);
                updateMatrixBuffer(); // Buffer aktualisieren
            }
        }

        // --- Minute- Taste (Minute dekrementieren) --- //
        static unsigned long lastMminusCheck = 0;
        if (currentMillis - lastMminusCheck >= debounceDelay) {
            if (digitalRead(Mminus) == LOW) { // Taste pressed
                lastMminusCheck = currentMillis;
                // Minute dekrementieren (wrap-around bei 0→59)
                currentMinute = (currentMinute == 0) ? 59 : currentMinute - 1;
                Serial.print("Minute adjusted to: "); Serial.println(currentMinute);
                updateMatrixBuffer(); // Buffer aktualisieren
            }
        }

        // --- Timeout-Reset bei jeder Tastenaktivität --- //
        // Verhindert Abbruch des Setup-Modus während der Eingabe
        if (digitalRead(Hplus) == LOW || digitalRead(Hminus) == LOW ||
            digitalRead(Mplus) == LOW || digitalRead(Mminus) == LOW) {
            setupStart = currentMillis; // Timer zurücksetzen
        }
    }
}