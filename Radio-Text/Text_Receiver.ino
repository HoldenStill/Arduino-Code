#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#define CE_PIN 2
#define CSN_PIN 4

//instantiate a radio object
RF24 radio(CE_PIN, CSN_PIN);

// addresses for the node
uint8_t address[][6] = { "1NODE" , "2NODE" };

bool radioNumber = 0; // 0: address[0] for transmission, 1: address[1] for transmission.

String payload;

void setup() {
    //  init serial
  Serial.begin(115200);
  while (!Serial) {
    //  ensure serial is established
  }

  // init transceiver
  if(!radio.begin()){
    Serial.println("RADIO NOT RESPONDING");
    while(true) {} // hold in infinite loop if radio could not begin.
  }

  //print which node this is
  Serial.println("** THIS IS TEXT RECEIVER NODE **");
  Serial.println("** WAIT FOR TRANSMISSION **");

  //  set PA level to low for testing
  radio.setPALevel(RF24_PA_LOW);

  radio.openReadingPipe(1, address[!radioNumber]);

  radio.startListening();
}

void loop() {
  uint8_t pipe;
  if (radio.available(&pipe)) {
    char buffer[32] = {0};  // Ensure it's zero-initialized
    radio.read(&buffer, sizeof(buffer));
    payload = String(buffer);

    Serial.print("Received ");
    Serial.print(strlen(buffer));
    Serial.print(" bytes on pipe ");
    Serial.println(pipe);
    
    Serial.print("Message: ");
    Serial.println(payload);
  }
}

