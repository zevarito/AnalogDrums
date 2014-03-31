
#define DEBUG 0

// Output serial communication rate
#define BAUD_RATE 115200

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

// Basic struct to handle input/instrument mapping
struct Pad {
  byte noteOn;
  byte noteOff;
  byte note;
  unsigned long lastPlayedMillis;
  int readingPasses;
  int bestReading;
  bool isSeekingBestStroke;
};

// Input / Instruments mapping
Pad instruments[6] = {
  {144, 128, 38, 0, 0, 0, false}, // Snare
  {145, 129, 51, 0, 0, 0, false}, // Ride
  {146, 130, 35, 0, 0, 0, false}, // Bass Drum
  {147, 131, 42, 0, 0, 0, false}, // Closed Hi Hat
  {148, 132, 50, 0, 0, 0, false}, // High Tom
  {149, 133, 41, 0, 0, 0, false}  // Low Floor Tom
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

// Reads analog port and determines if it is a valid keystroke or not.
// Will return -1 if a value couldn't be returned otherwise > 0 value.
int readInputInstrument(int analogPort, int instrumentIndex) {

  Pad *pInstrument = &instruments[instrumentIndex];

  // If we are not over passed silent time, we return unsuccessful.
  if (millis() < pInstrument->lastPlayedMillis + PLAYED_AGAIN_WAITTIME_MILLISECS)
    return -1;

  // Perform first Analog read.
  int reading = analogRead(analogPort);
  
  // We are not over min stroke threshold?
  if (pInstrument->isSeekingBestStroke == false && reading < INPUT_SIGNAL_MINIMUN_STROKE) {
    return -1;
  } else {
    pInstrument->readingPasses++;
    //Serial.println(pInstrument->readingPasses);
  }

  // Attempt to select best reading
  if (reading > pInstrument->bestReading)
    pInstrument->bestReading = reading;
    
  if (pInstrument->readingPasses >= RESOLUTION_PASSES)
    return pInstrument->bestReading;
  else
    return -1;
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
  instruments[index].readingPasses = 0;
  instruments[index].bestReading = 0;
  instruments[index].isSeekingBestStroke = false;
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

