#include <LiquidCrystal.h>
const int rs = 2, en = 3, d11 = 11, d12 = 12, d13 = 13, d14 = 14;
LiquidCrystal lcd(rs, en, d11, d12, d13, d14);

int buzzer = 10; // Buzzer pin
int ledPin = 15; // LED pin
int k = -1; // Keypad value holder
int b_val1 = -1; // Button 1 status for DISARM state
int b_val2 = -1; // Button 2 status for DISARM state
int b_val3 = 0; // Alarm activation state for ARM

int n = 0; // Code input count
int state = 0; // System state (disarmed/armed/trigger)
char keySymbol; // KeyPad symbol pressed

unsigned long previousMillis = 0; //For parallel working
const unsigned long interval = 500; // LED blink interval
bool ledState = LOW;

const int button1 = 0; // Button 1 pin
const int button2 = 1; // Button 2 pin

int button1St = 0; //Button 1 state
int button2St = 0; //Button 2 state

const char correctCode[5] = {'2', '9', '1', '8'}; // Correct code
char enteredCode[5]; // Entered code

bool correct;
int incAttempts = 0; // Incorrect attempts count
char KeyPadChar[] = "123F456E789DA0BC"; // Keypad character mapping
int KeyPadInt[16] = {1, 2, 3, -1, 4, 5, 6, -1, 7, 8, 9, -1, -1, 0, -1, -1}; // Keypad integer mapping

void setup() {
    pinMode(16, INPUT_PULLUP); // Row/Column pin configuration
    pinMode(17, INPUT_PULLUP);
    pinMode(18, INPUT_PULLUP);
    pinMode(19, INPUT_PULLUP);
    pinMode(20, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(26, OUTPUT);

    pinMode(buzzer, OUTPUT);
    pinMode(ledPin, OUTPUT);

    pinMode(button1, INPUT);
    pinMode(button2, INPUT);

    lcd.begin(16, 2); // Initialize LCD (16 columns, 2 rows)
    Serial.begin(9600); // Begin serial communication
}

// Function declarations
void alarmbuzz(); // Activate alarm for DISARMED state
int RdKeys(); // Read keypad keys
int ReadKeypad(); // Confirm keypad reading
void insertingcode(); // Insert code in DISARMED state
void insertingcode_armed(); // Insert code in ARMED state
void disarmed_state(); // DISARMED state logic
void armed_state(); // ARMED state logic
void val_change(); // Reset button status
void arm_alarm(); // Activate alarm for ARMED state

void loop() {
    if (state == 0) {
        disarmed_state(); // Handle disarmed state
        Serial.println("You are here! State=0");   //Loop working Verification statements on Serial Monitor
    } else if (state == 1) {
        armed_state(); // Handle armed state
        Serial.println("You are here! State=1");
    } else if (state == 2) {
        Serial.println("You are here! State=2");
        insertingcode_armed(); // Handle code entry in armed state
    }
}

// Helper functions
int RdKeys() {
    int row, col;
    int KeyRow[4] = {20, 21, 22, 26};
    int KeyCol[4] = {16, 17, 18, 19};

    // Set all rows to high initially
    digitalWrite(KeyRow[0], 1);
    digitalWrite(KeyRow[1], 1);
    digitalWrite(KeyRow[2], 1);
    digitalWrite(KeyRow[3], 1);

    // Loop through rows and columns to detect key press
    for (row = 0; row < 4; row++) {
        digitalWrite(KeyRow[row], 0); // Set row to low to detect key press
        for (col = 0; col < 4; col++) {
            if (digitalRead(KeyCol[col]) == 0) {
                return row * 4 + col; // Return key value
            }
        }
        digitalWrite(KeyRow[row], 1); // Reset row to high
    }
    return -1; // No key pressed
}

int ReadKeypad() {
    int key;
    while (1) {
        key = RdKeys(); // Read keys
        delay(20); // Debounce delay
        if (key == RdKeys()) {
            return key; // Return stable key press
        }
    }
}

void insertingcode() {
    b_val1 = 0;
    b_val2 = 0;
    n = 0;

    while (n < 4) { // Loop until 4 digits are entered
        k = ReadKeypad(); 
        keySymbol = KeyPadChar[k];  

        if (keySymbol == 'A' || keySymbol == 'B' || keySymbol == 'C' || keySymbol == 'D' || keySymbol == 'E' || keySymbol == 'F') {
            k = -1; // Ignore invalid keys
        } else if (k != -1) {
            enteredCode[n] = keySymbol; // Store entered digit
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("Code:____");

            for (int i = 0; i <= n; i++) {
                lcd.setCursor(5 + i, 1); // Position for each key
                lcd.print("*"); // Display '*' for each entered key
            }
            delay(500);
            n++;
        }
    }

    lcd.setCursor(0, 0);
    lcd.print("Press A "); // Prompt to confirm entry
    Serial.println(enteredCode);
    int loop_ = 0; //Staying in loop

    while (loop_ < 1) {
        k = ReadKeypad();
        keySymbol = KeyPadChar[k];
        if (k != -1) {
            if (keySymbol == 'A') {  //if A is pressed
                lcd.clear();
                if (codeverif() == true) {  //Check code entered
                    lcd.setCursor(0, 0);
                    lcd.print("ARMED");
                    state = 1;           //go to Armed state
                    alarmbuzz();         //Sound the buzzer and LED for 30 seconds
                    Serial.println(state);
                } else {
                    if (incAttempts == 2) {  //if incorrect code entered 3 times (0,1,2)
                        alarmbuzz();
                        val_change();    //change (reset) button status value to allow printing "DISARM" in disarmed state
                        lcd.clear();
                        disarmed_state();
                    } else {
                        lcd.setCursor(0, 0);
                        lcd.print("Incorrect Code");
                        delay(5000);
                        incAttempts++;  //Increment Incorrect code attempt number
                        val_change();  //Go back to disarmed state
                        lcd.clear();
                        disarmed_state();
                    }
                }
            } else {
                val_change();
                lcd.clear();
                disarmed_state();
            }
            loop_++;
        }
    }
}


void insertingcode_armed() {
    n = 0;

    while (n < 4) { // Loop until 4 digits are entered
        k = ReadKeypad();
        keySymbol = KeyPadChar[k];
        arm_alarm(); // Activate alarm on 

        if (keySymbol == 'A' || keySymbol == 'B' || keySymbol == 'C' || keySymbol == 'D' || keySymbol == 'E' || keySymbol == 'F') {
            k = -1; // Ignore invalid keys and keep "_"
        } else if (k != -1) {
            enteredCode[n] = keySymbol; // Store entered digit
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("Code:____");

            for (int i = 0; i <= n; i++) {
                lcd.setCursor(5 + i, 1); // Position for each key
                lcd.print("*"); // Display '*' for each entered key for security
            }
            delay(500);
            n++;
        }
    }

    lcd.setCursor(0, 0);
    lcd.print("Press D "); // Prompt to confirm entry
    Serial.println(enteredCode);
    int loop_ = 0;

    while (loop_ < 1) {
        k = ReadKeypad();
        keySymbol = KeyPadChar[k];
        arm_alarm();
        if (k != -1) {
            if (keySymbol == 'D') {
                lcd.clear();
                lcd.setCursor(0, 0);
                if (codeverif() == true) {
                    state = 0; // Switch to disarmed state
                    val_change(); //Resetting button status for disarmed state
                    b_val3 = 0; //Reset status for armed state
                    digitalWrite(ledPin, LOW); 
//                    noTone(buzzer);
                } else {
                    if (incAttempts == 3) {
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Report"); // Trigger report after 3 failed attempts
                    } else {
                        lcd.setCursor(0, 0);
                        lcd.print("Incorrect Code");
                        incAttempts++; //Increment wrong attempts
                        val_change(); 
                    }
                }
            } else {
                val_change();
                lcd.clear();
                lcd.print("Wrong Key.Retry"); // Invalid key pressed
            }
            loop_++;
        }
    }
}

void val_change() {
    b_val1 = -1; // Reset button 1 status
    b_val2 = -1; // Reset button 2 status
}

void armed_state() {
    button1St = digitalRead(button1);
    button2St = digitalRead(button2);
    if (b_val3 == 0) {
        lcd.setCursor(0, 0);
        lcd.print("ARMED"); // Display "ARMED" when system is armed
    }
    if (button1St == HIGH || button2St == HIGH) { 
        b_val3 = 1; // Set alarm trigger
    }
    if (b_val3 == 1) {
        state = 2; // Switch to code insertion state
    }
}

void arm_alarm() {   //using ticks in system to continue alarm in background simultaneously in ARM state
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Update the time
        ledState = !ledState; // Toggle LED state
        digitalWrite(ledPin, ledState); // Apply new state to LED
        //        tone(buzzer, 1000); //Buzzer buzzing at given frequency 1kHz

    }
}

