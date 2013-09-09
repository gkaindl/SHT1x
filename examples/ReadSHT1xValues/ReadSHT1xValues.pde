/**
 * ReadSHT1xValues
 *
 * Read temperature and humidity values from an SHT1x-series (SHT10,
 * SHT11, SHT15) sensor.
 *
 * Copyright 2009 Jonathan Oxer <jon@oxer.com.au>
 * www.practicalarduino.com
 */

#include <SHT1x.h>

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  10
#define clockPin 11
SHT1x sht1x(dataPin, clockPin);

void setup()
{
   Serial.begin(38400); // Open serial connection to report values to host
   Serial.println("Starting up");
}

void loop()
{
  sht1x_value temp_c;
  sht1x_value temp_f;
  sht1x_value humidity;

  // Read values from the sensor
  temp_c = sht1x.readTemperatureC();
  temp_f = sht1x.readTemperatureF();
  humidity = sht1x.readHumidity();

  // Print the values to the serial port
  Serial.print("Temperature: ");
  Serial.print(sht1x_value_get_integer_part(temp_c), DEC);
  Serial.print('.');
  Serial.print(sht1x_value_get_fract_part(temp_c, 8), DEC);
  Serial.print("C / ");
  Serial.print(sht1x_value_get_integer_part(temp_f), DEC);
  Serial.print('.');
  Serial.print(sht1x_value_get_fract_part(temp_f, 8), DEC);
  Serial.print("F. Humidity: ");
  Serial.print(sht1x_value_get_integer_part(humidity), DEC);
  Serial.print('.');
  Serial.print(sht1x_value_get_fract_part(humidity, 8), DEC);
  Serial.println("%");

  delay(2000);
}
