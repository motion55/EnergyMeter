/*
This is a Beta version.
last modified 18/08/2012.

This library is based on one developed by Arduino Labs
and it is modified to preserve the compability
with the Arduino's product.

The library is modified to use the GSM Shield,
developed by www.open-electronics.org
(http://www.open-electronics.org/arduino-gsm-shield/)
and based on SIM900 chip,
with the same commands of Arduino Shield,
based on QuectelM10 chip.
*/

#include "GSM.h"
//#include "WideTextFinder.h"

//De-comment this two lines below if you have the
//first version of GSM GPRS Shield
//#define _GSM_TXPIN_ 4
//#define _GSM_RXPIN_ 5

//De-comment this two lines below if you have the
//second version of GSM GPRS Shield
#define _GSM_TXPIN_ 2
#define _GSM_RXPIN_ 3

#ifdef WideTextFinder_h
GSM::GSM(): _tf(_cell, 10)
#else
GSM::GSM() 
#endif
{
	_status = IDLE;
	_GSM_ON = 0;
	_GSM_RESET = 0;
};

int GSM::begin(long baud_rate)
{
	// Set pin modes
	if (_GSM_ON > 0) pinMode(_GSM_ON, OUTPUT);
	if (_GSM_RESET > 0) pinMode(_GSM_RESET, OUTPUT);

	int response=-1;
	int cont=0;
	boolean noresp=true;
	boolean turnedON=false;
	SetCommLineStatus(CLS_ATCMD);
	_cell.begin(baud_rate);
	setStatus(IDLE);
	comm_buf = String();

	// if no-reply we turn to turn on the module
	for (cont=0; cont<4; cont++) 
	{
		if (AT_RESP_ERR_NO_RESP == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5)&&!turnedON) 
		{	//check power
			// there is no response => turn on the module
			#ifdef DEBUG_SERIAL
			DEBUG_SERIAL.println(F("DB:NO RESP"));
			#endif
			// generate turn on pulse
			if (_GSM_ON > 0)
			{
				digitalWrite(_GSM_ON, HIGH);
				delay(2000);
				digitalWrite(_GSM_ON, LOW);
				delay(8000);
			}
			else if (_GSM_RESET> 0)
			{
				digitalWrite(_GSM_RESET, HIGH);
				delay(2000);
				digitalWrite(_GSM_RESET, LOW);
				delay(8000);
			}
			WaitResp(1000, 1000);
		} 
		else 
		{
			#ifdef DEBUG_SERIAL
			DEBUG_SERIAL.println(F("DB: With RESP"));
			#endif
			WaitResp(1000, 1000);
			break;
		}
	}

	if (AT_RESP_OK == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5)) 
	{
		#ifdef DEBUG_SERIAL
		DEBUG_SERIAL.println(F("DB:CORRECT BR"));
		#endif
		turnedON=true;
		noresp=false;
	}

	if (AT_RESP_ERR_DIF_RESP == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5)&&!turnedON) 
	{		//check OK
		#ifdef DEBUG_SERIAL
		DEBUG_SERIAL.println(F("DB:AUTO BAUD RATE"));
		int prev_baud;
		#endif
		for (int i=0; i<8; i++) {
			switch (i) {
			case 0:
				_cell.begin(1200);
				#ifdef DEBUG_SERIAL
				prev_baud = 1200;
				#endif
				break;

			case 1:
				_cell.begin(2400);
				#ifdef DEBUG_SERIAL
				prev_baud = 2400;
				#endif
				break;

			case 2:
				_cell.begin(4800);
				#ifdef DEBUG_SERIAL
				prev_baud = 4800;
				#endif
				break;

			case 3:
				_cell.begin(9600);
				#ifdef DEBUG_SERIAL
				prev_baud = 9600;
				#endif
				break;

			case 4:
				_cell.begin(19200);
				#ifdef DEBUG_SERIAL
				prev_baud = 19200;
				#endif
				break;

			case 5:
				_cell.begin(38400);
				#ifdef DEBUG_SERIAL
				prev_baud = 38400;
				#endif
				break;

			case 6:
				_cell.begin(57600);
				#ifdef DEBUG_SERIAL
				prev_baud = 57600;
				#endif
				break;

			case 7:
				_cell.begin(115200);
				#ifdef DEBUG_SERIAL
				prev_baud = 115200;
				#endif
				break;

				// if nothing else matches, do the default
				// default is optional
			}

			delay(1000);

			if (AT_RESP_OK == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5)) 
			{
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.print(F("DB:FOUND PREV BR:"));
				DEBUG_SERIAL.println(prev_baud);
				#endif
				_cell.print("AT+IPR=");
				_cell.print(baud_rate);
				_cell.print(0x0d); // send <CR>
				delay(1000);
				_cell.begin(baud_rate);
				delay(1000);
				if (AT_RESP_OK == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5)) 
				{
					#ifdef DEBUG_SERIAL
					DEBUG_SERIAL.println(F("DB:OK BR"));
					#endif
					turnedON = true;
					break;
				}
			}
			#ifdef DEBUG_SERIAL
			DEBUG_SERIAL.println(F("DB:NO BR"));
			#endif
		}
		// communication line is not used yet = free
		SetCommLineStatus(CLS_FREE);
		// pointer is initialized to the first item of comm. buffer
	}

	if(noresp==true&&!turnedON) 
	{
		#ifdef DEBUG_SERIAL
		DEBUG_SERIAL.println(F("Trying to force the baud-rate to 9600\n"));
		#endif
		for (int i = 0; i < 8; i++) 
		{
			switch (i) {
			case 0:
				_cell.begin(1200);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("1200"));
				#endif
				break;

			case 1:
				_cell.begin(2400);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("2400"));
				#endif
				break;

			case 2:
				_cell.begin(4800);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("4800"));
				#endif
				break;

			case 3:
				_cell.begin(9600);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("9600"));
				#endif
				break;

			case 4:
				_cell.begin(19200);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("19200"));
				#endif
				break;

			case 5:
				_cell.begin(38400);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("38400"));
				#endif
				break;

			case 6:
				_cell.begin(57600);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("57600"));
				#endif
				break;

			case 7:
				_cell.begin(115200);
				#ifdef DEBUG_SERIAL
				DEBUG_SERIAL.println(F("115200"));
				#endif
				break;
			}

			//delay(1000);
			//_cell.print(F("AT+IPR=9600\r"));
			//delay(1000);
			//_cell.begin(9600);
			delay(1000);
			if (AT_RESP_OK == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5))
			{
				delay(100);
				if (AT_RESP_OK == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5))
				{
					#ifdef DEBUG_SERIAL
					DEBUG_SERIAL.println(F("DB:BR found"));
					#endif
					turnedON = true;
					break;
				}
			}
			delay(1000);
			WaitResp(1000, 1000);
		}

		if (!turnedON)
		{
			#ifdef DEBUG_SERIAL
			DEBUG_SERIAL.println(F("ERROR: SIM900 doesn't answer. Check power and serial pins in GSM.cpp"));
			#endif
			if (_GSM_ON > 0)
			{
				digitalWrite(_GSM_ON, HIGH);
				delay(1200);
				digitalWrite(_GSM_ON, LOW);
				delay(10000);
			}
			return 0;
		}
	}

	SetCommLineStatus(CLS_FREE);

	if(turnedON) 
	{
		WaitResp(50, 50);
		#ifdef DEBUG_SERIAL
		DEBUG_SERIAL.println(F("DB:InitParam(PARAM_SET_0)"));
		#endif
		InitParam(PARAM_SET_0);
		#ifdef DEBUG_SERIAL
		DEBUG_SERIAL.println(F("DB:InitParam(PARAM_SET_1)"));
		#endif
		InitParam(PARAM_SET_1);//configure the module
		Echo(0);               //enable AT echo
		#ifdef DEBUG_SERIAL
		DEBUG_SERIAL.println(F("DB:Final AT OK check"));
		#endif
		for (cont = 0; cont < 3; cont++)
		{
			if (AT_RESP_OK == SendATCmdWaitResp(str_at, 500, 100, str_ok, 5))
				break;
			WaitResp(100, 50);
		}
		setStatus(READY);
		return(1);

	}
	else 
	{
		//just to try to fix some problems with 115200 baudrate
		_cell.begin(115200);
		delay(1000);
		_cell.print("AT+IPR=");
		_cell.print(baud_rate);
		_cell.print(0x0d); // send <CR>
		return(0);
	}
}

