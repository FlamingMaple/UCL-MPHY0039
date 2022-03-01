#include <compat/deprecated.h>
#include <FlexiTimer2.h>

// All definitions
#define HEADERLEN 4
#define PACKETLEN (HEADERLEN + 3)
#define SAMPFREQ 256 // ADC sampling rate 256 Hz
#define TIMER2VAL (1024/(SAMPFREQ)) // Set 256Hz sampling frequency

// Global constants and variables
// These are volatile as they are handled in the ISR
volatile unsigned char TXBuf[PACKETLEN]; // The transmission packet
volatile unsigned char TXIndex; // Next byte to write in the transmission packet.
volatile unsigned int ADC_value = 0; // ADC current value

void setup() {
 Serial.begin(57600); // Begins serial communications at baud rate of 57600
 establishContact();
 delay(100);

 noInterrupts(); // Disable all interrupts before initialization is complete

 // Write packet header and footer
 TXBuf[0] = 0xa5;  // Sync 0
 TXBuf[1] = 0x5a;  // Sync 1
 TXBuf[2] = 2;     // Protocol version
 TXBuf[3] = 0;     // Packet counter
 TXBuf[4] = 0x02;  // ADC High Byte
 TXBuf[5] = 0x00;  // ADC Low Byte
 TXBuf[6] = 0x01;  // Switches state

 // Timer2 is used to setup the analag channels sampling frequency and packet update.
 // Whenever interrupt occures, the current read packet is sent to the PC
 FlexiTimer2::set(TIMER2VAL, Timer2_Overflow_ISR);
 FlexiTimer2::start();

 interrupts(); // Enable all interrupts after initialization has been completed
}

// Holds Arduino in while loop while no serial data in input buffer
// Once data recieved, if character 'S', then code continues
// If not, then we wait!
void establishContact() {
  bool waiting = true;
  while (waiting) {
    while (Serial.available() <= 0) {
      delay(250);
    }
    char t = Serial.read();
    if (t == 'S') {
      waiting = false;
    }
  }
}

// Interrupt service routine (ISR) for reading ADC and transmitting packet from buffer
void Timer2_Overflow_ISR() {
  // Read ADC channel 0 and split the value into two bytes in transmit buffer
  ADC_value = analogRead(0);
  TXBuf[4] = ((unsigned char)((ADC_value & 0xFF00) >> 8)); // Write most significant byte
  TXBuf[5] = ((unsigned char)(ADC_value & 0x00FF));	// Write least significant byte

  // Send Packet by iterating over the 7 bytes in transmit buffer
  for (TXIndex=0; TXIndex<7; TXIndex++){
    Serial.write(TXBuf[TXIndex]);
  }

  // Increment the packet counter
  TXBuf[3]++;
}

void loop() {
}
