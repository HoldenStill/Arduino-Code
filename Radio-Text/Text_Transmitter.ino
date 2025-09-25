#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#define CE_PIN 7
#define CSN_PIN 8

//instantiate a radio object
RF24 radio(CE_PIN, CSN_PIN);

// addresses for the node
uint8_t address[][6] = { "1NODE", "2NODE" };

bool radioNumber = 0;  // 0: address[0] for transmission, 1: address[1] for transmission.

// idle or sending
bool transmitting = false;

bool tooLong = false;

bool report;

unsigned long timer_start;
unsigned long timer_end;

int fullPayloads;
int remainderBytes;

//I'm going to try and allow arbitrary string payload--i have no idea how.
String payload;
String wholePayload;

void setup() {
  //  init serial
  Serial.begin(115200);
  while (!Serial) {
    //  ensure serial is established
  }

  // init transceiver
  if (!radio.begin()) {
    Serial.println("RADIO NOT RESPONDING");
    while (true) {}  // hold in infinite loop if radio could not begin.
  }

  //print which node this is
  Serial.println("** THIS IS TEXT TRANSMISSION NODE **");
  Serial.println("** ENTER TEXT TO TRANSMIT TO OTHER NODE **");

  //  set PA level to low for testing
  radio.setPALevel(RF24_PA_LOW);

  radio.stopListening();

  radio.openWritingPipe(address[!radioNumber]);
}

void loop() {


  while (!Serial.available()) {
    //wait for user input
  }

  String input = Serial.readStringUntil('\n');
  payload = input;
  wholePayload = payload;
  
  int length = payload.length(); // for some reason this adds 5 to the real character length of a string. 
  Serial.println("length: " + (String)length);

  if (length > 32) { 
    tooLong = true;
    fullPayloads = (int)(length / 32);
    Serial.println("fullPayloads: " + (String)fullPayloads);
    remainderBytes = (length % 32); 
    Serial.println("remainderBytes: " + (String)remainderBytes);
  }

  if(tooLong) {
    while (fullPayloads > 0) {
      char buffer[32];
      payload.toCharArray(buffer, sizeof(buffer));  // copy safely
      payload.remove(0, 31);
      length = payload.length();
      timer_start = micros();
      report = radio.write(&buffer, strlen(buffer) + 1);  // +1 to send null terminator
      if (!report) { 
        Serial.println("Transmission failed/timed out.");
        goto jump;
      }
      fullPayloads--;
    }
    if (remainderBytes > 0){
      char buffer[32];
      // Serial.println("remainderBytes: " + (String)remainderBytes);
      payload.toCharArray(buffer, sizeof(buffer));
      report = radio.write(&buffer, sizeof(buffer) + 1);  // +1 to send null terminator
      if (!report) { 
        Serial.println("Transmission failed/timed out.");
        goto jump;
      }
      timer_end = micros();
      Serial.print("Successful transmission, ");
      Serial.print((timer_end - timer_start));
      Serial.print("us to transmit. Payload:  ");
      Serial.println(wholePayload);
    }
  } else {
    char buffer[32];                              // NRF24L01 has a max payload size of 32 bytes
    payload.toCharArray(buffer, sizeof(buffer));  // copy safely
    timer_start = micros();
    report = radio.write(&buffer, strlen(buffer) + 1);  // +1 to send null terminator
    timer_end = micros();
    if (report) {
      Serial.print("Successful transmission, ");
      Serial.print((timer_end - timer_start));
      Serial.print("us to transmit. Payload:  ");
      Serial.println(payload);
    } else {
      Serial.println("Transmission failed/timed out.");
    }
  }

  jump:

  Serial.println("\nType again to transmit another message.");
}
