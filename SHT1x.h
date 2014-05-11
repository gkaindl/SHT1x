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
#ifndef SHT1x_h
#define SHT1x_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define SHT1X_USE_FIXED_POINT 1

#if SHT1X_USE_FIXED_POINT
typedef signed long sht1x_value;

#define sht1x_fixed_fract        21
#define sht1x_fixed_fract_mask   ((((long)1) << sht1x_fixed_fract)-1)

#define sht1x_value_make(x)               ((long)((float)(x)*(sht1x_fixed_fract_mask+1)))
#define sht1x_value_get_integer_part(x)   ((long)((x) >> sht1x_fixed_fract))
unsigned long sht1x_value_get_fract_part(sht1x_value x, byte precision);

#define sht1x_value_add(x,y)    ((x) + (y))
#define sht1x_value_sub(x,y)    ((x) - (y))
#define sht1x_value_mul(x,y)    (((long long)(x) * (long long)(y)) >> sht1x_fixed_fract)
#define sht1x_value_div(x,y)    (((x) << sht1x_fixed_fract) / (y))

#define sht1x_value_to_float(x) ((float)((x) >> sht1x_fixed_fract) + (float)((x) & sht1x_fixed_fract_mask) / (float)(1+sht1x_fixed_fract_mask))

#else
typedef float sht1x_value;

#define sht1x_fixed_fract        0
#define sht1x_fixed_fract_mask   0

#define sht1x_value_make(x)               ((float)x)
#define sht1x_value_get_integer_part(x)   ((long)(x))
unsigned long sht1x_value_get_fract_part(sht1x_value x, byte precision);

#define sht1x_value_add(x,y)    ((x) + (y))
#define sht1x_value_sub(x,y)    ((x) - (y))
#define sht1x_value_mul(x,y)    ((x) * (y))
#define sht1x_value_div(x,y)    ((x) / (y)) 

#define sht1x_value_to_float(x)  (x)
#endif

class SHT1x
{
  public:
    SHT1x(int dataPin, int clockPin);
    sht1x_value readHumidity();
    sht1x_value readTemperatureC();
    sht1x_value readTemperatureF();
  private:
    int _dataPin;
    int _clockPin;
    int _numBits;
    sht1x_value readTemperatureRaw();
    int shiftIn(int _dataPin, int _clockPin, int _numBits);
    void sendCommandSHT(int _command, int _dataPin, int _clockPin);
    void waitForResultSHT(int _dataPin);
    int getData16SHT(int _dataPin, int _clockPin);
    void skipCrcSHT(int _dataPin, int _clockPin);
};

#endif
