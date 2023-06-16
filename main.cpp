#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <string.h>
#define RELAY_PIN A0
#define UP '2'
#define DOWN '8'
#define LEFT '4'
#define RIGHT '6'
#define GO '5'
#define ON 1
#define OFF 0
#define cardNo_address 10
#define ID1_address 150
#define ID2_address 200
#define ID3_address 300
#define RST_PIN 9
#define SS_PIN 10
#define led A0
#define MAINID 20
#define MFRCID 21
#define PASSID 22
#define FACEID 23
#define OPTIONID 24
#define NEWPASSID1 25
#define NEWPASSID2 26
#define NEWCARDID1 30
#define DELETECARDID1 31
#define EDITCARDID 32

void mainMenu();
void faceMenu();
void passMenu();
void mfrcMenu();
void optionMenu();
void newCardMenu();
void newPass1Menu();
void deleteCardMenu();
void printWrongPass();
void printCorrectPass();
void editPassword();
unsigned char getSelect(unsigned char select, char key);
void menuIdControll(unsigned char *select, char customKey);
int writeStringToEEPROM(int addrOffset, const String &strToWrite);
int readStringFromEEPROM(int addrOffset, char *strToRead);
int writeCardToEEPROM(int addrOffset, unsigned char strToWrite[], unsigned char len);
int readCardFromEEPROM(int addrOffset, unsigned char *strToRead);
unsigned char checkMFRC();
void readMFRC(unsigned char select, char customKey);
void deleteDataEEPROM(int low_address, int high_address);
void deleteMFRC(unsigned char select, char customKey);
unsigned char writeDataEEPROM(unsigned char address, unsigned char data);
unsigned char readDataEEPROM(unsigned char address);

unsigned char UID[4];
unsigned char ID1[4] = {0x63, 0x9A, 0x15, 0x13}; // uid for user card
unsigned char ID2[4];
unsigned char ID3[4];
unsigned char cardNo = 1;
char initial_password[4];
char editpassword[4];
char newpassword[4];
unsigned char i = 0;
bool RF_FLAG = true;
unsigned char mode = 0;
String readData;
////////////////////////////////// KEYPAD ////////////////////////////////////
const byte ROWS = 4;               // four rows
const byte COLS = 4;               // four columns
byte rowPins[ROWS] = {2, 3, 4, 5}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9}; // connect to the column pinouts of the keypad
char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
uint8_t arrow[] = {
    B01000,
    B01100,
    B01110,
    B01111,
    B01111,
    B01110,
    B01100,
    B01000};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
////////////////////////////////// MENU ////////////////////////////////////
typedef struct menuDisplay
{
   struct menuDisplay *PRE;
   unsigned char menuID;
   struct menuDisplay *NEXT1;
   struct menuDisplay *NEXT2;
   struct menuDisplay *NEXT3;
} menuDisplay;
struct menuDisplay menu1, menu2, menu3, menu4, menu5, menu6, menu7, menu8, menu9, menu10;
menuDisplay *menuPtr = &menu1;
////////////////////////////////// PERIPHERIAL ////////////////////////////////////
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
MFRC522 mfrc522(SS_PIN, RST_PIN);

