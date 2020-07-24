//*******************************************************
//* Author: M. Hernandez
//
//* Date: 6-2-2020
//
//* Purpose: The program is timer that reads a dial switch
//* with defined positions 1, 2, 3, 4 for the time interval
//* duration for the pulse. 
//* A second dial switch outputs a pulse with freq, 
//* 50% duty-cycle signal at will be on at Digital Pin 10
//* The time the signal is ON depends on what the switch 
//* indicates. 
//* The switch is using the internal Pull up:  
//* By moving the switch to the ON position.
//*       1 (default) =>  15minutes
//*       2           =>  30minutes
//*       3           =>  45minutes
//*       4           =>  60minutes
//
//******************************************************* 

#include <Adafruit_ssd1306syp.h>

#define SDA_PIN 9
#define SCL_PIN 8

int ledPin = 13;  // LED connected to digital pin 13
int Point9HzPin = 10;  // Updated Output pin
int potFreqPin = A0;       // Frequency Potentiometer pin
int EnterPin = 7;      // To enter the values

int FifteenMinPin = 2;    // Switch pin 1 connected to digital pin 2
int ThirtyMinPin = 3;     // Switch pin 2 connected to digital pin 3
int FortyFiveMinPin = 4;    // Switch pin 3 connected to digital pin 4
int SixtyMinPin = 5;     // Switch pin 4 connected to digital pin 5

bool stopVar = 0; 
unsigned maskVal = 0;      // This is the mask to set the bit
unsigned int val = 0;      // variable to store a switch value
int PotFreqVal =0;          // variable to store the Potentiometer value
int valArray[4] = {0,0,0,0};
int Freq = 0; 

// enum FREQ_VAL {P_one, P_two, P_three, P_four, P_five, P_six, P_seven, P_eight, P_nine, One, Ten, Twenty, Thirty};

int ii = 0; 
int ledState = LOW;
char *buf; 

unsigned long int myCycles = 0; 
unsigned long int interval = 0; 
unsigned long myInitMillis = 0;  
unsigned long previousMillis =0;
unsigned long myPreviousMillis =0;
const long point9HzInterval = 555; 

unsigned long HzInterval = 0;
const long PeriodArrayMs[15] = {5000, 2500, 1666, 1250, 1000, 833, 714, 625, 555, 500, 100, 50, 33, 25, 17}; 

void Point9HzFunction();
void HzFunciton();
void StrToDisplayln(char *myStr, int PixX, int PixY);

//
// Instantiate a display
//
Adafruit_ssd1306syp display(SDA_PIN,SCL_PIN);

