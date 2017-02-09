#ifndef SIMCOM_H
#define SIMCOM_H

#include "GSM.h"


class SIMCOM : public virtual GSM {

private:
	int configandwait(char* pin);
	int setPIN(char *pin);
	int changeNSIPmode(char);

public:
	SIMCOM() {};
	~SIMCOM() {};
	static SIMCOM& GetInstance()
	{	
		static SIMCOM* s_instance;
		if (!s_instance) s_instance = new SIMCOM;
		return *s_instance;
	}
	SIMCOM(SIMCOM const&) = delete;
	void operator = (SIMCOM const&) = delete;

	int getCCI(char* cci);
	int getIMEI(char* imei);
	int sendSMS(const char* to, const char* msg);
	boolean readSMS(char* msg, int msglength, char* number, int nlength);
	boolean readCall(char* number, int nlength);
	boolean call(char* number, unsigned int milliseconds);
	char forceON();
	virtual int read(char* result, int resultlength);
	int readCellData(int &mcc, int &mnc, long &lac, long &cellid);
};

#define gsm (SIMCOM::GetInstance())

#define _GNSS_

#endif

