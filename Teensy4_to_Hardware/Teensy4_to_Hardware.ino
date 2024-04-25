/*  
*   ~SWR~
*   N64 Retro-fit Contoller Project
*
*   Essentially a Joybus driver. 
*   Contsantly writes the controller input from Serial1 to a buffer, 
*   Manchester enocdes the buffer in pulse invertals as defined by Joybus,
*   writes the pulses to the data pin upon receiving a poll for the Nintendo 64 (on the same pin).
*
*   This sketch is uploaded to a Teensy 4.1 that is wired to a Bluetooth board in the CENTRAL role
*   via hardware Serial1.
*
*/

/*TODO 
* >implement controller signal on poll
* >write controller ID response for N64 startup
* >clean up
*
*/

uint32_t controller_response = 0x0;
uint8_t controller_response_buf[4] = {0};


volatile int stop_flag = 0;
volatile int poll = 0;

//150MHz intervals
const uint32_t comp_4 = 600;//4us
const uint32_t comp_3 = 407;//3us
const uint32_t comp_1 = 112; //1us

//init buffer
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
      //leave off at start ^  | sour ce poll pin  | TODO poll pin needs to be the same as data pin
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

int master_count = 0;

void setup() {

    Serial.begin(115200);
    Serial.println("Crash Test...");
    Serial1.begin(1000000);
    
    config_pin();
    
    poll_counter_init();
    config_timers();
    //enable_poll_counter();
    
    
}



void loop() {

 // if (poll){
    
      //disable_poll_counter();
    
      //Clear the data pin
      //GPIO7_DR_SET = data_pin;
      encode_byte_to_out_comp(controller_response, comp_vals);
      GPT1_CR |= GPT_CR_EN;
       //GPT2_CR |= GPT_CR_EN;
      //delayMicroseconds(200);//wait for the signal to go
      delay(5);

        if(Serial1.available()){

          
          Serial1.readBytes((uint8_t *) controller_response_buf, 4);
          controller_response = (((uint32_t) controller_response_buf[0] << 24) | 
                                 ((uint32_t) controller_response_buf[1] << 16) |
                                 ((uint32_t) controller_response_buf[2] << 8) |
                                 ((uint32_t) controller_response_buf[3] ));
          // Serial.print(controller_response_buf[0], BIN); 
          // Serial.print(controller_response_buf[1], BIN);
          // Serial.print(controller_response_buf[2], BIN);
          // Serial.print(controller_response_buf[3], BIN);
          // Serial.println();
          
          //Serial.println(controller_response, BIN);
          
        } 

      poll = 0;
      //enable_poll_counter();//enable for new poll
      
   //}
   

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