void disarmed_state() {
    button1St = digitalRead(button1);
    button2St = digitalRead(button2);
    if (button1St == HIGH) {
        b_val1 = 1; // Windows check triggered
    }
    if (button2St == HIGH) {
        b_val2 = 1; // Sensors check triggered
    }
    if (b_val1 == -1 && b_val2 == -1) {
        lcd.setCursor(0, 0);
        lcd.print("DISARMED"); // Display "DISARMED" when system is disarmed
    } else if (b_val1 == 1 && b_val2 == -1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Check Windows"); // Prompt window check
        k = ReadKeypad();
        if (k != -1) {
            insertingcode(); // Handle code insertion
        }
    } else if (b_val1 == -1 && b_val2 == 1) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Check Sensors"); // Prompt sensor check
        if (k != -1) {
            insertingcode();
        }
    } else if (b_val1 == 1 && b_val2 == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Check Windows");
        lcd.setCursor(0, 1);
        lcd.print("Check Sensors");
        k = ReadKeypad();
        if (k != -1) {
            insertingcode();
        }
    }
}

void alarmbuzz() {
    for (int d = 0; d < 5; d++) {
   //  Toggle LED, Turn ON Buzzer
        digitalWrite(ledPin, HIGH);
      //tone(buzzer,1000);
        delay(500);
        digitalWrite(ledPin, LOW);
        delay(500);
    }
    digitalWrite(ledPin, LOW);
    //noTone(buzzer);
    incAttempts = 0; // Reset incorrect attempts
}

bool codeverif() {
    for (int q = 0; q < 4; q++) {
        if ((enteredCode[q]) != (correctCode[q])) { //Match keys pressed to Code set
            return false; // Code mismatch
        }
    }
    return true; // Code match
}