void GSM::InitParam(byte group)
{
	switch (group) {
	case PARAM_SET_0:
		// check comm line
		//if (CLS_FREE != GetCommLineStatus()) return;

		SetCommLineStatus(CLS_ATCMD);
		// Reset to the factory settings
		SendATCmdWaitResp(F("AT&F"), 1000, 50, str_ok, 5);
		// switch off echo
		SendATCmdWaitResp(F("ATE0"), 500, 50, str_ok, 5);
		// setup fixed baud rate
		//SendATCmdWaitResp("AT+IPR=9600", 500, 50, str_ok, 5);
		// setup mode
		//SendATCmdWaitResp("AT#SELINT=1", 500, 50, str_ok, 5);
		// Switch ON User LED - just as signalization we are here
		//SendATCmdWaitResp("AT#GPIO=8,1,1", 500, 50, str_ok, 5);
		// Sets GPIO9 as an input = user button
		//SendATCmdWaitResp("AT#GPIO=9,0,0", 500, 50, str_ok, 5);
		// allow audio amplifier control
		//SendATCmdWaitResp("AT#GPIO=5,0,2", 500, 50, str_ok, 5);
		// Switch OFF User LED- just as signalization we are finished
		//SendATCmdWaitResp("AT#GPIO=8,0,1", 500, 50, str_ok, 5);
		SetCommLineStatus(CLS_FREE);
		break;

	case PARAM_SET_1:
		// check comm line
		//if (CLS_FREE != GetCommLineStatus()) return;
		SetCommLineStatus(CLS_ATCMD);
		// Request calling line identification
		SendATCmdWaitResp(F("AT+CLIP=1"), 500, 50, str_ok, 5);
		// Mobile Equipment Error Code
		SendATCmdWaitResp(F("AT+CMEE=0"), 500, 50, str_ok, 5);
		// Echo canceller enabled
		//SendATCmdWaitResp("AT#SHFEC=1", 500, 50, str_ok, 5);
		// Ringer tone select (0 to 32)
		//SendATCmdWaitResp("AT#SRS=26,0", 500, 50, str_ok, 5);
		// Microphone gain (0 to 7) - response here sometimes takes
		// more than 500msec. so 1000msec. is more safety
		//SendATCmdWaitResp("AT#HFMICG=7", 1000, 50, str_ok, 5);
		// set the SMS mode to text
		SendATCmdWaitResp(F("AT+CMGF=1"), 500, 50, str_ok, 5);
		// Auto answer after first ring enabled
		// auto answer is not used
		//SendATCmdWaitResp("ATS0=1", 500, 50, str_ok, 5);
		// select ringer path to handsfree
		//SendATCmdWaitResp("AT#SRP=1", 500, 50, str_ok, 5);
		// select ringer sound level
		//SendATCmdWaitResp("AT+CRSL=2", 500, 50, str_ok, 5);
		// we must release comm line because SetSpeakerVolume()
		// checks comm line if it is free
		SetCommLineStatus(CLS_FREE);
		// select speaker volume (0 to 14)
		//SetSpeakerVolume(9);
		// init SMS storage
		InitSMSMemory();
		// select phonebook memory storage
		SendATCmdWaitResp(F("AT+CPBS=\"SM\""), 1000, 50, str_ok, 5);
		SendATCmdWaitResp(F("AT+CIPSHUT"), 500, 50, F("SHUT OK"), 5);
		break;
	}
}

