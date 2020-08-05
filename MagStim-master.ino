//***********************************************************
//* Author: M. Hernandez
//
//* Penn Electronic Design Shop
//
// Purpose: The program is timer that reads a dial switch
// with defined positions 1, 2, 3, 4 for the time interval
// duration for the pulse. 
// A second dial switch outputs a pulse with freq, 
// 50% duty-cycle signal at will be on at Digital Pin 10
// The time the signal is ON depends on what the switch 
// indicates. 
// The switch is using the internal Pull up:  
// By moving the switch to the ON position.
//       1 (default) =>  15minutes
//       2           =>  30minutes
//       3           =>  45minutes
//       4           =>  60minutes
// Added ON-OFF burst mode using the same knob. 
// Added an I2C display
//
//*   Rev 1.0 6-10-2020 Initial Revision. 
//*   Rev 1.1 7-6-2020 Revise code to add new features
//*   Rev 2.0 7-22-2020 Revise code to fix issues.
//*   Rev 2.1 7/31/2020 Revise code to add 600 ON-OFF intervals 
//*   Rev 2.2 7/31/2020 Revise code to add Frequency. 
//************************************************************** 

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

int ii = 0; 
int ledState = LOW;
char *buf; 
int valtest=0;

int OnBurst =0; 
int OffBurst=0;

int OnSecBurst =0; 
int OffSecBurst=0; 
int OnMinBurst =0; 
int OffMinBurst=0; 
 

unsigned long int myCycles = 0; 
unsigned long int interval = 0; 
unsigned long int OnInterval = 0;
unsigned long int OffInterval = 0;
unsigned long int OnSecInterval = 0;
unsigned long int OffSecInterval = 0; 
unsigned long int OnMinInterval = 0;
unsigned long int OffMinInterval = 0; 

unsigned long myInitMillis = 0;  
unsigned long previousMillis =0;
unsigned long myPreviousMillis =0;
unsigned long myONInitMillis =0; 
const long point9HzInterval = 555; 

float freq = 0; 
unsigned long HzInterval = 0;
const long PeriodArrayMs[15] = {5000, 2500, 1666, 1250, 1000, 833, 714, 625, 555, 500, 100, 50, 33, 25, 17}; 
const char *FreqStrings[] = {"0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0","5.0","10.0","15.0","20.0","30.0"}; 

// 
// ** Prototypes
//

void Point9HzFunction();
void HzFunction();
void SelectTime(); 
void SelectONSeconds();
void SelectONMinutes();
void SelectOFFSeconds();
void SelectOFFMinutes(); 
void SelectFrequency(); 


String myString;      //String for the OLED display
void StrToDisplayln(int PixX, int PixY);  // Use a global variable instead

//
// Instantiate a display
//
Adafruit_ssd1306syp display(SDA_PIN,SCL_PIN);

void setup() {
  
  delay(500); 
  display.initialize();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  display.clear(); 
  myString = "MAGNETIC   PULSE     GENERATOR V2.1"; 
  StrToDisplayln(0, 0);
  delay(3000);
  display.clear(); 
  
  Serial.begin(9600); 
  
  pinMode(ledPin, OUTPUT);  // sets the digital pin 13 as output
  pinMode(Point9HzPin, OUTPUT);  // sets the digital pin 9 as output
  pinMode(EnterPin, INPUT_PULLUP);  // sets the digital pin 7 as input
  
  for(ii=2; ii<=5; ii++) pinMode(ii, INPUT_PULLUP);    // sets digital pins as input with an internal 20Kohm Pull up
   
  // Select the time in the switch 
  SelectTime(); 
   
  //** Select the ON-OFF Burst: Select seconds first
  //** Select the ON-OFF Burst: Selelect minutes next
  SelectONSeconds();
  //SelectONMinutes();
  SelectOFFSeconds();
  //SelectOFFMinutes(); 
  SelectFrequency(); 
  
  display.clear();  
  myString=String(freq, 2) + " Hz" + "  Session in progress!";
  display.setTextSize(2);     
  StrToDisplayln(0,0);
    
    // Report for Experiment parameters
//    Serial.print("interval = "); // Start cycle
//    Serial.println(interval);
//    Serial.print("OnBurst = ");
//    Serial.println(OnBurst); 
//    Serial.print("OffBurst = ");
//    Serial.println(OffBurst);
//    Serial.print("val = 0x");
//    Serial.println(val, HEX);
//      Serial.print("Period (sec) = ");  
//      Serial.println(HzInterval);
  
  if ((OnSecInterval == 0) && (OffSecInterval == 0)){
    
    // infinite loop to handle the simple case continuos pulse
    unsigned long myInitMillis = millis();  // Get the miliseconds to start with 
    
    while(1){ 
       unsigned long myCurrentMillis = millis();
//       unsigned long int leftSec = (interval - myCurrentMillis)/1000.0;
//       
//       display.clear();  
//       myString = String(leftSec, DEC) + " sec   left";
//       display.setTextSize(2);     
//       StrToDisplayln(0,10);
       
       if (myCurrentMillis - myInitMillis <= interval ) { // has the Top interval in seconds elapsed? 
            HzFunction();  // Turn on the pulse requested
        }else {
            digitalWrite(ledPin, LOW);
            digitalWrite(Point9HzPin, LOW);
          //  Serial.println("Done!");
            display.clear();  
            myString=String("Session   Complete!");
            display.setTextSize(2);     
            StrToDisplayln(0,25);
        }
    }
  } else { // This is for the ON - OFF burst pulses 
      unsigned long myInitMillis = millis();  // Get the miliseconds to start with
      myONInitMillis = myInitMillis;
      //Serial.println("Start ON cycle"); 
  }
}

