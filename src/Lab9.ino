/*
 * Project Lab9
 * Description:
 * Author:
 * Date:
 */

SYSTEM_THREAD(ENABLED);
#include <Wire.h>
#include "oled-wing-adafruit.h"
#include "SparkFun_VCNL4040_Arduino_Library.h"
#include <blynk.h>

VCNL4040 sensor;
OledWingAdafruit display;

String mode = "init";
uint16_t lowCal;
uint16_t highCal;
bool lowCalibrated;
bool highCalibrated;
bool isLight;
bool isTemp;
uint16_t prevValue;

// setup() runs once, when the device is first turned on.
void setup()
{
  //the lights
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);

  //potentiometer, button, thermometer
  pinMode(D3, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);

  Wire.begin();

  //display commands
  display.setup();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Potentiometer and button will monitor the temperature");
  display.display();

  sensor.powerOffProximity();
  sensor.powerOnAmbient();


  digitalWrite(D4, LOW);
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);

  //blynk set up 
  Blynk.begin("ICDELDnCKVoIcgHndxn_L5u35xhmgmHW", IPAddress(167, 172, 234, 162), 9090);
  Blynk.run();


  prevValue = sensor.getAmbient();

  lowCalibrated = false;
  highCalibrated = false;
  isLight = false;
  isTemp = false;


  // Put initialization like pinMode and begin functions here.
}

void printToDisplay(String output)
{
  //reset display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);


  display.println(output);
  display.display();



}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  delay(2000);


}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // The core of your code will likely live here.
  display.loop();
  display.loop();


  
  if ((highCalibrated == true) && (lowCalibrated == true))
  { 

    
    if (display.pressedA())
    {
      isLight = true;
      isTemp = false;
      
    }
    if (display.pressedB())
    {
      isLight = false;
      isTemp = true;
      
    }


    //ambient light mode
    if (isLight)
    {
     
      sensor.powerOnAmbient();
      uint16_t ambiValue = sensor.getAmbient();

    
      String output = ""; 
      
      if (ambiValue < lowCal)
      {
        digitalWrite(D4, HIGH);
        digitalWrite(D5, LOW);
        digitalWrite(D6, LOW);

        
        if (!(prevValue < lowCal))
        {
          Blynk.notify("Light value dipped below lower bound!");
          output += "Light value has dipped below lower bound!";
        }

      }
      else if (ambiValue < highCal)
      {
        digitalWrite(D4, LOW);
        digitalWrite(D5, HIGH);
        digitalWrite(D6, LOW);

        
        if (!((prevValue > lowCal) && (prevValue < highCal)))
        {
          Blynk.notify("Light value has reached target range!");
          output += "Light value has reached target range!";
        }
      }
      else if (ambiValue > highCal)
      {
        digitalWrite(D4, LOW);
        digitalWrite(D5, LOW);
        digitalWrite(D6, HIGH);

        //check if changed from previous bounds and notify
        if (!((prevValue > highCal)))
        {
          Blynk.notify("Light value has passed higher bound!");
          output += "Light value has passed higher bound!";
        }
      }


      output += "\nAmbient Value: ";
      output += (String) ambiValue;

      //output to display
      printToDisplay(output);

      prevValue = ambiValue;
    }

    //print proximity value when button A pressed
    if (isTemp)
    {
      digitalWrite(D4, LOW);
      digitalWrite(D5, LOW);
      digitalWrite(D6, LOW);

      uint16_t reading = analogRead(A4);
      double voltage = (reading * 3.3) / 4095.0;
      double temperature = (voltage - 0.5) * 100;
      String output = "";
        
      //set up output string with celsius and fahrenheit
      output += "Celsius: ";
      output += (String) temperature;
      output += "\n";
      output += "Fahrenheit: ";
      output += (String) ((temperature*9/5)+32);

      //print to display
      printToDisplay(output);

      Blynk.virtualWrite(V0, temperature);

    }

    delay(100);

  }
  else
  {

    digitalWrite(D4, LOW);
    digitalWrite(D5, LOW);
    digitalWrite(D6, LOW);
    
    //get values
    bool button = digitalRead(D2);
    uint16_t value = analogRead(A5); 
    Serial.println(value);

    //set values
    if (button == false)
    {
      if (!lowCalibrated)
      {
        printToDisplay("calibrating");
        lowCal = value;

        //notify of first calibration
        String output = "First light level set to\n";
        output += (String) lowCal;
        printToDisplay(output);  
        lowCalibrated = true;
        delay(2000);
      }
      else if (!highCalibrated)
      {
        printToDisplay("calibrating");
        highCal = value;

        //notify of first calibration
        String output = "Second light level set to\n";
        output += (String) highCal;
        printToDisplay(output);  
        highCalibrated = true;
        delay(2000);
      }
      delay(2000);
    }
    String output = "Potentiometer value:\n";  
    output += (String) value;
    printToDisplay(output);
    delay(500);
  }
}


  

  
  