/**********************************************************/

byte GSM::WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
	byte status;

	RxInit(start_comm_tmout, max_interchar_tmout);
	// wait while response is not finished
	do {
		status = IsRxFinished();
	} while (status == RX_NOT_FINISHED);

	return (status);
}

byte GSM::WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
	const char *expected_resp_string)
{
	byte ret_val;
	byte status = WaitResp(start_comm_tmout, max_interchar_tmout);

	if (status == RX_FINISHED) 
	{
		// something was received but what was received?
		// ---------------------------------------------
		if(IsStringReceived(expected_resp_string)) 
		{
			// expected string was received
			// ----------------------------
			ret_val = RX_FINISHED_STR_RECV;
		} 
		else 
		{
			ret_val = RX_FINISHED_STR_NOT_RECV;
		}
	}
	else 
	{
		// nothing was received
		// --------------------
		ret_val = RX_TMOUT_ERR;
	}
	return (ret_val);
}

byte GSM::WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
	const __FlashStringHelper *expected_response)
{
	String expected_resp_string(expected_response);
	return WaitResp(start_comm_tmout, max_interchar_tmout, expected_resp_string.c_str());
}


/**********************************************************
Method sends AT command and waits for response

return:
	 AT_RESP_ERR_NO_RESP = -1,   // no response received
	 AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
	 AT_RESP_OK = 1,             // response_string was included in the response
**********************************************************/
char GSM::SendATCmdWaitResp(char const *AT_cmd_string,
					   uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
					   char const *response_string,
					   byte no_of_attempts)
{
	byte status;
	char ret_val = AT_RESP_ERR_NO_RESP;
	byte i;

	for (i = 0; i < no_of_attempts; i++) {
		// delay 500 msec. before sending next repeated AT command
		// so if we have no_of_attempts=1 tmout will not occurred
		if (i > 0) delay(500);

		_cell.println(AT_cmd_string);
		status = WaitResp(start_comm_tmout, max_interchar_tmout);
		if (status == RX_FINISHED) 
		{
			// something was received but what was received?
			// ---------------------------------------------
			if(IsStringReceived(response_string)>0) {
				ret_val = AT_RESP_OK;
				break;  // response is OK => finish
			} else ret_val = AT_RESP_ERR_DIF_RESP;
		} 
		else 
		{
			// nothing was received
			// --------------------
			ret_val = AT_RESP_ERR_NO_RESP;
		}
	}
	WaitResp(1000, 100);

	return (ret_val);
}


