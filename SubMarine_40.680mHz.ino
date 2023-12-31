/**
//          Copyright Niels Post 2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
 */

#include <Adafruit_SI5351.h>

/**
 * Type that defines how long a pulse is.
 *
 * In cases where the MCU speed is too low to properly process the pulse length, enableWait can be set to false to disable
 * the sleep completely, and produce the shortest possible pulse.
 *
 * Note that this will not work if more than one pulse types are too short for the chosen MCU.
 */
struct SubmarinePulseType {
    bool enableWait;
    long waitDuration;
};

/**
 *  These are the three available pulse types for the used submarine.
 *
 *  These are the values tested on an Arduino Nano (Atmega 328p). Other values may be needed for another microcontroller.
 *
 *
 *  The values chosen should roughly result in pulses of the lengths below. This can be tested using a logic analyzer or an RF receiver
 *  - SHORT: 400 microseconds
 *  - LONG: 850 microseconds
 *  - SYNC: 1.3 milliseconds
 */
const SubmarinePulseType PULSE_SHORT{false, 0};
const SubmarinePulseType PULSE_LONG{true, 450};
const SubmarinePulseType PULSE_SYNC{true, 900};

/**
 * Enum containing possible commands for the chosen submarine
 */
enum class SubmarineCommand : uint8_t {
    STOP = 0x00,
    FORWARD = 0x01,
    BACK = 0x02,
    LEFT = 0x04,
    RIGHT = 0x08,
    UP = 0x10,
    DOWN = 0x20,
    CMD1 = 0x40, // CMD1 and CMD2 are theoretically available, but don't seem to be used for this submarine
    CMD2 = 0x80,
};


class Submarine {
private:
    Adafruit_SI5351 clockGenerator;
public:

    Submarine() : clockGenerator{} {
        if (clockGenerator.begin() != ERROR_NONE) {
            Serial.println("ERROR: SI5351 Not Found! Exiting...");
            while (1);
        }

        // Set Clock Generator Values.
        // Using SI5351 ClockBuilder Desktop with the following values
        // Input clock: 25MhZ
        // Output clock0: 40.680MhZ
        clockGenerator.setupPLL(SI5351_PLL_A, 29, 181, 625);
        clockGenerator.setupMultisynth(0, SI5351_PLL_A, 18, 0, 1);
        clockGenerator.setupRdiv(0, 0);
        clockGenerator.enableOutputs(false);
    }

    /**
     * Send one or multiple pulses of a specific type
     *
     *
     * @param pulseType Type of pulse to send. Possible values for this project: PULSE_SHORT, PULSE_LONG or PULSE_SYNC
     * @param pulseCount Number of pulses to send
     */
    void sendPulses(SubmarinePulseType pulseType, int pulseCount) {
        if (pulseType.enableWait) {
            for (int i = 0; i < pulseCount; i++) {
                clockGenerator.enableOutputs(true);
                delayMicroseconds(pulseType.waitDuration);
                clockGenerator.enableOutputs(false);
                delayMicroseconds(pulseType.waitDuration);
            }
        } else {
            for (int i = 0; i < pulseCount; i++) {
                clockGenerator.enableOutputs(true);
                clockGenerator.enableOutputs(false);
            }
        }
    }

    /**
     * Send a full 16 bits RC dataframe, including 1 leading sync pulses and one trailing short pulse
     *
     * Usually, a frame consists of two bytes.
     * - most significant byte: A byte filled with 1s, except for one bit, this bit being a 0
     * - least significant byte: A byte filled with 0s, except for one bit. The divergent bit should be in the same position as the divergent bit in the msB
     * Example: 0b 1111 1101  0000 0010
     * Note how the divergent bit is the second bit from the right in both bytes
     *
     * @param frame Dataframe to send
     */
    void sendFrame(uint16_t frame) {
        uint16_t currentPosition = 0x8000;
        sendPulses(PULSE_SYNC, 2);

        for (uint8_t i = 0; i < 16; i++) {
            sendPulses((currentPosition & frame) > 0 ? PULSE_LONG : PULSE_SHORT, 1);
            currentPosition >>= 1;
        }
        sendPulses(PULSE_SHORT, 1);
    }

    /**
     * Send a single stop frame
     */
    void sendStop() {
        sendFrame(0xFF00);
    }

    /**
     * Send one of the eight available commands. Hold it for a given duration. After this, the stop command is sent three
     * times
     *
     * @param command The command to send
     * @param hold_time_ms How many ms to hold the command. Holding is done by continuously retransmitting the command.
     */
    void sendCommand(SubmarineCommand command, long hold_time_ms = 10) {
        uint8_t lsB = static_cast<uint8_t>(command);
        uint8_t msB = ~lsB;

        uint16_t frame = (msB << 8) | lsB;

        long start_time = millis();
        sendFrame(frame);
        delay(35);
        sendFrame(frame);

        while (millis() < (start_time + hold_time_ms)) {
            delay(35);
            sendFrame(frame);
        }

        for (uint8_t i = 0; i < 3; i++) {
            delay(35);
            sendStop();
        }
    }

};

/**
 * Simple example code that sends a forward command for 3 seconds, and waits for 2 seconds
 */
void setup(void) {
    Serial.begin(9600);
    Submarine sub = Submarine();

    char read;
    while (true) {
        read = Serial.read();
        switch (read) {
            case 'f':
                sub.sendCommand(SubmarineCommand::FORWARD, 3000);
                break;
            case 'b':
                sub.sendCommand(SubmarineCommand::BACK, 3000);
                break;
            case 'd':
                sub.sendCommand(SubmarineCommand::DOWN, 3000);
                break;
            case 'u':
                sub.sendCommand(SubmarineCommand::UP, 3000);
                break;
            case 'l':
                sub.sendCommand(SubmarineCommand::LEFT, 3000);
                break;
            case 'r':
                sub.sendCommand(SubmarineCommand::RIGHT, 3000);
                break;
        }
    }
}


/**
 * Left empty to avoid global variable initialization
 */
void loop(void) {}



