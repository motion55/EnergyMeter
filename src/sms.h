#ifndef _SMS_H_
#define _SMS_H_

#include "SIMCOM.h"
#include <Time.h>

class SMSGSM {
public:
	// SMS's methods
	char SendSMS(char *number_str, const char *message_str);
	char SendSMS(byte sim_phonebook_position, const char *message_str);
	char IsSMSPresent(byte required_status);
	char GetSMS(byte position, char *phone_number,byte max_phone_len,
				char *SMS_text, byte max_SMS_len, time_t *timestamp);
	inline char GetSMS(byte position, char *phone_number,byte max_phone_len, char *SMS_text, byte max_SMS_len)
	{
		return GetSMS(position, phone_number, max_phone_len, SMS_text, max_SMS_len, NULL);
	}
	char GetAuthorizedSMS(byte position, char *phone_number, byte max_phone_len,
				char *SMS_text, byte max_SMS_len,
				byte first_authorized_pos, byte last_authorized_pos);
	char DeleteSMS(byte position);
};

#endif