/**********************************************************
Method sends AT command and waits for response

return:
	 AT_RESP_ERR_NO_RESP = -1,   // no response received
	 AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
	 AT_RESP_OK = 1,             // response_string was included in the response
**********************************************************/
char GSM::SendATCmdWaitResp(const __FlashStringHelper *AT_cmd,
					   uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
					   const __FlashStringHelper *response,
					   byte no_of_attempts)
{
	byte status;
	char ret_val = AT_RESP_ERR_NO_RESP;
	byte i;

	for (i = 0; i < no_of_attempts; i++) 
	{
		// delay 500 msec. before sending next repeated AT command
		// so if we have no_of_attempts=1 tmout will not occurred
		if (i > 0) delay(500);

		_cell.println(AT_cmd);
		status = WaitResp(start_comm_tmout, max_interchar_tmout);
		if (status == RX_FINISHED) 
		{
			// something was received but what was received?
			// ---------------------------------------------
			String response_string(response);
		    if(IsStringReceived(response_string.c_str())>0) 
			{
				ret_val = AT_RESP_OK;
				break;  // response is OK => finish
			} else ret_val = AT_RESP_ERR_DIF_RESP;
		} 
		else 
		{
			// nothing was received
			// --------------------
			ret_val = AT_RESP_ERR_NO_RESP;
		}
	}

	return (ret_val);
}

byte GSM::IsRxFinished(void)
{
	byte ret_val = RX_NOT_FINISHED;  // default not finished

	// Rx state machine
	// ----------------
	if (rx_state == RX_NOT_STARTED) 
	{
		// Reception is not started yet - check tmout
		if (!_cell.available()) 
		{
			// still no character received => check timeout
			if ((unsigned long)(millis() - prev_time) >= start_reception_tmout) 
			{
				// timeout elapsed => GSM module didn't start with response
				// so communication is takes as finished
				comm_buf = String(NULL);
				ret_val = RX_TMOUT_ERR;
			}
		}
		else 
		{
			// at least one character received => so init inter-character
			// counting process again and go to the next state
			comm_buf = String(NULL);
			prev_time = millis();
			rx_state = RX_ALREADY_STARTED;
		}
		yield();
		return (ret_val);
	}

    if (rx_state == RX_ALREADY_STARTED) 
	{
		// Reception already started
		// check new received bytes
		// only in case we have place in the buffer
		// if there are some received bytes postpone the timeout

		// read all received bytes
		if ((unsigned long)(millis() - prev_time) <= interchar_tmout)
		{
			if (_cell.available() > 0)
			{
				do {
					char dat = _cell.read();
					if (comm_buf.length() < COMM_BUF_LEN)
					{
						comm_buf += dat;
					}
				} while (_cell.available() > 0);
				prev_time = millis();
			}
		}
		else
		{
			ret_val = RX_FINISHED;
		}
	}

	return (ret_val);
}

