
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

  #define PA2_Serial  Serial
  #define USE_GSM     1
  #define USE_SDCARD  0
  #define USE_RTC     1
  #define USE_SMART   0
  #define USE_KEYPAD  1
  #define USE_REPORT  1

  #define PesoPerWatt 2.5f

#if USE_REPORT
  #define REPORT_HOUR 16
  #define REPORT_MIN  35
  #define REPORT_SEC  00

  char send_report;
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
  const int RelayPin = 8;
  boolean RelayON;
  float Credit_WattHr = 0.0f;
  float Prev_WattHr = 0.0f;
  
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
    PA2_Serial.begin(9600);
    PA2_Serial.write(0x02);
    PA2_Serial.print(F("M2"));
    PA2_Serial.write(0x03);
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
            if (WattHr>Prev_WattHr)
            {
              Credit_WattHr -= WattHr - Prev_WattHr;
            #if USE_RTC
              time_t t = Credit_WattHr;
              DS3231_saveAlarmOne(t);
            #endif
            }
            Prev_WattHr = WattHr;
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
              setTime(time_stamp);
            #if USE_RTC
              DS3231_setTime(time_stamp);
            #endif
            }
            else if(strstr(smsbuffer,"REPORT"))
            {
              send_report = 0xFF;  
              #if 1
                 lcd.setCursor(0,2);
                 lcd.print(stringOne);
                 lcd.print(F("     "));
               #endif
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
                  stringOne = "Loaded"+String(loadCredit);
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
                    stringOne = "New PIN:"+passnew;
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
//          else
//          if (time_not_set)
//          {
//            time_not_set = false;
//            setTime(time_stamp);
//          #if USE_RTC
//            DS3231_setTime(time_stamp);
//          #endif
//          }
          sms.DeleteSMS(pos); //after reading, delete SMS
        }  
      }
      else
      {
        if (send_report)
        {
          send_report = 0;
          char smsbuffer[160];
          String stringOne = "KW-Hr="+String(WattHr/1000.0f,5);
          float usage = WattHr * PesoPerWatt;
          stringOne += " Pesos="+String(usage,2);
          lcd.setCursor(0,2);
          lcd.print(F("Sending daily report..."));
          stringOne.toCharArray(smsbuffer,160);
          sms.SendSMS(phone_book[DEFAULT_NUMBER], smsbuffer);
          WattHr = 0.0f;
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
    if (Credit_WattHr>0)
    {
      digitalWrite(LED_BUILTIN,HIGH);
      digitalWrite(RelayPin,HIGH);
      if (RelayON!=true)
      {
        RelayON = true;
        LCDInit();
      }
    }
    else
    {
      Credit_WattHr = 0;
      digitalWrite(LED_BUILTIN,LOW);
      digitalWrite(RelayPin,LOW);
      if (RelayON!=false)
      {
        RelayON = false;
        LCDInit();
      }
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Total KWH ="));
    lcd.print(String(WattHr/1000.0f,5));

    lcd.setCursor(0,1);
    lcd.print(F("Credit KWH="));
    lcd.print(Credit_WattHr/1000.0f,5);
    
    lcd.setCursor(0,2);
#if 0
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
#endif    
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
  
  void LCD_PIN_reject(void)
  {
    lcd.clear();
    lcd.setCursor(0,2);
    lcd.print(F(" Password rejected. "));
    delay(2000);
  }