////////////////////////////////// FUNCTION ////////////////////////////////////
void mainMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("       WELCOME      "));
   lcd.setCursor(0, 1);
   lcd.print(F("  Face Recognition  "));
   lcd.setCursor(0, 2);
   lcd.print(F("  Password          "));
   lcd.setCursor(0, 3);
   lcd.print(F("  RFID Cards        "));
}
void faceMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("   FACE DECTECTION  "));
   lcd.setCursor(0, 1);
   lcd.print(F("  Detecting face... "));
   lcd.setCursor(0, 2);
   lcd.print(F("  Please Wait       "));
   lcd.setCursor(0, 3);
   lcd.print(F("                    "));
}
void passMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("       PASSWORD     "));
   lcd.setCursor(0, 1);
   lcd.print(F("  Enter Pass:       "));
   lcd.setCursor(0, 2);
   lcd.print(F("                    "));
   lcd.setCursor(0, 3);
   lcd.print(F("                    "));
}
void mfrcMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("        RFID        "));
   lcd.setCursor(0, 1);
   lcd.print(F("  Taking RF Card... "));
   lcd.setCursor(0, 2);
   lcd.print(F("  Put on your Card  "));
   lcd.setCursor(0, 3);
   lcd.print(F("                    "));
}
void optionMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("       OPTIONS      "));
   lcd.setCursor(0, 1);
   lcd.print(F("  Edit Password     "));
   lcd.setCursor(0, 2);
   lcd.print(F("  Edit RFID Cards   "));
   lcd.setCursor(0, 3);
   lcd.print(F("  Display           "));
}
void editCardMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("      EDIT CARD     "));
   lcd.setCursor(0, 1);
   lcd.print(F("  Add a New Card ID "));
   lcd.setCursor(0, 2);
   lcd.print(F("  Delete a Card ID  "));
   lcd.setCursor(0, 3);
   lcd.print(F("  None              "));
}
void newPass1Menu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("       NEW PASS     "));
   lcd.setCursor(0, 1);
   lcd.print(F("  Old Pass   :      "));
   lcd.setCursor(0, 2);
   lcd.print(F("  New Pass   :      "));
   lcd.setCursor(0, 3);
   lcd.print(F("  Enter Again:      "));
}
void printWrongPass()
{
   lcd.setCursor(0, 0);
   lcd.print(F("       NEW PASS     "));
   lcd.setCursor(0, 1);
   lcd.print(F(" Enter Wrong Pass!  "));
   lcd.setCursor(0, 2);
   lcd.print(F(" Fail to change Pass"));
   lcd.setCursor(0, 3);
   lcd.print(F("                    "));
}
void printCorrectPass()
{
   lcd.setCursor(0, 0);
   lcd.print(F("       NEW PASS     "));
   lcd.setCursor(0, 1);
   lcd.print(F(" Change Password    "));
   lcd.setCursor(0, 2);
   lcd.print(F(" Successfully !!!   "));
   lcd.setCursor(0, 3);
   lcd.print(F("                    "));
}
void newCardMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("       NEW CARD     "));
   lcd.setCursor(0, 1);
   lcd.print(F(" Scanning Card...   "));
   lcd.setCursor(0, 2);
   lcd.print(F(" Send in ur Card    "));
   lcd.setCursor(0, 3);
   lcd.print(F("                    "));
}
void deleteCardMenu()
{
   lcd.setCursor(0, 0);
   lcd.print(F("     DELETE CARD    "));
   lcd.setCursor(0, 1);
   lcd.print(F(" Scanning Card...   "));
   lcd.setCursor(0, 2);
   lcd.print(F(" Send in ur Card    "));
   lcd.setCursor(0, 3);
   lcd.print(F("                    "));
}
void menuIdControll(unsigned char *select, char customKey)
{
   unsigned char point = *select;
   switch (menuPtr->menuID)
   {
   case MAINID:
      mainMenu();
      break;
   case FACEID:
      faceMenu();
      break;
   case PASSID:
      passMenu();
      break;
   case MFRCID:
      mfrcMenu();
      break;
   case OPTIONID:
      optionMenu();
      break;
   case NEWPASSID1:
      newPass1Menu();
      break;
   case NEWCARDID1:
      newCardMenu();
      break;
   case DELETECARDID1:
      deleteCardMenu();
      break;
   case EDITCARDID:
      editCardMenu();
      break;
   default:
      break;
   }
   *select = getSelect(point, customKey);
}

