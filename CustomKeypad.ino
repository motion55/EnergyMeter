/* @file CustomKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates changing the keypad size and key values.
|| #
*/
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
byte rowPins[ROWS] = {11, 12, A0, A1}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A2, A3, A4, A5}; //connect to the column pinouts of the keypad

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

boolean GetPassword(String passwd) 
{
  String guess;
  boolean result = false;
  lcd.clear();
  lcd.setCursor(0,0);
             //01234567890123456789
  lcd.print(F("   Enter the PIN.   "));
  lcd.setCursor(0,1);
  lcd.print(F("    then press #.   "));
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
    if (customKey>='#') 
    {
      done = true;
      if (guess == passwd) result = true;
    }
  }
  return result;
}

float GetWattHr(void)
{
  String sLoad;
  float fLoad = 0.0f;
             //01234567890123456789
  lcd.print(F(" Enter KWHr to load "));
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

