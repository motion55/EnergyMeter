/* @file CustomKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates changing the keypad size and key values.
|| #
*/
#include <EEPROM.h>
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {12, 11, 10, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

#ifndef USE_KEYPAD

#include "LiquidCrystal.h"
LiquidCrystal lcd(10, 9, 7, 6, 5, 4);

String password("12345678");

void setup(){
  Serial.begin(9600);
  lcd.begin(20, 4);
  lcd.setCursor(0,0);
  lcd.print(F("    Hello World    "));
}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    Serial.print(customKey);
    switch (customKey) {
    case 'A':
      if (GetPassword(password))
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F(" Password accepted. "));
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F(" Password rejected. "));
      }
      break;
    case 'B':
      break;
    case 'C':
      break;
    case 'D':
      break;
    }
  }
}
#endif

boolean CheckPassword(String passwd) 
{
  String guess;
  boolean result = false;
  lcd.clear();
  lcd.setCursor(0,0);
             //01234567890123456789
  lcd.print(F("   Enter the PIN.   "));
  lcd.setCursor(0,1);
  lcd.print(F("    then press #.   "));
#if 1  
  lcd.setCursor(0,3);
  lcd.print(Get_EEPROM_password());
#endif  
  lcd.setCursor(0,2);
  char done = 0;
  while (!done) {
    char customKey = customKeypad.getKey();
    if ((customKey>='0')&&(customKey<='9'))
    {
      guess += customKey;
      lcd.write('*');
      if (guess.length()>20) done = true;
    }
    else
    if (customKey=='#') 
    {
      done = true;
      if (guess == passwd) result = true;
    }
  }
  return result;
}

String GetNewPassword(void)
{
  String passwd;
  lcd.clear();
  lcd.setCursor(0,0);
             //01234567890123456789
  lcd.print(F("   Enter new PIN.   "));
  lcd.setCursor(0,1);
  lcd.print(F("    then press #.   "));
#if 1  
  lcd.setCursor(0,3);
  lcd.print(Get_EEPROM_password());
#endif  
  lcd.setCursor(0,2);
  char done = 0;
  while (!done) {
    char customKey = customKeypad.getKey();
    if (customKey==0) continue;
    if ((customKey>='0')&&(customKey<='9'))
    {
      passwd += customKey;
      lcd.write(customKey);
      if (passwd.length()>20) done = true;
    }
    else
    if (customKey=='#') 
    {
      done = true;
    }
    else
    {
      done = true;
      passwd = "";
    }
  }
  return passwd;
}

float GetWattHr(boolean bLoad)
{
  String sLoad;
  float fLoad = 0.0f;
  lcd.clear();
  lcd.setCursor(0,0);
               //01234567890123456789
  if (bLoad)
    lcd.print(F(" Enter WHr to load  "));
  else
    lcd.print(F(" Enter WHr to unload"));
  lcd.setCursor(0,1);
  lcd.print(F("    then press #.   "));
  lcd.setCursor(0,2);
  char done = 0;
  while (!done) {
    char customKey = customKeypad.getKey();
    if ((customKey>='0')&&(customKey<='9'))
    {
      sLoad += customKey;
      lcd.write(customKey);
      if (sLoad.length()>20) done = true;
    }
    else
    if (customKey>='#') 
    {
      done = true;
      fLoad = sLoad.toInt();
    }
  }
  return fLoad;
}

#define PASSWORD_ADDR 0

String Get_EEPROM_password(void)
{
  String passwd;
  int addr = PASSWORD_ADDR; 
  for (int i = 0; i<20; i++)
  {
    char Key = EEPROM.read(addr++);
    if ((Key>='0')&&(Key<='9'))
    {
      passwd += Key;
    }
    else break;
  }
  return passwd;
}

void Set_EEPROM_password(String passwd)
{
  int len = passwd.length();
  if (len>20) len = 20;
  int addr = PASSWORD_ADDR; 
  for (int i = 0; i<len; i++)
  {
    char Key = passwd[i];
    if ((Key>='0')&&(Key<='9'))
    {
      EEPROM.update(addr++,Key);
    }
    else 
    {
      EEPROM.update(addr,0);
      break;
    }
  }
}


