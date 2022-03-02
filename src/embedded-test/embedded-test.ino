#include <compat/deprecated.h>
#include <FlexiTimer2.h>
#include <CircularBuffer.h>

// All definitions
#define SAMPFREQ 256 // ADC sampling rate 256 Hz
#define TIMER2VAL (1024/(SAMPFREQ)) // Set 256Hz sampling frequency
#define NUM_AVE 30
// Global constants and variables
// These are volatile as they are handled in the ISR
CircularBuffer<unsigned int,NUM_AVE> ADCBuffer;
volatile unsigned int average;
volatile unsigned int tick = 0;
volatile bool needs_avg = false;

void setup() {
 Serial.begin(57600); // Begins serial communications at baud rate of 57600
 delay(100);

 noInterrupts(); // Disable all interrupts before initialization is complete

 // Timer2 is used to setup the analag channels sampling frequency and packet update.
 // Whenever interrupt occures, the current read packet is sent to the PC
 FlexiTimer2::set(TIMER2VAL, Timer2_Overflow_ISR);
 FlexiTimer2::start();

 interrupts(); // Enable all interrupts after initialization has been completed
}

// Interrupt service routine (ISR) for reading ADC
void Timer2_Overflow_ISR() {
  ADCBuffer.push(analogRead(0));
  if (tick >= (NUM_AVE - 1)) {
    needs_avg = true;
  }
  tick++;
}

void loop() {
  if (needs_avg) {
    needs_avg = false;
    float inst_voltage = 0;
    float squared_voltage = 0;
    float sum_squared_voltage = 0;
    float mean_square_voltage = 0;
    float root_mean_square_voltage = 0;


    for (int n=0; n<NUM_AVE; n++) {
      // inst_voltage calculation from raw ADC input goes here.
      inst_voltage = float(ADCBuffer[n]) / 1024.00 * 3.30;
      //Serial.print(ADCBuffer[n]);
      //Serial.print(",");
      //Serial.print(inst_voltage);
      //Serial.println();
      Serial.print(inst_voltage);
      Serial.print(",");
      inst_voltage /= 2480.00;
      Serial.print(inst_voltage);
      Serial.println();
      squared_voltage = inst_voltage * inst_voltage;
      sum_squared_voltage += squared_voltage;
    }
    
    mean_square_voltage = sum_squared_voltage / NUM_AVE;
    root_mean_square_voltage = sqrt(mean_square_voltage);
    //Serial.println(root_mean_square_voltage);
    
    /*
    unsigned int total = 0;
    for (unsigned int i = 0; i < NUM_AVE; i++) {
      total = total + mean_buffer[i];
    }
    float mean_calc = float(total) / float(NUM_AVE);
    Serial.println(mean_calc);
    mean_calc = 0;
    */
  }
}