void setup() {
  Serial.begin(9600); 
  
  pinMode(ledPin, OUTPUT);  // sets the digital pin 13 as output
  pinMode(Point9HzPin, OUTPUT);  // sets the digital pin 9 as output
  pinMode(EnterPin, INPUT_PULLUP);  // sets the digital pin 9 as output
  
  for(ii=2; ii<=5; ii++) pinMode(ii, INPUT_PULLUP);    // sets digital pins as input with an internal 20Kohm Pull up
  
  delay(1000); 
 
  display.initialize();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.clear(); 
  StrToDisplayln("MAGNETIC PULSE GEN", 0, 0);
  
//  // Select the ON Burst in minutes
//  StrToDisplayln("Select ON Burst(min)?", 0, 10);
//   
//   do {
//        int AnaVar = analogRead(potFreqPin); // Read the analog value from the Pot
//        int OnBurstMin = map(AnaVar, 0, 1023, 1, 10);
//        String MyString = String(OnBurstMin);
//        MyString.toCharArray(buf, 2);  
//        StrToDisplayln(buf, 20, 20);
//        display.clear();   
//   } while(!EnterPin);

  // Select the OFF Burst in minutes

    
   // Find the time loop set by the user
   do{ 
      // read the input pin and stay there until we read anything >0 
      for(ii=0; ii<4; ii++){
        valArray[ii] = digitalRead(ii+2); // Store them first in an array
        Serial.print("Val["); 
        Serial.print(ii,DEC); 
        Serial.print("] = ");
        Serial.println(valArray[ii], DEC); 
      }
      
      // Logic with the new switch: 
      // OPEN   = Logical 1
      // CLOSED = Logical 0
      // now test each member of the array and go back to read 
      // if we have not received a 0 in one of the pins go back to read
      for(ii=0; ii<4; ii++){
        if (valArray[ii] == 1) maskVal = 1; 
        else                   maskVal = 0; 
     //   Serial.print("maskVal = 0x");
     //   Serial.println(maskVal, HEX); 
        maskVal<<=ii;
        val=val|maskVal;
        maskVal=0; 
        Serial.print("val = 0x");
        Serial.println(val, HEX); 
      };
      val=~val;        // 1's complement the value of val for the new logic 
      val=val&0x000F;  // Make 0's the rest the MSBits and leave the LSBits intact
   }while(val==0);

  // Now set the value for the Frequency from
  // what is read at the potentiometer
  PotFreqVal = analogRead(potFreqPin);
  Serial.print("Analog val = "); 
  Serial.println(PotFreqVal);

  if (PotFreqVal <= 67) HzInterval = PeriodArrayMs[0]; 
  else if ((PotFreqVal>=68)  && (PotFreqVal<=136)) HzInterval = PeriodArrayMs[1]; 
  else if ((PotFreqVal>=137) && (PotFreqVal<=204)) HzInterval = PeriodArrayMs[2];
  else if ((PotFreqVal>=205) && (PotFreqVal<=272)) HzInterval = PeriodArrayMs[3];
  else if ((PotFreqVal>=273) && (PotFreqVal<=340)) HzInterval = PeriodArrayMs[4];
  else if ((PotFreqVal>=341) && (PotFreqVal<=409)) HzInterval = PeriodArrayMs[5];
  else if ((PotFreqVal>=410) && (PotFreqVal<=477)) HzInterval = PeriodArrayMs[6];
  else if ((PotFreqVal>=478) && (PotFreqVal<=545)) HzInterval = PeriodArrayMs[7];
  else if ((PotFreqVal>=546) && (PotFreqVal<=613)) HzInterval = PeriodArrayMs[8];
  else if ((PotFreqVal>=614) && (PotFreqVal<=682)) HzInterval = PeriodArrayMs[9];
  else if ((PotFreqVal>=683) && (PotFreqVal<=750)) HzInterval = PeriodArrayMs[10];
  else if ((PotFreqVal>=751) && (PotFreqVal<=818)) HzInterval = PeriodArrayMs[11];
  else if ((PotFreqVal>=819) && (PotFreqVal<=886))HzInterval  = PeriodArrayMs[12];
  else if ((PotFreqVal>=887) && (PotFreqVal<=955))HzInterval  = PeriodArrayMs[13];
  else if ((PotFreqVal>=956) && (PotFreqVal<=1023))HzInterval = PeriodArrayMs[14];

  Serial.print("Period (sec) = ");  
  Serial.println(HzInterval); 
  
  switch (val) {
    case 1: // First pin in the closed position 15 min 
      //do 15min when val equals 1   
      Serial.println("At pin 1");
      //myCycles = 1; // 15 minutes for option 1 to test
      myCycles = 15; // 15 minutes for option 1 to test
      break;

    case 2:
      //do something when val equals 2
       Serial.println("At pin 2");
       myCycles = 30; // 30 minutes for option 2
      break;
    
    case 4:
      //do something when val equals 4
      Serial.println("At pin 3");
      myCycles =45; 
      break;
    
    case 8:
      //do something when val equals 8
      Serial.println("At pin 4");
      myCycles =60;
      break;
    
    default:
      // if nothing else matches, do the default
      // default is optional
      // invalid state, make the signal low
      Serial.println("At default");
      myCycles =0;
      digitalWrite(ledPin, LOW);
      digitalWrite(Point9HzPin, LOW);
      break;
  };
  interval = myCycles*60*1000;
  
  Serial.print("interval = "); // Start cycle
  Serial.println(interval); 
  
  unsigned long myInitMillis = millis();  // Get the miliseconds to start with
}

void loop() {
    unsigned long myCurrentMillis = millis(); 
    if (myCurrentMillis - myInitMillis <= interval ) { // has the interval in seconds elapsed? 
            HzFunction();  // Turn on the pulse requested
    } else {              // Has elapsed shut down the LED and the pulse signals.
          stopVar = 1;   
          digitalWrite(ledPin, LOW);
          digitalWrite(Point9HzPin, LOW);
          Serial.println("Done!"); 
    } 
}

// 
// ** Function to set any Frequency
//

void HzFunction(){

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= HzInterval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
    digitalWrite(Point9HzPin, ledState);
  }
}



// 
// ** Legacy function to set the 
// ** output pulse to 0.9Hz
//

void Point9HzFunction(){

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= point9HzInterval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
    digitalWrite(Point9HzPin, ledState);
  }
}

// 
// Display a string
// 
void StrToDisplayln(char *myStr, int PixX, int PixY){
  display.setCursor(PixX,PixY);
  display.println(myStr);
  display.update();
  delay(2000);
}
