//
// include the library code:
#include <SoftwareSerial.h>
#include "src/SIMCOM.h"
#include "src/sms.h"
SMSGSM sms;
int numdata;
boolean started=false;
#define SMS_TARGET "09297895641" //<-use your own number 

#include "LiquidCrystal.h"

#include <SPI.h>
#include <SD.h>

const int chipSelect = 8;
boolean CardPresent = 0;

/*The circuit:
  * LCD RS pin to digital pin 10
  * LCD R/W pin to GND
  * LCD Enable pin to digital pin 9
  * LCD D4 pin to digital pin 7
  * LCD D5 pin to digital pin 6
  * LCD D6 pin to digital pin 5
  * LCD D7 pin to digital pin 4
  * LCD VSS pin to ground
  * LCD VCC pin to 5V
  * 10K resistor:
  * ends to +5V and ground
  * wiper to LCD VO pin (pin 3)
  */
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(10, 9, 7, 6, 5, 4);

int i, j, k;
long previousMillis = 0;
const long interval = 3000;
long previousMillis2 = 0;
const long interval2 = 1000;

char data[100];

String sVrms = "";
String sIrms = "";
String sPreal = "";
String sVA = "";
String sQA = "";
String sQI = "";
String sPF = "";
String sTemp = "";
String sPFU = "";
String sPQF = "";
String sWattHr = "";
String sPH = "";
String sInterval = "";

float Vrms = 0.0f;
float Irms = 0.0f;
float Preal = 0.0f;
float VA = 0.0f;
float QA = 0.0f;
float QI = 0.0f;
float PF = 0.0f;
float Temp = 0.0f;
float PFU = 0.0f;
float PQF = 0.0f;
float WattHr = 0.0f;
float PH = 0.0f;
unsigned long Interval = 0;

#define PA2_Serial	Serial
#define USE_GSM 1


void setup() {
  
  // put your setup code here, to run once:
  if (!SD.begin(chipSelect)) 
  {
    CardPresent = false;
  }
  else 
  {
    CardPresent = true;
    String logStr(F("Card is available"));
    PrintToLog(logStr);
  }
  
  //SerialUSB.begin(9600);
  PA2_Serial.begin(9600);
  PA2_Serial.write(0x02);
  PA2_Serial.print("M2");
  PA2_Serial.write(0x03);
  i = 0;
  j = 0;
  k = 0;
  lcd.begin(20, 4);
  lcd.setCursor(0,0);
  lcd.print("  Power  Meter  ");
  lcd.setCursor(0,1);
  lcd.print("   Prototype   ");
#if USE_GSM  
  if (gsm.begin(9600))
  {
    started=true; 
    //Send a message to indicate successful connection
    sms.SendSMS(SMS_TARGET, "Power Meter is now online!");
  }
  else
  {
    PA2_Serial.println(F("Power Meter is offline!"));
  }
#endif  
  previousMillis = millis();
  previousMillis2 = millis();
}

#define C_STX 0x02
#define C_ETX 0x03

void ParseData()
{
  String sdata(data);
  
  switch (j) {
    case 0:  // Status
      break;
    case 1:  // Version
      break;
    case 2:  // Vrms
      sVrms = sdata;
      break;   
    case 3:  // Irms
      sIrms = sdata;
      break;   
    case 4:  // Preal
      sPreal = sdata;
      break;   
    case 5:
      sVA = sdata;
      break;   
    case 6:
      sQA = sdata;
      break;
    case 7:
      sQI = sdata;
      break;      
    case 8:
      sPF = sdata;
      break;   
    case 9:
      sTemp = sdata;
      break;   
    case 10:
      sPH = sdata;
      break;   
    case 11:
      sPFU = sdata;
      break;   
    case 12:
      sPQF = sdata;
      break;   
    case 13:
      sWattHr = sdata;
      break;   
    case 14:
      sInterval = sdata;
      break;   
  }
  i = 0;
  data[i] = 0;
  j++;
}

