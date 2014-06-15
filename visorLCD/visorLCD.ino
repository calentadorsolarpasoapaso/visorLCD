#include <Wire.h>                       // For some strange reasons, Wire.h must be included here
#include <VirtualWire.h>
#include <DS1307new.h>
#include <LiquidCrystal.h>


/*
  LiquidCrystal Library - Hello World
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch prints "Hello World!" to the LCD
 and shows the time.
 
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 
 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define BACKLIGHT_PIN     13
// *********************************************
// DEFINE
// *********************************************

// *********************************************
// VARIABLES
// *********************************************
uint16_t startAddr = 0x0000;            // Start address to store in the NV-RAM
uint16_t lastAddr;                      // new address for storing in NV-RAM
uint16_t TimeIsSet = 0xaa55;            // Helper that time must not set again

const int INDEFINIDO=999999;
const uint8_t PIN_RADIO_FRECUENCIA=9;

void setup() {
  // set up the LCD's number of columns and rows: 
  //lcd.clear();
  Serial.begin(9600);
  
  //Setup reloj
  pinMode(2, INPUT);                    // Test of the SQW pin, D2 = INPUT
  digitalWrite(2, HIGH);                // Test of the SQW pin, D2 = Pullup on
/*
   PLEASE NOTICE: WE HAVE MADE AN ADDRESS SHIFT FOR THE NV-RAM!!!
                  NV-RAM ADDRESS 0x08 HAS TO ADDRESSED WITH ADDRESS 0x00=0
                  TO AVOID OVERWRITING THE CLOCK REGISTERS IN CASE OF
                  ERRORS IN YOUR CODE. SO THE LAST ADDRESS IS 0x38=56!
*/
  RTC.setRAM(0, (uint8_t *)&startAddr, sizeof(uint16_t));// Store startAddr in NV-RAM address 0x08 

  RTC.getRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
  if (TimeIsSet != 0xaa55)
  {
    RTC.stopClock();
        
    RTC.fillByYMD(2014,05,06);
    RTC.fillByHMS(19,00,0);
    
    RTC.setTime();
    TimeIsSet = 0xaa55;
    RTC.setRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
    RTC.startClock();
  }
  else
  {
    RTC.getTime();
  }

/*
   Control Register for SQW pin which can be used as an interrupt.
*/
  RTC.ctrl = 0x00;                      // 0x00=disable SQW pin, 0x10=1Hz,
                                        // 0x11=4096Hz, 0x12=8192Hz, 0x13=32768Hz
  RTC.setCTRL();

  Serial.println("Tiempo Actual");
  
  uint8_t MESZ;
      
  Serial.println();
  //SETUP LCD
  
  lcd.begin(16, 2);
  lcd.clear();

  delay(2000);
  
  setupRadioFrecuencia();  

}

void loop() {
  int watts=leerValorRadioFrecuencia();
  
  if(watts!=INDEFINIDO){
  
  String texto="Watts:" ;
  texto+=watts;
  
  Serial.println(texto);
  
  lcd.setCursor(0, 0);
  // print the number of seconds since reset:
  lcd.print(texto);

  String fechaHora=getFechaHora();
  
  
  lcd.setCursor(0, 1);
  lcd.print(fechaHora);
  
  }
  
}

int leerValorRadioFrecuencia(){
    uint8_t buf[50];
    uint8_t buflen = 50;

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
	int i;
        digitalWrite(BACKLIGHT_PIN, HIGH); // Flash a light to show received good message
	// Message with a good checksum received, dump it.
	String valor="";
	for (i = 0; i < buflen; i++)
	{
            char caracter=(char)buf[i];
            valor+=caracter;
	}
       
        String watts=valor.substring(0,valor.indexOf(' '));
	Serial.println(watts);

        int iWatts=watts.toInt();
        if(iWatts==0) iWatts=INDEFINIDO;
        digitalWrite(BACKLIGHT_PIN, LOW);
        Serial.println(iWatts);
        return iWatts;
    }
    return INDEFINIDO;
}

void setupRadioFrecuencia(){
    Serial.println("Inicializando RF");
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, LOW);

    // Initialise the IO and ISR
    vw_set_rx_pin(PIN_RADIO_FRECUENCIA);

    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);	 // Bits per sec

    vw_rx_start();       // Start the receiver PLL running
}

String getFechaHora(){
  String fechaHora="";
  
  RTC.getTime();
  if (RTC.hour < 10)                    // correct hour if necessary
  {
    fechaHora+="0";
    fechaHora+=RTC.hour;
  } 
  else
  {
    fechaHora+=RTC.hour;
  }
  fechaHora+=":";
  if (RTC.minute < 10)                  // correct minute if necessary
  {
    fechaHora+="0";
    fechaHora+=RTC.minute;
  }
  else
  {
    fechaHora+=RTC.minute;
  }
  fechaHora+=":";
  if (RTC.second < 10)                  // correct second if necessary
  {
    fechaHora+="0";
    fechaHora+=RTC.second;
  }
  else
  {
    fechaHora+=RTC.second;
  }
  fechaHora+=" ";
  if (RTC.day < 10)                    // correct date if necessary
  {
    fechaHora+="0";
    fechaHora+=RTC.day;
  }
  else
  {
    fechaHora+=RTC.day;
  }
  fechaHora+="/";
  if (RTC.month < 10)                   // correct month if necessary
  {
    fechaHora+="0";
    fechaHora+=RTC.month;
  }
  else
  {
    Serial.print(RTC.month, DEC);
  }
  /*
  Serial.print("/");
  fechaHora+=RTC.year;          // Year need not to be changed
  Serial.print(" ");
  switch (RTC.dow)                      // Friendly printout the weekday
  {
    case 1:
      Serial.print("MON");
      break;
    case 2:
      Serial.print("TUE");
      break;
    case 3:
      Serial.print("WED");
      break;
    case 4:
      Serial.print("THU");
      break;
    case 5:
      Serial.print("FRI");
      break;
    case 6:
      Serial.print("SAT");
      break;
    case 7:
      Serial.print("SUN");
      break;
  }
  uint8_t MESZ = RTC.isMEZSummerTime();
   
  RTC.getRAM(0, (uint8_t *)&lastAddr, sizeof(uint16_t));
  Serial.print("\n");
  lastAddr = lastAddr + 1;              // we want to use it as addresscounter for example
  RTC.setRAM(0, (uint8_t *)&lastAddr, sizeof(uint16_t));
  RTC.getRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
  if (TimeIsSet == 0xaa55)              // check if the clock was set or not
  {
  
  }
  else
  {
  
  } 
*/  
  return fechaHora;
}

