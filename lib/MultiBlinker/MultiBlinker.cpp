#include "MultiBlinker.h"

MultiBlinker::MultiBlinker(int led1, int led2, int led3, int led4) {
    ledPins[0] = led1;
    ledPins[1] = led2;
    ledPins[2] = led3;
    ledPins[3] = led4;
    numLeds = 0;
    for (int i = 0; i < 4; i++) {
        if (ledPins[i] != -1) {
            numLeds++;
            pinMode(ledPins[i], OUTPUT);
        }
    }
}

void MultiBlinker::setState(int state) {
    if (numLeds == 0) {
        return;
    }
    if (state == currentState) {
        return;
    }
    debugV("state: %i: ", state);
    currentState = state;
    if (numLeds == 1 && state != 0) {
        updateLEDs();
        if (state == -1) {
            interval = 3000;
        } else {
            interval = INTERVAL_MULTIPLIER * state;
        }
    } else {
        interval = MULTI_BLINKER_INTERVAL;
    }
}

void MultiBlinker::start() {
    if (numLeds == 0) {
        return;
    }
    running = true;
    xTaskCreate(runTask, "MultiBlinkerTask", 2048, this, 1, &taskHandle);
}

void MultiBlinker::stop() {
    if (numLeds == 0) {
        return;
    }
    running = false;
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
        taskHandle = NULL;
    }
}

void MultiBlinker::runTask(void *pvParameters) {
    MultiBlinker *blinker = static_cast<MultiBlinker*>(pvParameters);
    blinker->run();
}

void MultiBlinker::run() {
    while (running) {
        ulong currentTime = millis();
        if (currentTime - lastUpdate >= interval) {
            lastUpdate = currentTime;
            updateLEDs();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task from hogging the CPU
    }
}

void MultiBlinker::updateLEDs() {
    if (numLeds == 1) {
        if (currentState == 0) {
            if (digitalRead(ledPins[0])) digitalWrite(ledPins[0], LOW);
        } else if (currentState == STATE_STARTED_WIFI_AP) {
            if (!digitalRead(ledPins[0])) digitalWrite(ledPins[0], HIGH);
        } else {
            // Blink the single LED using the interval value - where the interval is multiplied by the state (so it gets slower as the state increases)
            bool newState = !digitalRead(ledPins[0]);
            digitalWrite(ledPins[0], newState);
        }
    } else if (numLeds == 4) {
        if (currentState == -1) {
            knightRider();
        } else {
            for (int i = 0; i < 4; i++) {
                digitalWrite(ledPins[i], (currentState & (1 << (3 - i))) ? HIGH : LOW);
            }
        }
    }
}

void MultiBlinker::knightRider() {
    static int direction = 1;
    static int position = 0;

    for (int i = 0; i < 4; i++) {
        digitalWrite(ledPins[i], LOW);
    }
    digitalWrite(ledPins[position], HIGH);

    position += direction;
    if (position == 3 || position == 0) {
        direction = -direction;
        // Keep the end LEDs on for a longer duration
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}