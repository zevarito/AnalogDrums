
#define DEBUG 1

#define BAUD_RATE 115200

// current reading pad
byte currentAnalogIndex = 0;

// Basic struct to handle input/instrument mapping
struct Pad {
  byte noteOn;
  byte noteOff;
  byte note;
};

// Input / Instruments mapping
Pad instruments[6] = {
  {144, 128, 38}, // Snare
  {145, 129, 51}, // Ride
  {146, 130, 35}, // Bass Drum
  {147, 131, 42}, // Closed Hi Hat
  {148, 132, 50}, // High Tom
  {149, 133, 41} // Low Floor Tom
};

// Setup our program.
// Notice that we are not using MIDI Baud Rate because we will be
// broadcasting MIDI messages through Serial port that a computer
// should recive and convert to real MIDI Input.
void setup() {
  Serial.begin(BAUD_RATE);
  currentAnalogIndex = 0;
  
  // Faster Analog Read
  // http://forum.arduino.cc/index.php/topic,6549.0.html
  // defines for setting and clearing register bits
  //
  #define FASTADC 1
  //
  #ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
  #endif
  #ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
  #endif

  // set prescale to 16
  #if FASTADC
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  #endif
}

unsigned long lastMilliRead = 0;
unsigned long lastPlayedMillis = 0;

int lastSendVelocity = 0;

void loop() {
  
  int reading = analogRead(0);
  
  // We are over min stroke threshold
  if (reading > 50) {// && millis() > lastPlayedMillis + 100) {
    
    // Select best reading
    int passes;
    
    while(passes < 16) {
      int newReading = analogRead(0);
      
      if (reading < newReading)
        reading = newReading;
        
      passes++;
    }
    
    playInstrument(0, reading / 8);
    
    lastPlayedMillis = millis();
    
  } else {
    return;
  }
}

void debugInstrument(byte index, int velocity) {
  turnLedOn();
  Serial.println("Debug Instrument");
  Serial.println(index);
  Serial.println(instruments[index].noteOn);
  Serial.println(velocity * 8);
  turnLedOff();
}

void playInstrument(byte index, int velocity) {
  // Turn led on, NoteOn, NoteOff and then turn led off.
  turnLedOn();
  midiMsg(instruments[index].noteOff, instruments[index].note, 127);
  
  midiMsg(instruments[index].noteOn, instruments[index].note, velocity);
  //delay(1);
  //midiMsg(instruments[index].noteOff, instruments[index].note, 127);
  turnLedOff();
}

void turnLedOn() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void turnLedOff() {
  digitalWrite(LED_BUILTIN, LOW);
}

void midiMsg(byte channel, byte note, byte velocity) {
  Serial.write(channel);
  Serial.write(note);
  Serial.write(velocity);
}

