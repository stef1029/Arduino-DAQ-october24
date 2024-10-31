#include <Arduino.h>
#include <stdint.h>

// Define your pins as before
#define BUZZER1 41
#define BUZZER2 37
#define BUZZER3 33
#define BUZZER4 30  // corrected from 34 20OCT23
#define BUZZER5 34  // corrected from 38 20OCT23
#define BUZZER6 38  // corrected from 42 20OCT23
#define LED_1 39
#define LED_2 35
#define LED_3 31
#define LED_4 32
#define LED_5 36
#define LED_6 40
#define VALVE1 47
#define VALVE2 45
#define VALVE3 43
#define VALVE4 42
#define VALVE5 44
#define VALVE6 46
#define SENSOR1 25
#define SENSOR2 27
#define SENSOR3 29
#define SENSOR4 28
#define SENSOR5 26
#define SENSOR6 24
#define SPOT1 12
#define SPOT2 7 // was 13
#define SPOT3 8
#define SPOT4 9
#define SPOT5 10
#define SPOT6 11
#define IR 6  // was 7
#define GO_CUE 48
#define NOGO_CUE 50

#define SYNC 2 // yellow wire
#define SYNC_GND 3  // purple wire

// Add your new pins here
#define NEW_PIN1 49
#define NEW_PIN2 51
#define NEW_PIN3 62  // A8 pin

// Update the pin_list array to include your new pins
uint8_t pin_list[] = {7, 8, 9, 10, 11, 12, // spotlights
                      24, 25, 26, 27, 28, 29,   // sensors
                      30, 31, 32, 33, 34, 35,   
                      36, 37, 38, 39, 40, 41, 
                      42, 43, 44, 45, 46, 47,   // valves
                      48, 50, 62, 63};  // go and non go cues and camera and laser

// Update the number of pins
const unsigned int num_pins = sizeof(pin_list) / sizeof(pin_list[0]);

void setup() {
  for (unsigned int i = 0; i < num_pins; i++) {
    pinMode(pin_list[i], INPUT);
    digitalWrite(pin_list[i], LOW);
  }
  pinMode(SYNC, OUTPUT);
  pinMode(SYNC_GND, OUTPUT);
  digitalWrite(SYNC_GND, LOW);

  Serial.begin(115200);
}

void send_message(uint64_t message, unsigned long message_number) {
  byte bytes_to_send[11];

  bytes_to_send[0] = 0x01; // Start byte

  // Interleaved message_number and message bytes
  bytes_to_send[1] = (message_number >> 24) & 0xFF;          // message_number byte 0 (MSB)
  bytes_to_send[2] = (message >> 32) & 0xFF;                 // message byte 0 (MSB)
  bytes_to_send[3] = (message_number >> 16) & 0xFF;          // message_number byte 1
  bytes_to_send[4] = (message >> 24) & 0xFF;                 // message byte 1
  bytes_to_send[5] = (message_number >> 8) & 0xFF;           // message_number byte 2
  bytes_to_send[6] = (message >> 16) & 0xFF;                 // message byte 2
  bytes_to_send[7] = message_number & 0xFF;                  // message_number byte 3 (LSB)
  bytes_to_send[8] = (message >> 8) & 0xFF;                  // message byte 3
  bytes_to_send[9] = message & 0xFF;                         // message byte 4 (LSB)

  bytes_to_send[10] = 0x02; // End byte

  // Ensure there's enough space in the serial buffer
  while (Serial.availableForWrite() < 11) {}
  digitalWrite(SYNC, HIGH);
  Serial.write(bytes_to_send, 11);
  digitalWrite(SYNC, LOW);
  Serial.flush();
}

unsigned long message_count = 0;
bool Send_messages = true;
bool start_wait = true;

void loop() {
  while (start_wait) {
    if (Serial.available() > 0) {
      if (Serial.read() == 's') {
        Serial.print("s");
        start_wait = false;
      }
    }
  }

  while (Send_messages) {
    uint64_t message = 0;

    for (unsigned int i = 0; i < num_pins; i++) {
      int sensorValue;

      if (pin_list[i] == 62) {
        // Read analog value from A8 (pin 62) for 3.3V TTL signal
        int analogValue = analogRead(62);
        int threshold = 600; // Adjust threshold as needed
        if (analogValue > threshold) {
          sensorValue = HIGH;
        } else {
          sensorValue = LOW;
        }
      } else {
        // Read digital value
        sensorValue = digitalRead(pin_list[i]);
      }

      if (sensorValue == HIGH) {
        message |= ((uint64_t)1 << i);
      }
    }

    send_message(message, message_count);
    message_count++;

    if (Serial.available() > 0) {
      if (Serial.read() == 'e') {
        start_wait = true;
        break;
      }
    }
  }
}
