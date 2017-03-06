  //
  // include the library code:
  #include "src/SIMCOM.h"
  #include <SoftwareSerial.h>
  #include "src/sms.h"
  SMSGSM sms;
  boolean started=false;
  #define DEFAULT_NUMBER  1 // 0 or 1
  #define SMS_TARGET0 "09065069294" //<-use your own number 
  #define SMS_TARGET1 "09297895641"
  #define SMS_TARGET2 "00000000000" //spare

  typedef char phone_number_t[14];
  phone_number_t phone_book[3] = { SMS_TARGET0, SMS_TARGET1, SMS_TARGET2 };
  
  #include "LiquidCrystal.h"

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
  const float peso_per_kw = 13.0f;
  
  #define PA2_Serial	Serial
  #define USE_GSM     1
  #define USE_SDCARD  0
  #define USE_RTC     1
  
#if USE_GSM  
  const int RX_pin = 2;
  const int TX_pin = 3;
  const int GSM_ON_pin = A3;
#endif  

#if USE_SDCARD 
  #include <SPI.h>
  #include <SD.h>

  const int chipSelect = 8;
  boolean card_present = false;
#endif  

  #include <Time.h>

  extern boolean time_not_set;
  
  char DateText[] = "02/22/2017 \0";
  //                 01234567890
  char TimeText[] = "00:00:00a\0";
  //                 123456789
  
  void setup() {
    
    // put your setup code here, to run once:
    //SerialUSB.begin(9600);
    PA2_Serial.begin(9600);
    PA2_Serial.write(0x02);
    PA2_Serial.print(F("M2"));
    PA2_Serial.write(0x03);
    i = 0;
    j = 0;
    k = 0;
    lcd.begin(20, 4);
    lcd.setCursor(0,0);
    lcd.print(F("    Energy Meter  "));
    lcd.setCursor(0,1);
    lcd.print(F("   Billing System   "));
    lcd.setCursor(0,2);
    lcd.print(F("  By Nery And Jasa   "));
    
  #if USE_RTC
    DS3231_setup();
  #endif    
    
  #if USE_SDCARD 
    lcd.setCursor(0,3);
    lcd.print(F(" Checking SD card..."));
    // see if the card is present and can be initialized:
    lcd.setCursor(0,3);
    if (SD.begin(chipSelect)) 
    {
      lcd.print(F("SD card initialized."));
      card_present = true;
      Save2Log(F("Log Started."));
    }
    else
    {
      lcd.print(F("SD card init failed."));
      card_present = false;
    }
    delay(1000);
  #endif  
#if USE_GSM  
    lcd.setCursor(0,3);
    lcd.print(F(" Initializing  GSM. "));
  #if defined(__AVR_ATmega328P__)
    gsm.SelectSoftwareSerial(RX_pin, TX_pin, GSM_ON_pin);
  #else
    gsm.SelectHardwareSerial(&Serial1, GSM_ON_pin);
  #endif
    if (gsm.begin(9600))
    {
      started=true;  
      //Send a message to indicate successful connection
      String hello(F("ADVISORY FROM NERY AND JASA'S THESIS PROJECT GSM MODULE: THIS MESSAGE IS TO INFORM YOU THAT NERY AND JASA'S POWER METER IS NOW WORKING. PLEASE DO NOT REPLY. AUTOMATED TXT ADVISORY FOR INQUIRIES TEXT OR CALL 09065069294"));
      sms.SendSMS(phone_book[DEFAULT_NUMBER], hello.c_str());
      //sms.SendSMS(SMS_TARGET1, hello.c_str());
      time_not_set = true;
      if (time_not_set) 
      {
        //Serial.println(F("Requesting load balance"));
        sms.SendSMS("222","BAL");
      }
    }
    else
    {
      PA2_Serial.println(F("Power Meter is offline!"));
      lcd.print(F("Initializing failed."));
      delay(3000);
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

      UpdateTime();
      
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
        time_t time_stamp;
        
        //Read if there are messages on SIM card and print them.
        if(sms.GetSMS(pos, phone_n, 20, smsbuffer, 160, &time_stamp))
        {
          if (CheckPhonebook(String(phone_n)))
          {
            String stringOne;
            String stringTwo;
            
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
            else if(strstr(smsbuffer,"BILL"))
            {
              stringOne = "BILL IN PHP="+String(WattHr*peso_per_kw/1000.0f,5);
              stringOne.toCharArray(smsbuffer,160);
              sms.SendSMS(phone_n, smsbuffer );
            }
            else if(strstr(smsbuffer,"KW"))
            {
              stringOne = "KW-Hr="+String(WattHr/1000.0f,5);
              stringOne.toCharArray(smsbuffer,160);
              
              sms.SendSMS(phone_n, smsbuffer );
            }
            else if(strstr(smsbuffer,"TIMESET"))
            {
              time_not_set = false;
              DS3231_setTime(time_stamp);
              setTime(time_stamp);
            }
          }
          else
          if (time_not_set)
          {
            time_not_set = false;
            DS3231_setTime(time_stamp);
            setTime(time_stamp);
          }
          sms.DeleteSMS(pos); //after reading, delete SMS
        }  
      }
    }
  }
  
  boolean CheckPhonebook(String number1)
  {
    for (int i=0; i<3; i++)
    {
      String number2(phone_book[i]);
      int len = number2.length() - 10;
      if (len>0)
      {
        number2 = number2.substring(len);
      }
      if (number1.endsWith(number2)) return true;
    }
    return false;
  }
  
  void LCD_refresh()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("KW-Hr="));
    lcd.print(String(WattHr/1000.0f,5));
    lcd.setCursor(0,1);
    lcd.print(F("Peso="));
    lcd.print(WattHr*peso_per_kw/1000.0f,5);
      
    lcd.setCursor(0,2);
    switch (k) {
    default:
      k = 0;
    case 0:
      lcd.print(F("Vrms="));
      lcd.print(String(Vrms,1));
      break;
    case 1:
      lcd.print(F("Irms="));
      lcd.print(String(Irms,2));
      break;
    case 2:
      lcd.print(F("Preal="));
      lcd.print(String(Preal,1));
      break;
    case 3:
      lcd.print(F("PF="));
      lcd.print(String(PF,3));
      break;
    }
    k++;
    UpdateTime();
  }

  void UpdateTime(void)
  {
    time_t tm = now();
      
    int hour = hourFormat12(tm);
    if (hour < 10)
    {
      TimeText[0] = ' ';
      TimeText[1] = '0' + hour;
    }
    else
    {
      TimeText[0] = '1';
      TimeText[1] = '0' + (hour - 10);
    }
  
    int min = minute(tm);
    int min10 = min / 10;
    TimeText[3] = '0' + min10;
    TimeText[4] = '0' + min - (min10 * 10);
  
    int sec = second(tm);
    int sec10 = sec / 10;
    TimeText[6] = '0' + sec10;
    TimeText[7] = '0' + sec - (sec10 * 10);
    
    if (isAM(tm))
    {
      TimeText[8] = 'a';
    }
    else
    {
      TimeText[8] = 'p';
    }
    
    int mon = month(tm);
    if (mon > 9)
    {
      DateText[0] = '1';
      mon -= 10;
    }
    else
    {
      DateText[0] = '0';
    }
    DateText[1] = '0' + mon;

    int dayOnes = day(tm);
    int dayTens = dayOnes / 10;
    dayOnes -= dayTens * 10;
    DateText[3] = '0' + dayTens;
    DateText[4] = '0' + dayOnes;

    String yearstr(year(tm));
    DateText[6] = yearstr.charAt(0);
    DateText[7] = yearstr.charAt(1);
    DateText[8] = yearstr.charAt(2);
    DateText[9] = yearstr.charAt(3);

    lcd.setCursor(0,3);
    lcd.print(DateText);
    lcd.setCursor(11,3);
    lcd.print(TimeText);
  }

  void Save2Log(String logString)
  {
  #if USE_SDCARD 
    if (card_present)
    {
      File dataFile = SD.open("logfile.txt", FILE_WRITE);
      // if the file is available, write to it:
      if (dataFile) 
      {
        dataFile.println(logString);
        dataFile.close();
      }
      else 
      {
        lcd.setCursor(0,3);
        lcd.print(F("Cannot open logfile."));
      }
    }
  #endif
  }