/**********************************************************
Method checks received bytes

compare_string - pointer to the string which should be find

return: 0 - string was NOT received
	   1 - string was received
**********************************************************/
byte GSM::IsStringReceived(const char *compare_string)
{
	char *ch;
	byte ret_val = 0;

	if(comm_buf.length()>0) 
	{
		#ifdef DEBUG_SERIAL
		DEBUG_SERIAL.print(F("ATT: "));
		DEBUG_SERIAL.write(compare_string);
		DEBUG_SERIAL.write("\r\n");
		DEBUG_SERIAL.print(F("BUF: "));
		DEBUG_SERIAL.write((const char*)comm_buf.c_str());
		DEBUG_SERIAL.write("\r\n");
		#endif
		ch = strstr((char*)comm_buf.c_str(), compare_string);
		if (ch != NULL) 
		{
			ret_val = 1;
			#ifdef DEBUG_SERIAL
			DEBUG_SERIAL.println(F("DEBUG: Expected string received"));
			#endif
		}
		else 
		{
			#ifdef DEBUG_SERIAL
			DEBUG_SERIAL.println(F("DEBUG: Expected string not received"));
			#endif
		}
	} 
	else 
	{
		#ifdef DEBUG_SERIAL
		if (rx_state == RX_ALREADY_STARTED)
		{
			DEBUG_SERIAL.print(F("ATT: "));
			DEBUG_SERIAL.println(compare_string);
			DEBUG_SERIAL.println(F("BUF: NO STRING RCVD"));
		}
		#endif
	}

	return (ret_val);
}

byte GSM::IsStringReceived(const __FlashStringHelper *compare_string)
{
	String compare_str(compare_string);
	return IsStringReceived(compare_str.c_str());
}

void GSM::RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
	_cell.flush(); // erase rx circular buffer
	rx_state = RX_NOT_STARTED;
	start_reception_tmout = start_comm_tmout;
	interchar_tmout = max_interchar_tmout;
	prev_time = millis();
	comm_buf = String();
}

void GSM::Echo(byte state)
{
	if (state == 0 or state == 1)
	{
		SetCommLineStatus(CLS_ATCMD);

		_cell.print(F("ATE"));
		_cell.print((int)state);
		_cell.print(0x0d);
		delay(500);
		SetCommLineStatus(CLS_FREE);
	}
}

char GSM::InitSMSMemory(void)
{
	char ret_val = -1;

	if (CLS_FREE != GetCommLineStatus()) return (ret_val);
	SetCommLineStatus(CLS_ATCMD);
	ret_val = 0; // not initialized yet

	// Disable messages about new SMS from the GSM module
	SendATCmdWaitResp(F("AT+CNMI=2,0"), 1000, 50, str_ok, 2);

	// send AT command to init memory for SMS in the SIM card
	// response:
	// +CPMS: <usedr>,<totalr>,<usedw>,<totalw>,<useds>,<totals>
	if (AT_RESP_OK == SendATCmdWaitResp(F("AT+CPMS=\"SM\",\"SM\",\"SM\""), 1000, 1000, F("+CPMS:"), 10)) {
		ret_val = 1;
	}	else ret_val = 0;

	SetCommLineStatus(CLS_FREE);
	return (ret_val);
}

int GSM::isIP(const char* cadena)
{
	int i;
	for (i=0; i<strlen(cadena); i++)
	{
		if (!(cadena[i]=='.' || ( cadena[i]>=48 && cadena[i] <=57)))
			return 0;
	}
	return 1;
}

/*/////////////////////////////////////////////////////////////////*/

int GSM::available()
{
	return _cell.available();
}

uint8_t GSM::read()
{
	return _cell.read();
}

void GSM::SimpleRead(Stream &_Serial)
{
	char datain;
	if (_cell.available() > 0) {
		datain = _cell.read();
		if (datain > 0) {
			_Serial.print(datain);
		}
	}
}

void GSM::SimpleWrite(char *comm)
{
	_cell.print(comm);
}

void GSM::SimpleWrite(const char *comm)
{
	_cell.print(comm);
}

void GSM::SimpleWrite(int comm)
{
	_cell.print(comm);
}

void GSM::SimpleWrite(const __FlashStringHelper *pgmstr)
{
	_cell.print(pgmstr);
}

void GSM::SimpleWriteln(char *comm)
{
	_cell.println(comm);
}

void GSM::SimpleWriteln(const __FlashStringHelper *pgmstr)
{
	_cell.println(pgmstr);
}

void GSM::SimpleWriteln(char const *comm)
{
	_cell.println(comm);
}

void GSM::SimpleWriteln(int comm)
{
	_cell.println(comm);
}

void GSM::WhileSimpleRead(Stream &_Serial)
{
	char datain;
	while (_cell.available() > 0) {
		datain = _cell.read();
		if (datain > 0) {
			_Serial.print(datain);
		}
	}
}

//---------------------------------------------
/**********************************************************
Turns on/off the speaker

off_on: 0 - off
1 - on
**********************************************************/
void GSM::SetSpeaker(byte off_on)
{
	if (CLS_FREE != GetCommLineStatus()) return;
	SetCommLineStatus(CLS_ATCMD);
	if (off_on) {
		//SendATCmdWaitResp("AT#GPIO=5,1,2", 500, 50, "#GPIO:", 1);
	}
	else {
		//SendATCmdWaitResp("AT#GPIO=5,0,2", 500, 50, "#GPIO:", 1);
	}
	SetCommLineStatus(CLS_FREE);
}


