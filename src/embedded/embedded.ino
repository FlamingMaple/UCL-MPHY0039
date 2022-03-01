#include <compat/deprecated.h>
#include <FlexiTimer2.h>

// All definitions
#define SAMPFREQ 256 // ADC sampling rate 256 Hz
#define TIMER2VAL (1024/(SAMPFREQ)) // Set 256Hz sampling frequency

// Global constants and variables
// These are volatile as they are handled in the ISR
volatile unsigned int ADC_value = 0; // ADC current value

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

// Interrupt service routine (ISR) for reading ADC and transmitting packet from buffer
void Timer2_Overflow_ISR() {
  // Read ADC channel 0 and split the value into two bytes in transmit buffer
  ADC_value = analogRead(0);

  // Print ADC value over serial
  Serial.println(ADC_value);
}

void loop() {
}
