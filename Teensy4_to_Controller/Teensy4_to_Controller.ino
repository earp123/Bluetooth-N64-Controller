/*  
*   ~SWR~
*   N64 Retro-fit Contoller Project
*
*   Sends poll signal to controller and receives the controller response (buttons pressed)
*   on the same pin poll_pin. Converts the response by comparing the intervals between pulses.
*   Writes the controller response as a byte array of 4 bytes to Serial1.
*   Hardware Serial1 pins (0 & 1) on the Teensy 4.0 are wired to a 
*   Bluetooth chip in the PERIPHERAL role via hardware Serial1 pins.
*/


int poll_pin = 9;


unsigned long duration[32];
unsigned long cont_rsp = 0;

//Returns the clock cycles between each pulse. Timeout returns 0. 
//Good for pulses <1us

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
    else if(duration[i] > 0) cont_rsp |= (1<<(31-i));  //else = 1
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

  
