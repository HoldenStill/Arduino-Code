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

int timerEnable;
int lastButtonPress;
int initTimeCheck = 0;
int endTimer;
int endTimerMin;
int lastChecked;
int lastCheckedMin;

void wakeUpNow()
{
   //any code to use immediately upon wakeup used here
  Serial.println("wake up");
  digitalWrite(A0, HIGH);
  analogWrite(A1, 255);
  detachInterrupt(0);
}

void setup() {
 Serial.begin(9600);

  pinMode(2, INPUT_PULLUP);
  
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  
  //A1 is the pin that enables or disables the backlight on the LCD.
  pinMode(A1, OUTPUT);
  //A0 is the pin that enables or disables the LCD in general.
  pinMode(A0, OUTPUT);
  
  //turn on backlight (analogWrite() because it will fade out instead of cutting out
  analogWrite(A1, 255);
  //enable lcd
  digitalWrite(A0, HIGH);
  
  lcd.begin(16, 2);
  lcd.noAutoscroll();
  
  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  
  //dt = clock.getDateTime();
  //lastChecked = dt.second;
  //startup();
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__);
  
  lcd.clear();
}

void sleepNow()         // here we put the arduino to sleep
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  attachInterrupt(0, wakeUpNow, FALLING);
  Serial.println("sleeping");
  
  digitalWrite(A0, LOW);
  
  
  
  sleep_enable();
  
  
  
  
  sleep_mode();

  sleep_disable();       // first thing after waking from sleep:
                          // disable sleep...
  detachInterrupt(0);
  
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

int sensorValue = analogRead(A3); //read the A0 pin value
float voltage = sensorValue * (5.00 / 1023.00) * 2; //convert the value to a true voltage.

float temperature;
float humidity;

void loop() {
  
  sensorValue = analogRead(A3);
  voltage = sensorValue * (5.00 / 1023.00) * 2;

  if (voltage < 6.50) //set the voltage considered low battery here
  {
    digitalWrite(A2, HIGH);
  }
  
  digitalWrite(A0, HIGH);
  
  dt = clock.getDateTime();
  a = sr04.Distance();

  
  
  
  
  if(initTimeCheck == 0){
    Serial.println("initTimeCheck");
    lastChecked = dt.second;
    lastCheckedMin = dt.minute;
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
      Serial.println("measuring environment C");
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
      Serial.println("measuring environment F");
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
    printVolts();
  }
  
  if(digitalRead(5) == LOW){
    lastButtonPress = 5;
    page++;
    lcd.clear();
    delay(200);
    lastChecked = dt.second;
    lastCheckedMin = dt.minute;
    digitalWrite(A0, HIGH);
  } else if(digitalRead(4) == LOW){
    lastButtonPress = 4;
    page--;
    lcd.clear();
    delay(200);
    lastChecked = dt.second;
    lastCheckedMin = dt.minute;
    digitalWrite(A0, HIGH);
  } else if(digitalRead(2) == LOW){
    lastButtonPress = 2;
    lcd.clear();
    delay(200);
    lastChecked = dt.second;
    lastCheckedMin = dt.minute;
    digitalWrite(A0, HIGH);
  }

  if(lastButtonPress == 2){
    timerEnable = 0;
  } else {
    timerEnable = 1;
  }
  
  TimerEndCorrection();
  
  if(page < 1){
    page = 1;
  } else if(page > 5){
    page = 5;
  }
  //Serial.println(page);
  if(dt.second > endTimer && dt.minute == endTimerMin){
    
    Serial.println("timer end");
    FadeDisplay();
    lcd.clear();
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
  if(lastChecked + 10 >= 60){
    endTimerMin = dt.minute + 1;
    endTimer = lastChecked + 10 - 50;
  } else if(timerEnable == 0){
    endTimer = 61;
  } else {
    endTimerMin = dt.minute;
    endTimer = lastChecked + 10;
  }
}

void FadeDisplay(){
  lcd.clear();
  analogWrite(A1, 0);
}

 void printVolts()
{
  //int sensorValue = analogRead(A3); //read the A0 pin value
  //float voltage = sensorValue * (5.00 / 1023.00) * 2; //convert the value to a true voltage.
  lcd.setCursor(0,0);
  lcd.print("Voltage = ");
  lcd.print(voltage); //print the voltage to LCD
  lcd.print(" V");
  //if (voltage < 6.50) //set the voltage considered low battery here
  //{
  //  digitalWrite(A2, HIGH);
  //}
}
