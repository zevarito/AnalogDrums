

int outPin = 13;
int analogPin = 0;

unsigned char SNARE = 38;
unsigned char RIDE = 51;

unsigned char NOTE_OFF = 128;
unsigned char NOTE_ON = 144;
unsigned char PITCH = RIDE;
unsigned char VELOCITY = 127;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  int reading;
  
  reading = analogRead(analogPin);
  
  if(reading > 200)
  {   
      Msg(reading / 10);
  }
  
}

void Msg(byte data) 
{
    digitalWrite(outPin, HIGH);

    Serial.write(NOTE_ON);
    Serial.write(PITCH);
    Serial.write(data);
    delay(1);
    Serial.write(NOTE_OFF);
    Serial.write(PITCH);
    Serial.write(data);
    
    digitalWrite(outPin,LOW);
}

