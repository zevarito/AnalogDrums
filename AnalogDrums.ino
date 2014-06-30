
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
  float velocityMultiplier;
};

// Function headers
int readInputInstrument(int analogPort, Pad *pInstrument);
void playInstrument(Pad *pInstrument, int velocity);

// Input / Instruments mapping
Pad instruments[7] = {
  {144, 128, 38, 0, 0, 0, false, 1.2}, // Snare
  {145, 129, 51, 0, 0, 0, false, 1.2}, // Ride
  {146, 130, 35, 0, 0, 0, false, 1.1}, // Bass Drum
  {147, 131, 42, 0, 0, 0, false, 1.2}, // Closed Hi Hat
  {148, 132, 50, 0, 0, 0, false, 1.2}, // High Tom
  {149, 133, 41, 0, 0, 0, false, 1.2}, // Low Floor Tom
  {150, 134, 49, 0, 0, 0, false, 1.2}  // Crash
};

#ifndef LED_BUILTIN
static const uint8_t LED_BUILTIN = 13;
#endif

// Setup our program.
// Notice that we are not using MIDI Baud Rate because we will be
// broadcasting MIDI messages through Serial port that a computer
// should recive and convert to real MIDI Input.
void setup() {
  Serial.begin(BAUD_RATE);
}

void loop() {
  
  int reading;
  Pad *pInstrument;
 
  pInstrument = &instruments[6];
  reading = readInputInstrument(0, pInstrument);
  if (reading > 0)
    playInstrument(pInstrument, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127));

  pInstrument = &instruments[2];
  reading = readInputInstrument(1, pInstrument);
  if (reading > 0)
    playInstrument(pInstrument, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127));

  pInstrument = &instruments[0];
  reading = readInputInstrument(2, pInstrument);
  if (reading > 0)
     playInstrument(pInstrument, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127));

  pInstrument = &instruments[3];
  reading = readInputInstrument(3, pInstrument);
  if (reading > 0)
    playInstrument(pInstrument, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127));

  pInstrument = &instruments[5];
  reading = readInputInstrument(4, pInstrument);
  if (reading > 0)
    playInstrument(pInstrument, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127));

  pInstrument = &instruments[1];
  reading = readInputInstrument(5, pInstrument);
  if (reading > 0)
    playInstrument(pInstrument, constrain(reading / INPUT_SIGNAL_DIVISOR, 0, 127)); 

}

// Reads analog port and determines if it is a valid keystroke or not.
// Will return -1 if a value couldn't be returned otherwise > 0 value.
int readInputInstrument(int analogPort, Pad *pInstrument) {

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
  }

  // Attempt to select best reading
  if (reading > pInstrument->bestReading) {
    pInstrument->bestReading = reading;
    pInstrument->isSeekingBestStroke = true;
  }
    
  if (pInstrument->readingPasses >= RESOLUTION_PASSES)
    return pInstrument->bestReading;
  else
    return -1;
}

void playInstrument(Pad *pInstrument, int velocity) {
  turnLedOn();
  
  // Send MIDI output message, first off then on.
  midiMsg(pInstrument->noteOff, pInstrument->note, 127);
  midiMsg(pInstrument->noteOn, pInstrument->note, (float)velocity * pInstrument->velocityMultiplier);
  
  // Reset note
  pInstrument->lastPlayedMillis = millis();
  pInstrument->readingPasses = 0;
  pInstrument->bestReading = 0;
  pInstrument->isSeekingBestStroke = false;
  
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

