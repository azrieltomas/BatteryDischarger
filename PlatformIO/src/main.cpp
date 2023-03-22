#include <Arduino.h>

// SET THESE TO DETERMINE CUTOFF POINT AND OSCILLATION POINT
const float VOLT_CUTOFF = 2.2; // voltage cutoff point
const float VOLT_TOGGLE_POINT = 0.1; // point when switching occurs again

// IO constants
const uint8_t LED_PIN = 0x20; // portB, DIO pin 13, internal
const uint8_t RELAY_PIN = 0x04; // portD, DIO pin 2
const uint8_t VOLTREAD_PIN = A0;

// regular constants
const uint16_t REFRESH_RATE = 1000; // milliseconds
const uint16_t BAUDRATE = 9600; // for serial connection
const uint16_t MAX_ANALOG = 1023; // 2^10 precision
const uint8_t MAX_VOLT = 5; // system voltage

// variables
uint32_t timerReadWrite = 0; // timer for reading data
uint16_t countOscillation = 0; // counts number of on/off oscillations
float lastVoltage = 0; // last recorded voltage
bool hasDroppedLow = false; // check if volt has already dropped below VOLT_CUTOFF

void setup() {
    // Serial setup
    Serial.begin(BAUDRATE);
    Serial.println("Initialised...");

    // IO setup
    DDRB = LED_PIN; // output
    DDRD = RELAY_PIN; // output
    pinMode(VOLTREAD_PIN, INPUT);

    // LED low
    PORTB = PORTB & ~LED_PIN;
    // relay low
    PORTD = PORTD & ~RELAY_PIN;

    // headers
    Serial.println("time,voltage,hasdropped");
}

void loop() {
    
    if (timerReadWrite == 0) {
        timerReadWrite = millis();
    }
    if ((timerReadWrite > 0) && ((millis() - timerReadWrite) > REFRESH_RATE)) {
        uint16_t voltValueRaw;
        float voltValueMap;

        voltValueRaw = analogRead(VOLTREAD_PIN);
        voltValueMap = (float) voltValueRaw / MAX_ANALOG * MAX_VOLT; // call float first

        Serial.println(String(millis()) + "," + String(voltValueMap, 3) + "," + String(countOscillation));
        
        // system has dropped below volt cutoff
        if (((voltValueMap <= VOLT_CUTOFF) && (lastVoltage > VOLT_CUTOFF))) {
            hasDroppedLow = true;
            countOscillation++;
        }

        // if voltage is low OR if (voltage is high but under cutoff AND it has already toggled before (ie voltage is increasing a little before stabilising))
        if ((voltValueMap <= VOLT_CUTOFF) || (((voltValueMap <= (VOLT_CUTOFF + VOLT_TOGGLE_POINT)) && hasDroppedLow))) {
            PORTD = PORTD & ~RELAY_PIN; // low (open)
            PORTB = PORTB & ~LED_PIN; // led off
        } else {
            PORTD = PORTD | RELAY_PIN; // high (closed)
            PORTB = PORTB | LED_PIN; // led on
            if (hasDroppedLow) {
                hasDroppedLow = false;
                countOscillation++;
            }
        }

        timerReadWrite = 0;
        lastVoltage = voltValueMap;
    }
}