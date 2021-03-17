// --------------------------------------------------
//
// Simple test for Ableton <> Arduino connection
//     -> where and 8-bit control byte and a 16-bit 
//        value is sent back and forth between 
//        Ableton and Arduino-compatible HW.
//
// Four bytes are expected from Ableton
// Byte 1: 255 (sync byte)
// Byte 2: <control> - [0:255]
// Byte 3: <value MSB> - [0:255]
// Byte 4: <value LSB> - [0:255]
//
// Written by mo thunderz (last update: 2.1.2021)
//
// --------------------------------------------------

#define CONNECTED_TIMEOUT 500000 // 0.5 second time for LED to indicate a command has been received

#define ID_SONG_BEAT 252
#define ID_SONG_MEASURE 251

// pin assignments
#define LED_1         8
#define LED_2         9
#define LED_3         10
#define LED_4         11
#define LED_COMS      6
#define LED_MEASURE   7

// internal variables
int rx_state = 0;
byte cc_type1;
byte cc_type2;
byte cc_val1;
byte cc_val2;

// needed for indication LED (goes on on arduino when serial info is received)
unsigned long   t = 0;                // current time
unsigned long   serial_t0 = 0;     // last time serial info was received

// simple function to merge most significant byte and least significant byte to single int
int bytesToInt(int l_highByte, int l_lowByte) {
  return ((unsigned int)l_highByte << 8) + l_lowByte;
}

void setup() {
  pinMode(LED_COMS, OUTPUT);   // initialize LED for serial connection verification
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_MEASURE, OUTPUT);

  digitalWrite(LED_COMS, false);  
  digitalWrite(LED_1, false);
  digitalWrite(LED_2, false);
  digitalWrite(LED_3, false);
  digitalWrite(LED_4, false);
  digitalWrite(LED_MEASURE, false);
  
  Serial.begin(38400);            // IMPORTANT: in MAX the same data rate must be used! Feel free to experiment with higher rates. 
}

void loop() {
  t = micros();           // take timestamp

  //-----------------------------update timeout led--------------------------------//
  if (digitalRead(LED_COMS))
    if((t - serial_t0) > CONNECTED_TIMEOUT)
      digitalWrite(LED_COMS, 0);

  //----------Check if control commands have been received from Ableton------------//
  if (Serial.available()) {
    serial_t0 = t;
    if (digitalRead(LED_COMS) == 0)
      digitalWrite(LED_COMS, 1);
      
    rx_state++;
    switch (rx_state) {
      case 1:                     // first byte is always 255 for sync
        cc_type1 = Serial.read();
        if(cc_type1 != 255) {     // reset if first is not 255 sync byte
          rx_state = 0;
        }
        break;
      case 2:                     // second is the control byte
        cc_type2 = Serial.read();
        break;        
      case 3:                     // third is the most significant byte of the value
        cc_val1 = Serial.read();     
        break;
      case 4:                     // fourth is the least significant byte of the value
        cc_val2 = Serial.read();
        rx_state = 0;

        int control = cc_type2;
        int value = bytesToInt(cc_val1, cc_val2);  

        if (control == ID_SONG_BEAT) {
          digitalWrite(LED_1, false);
          digitalWrite(LED_2, false);
          digitalWrite(LED_3, false);
          digitalWrite(LED_4, false);
          
          if(value == 1)
            digitalWrite(LED_1, true);
          else if (value == 2)
            digitalWrite(LED_2, true);
          else if (value == 3)
            digitalWrite(LED_3, true);
          else if (value == 4)
            digitalWrite(LED_4, true);
        }
        else if (control == ID_SONG_MEASURE) {
          if(value == 0)
            digitalWrite(LED_MEASURE, 0);
          else
            digitalWrite(LED_MEASURE, !digitalRead(LED_MEASURE));
        }
        
        
        break;
    }
  }
}
