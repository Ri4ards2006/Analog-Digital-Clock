/*
 * Project: LED Time Display with Button Controls
 * Author: Richard Zuikov
 * Date: December 8, 2025
 * Version: 1.0
 * Description: Arduino code to control an LED-based time display (hours, minutes, seconds) using physical button inputs. 
 *              Buttons adjust time manually, and the display updates accordingly. Pins are defined for buttons and LEDs.
 *              Note: Contains potential logical gaps (e.g., time progression, LED activation duration) – see inline comments.
 * Pins Used:
 *   Buttons (INPUT_PULLUP):
 *     - Hplus: A13 (Hour increment)
 *     - Hminus: A12 (Hour decrement)
 *     - set: A11 (Enter/exit adjustment mode)
 *     - Mplus: A10 (Minute increment)
 *     - Mminus: A9 (Minute decrement)
 *   LEDs (Ground/Power pairs):
 *     - ledPinsSM[60][2]: 60 entries for seconds/minutes (A0-A8, 22-29)
 *     - ledPinsH[12][2]: 12 entries for hours (A2-A8, 22-29)
 */

// === Button Pin Definitions ===
// Buttons are connected to analog pins (A9-A13) using INPUT_PULLUP mode.
// INPUT_PULLUP means the pin is normally HIGH (pulled to VCC via internal resistor) and goes LOW when pressed (button grounds it).
int Hplus = A13;     // Hour + button: Pressed to increment hours (active-low)
int Hminus = A12;    // Hour - button: Pressed to decrement hours (active-low)
int set = A11;       // Setup button: Pressed to enter adjustment mode; pressed again to exit (active-low)
int Mplus = A10;     // Minute + button: Pressed to increment minutes (active-low)
int Mminus = A9;     // Minute - button: Pressed to decrement minutes (active-low)

// === LED Pins Arrays ===
// LEDs are controlled via two pins per LED: a "ground pin" (connects cathode to GND) and a "power pin" (connects anode to VCC).
// To activate an LED: groundPin = LOW (connect to GND), powerPin = HIGH (supply VCC). LED turns off when pins reset to HIGH/LOW.

// Array for Seconds/Minutes LEDs (ledPinsSM): 60 entries (0-59) to map 0-59 seconds/minutes.
// Format: {groundPin, powerPin} (e.g., {A0, 22} means groundPin = A0, powerPin = 22)
const int ledPinsSM[60][2] = {
    // Entries 0-6 (A0 ground, power 22-28)
    {A0, 22}, {A0, 23}, {A0, 24}, {A0, 25}, {A0, 26}, {A0, 27}, {A0, 28},
    // Entries 7-11 (A0 ground, power 29; A1 ground, power 25-28)
    {A0, 29}, {A1, 25}, {A1, 26}, {A1, 27}, {A1, 28},
    // Entries 12-15 (A2-A4 ground, power 28)
    {A2, 28}, {A3, 28}, {A4, 28},
    // Entries 16-20 (A1-A5 ground, power 29)
    {A1, 29}, {A2, 29}, {A3, 29}, {A4, 29}, {A5, 29},
    // Entries 21-25 (A6-A5 ground, power 29-25)
    {A6, 29}, {A7, 29}, {A5, 28}, {A6, 28}, {A6, 27},
    // Entries 26-29 (A6 ground, power 26-24)
    {A6, 26}, {A6, 25}, {A5, 25}, {A5, 26},
    // Entries 30-33 (A5 ground, power 27; A8 ground, power 29-26)
    {A5, 27}, {A8, 29}, {A8, 28}, {A8, 27},
    // Entries 34-39 (A8 ground, power 26-22)
    {A8, 26}, {A8, 25}, {A8, 24}, {A8, 23}, {A8, 22},
    // Entries 40-43 (A7 ground, power 28-25)
    {A7, 28}, {A7, 27}, {A7, 26}, {A7, 25},
    // Entries 44-47 (A7 ground, power 24; A6 ground, power 24; A5 ground, power 24)
    {A7, 24}, {A6, 24}, {A5, 24},
    // Entries 48-52 (A7-A1 ground, power 22)
    {A7, 22}, {A6, 22}, {A5, 22}, {A4, 22}, {A3, 22},
    // Entries 53-57 (A2-A1 ground, power 22)
    {A2, 22}, {A1, 22},
    // Entries 58-60 (A7-A1 ground, power 23; A1 ground, power 24)
    {A7, 23}, {A6, 23}, {A5, 23}, {A4, 23}, {A3, 23}, {A2, 23}, {A1, 23}, {A1, 24}
};