byte GSM::IsRegistered(void)
{
	return (module_status & STATUS_REGISTERED);
}

byte GSM::IsInitialized(void)
{
	return (module_status & STATUS_INITIALIZED);
}


/**********************************************************
Method checks if the GSM module is registered in the GSM net
- this method communicates directly with the GSM module
in contrast to the method IsRegistered() which reads the
flag from the module_status (this flag is set inside this method)

- must be called regularly - from 1sec. to cca. 10 sec.

return values:
REG_NOT_REGISTERED  - not registered
REG_REGISTERED      - GSM module is registered
REG_NO_RESPONSE     - GSM doesn't response
REG_COMM_LINE_BUSY  - comm line between GSM module and Arduino is not free
for communication
**********************************************************/
byte GSM::CheckRegistration(void)
{
	byte status;
	byte ret_val = REG_NOT_REGISTERED;

	if (CLS_FREE != GetCommLineStatus()) return (REG_COMM_LINE_BUSY);
	SetCommLineStatus(CLS_ATCMD);
	_cell.println(F("AT+CREG?"));
	// 5 sec. for initial comm tmout
	// 50 msec. for inter character timeout
	status = WaitResp(5000, 50);

	if (status == RX_FINISHED) {
		// something was received but what was received?
		// ---------------------------------------------
		if (IsStringReceived(F("+CREG: 0,1"))>0
			|| IsStringReceived(F("+CREG: 0,5"))>0) {
			// it means module is registered
			// ----------------------------
			module_status |= STATUS_REGISTERED;


			// in case GSM module is registered first time after reset
			// sets flag STATUS_INITIALIZED
			// it is used for sending some init commands which
			// must be sent only after registration
			// --------------------------------------------
			if (!IsInitialized()) {
				module_status |= STATUS_INITIALIZED;
				SetCommLineStatus(CLS_FREE);
				InitParam(PARAM_SET_1);
			}
			ret_val = REG_REGISTERED;
		}
		else {
			// NOT registered
			// --------------
			module_status &= ~STATUS_REGISTERED;
			ret_val = REG_NOT_REGISTERED;
		}
	}
	else {
		// nothing was received
		// --------------------
		ret_val = REG_NO_RESPONSE;
	}
	SetCommLineStatus(CLS_FREE);


	return (ret_val);
}


/**********************************************************
Method sets speaker volume

speaker_volume: volume in range 0..14

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module did not answer in timeout
-3 - GSM module has answered "ERROR" string

OK ret val:
-----------
0..14 current speaker volume
**********************************************************/
/*
char GSM::SetSpeakerVolume(byte speaker_volume)
{

char ret_val = -1;

if (CLS_FREE != GetCommLineStatus()) return (ret_val);
SetCommLineStatus(CLS_ATCMD);
// remember set value as last value
if (speaker_volume > 14) speaker_volume = 14;
// select speaker volume (0 to 14)
// AT+CLVL=X<CR>   X<0..14>
_cell.print("AT+CLVL=");
_cell.print((int)speaker_volume);
_cell.print("\r"); // send <CR>
// 10 sec. for initial comm tmout
// 50 msec. for inter character timeout
if (RX_TMOUT_ERR == WaitResp(10000, 50)) {
ret_val = -2; // ERROR
}
else {
if(IsStringReceived(str_ok)) {
last_speaker_volume = speaker_volume;
ret_val = last_speaker_volume; // OK
}
else ret_val = -3; // ERROR
}

SetCommLineStatus(CLS_FREE);
return (ret_val);
}
*/
/**********************************************************
Method increases speaker volume

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module did not answer in timeout
-3 - GSM module has answered "ERROR" string

OK ret val:
-----------
0..14 current speaker volume
**********************************************************/
/*
char GSM::IncSpeakerVolume(void)
{
char ret_val;
byte current_speaker_value;

current_speaker_value = last_speaker_volume;
if (current_speaker_value < 14) {
current_speaker_value++;
ret_val = SetSpeakerVolume(current_speaker_value);
}
else ret_val = 14;

return (ret_val);
}
*/
/**********************************************************
Method decreases speaker volume

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module did not answer in timeout
-3 - GSM module has answered "ERROR" string

OK ret val:
-----------
0..14 current speaker volume
**********************************************************/
/*
char GSM::DecSpeakerVolume(void)
{
char ret_val;
byte current_speaker_value;

current_speaker_value = last_speaker_volume;
if (current_speaker_value > 0) {
current_speaker_value--;
ret_val = SetSpeakerVolume(current_speaker_value);
}
else ret_val = 0;

return (ret_val);
}
*/