void setup()
{
   strncpy(initial_password, "2356", sizeof(initial_password));
   initial_password[5] = '\0';
   lcd.init(); // initialize the lcd
   lcd.backlight();
   lcd.createChar(0, arrow);
   lcd.home();
   Serial.begin(9600);
   SPI.begin();
   mfrc522.PCD_Init();
   pinMode(led, OUTPUT);
   digitalWrite(led, LOW);
   menu1 = {
       NULL,
       MAINID,
       &menu2,
       &menu3,
       &menu4,
   };
   menu2 = {
       &menu1,
       FACEID,
       NULL,
       NULL,
       NULL,
   };
   menu3 = {
       &menu1,
       PASSID,
       NULL,
       NULL,
       NULL,
   };
   menu4 = {
       &menu1,
       MFRCID,
       NULL,
       NULL,
       NULL,
   };
   menu5 = {
       &menu1,
       OPTIONID,
       &menu6,
       &menu7,
       &menu8,
   };
   menu6 = {
       &menu5,
       NEWPASSID1,
       NULL,
       NULL,
       NULL,
   };
   menu7 = {
       &menu5,
       EDITCARDID,
       &menu8,
       &menu9,
       NULL,
   };
   menu8 = {
       &menu7,
       NEWCARDID1,
       NULL,
       NULL,
       NULL,
   };
   menu9 = {
       &menu7,
       DELETECARDID1,
       NULL,
       NULL,
       NULL,
   };
}

