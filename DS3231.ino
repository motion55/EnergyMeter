
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

#include <Wire.h>
#include <RtcDS3231.h>

RtcDS3231<TwoWire> Rtc(Wire);

void DS3231_setup()
{
	Wire.begin();
	Rtc.Begin();

	RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

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

void DS3231_setTime(time_t t)
{
	RtcDateTime Time(t);
	Rtc.SetDateTime(Time);
}
