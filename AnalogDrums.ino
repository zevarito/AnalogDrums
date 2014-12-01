#define DEBUG 0

#if DEBUG
long lastMillis = 0;
long loops = 0;
#endif

// Output serial communication rate
#define BAUD_RATE 115200

/* 
 * Configurable stuff
 */

// Threshold value to read signal as a valid stroke.
#define INPUT_SIGNAL_MINIMUN_STROKE 10

// Silent time in millisecons an instrument should wait before.
// be read his input again.
#define PLAYED_AGAIN_WAITTIME_MILLISECS 24

// Analog reads amount needed to define the best stroke.
#define RESOLUTION_PASSES 1

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
  float velocityMultiplier;
  int waitTimeMillis;
};

// Function headers
int readInputInstrument(int analogPort, Pad *pInstrument);
void resetInstrument(Pad *pInstrument);
void playInstrument(Pad *pInstrument, int velocity);

// Input / Instruments mapping
Pad instruments[7] = {
  {144, 128, 38, 0, 0, 0, false, 1.8, PLAYED_AGAIN_WAITTIME_MILLISECS}, // Snare
  {145, 129, 51, 0, 0, 0, false, 1.8, PLAYED_AGAIN_WAITTIME_MILLISECS}, // Ride
  {146, 130, 35, 0, 0, 0, false, 1.8, PLAYED_AGAIN_WAITTIME_MILLISECS}, // Bass Drum
  {147, 131, 42, 0, 0, 0, false, 1.8, PLAYED_AGAIN_WAITTIME_MILLISECS}, // Closed Hi Hat
  {148, 132, 50, 0, 0, 0, false, 1.8, PLAYED_AGAIN_WAITTIME_MILLISECS}, // High Tom
  {149, 133, 41, 0, 0, 0, false, 1.8, PLAYED_AGAIN_WAITTIME_MILLISECS}, // Low Floor Tom
  {150, 134, 49, 0, 0, 0, false, 1.8, PLAYED_AGAIN_WAITTIME_MILLISECS}  // Crash
};

#define SNARE  0
#define RIDE   1
#define BDRUM  2
#define CHIHAT 3
#define HITOM  4
#define LFTOM  5
#define CRASH  6

#ifndef LED_BUILTIN
//static const uint8_t LED_BUILTIN = 13;
#endif

// Setup our program.
// Notice that we are not using MIDI Baud Rate because we will be
// broadcasting MIDI messages through Serial port that a computer
// should recive and convert to real MIDI Input.
void setup() {
  Serial.begin(BAUD_RATE);
}

byte INPUT_INSTRUMENT_READING_LOOP[6][2] = {
  {0, BDRUM},
  {1, CHIHAT},
  {2, SNARE},
  {3, RIDE},
  {4, SNARE},
  {5, RIDE},
};

int currentInputInstrumentIndex = 0;

void loop() {
  
  #if DEBUG
    long currentMillis = millis();
    loops++;
  #endif
  
  int reading;
  Pad *pInstrument;
  
  currentInputInstrumentIndex = 0;
  
  while(currentInputInstrumentIndex<6) {
    pInstrument = &instruments[INPUT_INSTRUMENT_READING_LOOP[currentInputInstrumentIndex][1]];
    reading = readInputInstrument(INPUT_INSTRUMENT_READING_LOOP[currentInputInstrumentIndex][0], pInstrument);
    if (reading > 0)
      playInstrument(pInstrument, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127));
    
     currentInputInstrumentIndex++;
    
  }
  
  #if DEBUG
    if(currentMillis - lastMillis > 1000){
      Serial.print("Loops last second:");
      Serial.println(loops);
    
      lastMillis = currentMillis;
      loops = 0;
    }
  #endif
}

// Reads analog port and determines if it is a valid keystroke or not.
// Will return -1 if a value couldn't be returned otherwise > 0 value.
int readInputInstrument(int analogPort, Pad *pInstrument) {

  // If we are not over passed silent time, we return unsuccessful.
  if (pInstrument->isSeekingBestStroke || millis() > pInstrument->lastPlayedMillis + pInstrument->waitTimeMillis)
    pInstrument->isSeekingBestStroke = true;
  else
    return -1;
    
  // Perform first Analog read.
  int reading = analogRead(analogPort);
  
  pInstrument->readingPasses++;
  
  // We are not over min stroke threshold?
  if (reading < INPUT_SIGNAL_MINIMUN_STROKE)
    return -1;

  // Attempt to select best reading
  if (reading > pInstrument->bestReading) {
    pInstrument->bestReading = reading;
  }
    
  if (pInstrument->readingPasses >= RESOLUTION_PASSES) {
    if (pInstrument->bestReading >= INPUT_SIGNAL_MINIMUN_STROKE) {
      return pInstrument->bestReading;
    } else {
      resetInstrument(pInstrument);
      return -1;
    }
  }
  else
    return -1;
}

void resetInstrument(Pad *pInstrument) {
  pInstrument->lastPlayedMillis = millis();
  pInstrument->readingPasses = 0;
  pInstrument->bestReading = 0;
  pInstrument->isSeekingBestStroke = false;
}

void playInstrument(Pad *pInstrument, int velocity) {
  turnLedOn();
  
  // Send MIDI output message, first off then on.
  midiMsg(pInstrument->noteOff, pInstrument->note, 127);
  midiMsg(pInstrument->noteOn, pInstrument->note, (float)velocity * pInstrument->velocityMultiplier);
  
  resetInstrument(pInstrument);
  
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

