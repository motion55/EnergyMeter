#include "inetGSM.h"
#define _GSM_CONNECTION_TOUT_ 5
#define _TCP_CONNECTION_TOUT_ 20
#define _GSM_DATA_TOUT_ 10

int InetGSM::httpGET(const char* server, int port, const char* path, char* result, int resultlength)
{
     boolean connected=false;
     int n_of_at=0;
     int length_write;
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     /*
     Status = ATTACHED.
     if(gsm.getStatus()!=GSM::ATTACHED)
       return 0;
     */
     while(n_of_at<3) {
          if(!connectTCP(server, port)) {
#ifdef DEBUG_SERIAL
			  DEBUG_SERIAL.println("DB:NOT CONN");
#endif
               n_of_at++;
          } else {
               connected=true;
               n_of_at=3;
          }
     }

     if(!connected) return 0;

     gsm.SimpleWrite(F("GET "));
     gsm.SimpleWrite(path);
     gsm.SimpleWrite(F(" HTTP/1.0\r\nHost: "));
     gsm.SimpleWrite(server);
     gsm.SimpleWrite(F("\r\n"));
     gsm.SimpleWrite(F("User-Agent: Arduino"));
     gsm.SimpleWrite(F("\r\n\r\n"));
     gsm.SimpleWrite(end_c);

     switch(gsm.WaitResp(10000, 10, F("SEND OK"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }


     delay(50);
#ifdef DEBUG_SERIAL
	 DEBUG_SERIAL.println("DB:SENT");
#endif
     int res = gsm.read(result, resultlength);

     //gsm.disconnectTCP();

     //int res=1;
     return res;
}

int InetGSM::httpPOST(const char* server, int port, const char* path, const char* parameters, char* result, int resultlength)
{
     boolean connected=false;
     int n_of_at=0;
     char itoaBuffer[8];
     int num_char;
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     while(n_of_at<3) {
          if(!connectTCP(server, port)) {
#ifdef DEBUG_SERIAL
			  DEBUG_SERIAL.println("DB:NOT CONN");
#endif
               n_of_at++;
          } else {
               connected=true;
               n_of_at=3;
          }
     }

     if(!connected) return 0;

     gsm.SimpleWrite(F("POST "));
     gsm.SimpleWrite(path);
     gsm.SimpleWrite(F(" HTTP/1.1\r\nHost: "));
     gsm.SimpleWrite(server);
     gsm.SimpleWrite(F("\r\n"));
     gsm.SimpleWrite(F("User-Agent: Arduino\r\n"));
     gsm.SimpleWrite(F("Content-Type: application/x-www-form-urlencoded\r\n"));
     gsm.SimpleWrite(F("Content-Length: "));
     itoa(strlen(parameters),itoaBuffer,10);
     gsm.SimpleWrite(itoaBuffer);
     gsm.SimpleWrite(F("\r\n\r\n"));
     gsm.SimpleWrite(parameters);
     gsm.SimpleWrite(F("\r\n\r\n"));
     gsm.SimpleWrite(end_c);

     switch(gsm.WaitResp(10000, 10, F("SEND OK"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }

     delay(50);
#ifdef DEBUG_SERIAL
	 DEBUG_SERIAL.println("DB:SENT");
#endif

     int res= gsm.read(result, resultlength);
     //gsm.disconnectTCP();
     return res;
}

int InetGSM::openmail(char* server, char* loginbase64, char* passbase64, char* from, char* to, char* subj)
{
     boolean connected=false;
     int n_of_at=0;
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     while(n_of_at<3) {
          if(!connectTCP(server, 25)) {
#ifdef DEBUG_SERIAL
			  DEBUG_SERIAL.println("DB:NOT CONN");
#endif
               n_of_at++;
          } else {
               connected=true;
               n_of_at=3;
          }
     }

     if(!connected) return 0;

     delay(100);
     gsm.SimpleWrite(F("HELO "));
     gsm.SimpleWrite(server);
     gsm.SimpleWrite(F("\n"));
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, F("OK"));
     if(!gsm.IsStringReceived("SEND OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);

     delay(100);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(F("AUTH LOGIN\n"));
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, F("OK"));
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);

     delay(100);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(loginbase64);
     gsm.SimpleWrite(F("\n"));
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, F("OK"));
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);

     delay(100);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(passbase64);
     gsm.SimpleWrite(F("\n"));
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, F("OK"));
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);


     delay(100);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(F("MAIL From: <"));
     gsm.SimpleWrite(from);
     gsm.SimpleWrite(F(">\n"));
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, F("OK"));
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100, F(""));

     delay(100);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(F("RCPT TO: <"));
     gsm.SimpleWrite(to);
     gsm.SimpleWrite(F(">\n"));
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, F("OK"));
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100, F(""));

     delay(100);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(F("Data\n"));
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, F("OK"));
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100, F(""));

     delay(100);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(F("Subject: "));
     gsm.SimpleWrite(subj);
     gsm.SimpleWrite(F("\n\n"));

     return 1;
}

