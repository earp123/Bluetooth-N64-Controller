/*  
*   ~SWR~
*   N64 Retro-fit Contoller Project
*
*   Essentially a Joybus driver. 
*   Contsantly reads 4 bytes of controller input from Serial1 into a buffer, 
*   Manchester enocdes the buffer to pulse invertals as defined by Joybus,
*   writes the pulses to the data pin TODO> upon receiving a poll rom the Nintendo 64 (on the same pin).
*
*   This sketch is uploaded to a Teensy 4.1 that is wired to a Bluetooth board in the CENTRAL role
*   via hardware Serial1.
*
*/

/*TODO 
* >the -x axis doens't work properly
*
*/

//Assign pin 9 to the input pin to measure the command sent from the N64
#define HF_PULSEIN_BITMASK CORE_PIN9_BITMASK

uint32_t controller_response = 0x0;
uint8_t controller_response_buf[4] = {0};

uint32_t device_info = 0x05000000;
int duration[8];
uint8_t joybus_command = 0;
int command_pin = 9;//ultimately, we're just using this to get the port of this pin


volatile int stop_flag = 0;
volatile int poll = 0;

//150MHz intervals for the comparator
const uint32_t comp_4 = 600;//4us
const uint32_t comp_3 = 407;//3us
const uint32_t comp_1 = 112; //1us

//init buffer for status response (butons pressed)
uint32_t comp_vals[33];



//response bit
int res_bit = 0;

//PIN 13 Mask, used below in config_pin()
#define data_pin (1 << 3)

void config_pin(){
   // Configure GPIO B0_03 (PIN 13) for output
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
    IOMUXC_GPR_GPR27 = 0xFFFFFFFF;

    CORE_PIN33_CONFIG = INPUT_PULLUP;
    GPIO7_GDIR |= data_pin;
    
}

void config_timers(){

    CCM_CSCMR1 &= ~CCM_CSCMR1_PERCLK_CLK_SEL; // turn off 24mhz mode

    CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) ;  // enable GPT1 module
    GPT1_CR = 0;   // clear prev config
    GPT1_PR = 0;   // No prescaler
    GPT1_OCR1 = comp_4;  // compare for one whole period
    GPT1_SR = 0x3F; // clear all prior status
    GPT1_IR = GPT_IR_OF1IE; //enable compare interrupt
    GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(1); //start timer, set clock,

    //enable the ISRs
    attachInterruptVector(IRQ_GPT1, gpt1_isr);
    NVIC_ENABLE_IRQ(IRQ_GPT1);

    CCM_CCGR0 |= CCM_CCGR0_GPT2_BUS(CCM_CCGR_ON);  // enable GPT2 module
    GPT2_CR = 0;
    GPT2_PR = 0;   
    GPT2_OCR1 = 150;//dummy value that won't get used
    GPT2_SR = 0x3F; 
    GPT2_IR = GPT_IR_OF1IE;
    GPT2_CR = GPT_CR_EN | GPT_CR_CLKSRC(1) | GPT_CR_ENMOD; //restart after reenable
    
    attachInterruptVector(IRQ_GPT2, gpt2_isr);
    NVIC_ENABLE_IRQ(IRQ_GPT2);
}

void poll_counter_init(){

  CCM_CCGR6 |= CCM_CCGR6_QTIMER4(CCM_CCGR_ON);

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 = 1;    // QT4 Timer2 on pin 9

  TMR4_CTRL2 = 0; 
  TMR4_LOAD2 = 0;  // start val after compare
  TMR4_COMP12 = 8; // n-1 for a 9 bit poll signal
  TMR4_CMPLD12 = 8;
  
  TMR4_CTRL2 = TMR_CTRL_CM(0) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ; //configure the poll pin
      //leave off at start ^  | source poll pin  
  attachInterruptVector(IRQ_QTIMER4, my_isr);
  NVIC_ENABLE_IRQ(IRQ_QTIMER4);
  
}

//Must be initialized before enabling. 
void enable_poll_counter(){
  TMR4_CNTR2 = 0;
  TMR4_LOAD2 = 0;  // start val after compare
  
  TMR4_CTRL2 |= TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ;
  TMR4_CSCTRL2 |= TMR_CSCTRL_TCF1EN;  // enable interrupt
}

void disable_poll_counter(){
  TMR4_CTRL2 = 0;
  TMR4_CSCTRL2 = 0;
  GPIO7_DR_SET = data_pin;
}

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

uint32_t flipFourBytes(uint32_t c){
  uint32_t r=0;
  for(uint32_t i = 0; i < 32; i++){
    r <<= 1;
    r |= c & 1;
    c >>= 1;
  }
  return r;
}

byte flipByte(byte c){
  char r=0;
  for(byte i = 0; i < 8; i++){
    r <<= 1;
    r |= c & 1;
    c >>= 1;
  }
  return r;
}

uint32_t device_info_flipped = flipFourBytes(device_info);

void setup() {

    Serial.begin(115200);
    Serial.println("Crash Test...");
    Serial1.begin(1000000);
    
    config_pin();
    
    poll_counter_init();
    config_timers();

    

    pinMode(command_pin, INPUT_PULLUP);
    
}





