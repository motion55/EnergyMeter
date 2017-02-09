/*
ComPort.cpp
*/

// 
// Includes
// 
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <util/delay_basic.h>
#include "ComPort.h"

//
// Constructor
//

ComPort::ComPort()
{
	_SW_Serial = NULL;
	_HW_Serial = NULL;
}

//
// Destructor
//
ComPort::~ComPort()
{
	if (_SW_Serial != NULL)
	{
		delete _SW_Serial;
		_SW_Serial = NULL;
	}
}

int ComPort::peek()
{
	return 0;
}

//
// Public methods
//

void ComPort::SelectHardwareSerial(HardwareSerial* HW_Serial)
{
	if (HW_Serial!=NULL) _HW_Serial = HW_Serial;

	if (_SW_Serial != NULL)
	{
		delete _SW_Serial;
		_SW_Serial = NULL;
	}
}

void ComPort::SelectSoftwareSerial(uint8_t receivePin, uint8_t transmitPin)
{
	if (_SW_Serial == NULL)
	{
		_SW_Serial = new SoftwareSerial(receivePin, transmitPin);
	}
}

void ComPort::begin(long speed)
{
	if ((_SW_Serial == NULL) && (_HW_Serial == NULL))
	{
		SelectSoftwareSerial(_COMPORT_SS_RXPIN_, _COMPORT_SS_TXPIN_);
		#if defined(HAVE_HWSERIAL0)
		if (_SW_Serial == NULL) _HW_Serial = &Serial;
		#elif defined(HAVE_HWSERIAL1)
		if (_SW_Serial == NULL) _HW_Serial = &Serial1;
		#endif
	}

	if (_SW_Serial != NULL)
	{
		_SW_Serial->begin(speed);
	}
	else if (_HW_Serial != NULL)
	{
		_HW_Serial->begin(speed);
	}

}

size_t ComPort::write(uint8_t dat)
{
	if (_SW_Serial != NULL)
	{
		return _SW_Serial->write(dat);
	}
	else if (_HW_Serial != NULL)
	{
		return _HW_Serial->write(dat);
	}
	return 0;
}

int ComPort::read()
{
	if (_SW_Serial != NULL)
	{
		return _SW_Serial->read();
	}
	else if (_HW_Serial != NULL)
	{
		return _HW_Serial->read();
	}
	return 0;
}

int ComPort::available()
{
	if (_SW_Serial != NULL)
	{
		return _SW_Serial->available();
	}
	else if (_HW_Serial != NULL)
	{
		return _HW_Serial->available();
	}
	return 0;
}

void ComPort::flush()
{
	if (_SW_Serial != NULL)
	{
		_SW_Serial->flush();
	}
	else if (_HW_Serial != NULL)
	{
		_HW_Serial->flush();
	}
}

