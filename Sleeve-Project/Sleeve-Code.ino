
#include <MPU6050.h>
//#include <MPU6050_6Axis_MotionApps20.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <DS3231.h>
#include <dht_nonblocking.h>
#include <SR04.h>

#define TRIG_PIN 6
#define ECHO_PIN 3
#define DHT_SENSOR_TYPE DHT_TYPE_11

 //Analog port 4 (A4) = SDA (serial data)
//Analog port 5 (A5) = SCL (serial clock)
#define SIGNAL_PATH_RESET 0x68
#define I2C_SLV0_ADDR 0x37
#define ACCEL_CONFIG 0x1C
#define MOT_THR 0x1F // Motion detection threshold bits [7:0]
#define MOT_DUR 0x20 // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define MOT_DETECT_CTRL 0x69
#define INT_ENABLE 0x38
#define WHO_AM_I_MPU6050 0x75 // Should return 0x68
#define INT_STATUS 0x3A
//when nothing connected to AD0 than address is 0x68
#define ADO 0
#if ADO
#define MPU6050_ADDRESS 0x69 // Device address when ADO = 1
#else
#define MPU6050_ADDRESS 0x68 // Device address when ADO = 0
#endif


int wakePin = 2; // pin used for waking up  
int led = 13;
int flag = 0;

unsigned long timer = 0;

unsigned long lastchecked;
unsigned long elapsed; 


static const int DHT_SENSOR_PIN = 13;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
float a;

DS3231 clock;
RTCDateTime dt;

int wait = 300;
char* starting[] = {"Starting   ", "Starting.  ", "Starting.. ", "Starting..."};

int page = 1;

int initTimeCheck = 0;
int endTimer;
int lastChecked;

void setup() {
 Serial.begin(9600);
 /*
   * #define SIGNAL_PATH_RESET  0x68
     #define I2C_SLV0_ADDR      0x37
     #define ACCEL_CONFIG       0x1C 
     #define MOT_THR            0x1F  // Motion detection threshold bits [7:0]
     #define MOT_DUR            0x20  // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
     #define MOT_DETECT_CTRL    0x69
     #define INT_ENABLE         0x38
     #define WHO_AM_I_MPU6050 0x75 // Should return 0x68
     #define INT_STATUS 0x3A*/
  Serial.begin(9600);
  writeByte(MPU6050_ADDRESS, 0x6B, 0x00);
  writeByte(MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07); //Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
  writeByte(MPU6050_ADDRESS, I2C_SLV0_ADDR, 0x20); //write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that stays until register (decimal) 58 is read, write 0x20.
  writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0x01); //Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
  writeByte(MPU6050_ADDRESS, MOT_THR, 10); //Write the desired Motion threshold to register 0x1F (For example, write decimal 20).  
  writeByte(MPU6050_ADDRESS, MOT_DUR, 40); //Set motion detect duration to 1  ms; LSB is 1 ms @ 1 kHz rate  
  writeByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15); //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )   
  writeByte(MPU6050_ADDRESS, INT_ENABLE, 0x40); //write register 0x38, bit 6 (0x40), to enable motion detection interrupt.     
  writeByte(MPU6050_ADDRESS, 0x37, 160); // now INT pin is active low

  pinMode(2, INPUT); // sets the digital pin 7 as input

  pinMode(wakePin, INPUT_PULLUP); // wakePin is pin no. 2
  pinMode(led, OUTPUT); //   led is pin no. 13
  // attachInterrupt(0, wakeUpNow, LOW); // use interrupt 0 (pin 2) and run function wakeUpNow when pin 2 gets LOW

  //A1 is the pin that enables or disables the backlight on the LCD.
  pinMode(A1, OUTPUT);
  //A0 is the pin that enables or disables the LCD in general.
  pinMode(A0, OUTPUT);
  
  //turn on backlight (analogWrite() because it will fade out instead of cutting out
  analogWrite(A1, 255);
  //enable lcd
  digitalWrite(A0, HIGH);
  clock.begin();
  lcd.begin(16, 2);
  lcd.noAutoscroll();
  
  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  
  //dt = clock.getDateTime();
  //lastChecked = dt.second;
  
  
  clock.setDateTime(__DATE__, __TIME__);
  //startup();
  lcd.clear();
}



void wakeUpNow()
{
   //any code to use immediately upon wakeup used here
  Serial.println("wake up");
  digitalWrite(A0, HIGH);
  analogWrite(A1, 255);
  detachInterrupt(0);
}

/*    Example for using write byte
      Configure the accelerometer for self-test
      writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0xF0); // Enable self test on all three axes and set accelerometer range to +/- 8 g */
void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
  Wire.begin();
  Wire.beginTransmission(address); // Initialize the Tx buffer
  Wire.write(subAddress); // Put slave register address in Tx buffer
  Wire.write(data); // Put data in Tx buffer
  Wire.endTransmission(); // Send the Tx buffer
  //  Serial.println("mnnj");

}

//example showing using readbytev   ----    readByte(MPU6050_ADDRESS, GYRO_CONFIG);
uint8_t readByte(uint8_t address, uint8_t subAddress) {
  uint8_t data; // `data` will store the register data   
  Wire.beginTransmission(address); // Initialize the Tx buffer
  Wire.write(subAddress); // Put slave register address in Tx buffer
  Wire.endTransmission(false); // Send the Tx buffer, but send a restart to keep connection alive
  Wire.requestFrom(address, (uint8_t) 1); // Read one byte from slave register address 
  data = Wire.read(); // Fill Rx buffer with result
  return data; // Return data read from slave register
}