int InetGSM::closemail()
{
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     gsm.SimpleWrite(F("\n.\n"));
     gsm.SimpleWrite(end_c);
     disconnectTCP();
     return 1;
}


int InetGSM::attachGPRS(char* domain, char* dom1, char* dom2)
{
     int i=0;
     delay(5000);

     //gsm._tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.
     gsm.WaitResp(50, 50);
     gsm.SimpleWriteln(F("AT+CIFSR"));
     if(gsm.WaitResp(5000, 50, F("ERROR"))!=RX_FINISHED_STR_RECV) {
		#ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("DB:ALREADY HAVE AN IP"));
		#endif
          gsm.SimpleWriteln(F("AT+CIPCLOSE"));
          gsm.WaitResp(5000, 50, F("ERROR"));
          delay(2000);
          gsm.SimpleWriteln(F("AT+CIPSERVER=0"));
          gsm.WaitResp(5000, 50, F("ERROR"));
          return 1;
     } else {

		#ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("DB:STARTING NEW CONNECTION"));
		#endif

          gsm.SimpleWriteln(F("AT+CIPSHUT"));

          switch(gsm.WaitResp(500, 50, F("SHUT OK"))) {

          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
		#ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("DB:SHUTTED OK"));
		#endif
          delay(1000);

          gsm.SimpleWrite(F("AT+CSTT=\""));
          gsm.SimpleWrite(domain);
          gsm.SimpleWrite(F("\",\""));
          gsm.SimpleWrite(dom1);
          gsm.SimpleWrite(F("\",\""));
          gsm.SimpleWrite(dom2);
          gsm.SimpleWrite(F("\"\r"));


          switch(gsm.WaitResp(500, 50, F("OK"))) {

          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
		#ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("DB:APN OK"));
		#endif
          delay(5000);

          gsm.SimpleWriteln(F("AT+CIICR"));

          switch(gsm.WaitResp(10000, 50, F("OK"))) {
          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
		#ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("DB:CONNECTION OK"));
		#endif

          delay(1000);


          gsm.SimpleWriteln(F("AT+CIFSR"));
          if(gsm.WaitResp(5000, 50, F("ERROR"))!=RX_FINISHED_STR_RECV) {
			#ifdef DEBUG_SERIAL
               DEBUG_SERIAL.println(F("DB:ASSIGNED AN IP"));
			#endif
               gsm.setStatus(gsm.ATTACHED);
               return 1;
          }
		#ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("DB:NO IP AFTER CONNECTION"));
		#endif
          return 0;
     }
}

int InetGSM::dettachGPRS()
{
     if (gsm.getStatus()==gsm.IDLE) return 0;

     //GPRS dettachment.
     gsm.SimpleWriteln(F("AT+CGATT=0"));
     if(gsm.WaitResp(5000, 50, F("OK"))!=RX_FINISHED_STR_NOT_RECV) {
          gsm.setStatus(gsm.ERROR);
          return 0;
     }
     delay(500);

     gsm.setStatus(gsm.READY);
     return 1;
}

int InetGSM::connectTCP(const char* server, int port)
{
     //Visit the remote TCP server.
     gsm.SimpleWrite(F("AT+CIPSTART=\"TCP\",\""));
     gsm.SimpleWrite(server);
     gsm.SimpleWrite(F("\","));
     gsm.SimpleWriteln(port);

     switch(gsm.WaitResp(1000, 200, F("OK"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
	#ifdef DEBUG_SERIAL
     DEBUG_SERIAL.println(F("DB:RECVD CMD"));
	#endif
     if (!gsm.IsStringReceived("CONNECT OK")) {
          switch(gsm.WaitResp(15000, 200, F("OK"))) {
          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
     }

	#ifdef DEBUG_SERIAL
     DEBUG_SERIAL.println(F("DB:OK TCP"));
	#endif

     delay(3000);
     gsm.SimpleWriteln(F("AT+CIPSEND"));
     switch(gsm.WaitResp(5000, 200, F(">"))) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }

	#ifdef DEBUG_SERIAL
     DEBUG_SERIAL.println(F("DB:>"));
	#endif
     delay(4000);
     return 1;
}

int InetGSM::disconnectTCP()
{
     //Status = TCPCONNECTEDCLIENT or TCPCONNECTEDSERVER.
     /*
     if ((getStatus()!=TCPCONNECTEDCLIENT)&&(getStatus()!=TCPCONNECTEDSERVER))
        return 0;
     */
     //gsm._tf.setTimeout(_GSM_CONNECTION_TOUT_);


     //_cell.flush();

     //Switch to AT mode.
     //_cell << "+++" << endl;

     //delay(200);

     //Close TCP client and deact.
     gsm.SimpleWriteln(F("AT+CIPCLOSE"));

     //If remote server close connection AT+QICLOSE generate ERROR
     /*if(gsm._tf.find("OK"))
     {
       if(getStatus()==TCPCONNECTEDCLIENT)
         gsm.setStatus(ATTACHED);
       else
         gsm.setStatus(TCPSERVERWAIT);
       return 1;
     }
     gsm.setStatus(ERROR);

     return 0;    */
     if(gsm.getStatus()==gsm.TCPCONNECTEDCLIENT)
          gsm.setStatus(gsm.ATTACHED);
     else
          gsm.setStatus(gsm.TCPSERVERWAIT);
     return 1;
}

int InetGSM::connectTCPServer(int port)
{
     /*
       if (getStatus()!=ATTACHED)
          return 0;
     */
     //gsm._tf.setTimeout(_GSM_CONNECTION_TOUT_);

     //_cell.flush();

     // Set port

     gsm.SimpleWrite(F("AT+CIPSERVER=1,"));
     gsm.SimpleWriteln(port);
     /*
       switch(gsm.WaitResp(5000, 50, "OK")){
     	case RX_TMOUT_ERR:
     		return 0;
     	break;
     	case RX_FINISHED_STR_NOT_RECV:
     		return 0;
     	break;
       }

       switch(gsm.WaitResp(5000, 50, "SERVER")){ //Try SERVER OK
     	case RX_TMOUT_ERR:
     		return 0;
     	break;
     	case RX_FINISHED_STR_NOT_RECV:
     		return 0;
     	break;
       }
     */
     //delay(200);

     return 1;

}

boolean InetGSM::connectedClient()
{
     /*
     if (getStatus()!=TCPSERVERWAIT)
        return 0;
     */

     gsm.SimpleWriteln(F("AT+CIPSTATUS"));
     // Alternative: AT+QISTAT, although it may be necessary to call an AT
     // command every second,which is not wise
     /*
     switch(gsm.WaitResp(1000, 200, "OK")){
     case RX_TMOUT_ERR:
     	return 0;
     break;
     case RX_FINISHED_STR_NOT_RECV:
     	return 0;
     break;
     }*/
     //gsm._tf.setTimeout(1);
     if(gsm.WaitResp(5000, 50, F("CONNECT OK"))!=RX_FINISHED_STR_RECV) {
          gsm.setStatus(gsm.TCPCONNECTEDSERVER);
          return true;
     } else
          return false;
}