void loop()
{
   unsigned char select = 1;
   signed char result = 1;
   char password[4];
   menuIdControll(&select, ' ');
   while (1)
   {
      /////////////////////////////////////////// LOG IN MODE PASSWORD ///////////////////////////////////////////////
      char customKey = customKeypad.getKey();
      if (mode == 1)
      { // LOG IN
         if (customKey >= '0' && customKey <= '9')
         {
            lcd.setCursor(14 + i, 1);
            password[i] = customKey;
            lcd.print("*");
            i++;
            if (i == 4)
            {
               lcd.setCursor(0, 2);
               if (password[0] == initial_password[0] && password[1] == initial_password[1] && password[2] == initial_password[2] && password[3] == initial_password[3])
                  result = 1;
               else
                  result = 0;
               lcd.setCursor(0, 2);
               if (result == 1)
               {
                  lcd.print(F("  Successful Login"));
                  lcd.setCursor(0, 3);
                  lcd.print(F("  Opening Door... "));
                  digitalWrite(led, RF_FLAG);
                  delay(2000);
                  digitalWrite(led, !RF_FLAG);
                  mode = 0;
                  menuPtr = &menu5;
                  // menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
                  menuIdControll(&select, customKey);
               }
               else
               {
                  lcd.print(F("  Fail to Login   "));
                  lcd.setCursor(0, 3);
                  lcd.print(F("  Wrong Pass!!!   "));
                  delay(1000);
                  lcd.setCursor(14, 1);
                  lcd.print(F("    "));
                  for (int j = 3; j >= 0; j--)
                  {
                     password[j] = '\0';
                  }
               }
               i = 0;
            }
         }
         customKey = '*';
      }
      while (mode == 3)
      {
         Serial.println("face recognition...");
         Serial.println(mode);
         if (Serial.available() > 0)
         {
            readData = Serial.readStringUntil('\n');
            if (readData == "on")
            {
               digitalWrite(led, HIGH);
               delay(1000);
               digitalWrite(led, LOW);
            }
         }
      }
      /////////////////////////////////////////// LOG IN MODE RFID ///////////////////////////////////////////////
      while (mode == 2)
      {
         bool error = true;
         if (!mfrc522.PICC_IsNewCardPresent())
         {
            return;
         }
         if (!mfrc522.PICC_ReadCardSerial())
         {
            return;
         }
         for (unsigned char id_count = 0; id_count < mfrc522.uid.size; id_count++)
         {
            UID[id_count] = mfrc522.uid.uidByte[id_count];
         }
         mfrc522.PICC_HaltA();
         mfrc522.PCD_StopCrypto1();
         lcd.setCursor(0, 2);
         if (UID[i] == ID1[i])
         {
            error = false;
         }
         else
         {
            if (UID[i] == ID2[i])
            {
               error = false;
            }
            else
            {
               if (UID[i] == ID3[i])
               {
                  error = false;
               }
               else
               {
                  error = true;
               }
            }
         }
         if (error == false)
         {
            lcd.print(F("  Successful Login"));
            lcd.setCursor(0, 3);
            lcd.print(F("  Opening Door... "));
            digitalWrite(led, RF_FLAG);
            delay(2000);
            digitalWrite(led, !RF_FLAG);
            mode = 0;
            menuPtr = &menu5;
            menuIdControll(&select, customKey);
         }
         else
         {
            lcd.print(F("  Fail to Login   "));
            lcd.setCursor(0, 3);
            lcd.print(F("  Wrong Card!!!   "));
            delay(1000);
            mode = 0;
            menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
            menuIdControll(&select, customKey);
         }
      }
      /////////////////////////////////////////// CONTROLLING KEYPAD///////////////////////////////////////////////
      if (customKey == UP && mode == 0)
      { // GO UP
         select = getSelect(select, customKey);
      }
      if (customKey == DOWN && mode == 0)
      { // GO DOWN
         select = getSelect(select, customKey);
      }
      if (customKey == RIGHT && mode == 0)
      { // GO TO NEXT MENU
         switch (select)
         {
         case 1:
            if (menuPtr->menuID == MAINID)
            {
               menuPtr = (menuPtr->NEXT1 != NULL) ? (menuPtr->NEXT1) : (menuPtr); // FACE
               mode = 3;
            }
            else if (menuPtr->menuID == OPTIONID || menuPtr->menuID == EDITCARDID)
            {
               menuPtr = (menuPtr->NEXT1 != NULL) ? (menuPtr->NEXT1) : (menuPtr);
            }
            break;
         case 2:
            if (menuPtr->menuID == MAINID)
            {
               menuPtr = (menuPtr->NEXT2 != NULL) ? (menuPtr->NEXT2) : (menuPtr); // PASS mode 1
               mode = 1;
            }
            else if (menuPtr->menuID == OPTIONID || menuPtr->menuID == EDITCARDID)
            {
               menuPtr = (menuPtr->NEXT2 != NULL) ? (menuPtr->NEXT2) : (menuPtr);
            }
            break;
         case 3:
            if (menuPtr->menuID == MAINID)
            {
               menuPtr = (menuPtr->NEXT3 != NULL) ? (menuPtr->NEXT3) : (menuPtr); // RFID mode 2
               mode = 2;
            }
            else if (menuPtr->menuID == OPTIONID || menuPtr->menuID == EDITCARDID)
            {
               menuPtr = (menuPtr->NEXT3 != NULL) ? (menuPtr->NEXT3) : (menuPtr);
            }
            break;
         }
         menuIdControll(&select, customKey);
      }
      if (customKey == LEFT && mode == 0)
      { // GO TO PREVIOUS MENU
         menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
         menuIdControll(&select, customKey);
      }
      if (customKey == GO && mode == 0)
      {
         switch (select)
         {
         case 0:
            break;
         case 1:
            break;
         case 2:
            break;
         case 3:
            break;
         }
      }
      switch (menuPtr->menuID)
      {
      case NEWPASSID1:
         editPassword();
         break;
      case NEWCARDID1:
         readMFRC(select, customKey);
         break;
      case DELETECARDID1:
         deleteMFRC(select, customKey);
         break;
      default:
         break;
      }
      customKey = '*';
   }
}

unsigned char getSelect(unsigned char select, char key)
{
   unsigned char point = select;
   if (key == UP)
   {
      lcd.setCursor(0, point);
      lcd.print(' ');
      point = (point == 1) ? (3) : (point - 1);
   }
   else if (key == DOWN)
   {
      lcd.setCursor(0, point);
      lcd.print(' ');
      point = (point == 3) ? (1) : (point + 1);
   }
   lcd.setCursor(0, point);
   lcd.write(0);
   return point;
}