void sleepNow()         // here we put the arduino to sleep
{
  //turn LCD back on
  digitalWrite(A0, HIGH);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0, wakeUpNow, LOW);
  sleep_mode();

  sleep_disable();       // first thing after waking from sleep:
                          // disable sleep...
  
}


static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every second. */
  if( millis( ) - measurement_timestamp > 1000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}



void loop() {

  

  digitalWrite(A0, HIGH);
  
  dt = clock.getDateTime();
  a = sr04.Distance();

  
  float temperature;
  float humidity;
  
  
  if(initTimeCheck == 0){
    Serial.println("initTimeCheck");
    lastChecked = dt.second;
    initTimeCheck = 1;
  }
  
  if(page == 1){
    lcd.setCursor(0,0);
    lcd.print("Date:");
    lcd.setCursor(0,1);
    lcd.print(dt.month); lcd.write("/"); lcd.print(dt.day); lcd.write("/"); lcd.print(dt.year); 

    
    lcd.setCursor(8,0);
  
    lcd.print("Time:s"); if(dt.second < 10){lcd.print("0");} lcd.print(dt.second);
    

    lcd.setCursor(11,1);
    if(dt.hour > 12){dt.hour = dt.hour - 12;}
    if(dt.hour == 0){dt.hour = 12;}
    if(dt.hour < 10){lcd.print("0");}
  
    lcd.print(dt.hour); lcd.write(":"); if(dt.minute < 10){lcd.print("0");} lcd.print(dt.minute);
  }
  
  if(page == 2){
    if( measure_environment( &temperature, &humidity ) == true )
    {
      lcd.setCursor(0,0);
      lcd.print("Temp:"); 
      lcd.setCursor(0,1);
      lcd.print(temperature, 1); lcd.print(" C.");
      
      lcd.setCursor(9,0);
      lcd.print("Humid:");
      lcd.setCursor(11,1);
      lcd.print(humidity, 1); lcd.print("%");
    }
  }
  
  
  
  if(page == 3){
    if( measure_environment( &temperature, &humidity ) == true )
    {
      float tempF = temperature * 1.8 + 32;
      lcd.setCursor(0,0);
      lcd.print("Temp:"); 
      lcd.setCursor(0,1);
      lcd.print(tempF, 1); lcd.print(" F.");
      
      lcd.setCursor(9,0);
      lcd.print("Humid:");
      lcd.setCursor(11,1);
      lcd.print(humidity, 1); lcd.print("%");
    }
  }
  if(page == 4){
    lcd.setCursor(0,0); lcd.print("Distance:");
    lcd.setCursor(0,1); 
    if(a > 100){
      a = a / 100; lcd.print(a); lcd.write("m  "); 
      
    } else {
      lcd.print(a); lcd.write("cm  "); 
    }
    lcd.setCursor(11,0);
    if(dt.hour > 12){dt.hour = dt.hour - 12;}
    if(dt.hour == 0){dt.hour = 12;}
    if(dt.hour < 10){lcd.print("0");}
  
    lcd.print(dt.hour); lcd.write(":"); if(dt.minute < 10){lcd.print("0");} lcd.print(dt.minute);
    //delay(100);
  }

  if(page == 5){
    
  }
  
  if(digitalRead(5) == LOW){
    page++;
    lcd.clear();
    delay(200);
    lastChecked = dt.second;
    digitalWrite(A0, HIGH);
  } else if(digitalRead(4) == LOW){
    page--;
    lcd.clear();
    delay(200);
    lastChecked = dt.second;
    digitalWrite(A0, HIGH);
  } 

  if(digitalRead(2) == LOW){
    lastChecked = dt.second;
    digitalWrite(A0, HIGH);
    delay(200);
  }
  
  TimerEndCorrection();
  
  if(page < 1){
    page = 1;
  } else if(page > 5){
    page = 5;
  }
  //Serial.println(page);
  if(dt.second > endTimer){
    Serial.println("timer end");
    FadeDisplay();
    //lcd.clear();
    sleepNow();
    
  }

  
}

void startup(){
  lcd.print(starting[0]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[1]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[2]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[3]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[4]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[0]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[1]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[2]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[3]);
  delay(wait);
  lcd.clear();
  lcd.print(starting[4]);
  delay(wait);

}

void TimerEndCorrection(){
  //if the timer starts at 50+ seconds, 
  //then it will never reach the end if it isnt corrected first.
  if(lastChecked == 50){
    endTimer = 0;
  } else if(lastChecked == 51){
    endTimer = 1;
  } else if(lastChecked == 52){
    endTimer = 2;
  } else if(lastChecked == 53){
    endTimer = 3;
  } else if(lastChecked == 54){
    endTimer = 4;
  } else if(lastChecked == 55){
    endTimer = 5;
  } else if(lastChecked == 56){
    endTimer = 6;
  } else if(lastChecked == 57){
    endTimer = 7;
  } else if(lastChecked == 58){
    endTimer = 8;
  } else if(lastChecked == 59){
    endTimer = 9;
  } else {
    endTimer = lastChecked + 10;
  }
}


void FadeDisplay(){
  //planning on making the backlight fade out using analogWrite()
  analogWrite(A1, 0);
}
