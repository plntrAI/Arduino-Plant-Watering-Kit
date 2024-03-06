/* ------------- START CONFIG ------------- */
constexpr int BUTTON_PIN = 4;
constexpr int LED_PIN    = 13;
constexpr int RELAY_PIN  = 13;
constexpr int MOIST_PIN  = A0;


int raw_moisture = 0;
/* ------------- END CONFIG ------------- */

#include "thingProperties.h"
#include <Bounce2.h>

Bounce b;
unsigned long startedWatering;

void setup() {
    Serial.begin(9600);
    delay(1500);

    b.attach(BUTTON_PIN,INPUT_PULLUP);
    b.interval(25);
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    // Make sure the pump is not running
    stopWatering();

    // Connect to Arduino IoT Cloud
    initProperties();
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
    setDebugMessageLevel(4);
    ArduinoCloud.printDebugInfo();

    // Blink LED to confirm we're up and running
    for (int i = 0; i<=4; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }

}

void loop() {
    ArduinoCloud.update();

    // Read the sensor and convert its value to a percentage
    // (0% = dry; 100% = wet)
    raw_moisture = analogRead(MOIST_PIN);
    moisture = map(raw_moisture, 793, 382, 0, 100);
    Serial.println(moisture);

    // Set the LED behavior according to the moisture percentage or watering status
    if (watering) {
        digitalWrite(LED_PIN, HIGH);
    } else if (moisture > 40) {
        // good, LED is turned off
        digitalWrite(LED_PIN, LOW);
    } else if (moisture > 10) {
        // warning, slow blink
        digitalWrite(LED_PIN, (millis()%1000)<500);
    } else {
        // need water, fast blink
        digitalWrite(LED_PIN, (millis()%500)<250);
    }

    // Stop watering after the configured duration
    if (watering && (millis() - startedWatering) >= waterTime*1000) {
        stopWatering();
    }

    // Read button status
    b.update();
    if (b.changed() && b.read() == false) { // button pressed
        if (watering) {
            stopWatering();
        } else {
            startWatering();
        }
    }
}

// This function is triggered whenever the server sends a change event,
// which means that someone changed a value remotely and we need to do
// something.
void onWateringChange() {
    if (watering) {
        startWatering();
    } else {
        stopWatering();
    }
}


void startWatering () {
    watering = true;
    startedWatering = millis();
    digitalWrite(RELAY_PIN, HIGH);
}

void stopWatering () {
    watering = false;
    digitalWrite(RELAY_PIN, LOW);
}

void onWaterTimeChange()  {
    // Add your code here to act upon WaterTime change
}