void loop() {

      if(Serial1.available()){

      Serial1.readBytes((uint8_t *) controller_response_buf, 4);

      uint8_t flipped_byte = flipByte(controller_response_buf[0]);
      controller_response_buf[0] = flipped_byte;

      flipped_byte = flipByte(controller_response_buf[3]);
      controller_response_buf[3] = flipped_byte;

      flipped_byte = flipByte(controller_response_buf[2]);
      controller_response_buf[2] = flipped_byte;

      flipped_byte = flipByte(controller_response_buf[1]);
      controller_response_buf[1] = flipped_byte;


      controller_response = (((uint32_t) controller_response_buf[2] << 24) | 
                             ((uint32_t) controller_response_buf[1] << 16) |
                             ((uint32_t) controller_response_buf[0] << 8)  |
                             ((uint32_t) controller_response_buf[3] ));
                              

      // Serial.print(controller_response_buf[0], BIN); 
      // Serial.print(controller_response_buf[1], BIN);
      // Serial.print(controller_response_buf[2], BIN);
      // Serial.print(controller_response_buf[3], BIN);
      // Serial.println();
      
      //Serial.println(controller_response, BIN);
      encode_byte_to_out_comp(controller_response, comp_vals);
      
    } 

  for(int i = 0; i <= 7; i++){
    duration[i] = hf_pulseIn(command_pin, 1000);
    
    if(duration[i] > 100)    joybus_command &= ~(1<<(7-i)); //greater than 2us? = 0
    else if(duration[i] > 3) joybus_command |= (1<<(7-i));  //else = 1, greater than 3 means we didn't timeout
    else return;//genius!  
  }

/* ~~~debugging the info command detection~~

  Serial.print(duration[0]);Serial.print("  ");
  Serial.print(duration[1]);Serial.print("  ");
  Serial.print(duration[2]);Serial.print("  ");
  Serial.print(duration[3]);Serial.print("  ");
  Serial.print(duration[4]);Serial.print("  ");
  Serial.print(duration[5]);Serial.print("  ");
  Serial.print(duration[6]);Serial.print("  ");
  Serial.println(duration[7]);
  Serial.println(joybus_command, HEX);
  if (joybus_command == 0) Serial.println("Info???");
*/


  if (joybus_command == 1){
    
      //disable_poll_counter();

      delayMicroseconds(3);
    
      //Clear the data pin
      GPIO7_DR_SET = data_pin;
      
      //SEND IT! triggers the pulses read from comp_vals
      GPT1_CR |= GPT_CR_EN;
      //GPT2_CR |= GPT_CR_EN; don't think we need to do this

      delayMicroseconds(200);//wait for the signal to go
      
  }

  else if (joybus_command == 0)
  {

    //Serial.println("Got info request");
    encode_byte_to_out_comp(device_info, comp_vals);

    //Clear the data pin
      GPIO7_DR_SET = data_pin;
      
      //SEND IT! triggers the pulses read from comp_vals
      GPT1_CR |= GPT_CR_EN;

      delayMicroseconds(200);//wait for the signal to go

      Serial.println("Sent device info 0x0500"); // This might be too many bytes, joybus asks for 3, we're giving it 4

  }

}

void encode_byte_to_out_comp(uint32_t cntrllr_bytes, uint32_t out_B[])
{
  //Serial.println(cntrllr_bytes, HEX);
  //Serial.println(", ");

  for (int i = 0; i < 32; i++){
    if ((cntrllr_bytes >> i) & 1U){
      out_B[i] = comp_1;
      // Serial.print(out_B[i], DEC);
      // Serial.print(" ");
    }
    else {
      out_B[i] = comp_3;
      // Serial.print(out_B[i], DEC);
      // Serial.print(" ");
    }
  }
  //Serial.println();
  out_B[32] = comp_1;
  
}

void gpt2_isr() {
  
  
  GPT2_SR |= GPT_SR_OF3;  // clear all set bits
  GPIO7_DR_SET = data_pin;
  while (GPT2_SR & GPT_SR_OF1); // wait for clear
  
}

void gpt1_isr() {
  
  
  GPT1_SR |= GPT_SR_OF3;  // clear all status register bits
  GPT2_CR &= ~GPT_CR_EN;  //disable the set pin timer
  
  GPT2_OCR1 = comp_vals[res_bit]; //load the next interval, 3us or 1us compare val
  if (res_bit > 32)//check for the end of the buffer
  {
    res_bit = 0;  //reset the idx
    GPT1_CR &= ~(GPT_CR_EN);//halts the pulses
  }
  else {
    res_bit++;
    GPIO7_DR_CLEAR = data_pin;//clear the pin so the HW can read the next interval
    GPT2_CR |= GPT_CR_EN;     //enables and clears the GPT2 set pin timer
  }
  
  while (GPT1_SR & GPT_SR_OF1); // wait for clear
  
}

void my_isr() {  // compare
  TMR4_CSCTRL2 &= ~(TMR_CSCTRL_TCF1);
  poll = 1;
  while(TMR4_CSCTRL2 & TMR_CSCTRL_TCF1);
}