// Array for Hours LEDs (ledPinsH): 12 entries (0-11) for 12-hour format (0-11 hours).
// Format: {groundPin, powerPin} (e.g., {A2, 26} means groundPin = A2, powerPin = 26)
const int ledPinsH[12][2] = {
    {A2, 26}, {A3, 25}, {A2, 27}, {A3, 27}, {A3, 26}, {A4, 27},
    {A4, 26}, {A4, 25}, {A4, 24}, {A3, 24}, {A2, 24}, {A2, 25}
};

// === Setup Function ===
// Runs once at startup to initialize hardware and pins.
void setup() {
    // Initialize serial communication (9600 baud) to print timing debug data (elapsed time per micro loop)
    Serial.begin(9600);  

    // Configure all button pins as INPUT_PULLUP (internal pull-up resistor enables active-low signaling)
    pinMode(Hplus, INPUT_PULLUP);   // Hplus button: INPUT with internal pull-up (normally HIGH, LOW when pressed)
    pinMode(Hminus, INPUT_PULLUP);  // Hminus button: Same as above
    pinMode(set, INPUT_PULLUP);     // set button: Same as above
    pinMode(Mplus, INPUT_PULLUP);   // Mplus button: Same as above
    pinMode(Mminus, INPUT_PULLUP);  // Mminus button: Same as above

    // Initialize all Seconds/Minutes LEDs (ledPinsSM) as digital outputs and turn them OFF initially
    // LEDs start off because: groundPin = HIGH (no GND connection), powerPin = LOW (no VCC supply)
    for (int i = 0; i < 60; ++i) {
        // Set ground pin of LED i as digital output
        pinMode(ledPinsSM[i][0], OUTPUT);  // First element of pair: groundPin
        // Set power pin of LED i as digital output
        pinMode(ledPinsSM[i][1], OUTPUT);  // Second element of pair: powerPin

        // Turn LED i OFF:
        digitalWrite(ledPinsSM[i][0], HIGH); // groundPin HIGH → disconnect cathode from GND
        digitalWrite(ledPinsSM[i][1], LOW);  // powerPin LOW → no VCC to anode (no current flow)
    }
}

// === LED Activation Functions ===
// These functions briefly "pulse" an LED by activating it then immediately deactivating it.
// Note: Current logic does NOT sustain LED light (LED turns off instantly). To keep LEDs on, add delays (see setLED()).

// Function to pulse a specific hour LED (indexed 0-11)
void ledH(int ledH) {
    // Activate hour LED ledH:
    digitalWrite(ledPinsH[ledH][0], LOW);   // groundPin LOW → connect cathode to GND
    digitalWrite(ledPinsH[ledH][1], HIGH);  // powerPin HIGH → supply VCC to anode (LED ON)

    // Deactivate hour LED ledH immediately:
    digitalWrite(ledPinsH[ledH][0], HIGH);  // groundPin HIGH → disconnect cathode (LED OFF)
    digitalWrite(ledPinsH[ledH][1], LOW);   // powerPin LOW → stop VCC supply
}

// Function to pulse a specific seconds/minutes LED (indexed 0-59)
void ledSM(int ledSM) {
    // Activate SM LED ledSM:
    digitalWrite(ledPinsSM[ledSM][0], LOW);   // groundPin LOW → connect cathode to GND
    digitalWrite(ledPinsSM[ledSM][1], HIGH);  // powerPin HIGH → supply VCC (LED ON)

    // Deactivate SM LED ledSM immediately:
    digitalWrite(ledPinsSM[ledSM][0], HIGH);  // groundPin HIGH → disconnect cathode (LED OFF)
    digitalWrite(ledPinsSM[ledSM][1], LOW);   // powerPin LOW → stop VCC supply
}

// === Function to Sustain LED Activation ===
// Attempts to keep hour/minute LEDs visible by rapidly pulsing them (may not work as intended).
// Total duration: 300 iterations × 1ms delay = 300ms
void setLED(int h, int m) {
    // Loop to "hold" LEDs (rapid pulses)
    for (int mi = 0; mi < 300; ++mi) {
        ledH(h);   // Pulse hour LED h
        ledSM(m);  // Pulse minute LED m
        delay(1);  // Short delay between pulses (1ms)
    }
    // Note: After 300ms, loop ends. LEDs turn off because ledH()/ledSM() deactivate immediately.
    // To sustain light, modify ledH()/ledSM() to keep pins HIGH/LOW longer (e.g., add delayMicroseconds()).
}

