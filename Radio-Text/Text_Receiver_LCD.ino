#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>
#include <LiquidCrystal.h>

#define CE_PIN 7
#define CSN_PIN 8

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

//instantiate a radio object
RF24 radio(CE_PIN, CSN_PIN);

//create lcd object
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// addresses for the node
uint8_t address[][6] = { "1NODE", "2NODE" };

bool radioNumber = 0;  // 0: address[0] for transmission, 1: address[1] for transmission.

String payload;

void setup() {
  // go ahead and set contrast for the lcd
  analogWrite(9, 100);

  lcd.begin(16, 2);
  //  init serial
  Serial.begin(115200);
  while (!Serial) {
    //  ensure serial is established
  }

  // init transceiver
  if (!radio.begin()) {
    Serial.println("RADIO NOT RESPONDING");
    lcd.print("RADIO NOT RESPONDING");
    while (true) {}  // hold in infinite loop if radio could not begin.
  }

  //print which node this is
  Serial.println("** THIS IS TEXT RECEIVER NODE **");
  lcd.print("RECEIVER NODE");
  delay(1000);
  Serial.println("** WAIT FOR TRANSMISSION **");
  lcd.setCursor(0, 0);
  lcd.print("WAIT FOR TRANSMISSION");
  delay(750);
  for (int i = 5; i != 0; i--) {lcd.scrollDisplayLeft(); delay(150);}
  delay(300);
  for (int i = 5; i != 0; i--) {lcd.scrollDisplayRight(); delay(150);}
  delay(500);
  lcd.clear();
  //  set PA level to low for testing
  radio.setPALevel(RF24_PA_LOW);

  radio.openReadingPipe(1, address[!radioNumber]);

  radio.startListening();
}

void loop() {
  uint8_t pipe;
  lcd.setCursor(0, 0);
  lcd.print("WAIT FOR ");
  lcd.setCursor(0, 1);
  lcd.print("TRANSMISSION.");
  //delay(750);
  //for (int i = 5; i != 0; i--) {lcd.scrollDisplayLeft(); delay(150);}
  //delay(300);
  //for (int i = 5; i != 0; i--) {lcd.scrollDisplayRight(); delay(150);}
  if (radio.available(&pipe)) {
    char buffer[32];  // Ensure it's zero-initialized
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
