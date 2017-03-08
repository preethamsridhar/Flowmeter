/*
Liquid flow rate sensor -DIYhacking.com Arvind Sanjeev

Measure the liquid/water flow rate using this code. 
Connect Vcc and Gnd of sensor to arduino, and the 
signal line to arduino digital pin 2.
 
 */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C  lcd(0x3F, 2, 1, 0, 4, 5, 6, 7);

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

float calibrationFactor = 45;                                                           // The hall-effect flow sensor outputs approximately 45 pulses per second per litre/minute of flow.
volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;
float maxFlow = 100.00;

void setup()
{
  
  Serial.begin(38400);                                                                  // Initialize a serial connection for reporting values to the host
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);                              // The Hall-effect sensor is connected to pin 2 which uses interrupt 0. Configured to trigger on a FALLING state change (transition from HIGH
                                                                                        // state to LOW state)
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
}

/**
 * Main program loop
 */
void loop()
{
   
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    detachInterrupt(sensorInterrupt);                                                   // Disable the interrupt while calculating flow rate and sending the value to the host
        
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;      // Because this loop may not complete in exactly 1 second intervals we calculate the number of milliseconds that have passed since the last execution and use 
                                                                                        // that to scale the output. We also apply the calibrationFactor to scale the output based on the number of pulses per second per units of measure (litres/minute in
                                                                                        // this case) coming from the sensor.
    
    oldTime = millis();                                                                 // Note the time this processing pass was executed. Note that because we've disabled interrupts the millis() function won't actually be incrementing right
                                                                                        // at this point, but it will still return the value it was set to just before interrupts went away.
    
    flowMilliLitres = (flowRate / 60) * 1000;                                                  // Divide the flow rate in litres/minute by 60 to determine how many litres have passed through the sensor in this 1 second interval, then multiply by 1000 to
                                                                                        // convert to millilitres.
    
    totalMilliLitres += flowMilliLitres;                                                // Add the millilitres passed in this second to the cumulative total
      
    Serial.print("  Output Liquid Quantity: ");                           
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    pulseCount = 0;                                                                     // Reset the pulse counter so we can start incrementing again
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);                            // Enable the interrupt again now that we've finished sending output

    lcd.home();                                                                         // set cursor to 0,0
    lcd.print("TOTAL ltr :");
    lcd.print(float(totalMilliLitres)/1000);
    lcd.setCursor (0,1);                                                                // go to start of 2nd line
    if(float(totalMilliLitres)/1000 >= maxFlow){
      lcd.print("CONTACT VYYKN");  
    }
    else{
      lcd.print("REMAIN ltr:");
      lcd.print(maxFlow - float(totalMilliLitres)/1000);  
    }
  }
}

/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  pulseCount++;                                                                         // Increment the pulse counter
}
