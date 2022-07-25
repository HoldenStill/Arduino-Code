/*
 * This is a very slightly modified version of the DHT11_Example by Elegoo. Modified to use the lcd attached to my project.
 */

  //www.elegoo.com
//2018.10.25

#include <LiquidCrystal.h>
#include <dht_nonblocking.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 13;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);


/*
 * Initialize the serial port.
 */
void setup( )
{
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  pinMode(A1, OUTPUT);
  digitalWrite(A1, HIGH);
  Serial.begin( 9600);
  lcd.begin(16, 2);
  lcd.noAutoscroll();
  lcd.clear();
}



/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
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



/*
 * Main program loop.
 */
void loop( )
{
  float temperature;
  float humidity;

  /* Measure temperature and humidity.  If the functions returns
     true, then a measurement is available. */
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
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
