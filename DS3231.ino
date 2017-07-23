
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <Wire.h>
#include <RtcDS3231.h>
#include <Time.h>
#include <TimeLib.h>

RtcDS3231<TwoWire> Rtc(Wire);

boolean time_not_set = false;

void DS3231_setup()
{
  Wire.begin();
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  time_not_set = false;

  if (!Rtc.IsDateTimeValid())
  {
    time_not_set = true;

    // Common Cuases:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing
    Serial.println(F("RTC lost confidence in the DateTime!"));

    // following line sets the RTC to the date & time this sketch was compiled
    // it will also reset the valid flag internally unless the Rtc device is
    // having an issue
    Rtc.SetDateTime(compiled);

    if (!Rtc.IsDateTimeValid())
    {
      // Common Cuases:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing
      Serial.println(F("RTC lost confidence in the DateTime!"));

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue
      Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
      Serial.println(F("RTC was not actively running, starting now"));
      Rtc.SetIsRunning(true);
    }
    
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled)
    {
      Serial.println(F("RTC is older than compile time!  (Updating DateTime)"));
      Rtc.SetDateTime(compiled);
    }
    else if (now > compiled)
    {
      Serial.println(F("RTC is newer than compile time. (this is expected)"));
    }
    else if (now == compiled)
    {
      Serial.println(F("RTC is the same as compile time! (not expected but all is fine)"));
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  }

  RtcDateTime now = Rtc.GetDateTime();
  time_t t = now.TotalSeconds();
  setTime(t);
}

void DS3231_setDateTime(uint16_t year, 
            uint8_t month,
            uint8_t day,
            uint8_t hour, 
            uint8_t minute, 
            uint8_t second)
{
  RtcDateTime Time(year, month, day, hour, minute, second);
  Rtc.SetDateTime(Time);
}

void DS3231_setTime(time_t t)
{
  RtcDateTime Time(t);
  Rtc.SetDateTime(Time);
}

void DS3231_saveAlarmOne(time_t t)
{
  RtcDateTime Time(t);
  DS3231AlarmOne alarm((int)0, Time.Hour(), Time.Minute(), Time.Second(), 0);
  Rtc.SetAlarmOne(alarm);
}

time_t DS3231_readAlarmOne(void)
{
  DS3231AlarmOne alarm = Rtc.GetAlarmOne();
  tmElements_t tm;
  tm.Second = alarm.Second();
  tm.Minute = alarm.Minute();
  tm.Hour = alarm.Hour();
  tm.Day = 1;
  tm.Month = 1;
  tm.Year = 0;
  return makeTime(tm);
}

void DS3231_saveAlarmTwo(time_t t)
{
  t *= 60;
  RtcDateTime Time(t);
  DS3231AlarmTwo alarm((int)0, Time.Hour(), Time.Minute(), 0);
  Rtc.SetAlarmTwo(alarm);
}

time_t DS3231_readAlarmTwo(void)
{
  DS3231AlarmTwo alarm = Rtc.GetAlarmTwo();
  tmElements_t tm;
  tm.Second = 0;
  tm.Minute = alarm.Minute();
  tm.Hour = alarm.Hour();
  tm.Day = 1;
  tm.Month = 1;
  tm.Year = 0;
  time_t t = makeTime(tm)/60;
  return t;
}

