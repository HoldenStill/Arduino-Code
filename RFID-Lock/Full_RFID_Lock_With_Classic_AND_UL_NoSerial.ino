/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 *
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 *
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * This sample shows how to read and write data blocks on a MIFARE Classic PICC
 * (= card/tag).
 *
 * BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
 *
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 *
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Stepper.h>
#include <Servo.h>
#include <IRremote.h>

#define RST_PIN         5           // Configurable, see typical pin layout above
#define SS_PIN          53   // Configurable, see typical pin layout above
int motorPin = 31;

bool unlocked = false;
const int stepsPerRevolution = 512;  // change this to fit the number of steps per revolution
const int rolePerMinute = 50;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm

int receiver = 6;

IRrecv irrecv(receiver);
decode_results results;

int Pval = 0;
int potVal = 0;

/*
 * You can create your own correct set of hex code to unlock the lock, 
 * just make sure to write it to the keycard as well. (see other files)
 */
byte correctData[]    = {
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0
        0x00, 0x00, 0x00, 0x00, // 0, 0, 0, 0
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0
        0x00, 0x00, 0x00, 0x00  //  0, 0, 0, 0
};

uint8_t pageAddr = 0x06;  //In this example we will write/read 16 bytes (page 6,7,8 and 9).
                            //Ultraligth mem = 16 pages. 4 bytes per page. 
                            //Pages 0 to 4 are for special functions.           

// initialize the stepper library on pins 8 through 11:
//Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);
Servo servo;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

byte buffer[18];
    byte size = sizeof(buffer);
/**
 * Initialize.
 */
void setup() {
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    servo.attach(9);
    

    //myStepper.setSpeed(rolePerMinute);
    //Serial.begin(9600); // Initialize serial communications with the PC
    //while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
 
    //Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
    //Serial.print(F("Using key (for A and B):"));
    //dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
    //Serial.println();
    irrecv.enableIRIn();
}


void translateIR(){
  switch(results.value)
  {
    case 0xFF629D: { 

      //Serial.println("LOCK");
      servo.write(160);
      unlocked = false;
      delay(300);
return;
      }
    case 0xFFA857: { 
      
      //Serial.println("UNLOCK");
      servo.write(20);
      unlocked = true;
      delay(300);
return;
    }
  }
  delay(500);
}



/**
 * Main loop.
 */





void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        //Serial.print(buffer[i], HEX);    
    }
}


 
void loop() {
  if(irrecv.decode(&results))
  {
    translateIR();
    delay(100);
    irrecv.resume();
  }
  /*
delay(2);
potVal = map(analogRead(A0),0,1024,0,500);
if (potVal>Pval){
  digitalWrite(31, HIGH);
  delay(5);
  myStepper.step(-25);
  delay(5);
}
if (potVal<Pval){
  digitalWrite(31, HIGH);
  delay(5);
  myStepper.step(25);
  delay(5);
}


Pval = potVal;


delay(10);
digitalWrite(31, LOW);
*/
    
    if(servo.read() == 160)
    {
      unlocked = false;
    } else if(servo.read() == 20)
    {
      unlocked = true;
    }
    if(digitalRead(2) == LOW)
    {
      //Serial.println("locked");
      servo.write(160);
      unlocked = false;
    }
    if(digitalRead(3) == LOW)
    {
      //Serial.println("unlocked");
      servo.write(20);
      unlocked = true;
    }
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    //Serial.print(F("Card UID:"));
    //dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    //Serial.println();
    //Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    //Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (      piccType == MFRC522::PICC_TYPE_MIFARE_1K) {
        //Serial.println(F("MiFare Classic detected."));
        
    byte sector     = 1;
    byte blockAddr  = 4;

    
    
    byte trailerBlock   = blockAddr + 3;
    MFRC522::StatusCode status;
    

    // Authenticate using key A
    //Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Show the whole sector as it currently is
    //Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    //Serial.println();

    // Read data from the block
    //Serial.print(F("Reading data from block ")); //Serial.print(blockAddr);
    //Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("MIFARE_Read() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
    }

    

    
    //Serial.print(F("Data in block ")); //Serial.print(blockAddr); //Serial.println(F(":"));
    //dump_byte_array(buffer, 16); //Serial.println();
    //Serial.println();

    


    // Authenticate using key B
    //Serial.println(F("Authenticating again using key B..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    //Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        if (buffer[i] == correctData[i])
            count++;
    }
    //Serial.print(F("Number of bytes that match = ")); //Serial.println(count);
    if (count == 16) 
      if(unlocked == false)
      {
      //Serial.println(F("Unlocked!"));
      unlocked = true;
      servo.write(20);
      delay(1000);
      loop();
      } else if(unlocked == true)
      {
        //Serial.println("locked");
        unlocked = false;
        servo.write(160);
        delay(1000);
        loop();
      }
    } else {
      loop(); 
    }
    } else{
      // Read data ***************************************************
      MFRC522::StatusCode status;
  //Serial.println(F("Reading data ... "));
  //data in 4 block is readed at once.
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(pageAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    //Serial.print(F("MIFARE_Read() failed: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //Serial.print(F("Readed data: "));
  //Dump a byte array to Serial
  for (byte i = 0; i < 16; i++) {
    //Serial.write(buffer[i]);
 
  }
  //Serial.println();
  byte count = 0;
  for (byte i = 0; i < 16; i++) {
        if (buffer[i] == correctData[i])
            count++;
    }
    //Serial.print(F("Number of bytes that match = ")); //Serial.println(count);
    if (count == 16) {
      if(unlocked == false)
      {
      //Serial.println(F("Unlocked!"));
      unlocked = true;
      servo.write(20);
      delay(1000);
      loop();
      } else if(unlocked == true)
      {
        //Serial.println("locked");
        unlocked = false;
        servo.write(160);
        delay(1000);
        loop();
      }
    } else {
      loop(); 
    }
  }
    

    // In this sample we use the second sector,
    // that is: sector #1, covering block #4 up to and including block #7



    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