/**********************************************************
Method sends DTMF signal
This function only works when call is in progress

dtmf_tone: tone to send 0..15

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module didn't answer in timeout
-3 - GSM module has answered "ERROR" string

OK ret val:
-----------
0.. tone
**********************************************************/
/*
char GSM::SendDTMFSignal(byte dtmf_tone)
{
char ret_val = -1;

if (CLS_FREE != GetCommLineStatus()) return (ret_val);
SetCommLineStatus(CLS_ATCMD);
// e.g. AT+VTS=5<CR>
_cell.print("AT+VTS=");
_cell.print((int)dtmf_tone);
_cell.print("\r");
// 1 sec. for initial comm tmout
// 50 msec. for inter character timeout
if (RX_TMOUT_ERR == WaitResp(1000, 50)) {
ret_val = -2; // ERROR
}
else {
if(IsStringReceived(str_ok)) {
ret_val = dtmf_tone; // OK
}
else ret_val = -3; // ERROR
}

SetCommLineStatus(CLS_FREE);
return (ret_val);
}
*/

/**********************************************************
Method returns state of user button


return: 0 - not pushed = released
1 - pushed
**********************************************************/
byte GSM::IsUserButtonPushed(void)
{
	byte ret_val = 0;
	if (CLS_FREE != GetCommLineStatus()) return(0);
	SetCommLineStatus(CLS_ATCMD);
	//if (AT_RESP_OK == SendATCmdWaitResp("AT#GPIO=9,2", 500, 50, "#GPIO: 0,0", 1)) {
	// user button is pushed
	//  ret_val = 1;
	//}
	//else ret_val = 0;
	//SetCommLineStatus(CLS_FREE);
	//return (ret_val);
}



/**********************************************************
Method reads phone number string from specified SIM position

position:     SMS position <1..20>

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module didn't answer in timeout
-3 - position must be > 0
phone_number is empty string

OK ret val:
-----------
0 - there is no phone number on the position
1 - phone number was found
phone_number is filled by the phone number string finished by 0x00
so it is necessary to define string with at least
15 bytes(including also 0x00 termination character)

an example of usage:
GSM gsm;
char phone_num[20]; // array for the phone number string

if (1 == GSM::GetPhoneNumber(1, phone_num)) {
// valid phone number on SIM pos. #1
// phone number string is copied to the phone_num array
#ifdef DEBUG_PRINT
GSM::DebugPrint("DEBUG phone number: ", 0);
GSM::DebugPrint(phone_num, 1);
#endif
}
else {
// there is not valid phone number on the SIM pos.#1
#ifdef DEBUG_PRINT
GSM::DebugPrint("DEBUG there is no phone number", 1);
#endif
}
**********************************************************/


char GSM::GetPhoneNumber(byte position, char *phone_number)
{
	char ret_val = -1;

	char *p_char;
	char *p_char1;

	if (position == 0) return (-3);
	if (CLS_FREE != GetCommLineStatus()) return (ret_val);
	SetCommLineStatus(CLS_ATCMD);
	ret_val = 0; // not found yet
	phone_number[0] = 0; // phone number not found yet => empty string

						 //send "AT+CPBR=XY" - where XY = position
	_cell.print(F("AT+CPBR="));
	_cell.print((int)position);
	_cell.print(F("\r"));

	// 5000 msec. for initial comm tmout
	// 50 msec. for inter character timeout
	switch (WaitResp(5000, 50, F("+CPBR"))) {
	case RX_TMOUT_ERR:
		// response was not received in specific time
		ret_val = -2;
		break;

	case RX_FINISHED_STR_RECV:
		// response in case valid phone number stored:
		// <CR><LF>+CPBR: <index>,<number>,<type>,<text><CR><LF>
		// <CR><LF>OK<CR><LF>

		// response in case there is not phone number:
		// <CR><LF>OK<CR><LF>
		p_char = strstr((char *)(comm_buf.c_str()), ",\"");
		if (p_char != NULL) {
			p_char++;
			p_char++;       // we are on the first phone number character
							// find out '"' as finish character of phone number string
			p_char1 = strchr((char *)(p_char), '"');
			if (p_char1 != NULL) {
				*p_char1 = 0; // end of string
			}
			// extract phone number string
			strcpy(phone_number, (char *)(p_char));
			// output value = we have found out phone number string
			ret_val = 1;
		}
		break;

	case RX_FINISHED_STR_NOT_RECV:
		// only OK or ERROR => no phone number
		ret_val = 0;
		break;
	}

	SetCommLineStatus(CLS_FREE);
	return (ret_val);
}