void editPassword()
{
   unsigned char select = 1;
   String pass;
   bool lock = true;
   int error = 0;
   while (lock == true)
   {
      char customKey = customKeypad.getKey();
      if (customKey >= '0' && customKey <= '9')
      {
         switch (select)
         {
         case 1:
            lcd.setCursor(15 + i, 1);
            editpassword[i] = customKey;
            lcd.print(editpassword[i]);
            i++;
            break;
         case 2:
            lcd.setCursor(15 + i, 2);
            newpassword[i] = customKey;
            lcd.print("*");
            i++;
            break;
         case 3:
            lcd.setCursor(15 + i, 3);
            editpassword[i] = customKey;
            lcd.print("*");
            i++;
            break;
         }
      }
      if (i == 4)
      {
         switch (select)
         {
         case 1:
            if (editpassword[0] != initial_password[0] || editpassword[1] != initial_password[1] || editpassword[2] != initial_password[2] || editpassword[3] != initial_password[3])
            {
               error++;
            }
            select++;
            i = 0;
            break;
         case 2:
            select++;
            i = 0;
            break;
         case 3:
            if (error != 1)
            {
               if (editpassword[0] == newpassword[0] && editpassword[1] == newpassword[1] && editpassword[2] == newpassword[2] && editpassword[3] == newpassword[3])
               {
                  printCorrectPass(); // CORRECT CHANGE PASS
                  newpassword[4] = '\0';
                  pass = String(newpassword);
                  writeStringToEEPROM(100, pass);
                  readStringFromEEPROM(100, initial_password);
                  delay(2000);
                  menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
                  menuIdControll(&select, customKey);
               }
               else
               {
                  printWrongPass();
                  delay(2000);
                  menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
                  menuIdControll(&select, customKey);
               }
            }
            else
            {
               printWrongPass();
               delay(2000);
               menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
               menuIdControll(&select, customKey);
            }
            i = 0;
            return;
         }
      }
      customKey = '*';
   }
}

int writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
   byte len = strToWrite.length();
   EEPROM.write(addrOffset, len);
   for (int i = 0; i < len; i++)
   {
      EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
   }
   return addrOffset + 1 + len;
}
int readStringFromEEPROM(int addrOffset, char *strToRead)
{
   int newStrLen = EEPROM.read(addrOffset);
   char data[newStrLen + 1];
   for (int i = 0; i < newStrLen; i++)
   {
      data[i] = EEPROM.read(addrOffset + 1 + i);
   }
   data[newStrLen] = '\0';
   strncpy(strToRead, data, newStrLen);
   strToRead[newStrLen] = '\0';
   return addrOffset + 1 + newStrLen;
}
int writeCardToEEPROM(int addrOffset, unsigned char strToWrite[], unsigned char len)
{
   EEPROM.write(addrOffset, len);
   for (int i = 0; i < len; i++)
   {
      EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
   }
   return addrOffset + 1 + len;
}
int readCardFromEEPROM(int addrOffset, unsigned char *strToRead)
{
   int newStrLen = EEPROM.read(addrOffset);
   for (int i = 0; i < newStrLen; i++)
   {
      strToRead[i] = EEPROM.read(addrOffset + 1 + i);
   }
   return addrOffset + 1 + newStrLen;
}

unsigned char checkMFRC()
{

   if (!mfrc522.PICC_IsNewCardPresent())
   {
      return -1;
   }

   if (!mfrc522.PICC_ReadCardSerial())
   {
      return -1;
   }
   return 1;
}