void loop() {
 // put your main code here, to run repeatedly:
  while(PA2_Serial.available())
  {
    char c = PA2_Serial.read();
    //SerialUSB.write(c);
    switch (c) {
      case C_STX:
        i = 0;
        j = 0;
        data[i] = 0;
        break;
      case ',':
        ParseData();
        break;  
      case C_ETX:
        ParseData();
        if (j==15)
        {
          Vrms = sVrms.toFloat();
          Irms = sIrms.toFloat();
          Preal = sPreal.toFloat();
          VA = sVA.toFloat();
          QA = sQA.toFloat();
          QI = sQI.toFloat();
          PF = sPF.toFloat();
          Temp = sTemp.toFloat();
          PFU = sPFU.toFloat();
          PQF = sPQF.toFloat();
          WattHr = sWattHr.toFloat();
          PH = sPH.toFloat();
          Interval = sInterval.toInt();
          //SerialUSB.print(F(" Vrms = "));
          //SerialUSB.print(Vrms);
          //SerialUSB.print(F(" Preal = "));
          //SerialUSB.print(Preal);
          //SerialUSB.print(F(" WattHr = "));
          //SerialUSB.println(WattHr);
        }
        break;  
      default:
        data[i++] = c;
        if (i>=sizeof(data)) i=0;
        data[i] = 0;
        break;      
    }
  } 
 
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;
    LCD_refresh();
  }
  else
  if (currentMillis - previousMillis2 > interval2)
  {
    previousMillis2 = currentMillis;
#if USE_GSM  
    SMS();
#endif    
  }
}
  
void SMS()
{
  if(started)
  {
    char pos = sms.IsSMSPresent(SMS_ALL);
    if(pos>0&&pos<=20)       //if message from 1-20 is found
    { 
      char smsbuffer[160];
      char phone_n[20];
      
      //Read if there are messages on SIM card and print them.
      if(sms.GetSMS(pos, phone_n, 20, smsbuffer, 160))
      {
        String stringOne;
        
        if(strstr(smsbuffer,"POWER"))
        {
          stringOne = "Preal="+String(Preal,1);
          stringOne.toCharArray(smsbuffer,160);
          sms.SendSMS(phone_n, smsbuffer );
        }
        else if(strstr(smsbuffer,"VOLTS"))
        {
          stringOne = "Vrms="+String(Vrms,1);
          stringOne.toCharArray(smsbuffer,160);
          sms.SendSMS(phone_n, smsbuffer );
        }
        else if(strstr(smsbuffer,"WATTHRS"))
        {
          stringOne = "Watt-Hr="+String(WattHr,1);
          stringOne.toCharArray(smsbuffer,160);
          sms.SendSMS(phone_n, smsbuffer );
        }
        sms.DeleteSMS(pos); //after reading, delete SMS
      }  
    }
  }
}

void LCD_refresh()
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("KW-Hr=");
    lcd.print(String(WattHr/1000,3));
    lcd.setCursor(0,1);
    lcd.print("Peso=");
    lcd.print(WattHr*11.0/1000,2);
    
    lcd.setCursor(0,2);
    switch (k) {
    default:
      k = 0;
    case 0:
      lcd.print("Vrms=");
      lcd.print(String(Vrms,1));
      lcd.setCursor(0,3);
      lcd.print("Irms=");
      lcd.print(String(Irms,2));
      break;
    case 1:
      lcd.print("Preal=");
      lcd.print(String(Preal,1));
      lcd.setCursor(0,3);
      lcd.print("PF=");
      lcd.print(String(PF,3));
      break;
    case 12:
    case 13:
      lcd.print("QI=");
      lcd.print(String(QI,3));
      lcd.setCursor(0,1);
      lcd.print("QA=");
      lcd.print(String(QA,3));
      break;
    case 14:
      lcd.print("VA=");
      lcd.print(String(VA,2));
      lcd.setCursor(0,1);
      lcd.print("Temp=");
      lcd.print(String(Temp,1));
      break;
    case 15:
      lcd.print("Pharm=");
      lcd.print(String(PH,3));
      break;
    }
    k++;
}

void PrintToLog(String logString)
{
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) 
  {
    dataFile.println(logString);
    dataFile.close();
  }
}