void loop() {
    unsigned long myCurrentMillis = millis();
//    unsigned long int leftSec = (interval - myCurrentMillis)/1000.0;
//    
//    display.clear();  
//    myString = String(leftSec, DEC) + " sec   left";
//    display.setTextSize(2);     
//    StrToDisplayln(0,10);
    
    if (myCurrentMillis - myInitMillis <= interval ) { // has the interval in milliseconds elapsed? 
      if (myCurrentMillis - myONInitMillis <= OnSecInterval){
            HzFunction();  // Turn on the pulse requested
      } else {
                //Serial.println("Start OFF cycle");
                unsigned long myOFFInitMillis  = millis(); 
                do{
                        unsigned long myCurrentMillis = millis();
                        if( myCurrentMillis - myOFFInitMillis <= OffSecInterval){
                                  digitalWrite(ledPin, LOW);
                                  digitalWrite(Point9HzPin, LOW);
                        } else break;
                }while(1);
                //Serial.println("Done OFF cycle");
                myONInitMillis = millis(); 
                //Serial.println("Start ON cycle");
      }       
    } else {              // Has elapsed shut down the LED and the pulse signals.  
          digitalWrite(ledPin, LOW);
          digitalWrite(Point9HzPin, LOW);
          Serial.println("Done!");
          
          display.clear();  
          myString=String("Session   Complete!");
          display.setTextSize(2);     
          StrToDisplayln(0,25);        
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
// ** Display myString
// 
void StrToDisplayln(int PixX, int PixY){
  display.setCursor(PixX,PixY);
  display.println(myString);
  display.update();
  //delay(100); 
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

void SelectFrequency(){
  
  do {
        // Now set the value for the Frequency from
        // what is read at the potentiometer
        PotFreqVal = analogRead(potFreqPin);
  
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

        // Select the ON Burst in minutes
        display.setTextSize(1);     
        myString = "Frequency selected?";
        StrToDisplayln(0, 10);
        
        // Calculate the frequency based on the period selected 
        float temp = ((float)HzInterval*2.0)/1000.0;
        Serial.print("temp = ");
        Serial.println(temp); 
        freq = 1.0/temp; 
       // Serial.print("freq = ");
       // Serial.println(freq);
        // Select the string based on the frequency selected
        
       // myString=String(HzInterval*2, DEC);
        myString = String(freq, 2) + " Hz"; 
        display.setTextSize(2);     
        StrToDisplayln(20,25);
        display.clear(); 
   } while(valtest = digitalRead(EnterPin));
   valtest =0;
}


//
// * Select time 
//
void SelectTime(){
      // Select the time
  do{
         // Find the time loop set by the user
     myString=String("Select    time(min):");
     display.setTextSize(2);     
     StrToDisplayln(0,0);
     display.clear(); 
     do{
              // read the input pin and stay there until we read anything >0 
              for(ii=0; ii<4; ii++){
                  valArray[ii] = digitalRead(ii+2); // Store them first in an array
      //          Serial.print("Val["); 
      //          Serial.print(ii,DEC); 
      //          Serial.print("] = ");
      //          Serial.println(valArray[ii], DEC); 
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
            };
            val=~val;        // 1's complement the value of val for the new logic 
            val=val&0x000F;  // Make 0's the rest the MSBits and leave the LSBits intact
     }while(val==0); 
        
   //Serial.print("val = 0x");
   //Serial.println(val, HEX);
   
   //Serial.print("Period (sec) = ");  
   //Serial.println(HzInterval);
  
  switch (val) {
    case 1: // First pin in the closed position 15 min 
      //do 15min when val equals 1   
     // Serial.println("At pin 1");
      myCycles = 15; // 15 minutes for option 1 to test
      break;

    case 2:
      //do something when val equals 2
      // Serial.println("At pin 2");
       myCycles = 30; // 30 minutes for option 2
      break;
    
    case 4:
      //do something when val equals 4
     // Serial.println("At pin 3");
      myCycles =45; 
      break;
    
    case 8:
      //do something when val equals 8
     // Serial.println("At pin 4");
      myCycles =60;
      break;
    
    default:
      // if nothing else matches, do the default
      // default is optional
      // invalid state, make the signal low
     // Serial.println("At default");
      myCycles =0;
      digitalWrite(ledPin, LOW);
      digitalWrite(Point9HzPin, LOW);
      break;
    };
    
    interval = myCycles*60*1000;
     
    myString=String(myCycles, DEC);
    display.setTextSize(2);     
    StrToDisplayln(60,35);
 }while(valtest = digitalRead(EnterPin)); 
    display.clear();  
}


// **
// ** Set the ONSecInterval 
// **   global variable
// ** 
void SelectONSeconds(){
   
   //** Select seconds for the ON Burst
   do {
        // Select the ON Burst in seconds
        display.setTextSize(1);     
        myString = "PICK ON Burst(sec)?";
        StrToDisplayln(0, 10);
        OnSecBurst = map(analogRead(potFreqPin), 0, 1023, 0, 600); 
        myString = String(OnSecBurst, DEC)+ "sec";
        display.setTextSize(2);     
        StrToDisplayln(30,25);
        display.clear(); 
   } while(valtest = digitalRead(EnterPin));
   
  // Serial.print("OnBurst = ");
  // Serial.println(OnBurst); 
   OnSecInterval = (unsigned long int)OnSecBurst*1000; // ONSsecinterval in millisec
     
  // Serial.print("OnInterval = "); 
  // Serial.println(OnInterval);
  StrToDisplayln(0, 0);
}

// **
// ** Set the ONSecInterval 
// **   global variable
// ** 
void SelectONMinutes(){
   
   //** Select minutes for the ON Burst
   do {
        // Select the ON Burst in seconds
        display.setTextSize(1);     
        myString = "PICK ON Burst(min)?";
        StrToDisplayln(0, 10);
        OnMinBurst = map(analogRead(potFreqPin), 0, 1023, 0, 60);   // Read the analog value from the Pot
        myString = String(OnMinBurst, DEC)+"min";
        display.setTextSize(2);     
        StrToDisplayln(30,25);
        display.clear(); 
   } while(valtest = digitalRead(EnterPin));
   
  // Serial.print("OnBurst = ");
  // Serial.println(OnBurst); 
   OnMinInterval = (unsigned long int)OnMinBurst*60*1000; // ONMinInterval in millisec
     
  // Serial.print("OnInterval = "); 
  // Serial.println(OnInterval);
  StrToDisplayln(0, 0);
}


// **
// ** Set the OFFSecInterval 
// **   global variable
// ** 
void SelectOFFSeconds(){
  
   //** Select seconds for the OFF Burst
   do {
        // Select the OFF Burst in seconds
        display.setTextSize(1);     
        myString = "PICK OFF Burst(sec)?";
        StrToDisplayln(0, 10);
        OffSecBurst = map(analogRead(potFreqPin), 0, 1023, 0, 600);   // Read the analog value from the Pot
        myString = String(OffSecBurst, DEC) + "sec";
        display.setTextSize(2);     
        StrToDisplayln(30,25);
        display.clear(); 
   } while(valtest = digitalRead(EnterPin));
   
   // Serial.print("OnBurst = ");
   // Serial.println(OnBurst); 
   OffSecInterval = (unsigned long int)OffSecBurst*1000; // OFFSsecinterval in millisec
     
  // Serial.print("OffInterval = "); 
  // Serial.println(OffInterval);
  // Select the OFF Burst in minutes
  StrToDisplayln(0, 0);
}

// **
// ** Set the ONSecInterval 
// **   global variable
// ** 
void SelectOFFMinutes(){
   
   //** Select minutes for the ON Burst
   do {
        // Select the ON Burst in seconds
        display.setTextSize(1);     
        myString = "PICK OFF Burst(min)?";
        StrToDisplayln(0, 10);
        OffMinBurst = map(analogRead(potFreqPin), 0, 1023, 0, 60);   // Read the analog value from the Pot
        myString = String(OffMinBurst, DEC) + "min";
        display.setTextSize(2);     
        StrToDisplayln(30,25);
        display.clear(); 
   } while(valtest = digitalRead(EnterPin));
   
  // Serial.print("OnBurst = ");
  // Serial.println(OnBurst); 
   OffMinInterval = (unsigned long int)OffMinBurst*60*1000; // OffMinInterval in millisec
     
  // Serial.print("OnInterval = "); 
  // Serial.println(OnInterval);
  StrToDisplayln(0, 0);
}
