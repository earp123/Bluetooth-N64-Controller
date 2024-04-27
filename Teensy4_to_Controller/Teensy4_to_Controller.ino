/*  
*   ~SWR~
*   N64 Retro-fit Contoller Project
*
*   Sends poll signal to controller and receives the controller response (buttons pressed)
*   on the same pin poll_pin. Converts the response by comparing the intervals between pulses.
*   Writes the controller response as a byte array of 4 bytes to Serial1.
*   The Teensy 4.0 is wired to a  Bluetooth chip in the PERIPHERAL role
*   via hardware Serial1 pins (0 & 1).
*/

#include "wiring_private.h"
#include "pins_arduino.h"

//Assign pin 9 to the input pin to measure the controller status resonse pulses AND
#define HF_PULSEIN_BITMASK CORE_PIN9_BITMASK

//Assign pin 9 to the output pin for the controller status poll 
int poll_pin = 9;


unsigned long duration[32];
unsigned long cont_rsp = 0;

//High Frequency pulse in
//Returns the clock cycles between each pulse. Timeout returns 0. 
//Good for pulses <1us
unsigned long hf_pulseIn(int pin, unsigned long timeout)
{

    int bit = HF_PULSEIN_BITMASK;//NOTE: digitialPinToBitMask doesn't seem to work here
    uint8_t port = digitalPinToPort(pin);
    uint8_t stateMask = (HIGH ? bit : 0);
    unsigned long width = 0; // keep initialization out of time critical area

    // convert the timeout from microseconds to a number of times through
    // the initial loop; it takes 16 clock cycles per iteration.
    unsigned long numloops = 0;
    unsigned long maxloops = microsecondsToClockCycles(timeout) / 16;

    // wait for any previous pulse to end
    while ((*portInputRegister(port) & bit) == stateMask)
        if (numloops++ == maxloops)
            return 1;

    // wait for the pulse to start
    while ((*portInputRegister(port) & bit) != stateMask)
        if (numloops++ == maxloops)
            return 2;

    // wait for the pulse to stop
    while ((*portInputRegister(port) & bit) == stateMask) {
        if (numloops++ == maxloops)
            return 3;
        width++;
    }
    
    return width; 
}



//Send N64 'controller input' poll signal 000000011
void generate_poll(){

  pinMode(poll_pin, OUTPUT);
  digitalWriteFast(poll_pin, HIGH);

  for(int j = 0; j<7; j++){
    digitalWriteFast(poll_pin, LOW);
    delayNanoseconds(2950);
    digitalWriteFast(poll_pin, HIGH);
    delayNanoseconds(1000);
    
  }

  
  digitalWriteFast(poll_pin, LOW);
  delayNanoseconds(1000);
  digitalWriteFast(poll_pin, HIGH);
  delayNanoseconds(3000);
  digitalWriteFast(poll_pin, LOW);
  delayNanoseconds(1000);
  digitalWriteFast(poll_pin, HIGH);

  pinMode(poll_pin, INPUT_PULLUP);
  
}


void setup()
{
  Serial1.begin(1000000);
  Serial.begin(115200);
  Serial.println("It's crashing");
  
  pinMode(poll_pin, INPUT_PULLUP);
  
}

void loop()
{

  generate_poll();
  
  for(int i = 0; i <= 31; i++){
    duration[i] = hf_pulseIn(poll_pin, 1000);
    
    if(duration[i] > 100)    cont_rsp &= ~(1<<(31-i)); //greater than 2us? = 0
    else if(duration[i] > 3) cont_rsp |= (1<<(31-i));  //else = 1, greater than 3 means we didn't timeout
  }

  //Debug the pulse interval lengths
/*
  for(int j = 0; j <=31; j++)
  {
    Serial.print(duration[j]); Serial.print(" ");
  }
  Serial.println();
*/

  Serial1.write((byte*) &cont_rsp, 4);
  //Serial.write((byte*) &cont_rsp, 4);
  //Serial.println(cont_rsp, BIN);
  delay(10);

  
}

  
