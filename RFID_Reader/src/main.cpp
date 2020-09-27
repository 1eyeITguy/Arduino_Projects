#include <LiquidCrystal.h>           //include the LiquidCrystal library

LiquidCrystal lcd(12, 11, 10, 9, 8, 7);  //define LCD pins (RS, E, D4, D5, D6, D7)

 
#define MAX_BITS 100                 // max number of bits 
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
 
unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code

int LED_BACKLIGHT = 5;

// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0() {
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME; 
}
 
// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1() {
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
void setup() {
  pinMode(LED_BACKLIGHT, OUTPUT);
  digitalWrite(LED_BACKLIGHT, HIGH);
  lcd.begin(16, 2);                       //initializes the LCD and specifies the dimensions
  lcd.print("HID PROX READER"); 
  lcd.setCursor(0, 1);
  lcd.print("By Matthew Miles");
  delay(5000);
  lcd.clear();
  digitalWrite(LED_BACKLIGHT, LOW);
  pinMode(2, INPUT);     // DATA0 (INT0)
  pinMode(3, INPUT);     // DATA1 (INT1)
 
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(0, ISR_INT0, FALLING);  
  attachInterrupt(1, ISR_INT1, FALLING);
 
  weigand_counter = WEIGAND_WAIT_TIME;
}
 
void loop() {
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1;  
  }
 
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
 
    if (bitCount == 35) {
      // 35 bit HID Corporate 1000 format
      // facility code = bits 2 to 14
      for (i=2; i<14; i++) {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 15 to 34
      for (i=14; i<34; i++) {
         cardCode <<=1;
         cardCode |= databits[i];
      }
 
      printBits();
    }
    else if (bitCount == 26) {
      // standard 26 bit format
      // facility code = bits 2 to 9
      for (i=1; i<9; i++) {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 10 to 23
      for (i=9; i<25; i++) {
         cardCode <<=1;
         cardCode |= databits[i];
      }
 
      printBits();  
    }
    else {
     // you can add other formats if you want!
     // Serial.println("Unable to decode."); 
    }
 
     // cleanup and get ready for the next card
     bitCount = 0;
     facilityCode = 0;
     cardCode = 0;
     for (i=0; i<MAX_BITS; i++) 
     {
       databits[i] = 0;
     }
  }
}
 
void printBits() {
      digitalWrite(LED_BACKLIGHT, HIGH);
      lcd.begin(16, 2);                       //initializes the LCD and specifies the dimensions
      lcd.print("FC = "); 
      lcd.print(facilityCode);
      lcd.setCursor(0, 1);
      lcd.print("CC = ");
      lcd.print(cardCode);
      delay(10000);
      lcd.clear();
      digitalWrite(LED_BACKLIGHT, LOW);
}