void readMFRC(unsigned char select, char customKey)
{
   bool lock = true;
   while (lock == true)
   {
      while (checkMFRC() == 1)
      {
         for (byte i = 0; i < mfrc522.uid.size; i++)
         {
            UID[i] = mfrc522.uid.uidByte[i];
            Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? F(" 0") : F(" "));
            Serial.print(UID[i], HEX);
            Serial.println(" ");
         }
         if (cardNo == 1)
         {
            if (ID1[0] != UID[0] || ID1[1] != UID[1] || ID1[2] != UID[2] || ID1[3] != UID[3])
            {
               cardNo++;
               writeDataEEPROM(10, cardNo);
               writeCardToEEPROM(ID2_address, UID, sizeof(UID));
               readCardFromEEPROM(ID2_address, ID2);
               lcd.setCursor(0, 3);
               lcd.print(F(" Succesful Adding   "));
               delay(1000);
            }
            else
            {
               lcd.setCursor(0, 3);
               lcd.print(F(" Fail to Add Card    "));
               delay(1000);
            }
         }
         else if (cardNo == 2)
         {
            if (ID1[0] != UID[0] || ID1[1] != UID[1] || ID1[2] != UID[2] || ID1[3] != UID[3])
            {
               if (ID2[0] != UID[0] || ID2[1] != UID[1] || ID2[2] != UID[2] || ID2[3] != UID[3])
               {
                  cardNo++;
                  writeDataEEPROM(10, cardNo);
                  writeCardToEEPROM(ID3_address, UID, sizeof(UID));
                  readCardFromEEPROM(ID3_address, ID3);
                  lcd.setCursor(0, 3);
                  lcd.print(F(" Succesful Adding   "));
                  delay(1000);
               }
               else
               {
                  lcd.setCursor(0, 3);
                  lcd.print(F(" Fail to Add Card    "));
                  delay(1000);
               }
            }
            else
            {
               lcd.setCursor(0, 3);
               lcd.print(F(" Fail to Add Card    "));
               delay(1000);
            }
         }
         else if (cardNo == 3)
         {
            lcd.setCursor(0, 3);
            lcd.print(F(" Card slot overflowed"));
            delay(1000);
         }

         mfrc522.PICC_HaltA();
         mfrc522.PCD_StopCrypto1();

         mfrc522.PICC_HaltA();
         mfrc522.PCD_StopCrypto1();
         menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
         menuIdControll(&select, customKey);
         return;
      }
   }
}
void deleteMFRC(unsigned char select, char customKey)
{
   bool lock = true;
   while (lock == true)
   {
      while (checkMFRC() == 1)
      {
         for (byte i = 0; i < mfrc522.uid.size; i++)
         {
            UID[i] = mfrc522.uid.uidByte[i];
            Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? F(" 0") : F(" "));
            Serial.print(UID[i], HEX);
            Serial.println(" ");
         }
         if (ID1[0] == UID[0] && ID1[1] == UID[1] && ID1[2] == UID[2] && ID1[3] == UID[3])
         {
            deleteDataEEPROM(ID1_address, ID1_address + 5);
            readCardFromEEPROM(150, ID1);
            lcd.setCursor(0, 3);
            lcd.print(F(" Succesful Delete   "));
         }
         else if (ID2[0] == UID[0] && ID2[1] == UID[1] && ID2[2] == UID[2] && ID2[3] == UID[3])
         {
            deleteDataEEPROM(ID2_address, ID2_address + 5);
            readCardFromEEPROM(200, ID2);
            lcd.setCursor(0, 3);
            lcd.print(F(" Succesful Delete   "));
         }
         else if (ID3[0] == UID[0] && ID3[1] == UID[1] && ID3[2] == UID[2] && ID3[3] == UID[3])
         {
            deleteDataEEPROM(ID3_address, ID3_address + 5);
            readCardFromEEPROM(300, ID3);
            lcd.setCursor(0, 3);
            lcd.print(F(" Succesful Delete   "));
         }
         else
         {
            lcd.setCursor(0, 3);
            lcd.print(F(" Wrong Card ID      "));
         }
         delay(1000);
         mfrc522.PICC_HaltA();
         mfrc522.PCD_StopCrypto1();

         mfrc522.PICC_HaltA();
         mfrc522.PCD_StopCrypto1();
         menuPtr = (menuPtr->PRE != NULL) ? (menuPtr->PRE) : (menuPtr);
         menuIdControll(&select, customKey);
         return;
      }
   }
}
unsigned char writeDataEEPROM(unsigned char address, unsigned char data)
{
   EEPROM.write(address, data);
   return data;
}
unsigned char readDataEEPROM(unsigned char address)
{
   return EEPROM.read(address);
}
void deleteDataEEPROM(int low_address, int high_address)
{
   for (int i = low_address + 1; i <= high_address; i++)
   {
      EEPROM.write(i, 0);
   }
   return;
}
