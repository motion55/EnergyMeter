 
  //
  // include the library code:
  #include "src/SIMCOM.h"
  #include "src/sms.h"
  SMSGSM sms;
  boolean started=false;
  #define DEFAULT_NUMBER  0 // 0 or 1
  #define SMS_TARGET0 "09065069294" //<-use your own number 
  #define SMS_TARGET1 "09297895641"
  #define SMS_TARGET2 "00000000000" //spare

  #define USE_GSM     1
  #define USE_SDCARD  0
  #define USE_RTC     1
  #define USE_SMART   0
  #define USE_KEYPAD  1
  #define USE_REPORT  1
  
#if (defined(HAVE_HWSERIAL2)&&USE_GSM)
  #define PA2_Serial  Serial2
#elif defined(HAVE_HWSERIAL1) 
  #define PA2_Serial  Serial1
#else
  #define PA2_Serial  Serial
#endif

  #define PesoPerWatt 2.5f

#if USE_REPORT
  #include <EEPROM.h>

  #define REPORT_HOUR_ADDR 30
  #define REPORT_MIN_ADDR  31
  
  #define REPORT_HOUR 0
  #define REPORT_MIN  0
  #define REPORT_SEC  0

  char send_report = 0;
  char report_sent = 0;
  
  char ReportHours = REPORT_HOUR;
  char ReportMins = REPORT_MIN;
#endif
  
  typedef char phone_number_t[14];
  phone_number_t phone_book[3] = { SMS_TARGET0, SMS_TARGET1, SMS_TARGET2 };

#if USE_RTC
  #include <Wire.h>
  //#include <LiquidCrystal_I2C.h>
  #include <LiquidCrystal_PCF8574.h>
#else
  #include "LiquidCrystal.h"
#endif  
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
#if defined(LiquidCrystal_I2C_h)
  LiquidCrystal_I2C lcd(0x27,20,4);
#elif defined(LiquidCrystal_PCF8574_h)
  LiquidCrystal_PCF8574 lcd(0x27);
#else   
  LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);
#endif  
  int i, j, k;
  long previousMillis = 0;
  const long period1 = 5000;
  long previousMillis2 = 0;
  const long period2 = 1000;
  
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
  const int RelayPin = A0;
  boolean RelayON;
  float Total_WattHr = 0.0f;
  float Credit_WattHr = 0.0f;
  float Prev_WattHr = 0.0f;
  
#if USE_GSM  
  #ifndef HAVE_HWSERIAL1
  const int RX_pin = 2;
  const int TX_pin = 3;
  #endif
  const int GSM_ON_pin = A3;
#endif  

#if USE_SDCARD 
  #include <SPI.h>
  #include <SD.h>

  const int chipSelect = 8;
  boolean card_present = false;
#endif  

