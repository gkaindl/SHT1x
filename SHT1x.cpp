/**
 * SHT1x Library
 *
 * Copyright 2009 Jonathan Oxer <jon@oxer.com.au> / <www.practicalarduino.com>
 * Based on previous work by:
 *    Maurice Ribble: <www.glacialwanderer.com/hobbyrobotics/?p=5>
 *    Wayne ?: <ragingreality.blogspot.com/2008/01/ardunio-and-sht15.html>
 *
 * Manages communication with SHT1x series (SHT10, SHT11, SHT15)
 * temperature / humidity sensors from Sensirion (www.sensirion.com).
 */
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "SHT1x.h"

SHT1x::SHT1x(int dataPin, int clockPin)
{
  _dataPin = dataPin;
  _clockPin = clockPin;
}

/* ================  Public methods ================ */

/**
 * Reads the current temperature in degrees Celsius
 */
sht1x_value SHT1x::readTemperatureC()
{
  int _val;                 // Raw value returned from sensor
  sht1x_value _temperature; // Temperature derived from raw value

  // Conversion coefficients from SHT15 datasheet
  static const sht1x_value D1 = sht1x_value_make(-40.0);  // for 14 Bit @ 5V
  static const sht1x_value D2 = sht1x_value_make(0.01);   // for 14 Bit DEGC

  // Fetch raw value
  _val = readTemperatureRaw();

  // Convert raw value to degrees Celsius
  _temperature = sht1x_value_add(_val * D2, D1);

  return (_temperature);
}

/**
 * Reads the current temperature in degrees Fahrenheit
 */
sht1x_value SHT1x::readTemperatureF()
{
  int _val;                  // Raw value returned from sensor
  sht1x_value _temperature;  // Temperature derived from raw value

  // Conversion coefficients from SHT15 datasheet
  static const sht1x_value D1 = sht1x_value_make(-40.1);  // for 14 Bit @ 5V
  static const sht1x_value D2 = sht1x_value_make(0.01);   // for 14 Bit DEGC

  // Fetch raw value
  _val = readTemperatureRaw();

  // Convert raw value to degrees Fahrenheit
  _temperature = sht1x_value_add(_val * D2, D1);

  return (_temperature);
}

/**
 * Reads current temperature-corrected relative humidity
 */
sht1x_value SHT1x::readHumidity()
{
  int _val;                          // Raw humidity value returned from sensor
  sht1x_value _linearHumidity;       // Humidity with linear correction applied
  sht1x_value _correctedHumidity;    // Temperature-corrected humidity
  sht1x_value _temperature;          // Raw temperature value

  // Conversion coefficients from SHT15 datasheet
  static const sht1x_value C1 = sht1x_value_make(-4.0);       // for 12 Bit
  static const sht1x_value C2 = sht1x_value_make(0.0405);     // for 12 Bit
  static const sht1x_value C3 = sht1x_value_make(-0.0000028); // for 12 Bit
  static const sht1x_value T1 = sht1x_value_make(0.01);       // for 14 Bit @ 5V
  static const sht1x_value T2 = sht1x_value_make(0.00008);    // for 14 Bit @ 5V

  // Command to send to the SHT1x to request humidity
  int _gHumidCmd = 0b00000101;

  // Fetch the value from the sensor
  sendCommandSHT(_gHumidCmd, _dataPin, _clockPin);
  waitForResultSHT(_dataPin);
  _val = getData16SHT(_dataPin, _clockPin);
  skipCrcSHT(_dataPin, _clockPin);

  // Apply linear conversion to raw value
  _linearHumidity = sht1x_value_mul(_val * _val, C3);
  _linearHumidity = sht1x_value_add(_linearHumidity, C2 * _val);
  _linearHumidity = sht1x_value_add(_linearHumidity, C1);

  // Get current temperature for humidity correction
  _temperature = readTemperatureC();

  // Correct humidity value for current temperature
  _correctedHumidity = sht1x_value_sub(_temperature, sht1x_value_make(25.0));
  _correctedHumidity = sht1x_value_mul(_correctedHumidity, sht1x_value_add(T1, _val * T2));
  _correctedHumidity = sht1x_value_add(_correctedHumidity, _linearHumidity);

  return (_correctedHumidity);
}

