int Hplus = A13;     // Button Pin h+
int Hminus = A12;    // Button Pin h-
int set = A11;       // Setup Button Pin
int Mplus = A10;     // Button Pin m+
int Mminus = A9;     // Button Pin m-

const int ledPinsSM[60][2] = {
    // Minuten/Sekunden LED's
    {A0, 22}, {A0, 23}, {A0, 24}, {A0, 25}, {A0, 26}, {A0, 27}, {A0, 28},
    {A0, 29}, {A1, 25}, {A1, 26}, {A1, 27}, {A1, 28}, {A2, 28}, {A3, 28},
    {A4, 28}, {A1, 29}, {A2, 29}, {A3, 29}, {A4, 29}, {A5, 29}, {A6, 29},
    {A7, 29}, {A5, 28}, {A6, 28}, {A6, 27}, {A6, 26}, {A6, 25}, {A5, 25},
    {A5, 26}, {A5, 27}, {A8, 29}, {A8, 28}, {A8, 27}, {A8, 26}, {A8, 25},
    {A8, 24}, {A8, 23}, {A8, 22}, {A7, 28}, {A7, 27}, {A7, 26}, {A7, 25},
    {A7, 24}, {A6, 24}, {A5, 24}, {A7, 22}, {A6, 22}, {A5, 22}, {A4, 22},
    {A3, 22}, {A2, 22}, {A1, 22}, {A7, 23}, {A6, 23}, {A5, 23}, {A4, 23},
    {A3, 23}, {A2, 23}, {A1, 23}, {A1, 24}
};

const int ledPinsH[12][2] = {
    // Stunden LED's
    {A2, 26}, {A3, 25}, {A2, 27}, {A3, 27}, {A3, 26}, {A4, 27}, {A4, 26},
    {A4, 25}, {A4, 24}, {A3, 24}, {A2, 24}, {A2, 25}
};

void setup() {
    Serial.begin(9600);

    pinMode(Hplus, INPUT_PULLUP);   // Setze den Buttonpin als Eingang
    pinMode(Hminus, INPUT_PULLUP);  // Setze den Buttonpin als Eingang
    pinMode(set, INPUT_PULLUP);     // Setze den Buttonpin als Eingang
    pinMode(Mplus, INPUT_PULLUP);   // Setze den Buttonpin als Eingang
    pinMode(Mminus, INPUT_PULLUP);  // Setze den Buttonpin als Eingang

    for (int i = 0; i < 60; ++i) {
        // Alle LED's einschalten
        pinMode(ledPinsSM[i][0], OUTPUT);  // Setze Groundpin als Ausgang
        pinMode(ledPinsSM[i][1], OUTPUT);  // Setze Spannungsversorgungpin als Ausgang
        digitalWrite(ledPinsSM[i][0], HIGH); // Ground ausschalten
        digitalWrite(ledPinsSM[i][1], LOW);  // Spannungsversorgung ausschalten
    }
}

void ledH(int ledH) {
    // Stunden LED einschalten mittels PWM
    digitalWrite(ledPinsH[ledH][0], LOW);   // Ground anschalten
    digitalWrite(ledPinsH[ledH][1], HIGH);  // Spannungsversorgung anschalten
    digitalWrite(ledPinsH[ledH][0], HIGH);  // Ground ausschalten
    digitalWrite(ledPinsH[ledH][1], LOW);   // Spannungsversorgung ausschalten
}

void ledSM(int ledSM) {
    // Minuten/Sekunden LED einschalten mittels PWM
    digitalWrite(ledPinsSM[ledSM][0], LOW);   // Ground anschalten
    digitalWrite(ledPinsSM[ledSM][1], HIGH);  // Spannungsversorgung anschalten
    digitalWrite(ledPinsSM[ledSM][0], HIGH);  // Ground ausschalten
    digitalWrite(ledPinsSM[ledSM][1], LOW);   // Spannungsversorgung ausschalten
}

void setLED(int h, int m) {
    // Aktuelle Stunden und Minuten eingeschaltet lassen
    for (int mi = 0; mi < 300; ++mi) {
        ledH(h);  // Stunden LED einschalten
        ledSM(m); // Minuten LED einschalten
        delay(1); // 1ms warten
    }
}

void loop() {
    for (int h = 0; h < 12; ++h) {
        ledH(h); // Stunden Schleife
        for (int m = 0; m < 60; ++m) {
            ledH(h);    // Minuten Schleife
            ledSM(m);   // Minuten LED einschalten
            for (int s = 0; s < 60; ++s) {
                int start = millis();        // Zeit merken
                delayMicroseconds(8);        // 8 Microsekunden warten
                for (int micro = 0; micro < 1005; ++micro) {
                    delayMicroseconds(925); // 925 Microsekunden warten
                    ledH(h);                // Stunden LED einschalten
                    ledSM(m);               // Minuten LED einschalten
                    ledSM(s);               // Sekunden LED einschalten

                    if (digitalRead(set) == 0) { // Prüfen, ob Set-Knopf gedrückt wird
                        s = 0;                 // Sekunden auf null setzen
                        delay(300);            // 300ms warten
                        for (int timer = 0; timer < 10000; ++timer) {
                            // 10 Sekunden Timer des Setups
                            if (digitalRead(Hplus) == 0) {
                                if (h == 11) h = 0; 
                                else h = h + 1;
                                setLED(h, m);
                                timer = 0;
                            }
                            else if (digitalRead(Hminus) == 0) {
                                if (h == 0) h = 11;
                                else h = h - 1;
                                setLED(h, m);
                                timer = 0;
                            }
                            else if (digitalRead(Mplus) == 0) {
                                if (m == 59) m = 0;
                                else m = m + 1;
                                setLED(h, m);
                                timer = 0;
                            }
                            else if (digitalRead(Mminus) == 0) {
                                if (m == 0) m = 59;
                                else m = m - 1;
                                setLED(h, m);
                                timer = 0;
                            }
                            else if (digitalRead(set) == 0) {
                                delay(300);
                                break;
                            }
                            else {
                                delay(1);
                                ledH(h);
                                ledSM(m);
                            }
                        }
                    }
                }
                int stop = millis();
                int div = stop - start;
                Serial.println(div);
            }
        }
    }
}