#if USE_KEYPAD
  #include <Keypad.h>
  extern Keypad customKeypad; 
  boolean Unlocked; 
  String password("1234567890");
  boolean CheckPassword(String passwd);
  String GetNewPassword(void);
  String Get_EEPROM_password(void);
  void Set_EEPROM_password(String passwd);
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
    i = 0;
    j = 0;
    k = 0;
    LCDInit();
    lcd.setCursor(0,0);
    lcd.print(F("    Energy Meter    "));
    lcd.setCursor(0,1);
    lcd.print(F("   Billing System   "));
    lcd.setCursor(0,2);
    lcd.print(F("  By Nery And Jasa  "));
    
  #if USE_RTC
    DS3231_setup();
    time_t t = DS3231_readAlarmOne();
    Credit_WattHr = t;    
    t = DS3231_readAlarmTwo();
    Total_WattHr = t;
  #endif    
  
  #if USE_KEYPAD
    Unlocked = false;
    String pw_eeprom = Get_EEPROM_password();
    if (pw_eeprom.length()>0)
    {
      password = pw_eeprom;    
    }
    Set_EEPROM_password(password);
  #endif    
  
  #if USE_REPORT
    unsigned char HourVal = EEPROM.read(REPORT_HOUR_ADDR);
    unsigned char MinVal = EEPROM.read(REPORT_MIN_ADDR);
    if ((HourVal<24)&&(MinVal<60))
    {
      ReportHours = HourVal;
      ReportMins = MinVal;
    }
    else
    {
      HourVal = REPORT_HOUR;
      MinVal = REPORT_MIN;
      EEPROM.update(REPORT_HOUR_ADDR, HourVal);
      EEPROM.update(REPORT_MIN_ADDR, MinVal);
    }
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
  #if defined(HAVE_HWSERIAL1)
    gsm.SelectHardwareSerial(&Serial1, GSM_ON_pin);
  #else
    gsm.SelectSoftwareSerial(RX_pin, TX_pin, GSM_ON_pin);
  #endif
    if (gsm.begin(9600))
    {
      started=true;  
      //Send a message to indicate successful connection
      String hello(\
F("ADVISORY FROM \
NERY AND JASA'S \
THESIS PROJECT \
GSM MODULE: THIS \
MESSAGE IS TO \
INFORM YOU THAT \
NERY AND JASA'S \
POWER METER IS \
NOW WORKING. \
PLEASE DO NOT \
REPLY. AUTOMATED \
TXT ADVISORY FOR \
INQUIRIES TEXT OR \
CALL 09065069294"));
      String stringOne(F("\r\n Report Time "));
      AddReportTime(stringOne);
      hello.concat(stringOne); 
      sms.SendSMS(phone_book[DEFAULT_NUMBER], hello.c_str());
      //sms.SendSMS(SMS_TARGET1, hello.c_str());
  #if USE_RTC
      time_not_set = false;
  #else
      time_not_set = true;
      if (time_not_set) 
      {
        //Serial.println(F("Requesting load balance"));
      #if USE_SMART
        sms.SendSMS("214","1515");
      #else  
        sms.SendSMS("222","BAL");
      #endif
      }
  #endif
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

    RelayON = false;
    pinMode(RelayPin, OUTPUT);
    digitalWrite(RelayPin,LOW);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    PA2_Serial.begin(9600);
    PA2_stop();
    PA2_reset();
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
    while (PA2_Serial.available()>0)
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
            if (WattHr>Prev_WattHr)
            {
              float delta = WattHr - Prev_WattHr;
              Credit_WattHr -= delta;
              Total_WattHr += delta;
            #if USE_RTC
              time_t t = 0;
              if (Credit_WattHr>0) 
                t = Credit_WattHr;
              else
                Credit_WattHr = 0.0f;
              DS3231_saveAlarmOne(t);
              t = Total_WattHr;
              DS3231_saveAlarmTwo(t);
            #endif
            }
            Prev_WattHr = WattHr;
            PA2_stop();
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
    if (currentMillis - previousMillis > period1)
    {
      previousMillis = currentMillis;
      LCD_refresh();
      PA2_start();
    }
    //else
    if (currentMillis - previousMillis2 > period2)
    {
      previousMillis2 = currentMillis;

      UpdateTime();
      
    #if USE_GSM  
      SMS();
    #endif    
    }
    
    #if USE_KEYPAD
    char customKey = customKeypad.getKey();
    
    if (customKey){
      switch (customKey) {
      case 'A':
        if (CheckPassword(password))
        {
          float newLoad = GetWattHr(true);
          Credit_WattHr += newLoad;
        #if USE_RTC
          if (Credit_WattHr>86000.0f) Credit_WattHr = 86000.0f;
          time_t t = Credit_WattHr;
          DS3231_saveAlarmOne(t);
        #endif
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0,3);
          lcd.print(F(" Password rejected. "));
          delay(2000);
        }
        break;
      case 'B':
        if (CheckPassword(password))
        {
          float newLoad = GetWattHr(false);
          Credit_WattHr -= newLoad;
          if (Credit_WattHr<0.0f) Credit_WattHr = 0.0f;
        #if USE_RTC
          time_t t = Credit_WattHr;
          DS3231_saveAlarmOne(t);
        #endif
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      case 'C':
        if (CheckPassword(password))
        {
          Credit_WattHr = 0.0f                ;
        #if USE_RTC
          time_t t = Credit_WattHr;
          DS3231_saveAlarmOne(t);
        #endif
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      case 'D':
        if (CheckPassword(password))
        {
          String passnew = GetNewPassword();
          if (passnew.length()>0)
          {
            password = passnew;
            Set_EEPROM_password(password);
          }
        }
        else
        {
          LCD_PIN_reject();
        }
        break;
      }
    }
    #endif
  }
    
  void LCDInit()
  {
  #if defined(LiquidCrystal_I2C_h)
    lcd.init();
    lcd.backlight();
  #elif defined(LiquidCrystal_PCF8574_h)
    lcd.begin(20,4);
    lcd.setBacklight(255);
  #else
    lcd.begin(20,4);
  #endif    
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
          if (CheckPhonebook(String(phone_n)))
          {
            if(strstr(smsbuffer,"DATE"))
            {
              char *pDateStr = strstr(smsbuffer,"DATE");
              pDateStr += 4; //move pointer to strring after "DATE"
              String sMonth(pDateStr);  //07/23
              if ((sMonth.length()>=5)&&(pDateStr[2]=='/'))
              {
                int MonthVal = sMonth.toInt();
                if ((MonthVal<1)||(MonthVal>12)) MonthVal = month();
                String sDay(pDateStr+3);
                int DayVal = sDay.toInt();
                if ((DayVal<1)||(DayVal>31)) DayVal = day();
                setTime(hour(),minute(),second(),DayVal,MonthVal,year());
                DS3231_setDateTime(year(),month(),day(),hour(),minute(),second());
              }
            }
            else if(strstr(smsbuffer,"TIME"))
            {
              char *pTimeStr = strstr(smsbuffer,"TIME");
              pTimeStr += 4; //move pointer to strring after "TIME"
              String sHour(pTimeStr);  //08:31
              if ((sHour.length()>=5)&&(pTimeStr[2]==':'))
              {
                int HourVal = sHour.toInt();
                if ((HourVal<0)||(HourVal>23)) HourVal = hour();
                String sMinute(pTimeStr+3);
                int MinVal = sMinute.toInt();
                if ((MinVal<0)||(MinVal>59)) MinVal = minute();
                setTime(HourVal,MinVal,second(),day(),month(),year());
                DS3231_setDateTime(year(),month(),day(),hour(),minute(),second());
              }
            }
            else if(strstr(smsbuffer,"BILL"))
            {
              String stringOne = "BILL IN PHP="+String(Total_WattHr*peso_per_kw/1000.0f,5);
              stringOne.toCharArray(smsbuffer,160);
              sms.SendSMS(phone_n, smsbuffer);
            }
            else if(strstr(smsbuffer,"KW"))
            {
              String stringOne = "KW-Hr="+String(Total_WattHr/1000.0f,5);
              stringOne.toCharArray(smsbuffer,160);
              sms.SendSMS(phone_n, smsbuffer);
            }
            else if(strstr(smsbuffer,"REPORT"))
            {
              char *pReport = strstr(smsbuffer,"REPORT");
            #if 1
              lcd.setCursor(0,2);
              lcd.print(pReport);
              delay(1000);
            #endif
              pReport += 6;
              String sHour(pReport);
              if (sHour.length()>=5)
              {
                if (sHour[2]==':')
                {
                  int tempHour = sHour.toInt();
                  pReport += 3;
                  String sMin(pReport);
                  int tempMin = sMin.toInt();
                  if ((tempHour<24)&&(tempMin<60))
                  {
                    ReportHours = tempHour;
                    ReportMins = tempMin;
                    String stringOne(F("Set Report Time "));
                    AddReportTime(stringOne);
                    stringOne.toCharArray(smsbuffer,160);
                    sms.SendSMS(phone_n, smsbuffer);
                    EEPROM.update(REPORT_HOUR_ADDR, ReportHours);
                    EEPROM.update(REPORT_MIN_ADDR, ReportMins);
                  }
                }
              }
              else
              {
                send_report = 0xFF;
              }
              report_sent = 0;
            }
            else 
            {
              char *pLOAD = strstr(smsbuffer,"LOAD");
              if(pLOAD)
              {
                pLOAD += 4;
                int loadCredit = String(pLOAD).toInt();
                if ((loadCredit>0)&&(loadCredit<10000))
                {
                  Credit_WattHr += loadCredit;
                  String stringOne = "Loaded"+String(loadCredit);
                #if 1
                  lcd.setCursor(0,2);
                  lcd.print(stringOne);
                  lcd.print(F("     "));
                #endif
                  stringOne.toCharArray(smsbuffer,160);
                  sms.SendSMS(phone_n, smsbuffer);
                #if USE_RTC
                  if (Credit_WattHr>86000.0f) Credit_WattHr = 86000.0f;
                  time_t t = Credit_WattHr;
                  DS3231_saveAlarmOne(t);
                #endif
                }
              }
              else
              {
                char *pPIN = strstr(smsbuffer,"PIN");
                if(pPIN)
                {
                  pPIN += 3;
                  String passnew(pPIN);
                  if (passnew.length()>0)
                  {
                    String stringOne = "New PIN:"+passnew;
                  #if 1
                    lcd.setCursor(0,2);
                    lcd.print(F("PIN was changed."));
                  #endif
                    stringOne.toCharArray(smsbuffer,160);
                    sms.SendSMS(phone_n, smsbuffer);
                    password = passnew;
                    Set_EEPROM_password(password);
                  }
                }
              }
            }
          }
          sms.DeleteSMS(pos); //after reading, delete SMS
        }  
      }
      else
      {
        if (send_report)
        {
          send_report = 0;
          report_sent = 0xFF;
          lcd.setCursor(0,2);
          lcd.print(F("Sending report..."));
          char smsbuffer[160];
          float usage = Total_WattHr * PesoPerWatt;
          String stringOne = String(DateText)+String(F("\r\n"))+String(TimeText)+String(F("\r\n"));
          String stringTwo = String(F("KW-Hr="))+String(Total_WattHr/1000.0f,5);
          String stringThree = String(F(" Pesos="))+String(usage,2);
          stringOne.concat(stringTwo);
          stringOne.concat(stringThree);
          stringOne.toCharArray(smsbuffer,160);
          sms.SendSMS(phone_book[DEFAULT_NUMBER],smsbuffer);
          Total_WattHr = 0.0f;
          time_t t = Total_WattHr;
          DS3231_saveAlarmTwo(t);
          PA2_reset();
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

  char bRestart;
  
  void LCD_refresh()
  {
    if (bRestart)
    {
      bRestart = 0;
      gsm.begin(9600);
      LCDInit();
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Total KWHr ="));
    lcd.print(String(Total_WattHr/1000.0f,5));

    lcd.setCursor(0,1);
    lcd.print(F("Credit KWHr="));
    lcd.print(Credit_WattHr/1000.0f,5);
    
    lcd.setCursor(0,2);
#if 0
    String stringOne(F("Report Time - "));
    AddReportTime(stringOne);
    lcd.print(stringOne);
#endif    
    UpdateTime();
    
    if (Credit_WattHr>0)
    {
      digitalWrite(LED_BUILTIN,HIGH);
      if (RelayON!=true)
      {
        RelayON = true;
        digitalWrite(RelayPin,HIGH);
        PA2_start();
        bRestart = 0xFF;
      }
    }
    else
    {
      Credit_WattHr = 0;
      digitalWrite(LED_BUILTIN,LOW);
      if (RelayON!=false)
      {
        RelayON = false;
        digitalWrite(RelayPin,LOW);
        bRestart = 0xFF;
      }
    }
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

    //String yearstr(year(tm));
    String yearstr(F("2017"));
    DateText[6] = yearstr.charAt(0);
    DateText[7] = yearstr.charAt(1);
    DateText[8] = yearstr.charAt(2);
    DateText[9] = yearstr.charAt(3);

    lcd.setCursor(0,3);
    lcd.print(DateText);
    lcd.setCursor(11,3);
    lcd.print(TimeText);

    DailyReport();
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
  
  void LCD_PIN_reject(void)
  {
    lcd.clear();
    lcd.setCursor(0,2);
    lcd.print(F(" Password rejected. "));
    delay(2000);
  }

  void DailyReport(void)
  {
    time_t tm = now();
    
    if (hour(tm)==ReportHours)
    {
      if (minute(tm)==ReportMins)
      {
        if (!report_sent) send_report = 0xFF;
      }
      else
      {
        report_sent = 0;
      }
    }
    else
    {
      report_sent = 0;
    }
  }

  void AddReportTime(String &stringOne)
  {
    if (ReportHours<10) 
    {
      stringOne.concat('0');
    }
    stringOne.concat(String((int)ReportHours));
    if (ReportMins<10)
    {
      stringOne.concat(F(":0"));
    }
    else
    {
      stringOne.concat(':');
    }
    stringOne.concat(String((int)ReportMins));
  }

  void PA2_start(void)
  {
    PA2_Serial.write(0x02);
    PA2_Serial.print(F("M2"));
    PA2_Serial.write(0x03);
  }
  
  void PA2_stop(void)
  {
    PA2_Serial.write(0x02);
    PA2_Serial.print(F("M3"));
    PA2_Serial.write(0x03);
  }
  
  void PA2_reset(void)
  {
    PA2_Serial.write(0x02);
    PA2_Serial.print(F("R"));
    PA2_Serial.write(0x03);
  }
  