/* ================  Private methods ================ */

/**
 * Reads the current raw temperature value
 */
sht1x_value SHT1x::readTemperatureRaw()
{
  int _val;

  // Command to send to the SHT1x to request Temperature
  int _gTempCmd  = 0b00000011;

  sendCommandSHT(_gTempCmd, _dataPin, _clockPin);
  waitForResultSHT(_dataPin);
  _val = getData16SHT(_dataPin, _clockPin);
  skipCrcSHT(_dataPin, _clockPin);

  return _val;
}

/**
 */
int SHT1x::shiftIn(int _dataPin, int _clockPin, int _numBits)
{
  int ret = 0;
  int i;

  for (i=0; i<_numBits; ++i)
  {
     digitalWrite(_clockPin, HIGH);
     delay(10);  // I don't know why I need this, but without it I don't get my 8 lsb of temp
     ret = ret*2 + digitalRead(_dataPin);
     digitalWrite(_clockPin, LOW);
  }

  return(ret);
}

/**
 */
void SHT1x::sendCommandSHT(int _command, int _dataPin, int _clockPin)
{
  int ack;

  // Transmission Start
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, LOW);

  // The command (3 msb are address and must be 000, and last 5 bits are command)
  shiftOut(_dataPin, _clockPin, MSBFIRST, _command);

  // Verify we get the correct ack
  digitalWrite(_clockPin, HIGH);
  pinMode(_dataPin, INPUT);
  ack = digitalRead(_dataPin);
  if (ack != LOW) {
    //Serial.println("Ack Error 0");
  }
  digitalWrite(_clockPin, LOW);
  ack = digitalRead(_dataPin);
  if (ack != HIGH) {
    //Serial.println("Ack Error 1");
  }
}

/**
 */
void SHT1x::waitForResultSHT(int _dataPin)
{
  int i;
  int ack;

  pinMode(_dataPin, INPUT);

  for(i= 0; i < 100; ++i)
  {
    delay(10);
    ack = digitalRead(_dataPin);

    if (ack == LOW) {
      break;
    }
  }

  if (ack == HIGH) {
    //Serial.println("Ack Error 2"); // Can't do serial stuff here, need another way of reporting errors
  }
}

/**
 */
int SHT1x::getData16SHT(int _dataPin, int _clockPin)
{
  int val;

  // Get the most significant bits
  pinMode(_dataPin, INPUT);
  pinMode(_clockPin, OUTPUT);
  val = shiftIn(_dataPin, _clockPin, 8);
  val *= 256;

  // Send the required ack
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);

  // Get the least significant bits
  pinMode(_dataPin, INPUT);
  val |= shiftIn(_dataPin, _clockPin, 8);

  return val;
}

/**
 */
void SHT1x::skipCrcSHT(int _dataPin, int _clockPin)
{
  // Skip acknowledge to end trans (no CRC)
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);

  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);
}

unsigned long sht1x_value_get_fract_part(sht1x_value x, byte precision)
#if SHT1X_USE_FIXED_POINT
{
   unsigned long f = 0, p, v = x;
   
   v &= sht1x_fixed_fract_mask;
   
   if (precision) {
      do {
         v *= 10;
         p = (v >> sht1x_fixed_fract);
         f = (f*10) + p;
         v &= sht1x_fixed_fract_mask;
      } while (--precision > 0);
   }
   
   return f;
}
#else
{
   unsigned long f = 0;
   
   if (precision) {
      float fp = 10.0;
      while (--precision) fp *= 10.0;
      
      f = (x < 0 ? (x + (unsigned long)x) : (x - (unsigned long)x)) * fp;
   }
   
   return f;
}
#endif
