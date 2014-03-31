
#define DEBUG 0

// Output serial communication rate
#define BAUD_RATE 115200

// Basic struct to handle input/instrument mapping
struct Pad {
  byte noteOn;
  byte noteOff;
  byte note;
  unsigned long lastPlayedMillis;
};

/* 
 * Configurable stuff
 */

// Threshold value to read signal as a valid stroke.
#define INPUT_SIGNAL_MINIMUN_STROKE 100

// Silent time in millisecons an instrument should wait before.
// be read his input again.
#define PLAYED_AGAIN_WAITTIME_MILLISECS 24

// Analog reads amount needed to define the best stroke.
#define RESOLUTION_PASSES 64

// Signal (1024 max) will be divided for this number and the
// result will be constrain to 127 max as it is the maximun
// velocity value accepted by MIDI std.
#define INPUT_SIGNAL_DIVISOR 8

// Input / Instruments mapping
Pad instruments[6] = {
  {144, 128, 38, 0}, // Snare
  {145, 129, 51, 0}, // Ride
  {146, 130, 35, 0}, // Bass Drum
  {147, 131, 42, 0}, // Closed Hi Hat
  {148, 132, 50, 0}, // High Tom
  {149, 133, 41, 0}  // Low Floor Tom
};

// Setup our program.
// Notice that we are not using MIDI Baud Rate because we will be
// broadcasting MIDI messages through Serial port that a computer
// should recive and convert to real MIDI Input.
void setup() {
  Serial.begin(BAUD_RATE);
  
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

void loop() {
  
  int reading;
  
  reading = readInputInstrument(0, 3);
  if (reading > 0) {
    playInstrument(3, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127));
  }
  
  reading = readInputInstrument(5, 0);
  if (reading > 0) {
    playInstrument(0, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127)); 
  }
}

int readInputInstrument(int analogPort, int instrumentIndex) {
  // If we are not over passed silent time, we return unsuccessful.
  if (millis() < instruments[instrumentIndex].lastPlayedMillis + PLAYED_AGAIN_WAITTIME_MILLISECS)
    return -1;
    
  return readInput(analogPort);
}

// Reads analog port and determines if it is a valid keystroke or not.
// Will return -1 if a value couldn't be returned otherwise > 0 value.
int readInput(int analogPort) {
    
  // Perform first Analog read.
  int reading = analogRead(analogPort);
  
  // We are not over min stroke threshold?
  if (reading < INPUT_SIGNAL_MINIMUN_STROKE)
    return -1;
    
  // Attempt to select best reading
  int passes;
    
  while(passes < RESOLUTION_PASSES) {
    int newReading = analogRead(analogPort);
      
    if (reading < newReading)
      reading = newReading;
        
    passes++;
  }
    
  return reading;
}

void debugInstrument(byte index, int reading) {
  turnLedOn();
  Serial.println("Debug Instrument");
  Serial.println(index);
  Serial.println(reading);
  turnLedOff();
}

void playInstrument(int index, int velocity) {
  turnLedOn();
  midiMsg(instruments[index].noteOff, instruments[index].note, 127);
  midiMsg(instruments[index].noteOn, instruments[index].note, velocity);
  instruments[index].lastPlayedMillis = millis();
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

