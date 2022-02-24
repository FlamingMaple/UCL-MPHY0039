#include <compat/deprecated.h>
#include <FlexiTimer2.h>
#include <Servo.h>
// http://www.arduino.cc/playground/Main/FlexiTimer2

// All definitions
#define HEADERLEN 4
#define PACKETLEN (HEADERLEN + 3)
#define SAMPFREQ 256 // ADC sampling rate 256 Hz
#define TIMER2VAL (1024/(SAMPFREQ)) // Set 256Hz sampling frequency

// Define servo objects for fingers
Servo s_1; // Finger 1
Servo s_2; // Finger 2
Servo s_3; // Finger 3
Servo s_4; // Finger 4
Servo s_T; // Thumb

// What GPIO is each fingers' servo connected to?
#define FINGER1 3
#define FINGER2 5
#define FINGER3 6
#define FINGER4 10
#define THUMB 11

int M[5]; // Motor command control integer array which is read from serial

// Used for storing servo values during serial read
unsigned char a1, a2; 

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

 // Attaches the servo on the GPIO (e.g. 3 default for finger 1) to the desired servo object
 s_1.attach(FINGER1);  
 s_2.attach(FINGER2); 
 s_3.attach(FINGER3);
 s_4.attach(FINGER4);
 s_T.attach(THUMB);
 delay (500);

 // Writes default position values to each servo
 s_1.write(90);
 s_2.write(90);
 s_3.write(90);
 s_4.write(90);
 s_T.write(120);

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
  // The loop() handles reading the desired servo values over serial and applying them
   if (Serial.available() > 9) {
      for (int i = 0; i<5; i++) {
        // Read the servo positions over serial and parse them into the correct format
        a1 = Serial.read();
        M[i] = (int) a1;
        M[i] = M[i]*256;
        a2 = Serial.read();
        M[i] |= (int) a2;
      }

      s_1.write(M[0]);
      s_2.write(M[1]);
      s_3.write(M[2]);
      s_4.write(M[3]);
      s_T.write(M[4]);

   }
}