/**********************************************************
Method writes phone number string to the specified SIM position

position:     SMS position <1..20>
phone_number: phone number string for the writing

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module didn't answer in timeout
-3 - position must be > 0

OK ret val:
-----------
0 - phone number was not written
1 - phone number was written
**********************************************************/
char GSM::WritePhoneNumber(byte position, char *phone_number)
{
	char ret_val = -1;

	if (position == 0) return (-3);
	if (CLS_FREE != GetCommLineStatus()) return (ret_val);
	SetCommLineStatus(CLS_ATCMD);
	ret_val = 0; // phone number was not written yet

				 //send: AT+CPBW=XY,"00420123456789"
				 // where XY = position,
				 //       "00420123456789" = phone number string
	_cell.print(F("AT+CPBW="));
	_cell.print((int)position);
	_cell.print(F(",\""));
	_cell.print(phone_number);
	_cell.print(F("\"\r"));

	// 5000 msec. for initial comm tmout
	// 50 msec. for inter character timeout
	switch (WaitResp(5000, 50, str_ok)) {
	case RX_TMOUT_ERR:
		// response was not received in specific time
		break;

	case RX_FINISHED_STR_RECV:
		// response is OK = has been written
		ret_val = 1;
		break;

	case RX_FINISHED_STR_NOT_RECV:
		// other response: e.g. ERROR
		break;
	}

	SetCommLineStatus(CLS_FREE);
	return (ret_val);
}


/**********************************************************
Method del phone number from the specified SIM position

position:     SMS position <1..20>

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module didn't answer in timeout
-3 - position must be > 0

OK ret val:
-----------
0 - phone number was not deleted
1 - phone number was deleted
**********************************************************/
char GSM::DelPhoneNumber(byte position)
{
	char ret_val = -1;

	if (position == 0) return (-3);
	if (CLS_FREE != GetCommLineStatus()) return (ret_val);
	SetCommLineStatus(CLS_ATCMD);
	ret_val = 0; // phone number was not written yet

	//send: AT+CPBW=XY
	// where XY = position
	_cell.print(F("AT+CPBW="));
	_cell.print((int)position);
	_cell.print(F("\r"));

	// 5000 msec. for initial comm tmout
	// 50 msec. for inter character timeout
	switch (WaitResp(5000, 50, str_ok)) {
	case RX_TMOUT_ERR:
		// response was not received in specific time
		break;

	case RX_FINISHED_STR_RECV:
		// response is OK = has been written
		ret_val = 1;
		break;

	case RX_FINISHED_STR_NOT_RECV:
		// other response: e.g. ERROR
		break;
	}

	SetCommLineStatus(CLS_FREE);
	return (ret_val);
}


/**********************************************************
Function compares specified phone number string
with phone number stored at the specified SIM position

position:       SMS position <1..20>
phone_number:   phone number string which should be compare

return:
ERROR ret. val:
---------------
-1 - comm. line to the GSM module is not free
-2 - GSM module didn't answer in timeout
-3 - position must be > 0

OK ret val:
-----------
0 - phone numbers are different
1 - phone numbers are the same


an example of usage:
if (1 == ComparePhoneNumber(1, "123456789")) {
// the phone num. "123456789" is stored on the SIM pos. #1
// phone number string is copied to the phone_num array
#ifdef DEBUG_PRINT
DebugPrint("DEBUG phone numbers are the same", 1);
#endif
}
else {
#ifdef DEBUG_PRINT
DebugPrint("DEBUG phone numbers are different", 1);
#endif
}
**********************************************************/
char GSM::ComparePhoneNumber(byte position, char *phone_number)
{
	char ret_val = -1;
	char sim_phone_number[20];


	ret_val = 0; // numbers are not the same so far
	if (position == 0) return (-3);
	if (1 == GetPhoneNumber(position, sim_phone_number)) {
		// there is a valid number at the spec. SIM position
		// -------------------------------------------------
		if (0 == strcmp(phone_number, sim_phone_number)) {
			// phone numbers are the same
			// --------------------------
			ret_val = 1;
		}
	}
	return (ret_val);
}

//-----------------------------------------------------