// === Main Loop ===
// Controls time progression and manual adjustment via buttons. Contains nested loops for hours, minutes, seconds.
void loop() {
    // Outer loop: Hours (0-11, 12-hour format). Loop variable h is modified by buttons (see timing notes below).
    for (int h = 0; h < 12; ++h) {
        ledH(h);  // Pulse current hour LED h (brief activation)

        // Middle loop: Minutes (0-59). Loop variable m is modified by buttons (see timing notes below).
        for (int m = 0; m < 60; ++m) {
            ledH(h);    // Re-pulse hour LED h (may be redundant, as ledH(h) was called in outer loop)
            ledSM(m);   // Pulse current minute LED m (brief activation)

            // Inner loop: Seconds (0-59). Loop variable s is modified by buttons (see timing notes below).
            for (int s = 0; s < 60; ++s) {
                // === Timing Measurement Start ===
                int start = millis();        // Record start time (ms) using millis() (system timer)
                delayMicroseconds(8);        // Short initial delay (8µs) – purpose unclear (possible calibration?)

                // Microsecond precision loop (1005 iterations). Attempts to synchronize LED updates?
                for (int micro = 0; micro < 1005; ++micro) {
                    delayMicroseconds(925); // Long delay (925µs) between operations (~1ms total per iteration)

                    // Pulse hour, minute, and second LEDs
                    ledH(h);                // Pulse hour LED h
                    ledSM(m);               // Pulse minute LED m
                    ledSM(s);               // Pulse second LED s

                    // === Adjustment Mode Check ===
                    // If 'set' button is pressed (LOW = active), enter manual adjustment mode
                    if (digitalRead(set) == 0) { 
                        s = 0;             // Reset seconds to 0 to avoid partial updates during adjustment
                        delay(300);         // Debounce delay (300ms) to prevent accidental triggers from button bounce

                        // Adjustment mode timer: Runs for ~10 seconds (10,000 iterations). Actual duration depends on delays.
                        int timer = 0;      // Timer counter to limit adjustment mode duration
                        while (timer < 10000) {  // Using while instead of for for clarity (original uses for)
                            // Check for button presses and update time
                            if (digitalRead(Hplus) == 0) {  // Hour + pressed
                                h = (h == 11) ? 0 : h + 1;  // Increment hour (wrap to 0 at 11)
                                setLED(h, m);              // Update display with new hour
                                timer = 0;                 // Reset timer to extend adjustment mode
                            }
                            else if (digitalRead(Hminus) == 0) {  // Hour - pressed
                                h = (h == 0) ? 11 : h - 1;  // Decrement hour (wrap to 11 at 0)
                                setLED(h, m);              // Update display
                                timer = 0;                 // Reset timer
                            }
                            else if (digitalRead(Mplus) == 0) {  // Minute + pressed
                                m = (m == 59) ? 0 : m + 1;  // Increment minute (wrap to 0 at 59)
                                setLED(h, m);              // Update display
                                timer = 0;                 // Reset timer
                            }
                            else if (digitalRead(Mminus) == 0) {  // Minute - pressed
                                m = (m == 0) ? 59 : m - 1;  // Decrement minute (wrap to 59 at 0)
                                setLED(h, m);              // Update display
                                timer = 0;                 // Reset timer
                            }
                            // Check if 'set' button is released to exit adjustment mode
                            else if (digitalRead(set) == 0) {  // Note: This is redundant (set is already LOW). Likely intended to check for *release*?
                                // Wait for button to be released (debounce)
                                delay(300);    
                                break;         // Exit adjustment timer loop
                            }
                            // No buttons pressed: Maintain current time display
                            else {
                                delay(1);      // Short delay (1ms) to reduce CPU load
                                ledH(h);       // Re-pulse hour LED h (to potentially sustain visibility, though ledH() deactivates immediately)
                                ledSM(m);      // Re-pulse minute LED m
                            }

                            // Increment timer (original code uses for loop: ++timer)
                            timer++;  
                        }  // End of adjustment mode timer loop

                        break;  // Exit microsecond loop to proceed to time measurement
                    }  // End of 'set' button check

                    // === Timing Measurement End ===
                }  // End of microsecond loop

                // Calculate and print elapsed time for microsecond loop (debugging)
                int stop = millis();         // Record end time (ms)
                int div = stop - start;     // Elapsed time = end - start (ms)
                Serial.println(div);        // Print to serial monitor (visible in Arduino IDE Serial Monitor)

            }  // End of seconds loop (s)
        }  // End of minutes loop (m)
    }  // End of hours loop (h)

    // === Critical Note: Time Progression Logic ===
    // The nested h/m/s loops use loop variables (h, m, s) that are modified by button inputs. 
    // This causes unexpected behavior:
    // - When h is incremented/decremented, the outer for-loop’s h counter will still increment (++h), leading to skipped/repeated hours.
    // - Same issue for m and s loop variables. Example: If m is manually set to 10 during the loop, the m++ will make it 11, skipping m=10.
    // To fix: Use separate variables for displayed time (e.g., currentH, currentM, currentS) and only update loop counters when not adjusting.
}