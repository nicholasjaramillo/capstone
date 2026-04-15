/* 
 * Project SEI - Smart Emotion Interpretation
 * Author: Nicholas Jaramillo
 * Date: 4/8/2026
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"
#include "Button.h"
#include "neopixel.h"
#include "Colors.h"
#include "DFRobotDFPlayerMini.h"
#include "MAX30105.h"
#include "spo2_algorithm.h"
#define FPSerial Serial1


SYSTEM_MODE(SEMI_AUTOMATIC);

// vairables 
const int PIXELCOUNT = (12);
int Pixel = 0;
int startPixel = 0;
int endPixel = 12;
int pixelNumber;
int track;
int color;
int currentTime;
int lastMinute;
int command;
int pulseSense = D14;
int readSense = D19;
int bufferLength; 

// functions
void lightTime(int startPixel, int endPixel, int color);
void configureHeartSense();
void takeRead();
void idle();
void startUp();

//oxygen
int32_t spo2; 
int8_t validSPO2; 
//heart
int32_t heartRate; 
int8_t validHeartRate;

// objects
MAX30105 particleSensor;
DFRobotDFPlayerMini myDFPlayer;

Adafruit_NeoPixel pixel (PIXELCOUNT,SPI1,WS2812B);




void setup() {
// configure heart sensor
Serial.begin(9600);
waitFor(Serial.isConnected, 5000);
pinMode(pulseSense, OUTPUT);
pinMode(readSense, OUTPUT);
Wire.begin();
Wire.setSpeed(CLOCK_SPEED_400KHZ);
if (!particleSensor.begin(Wire, I2C_SPEED_FAST, 0x57)){ 
    Serial.printf("MAX30105 was not found. Please check wiring/power.\n");
}
else{
  Serial.printf("good to go\n");
}
configureHeartSense();
delay(2000);
// ..................................................................................
// configure everything else
Serial1.begin(9600);
pixel.begin();
pixel.show(); 
Serial2.begin(9600);
myDFPlayer.begin(Serial1); 
myDFPlayer.volume(10); 
delay(2000);
startUp();
}

//...................................................................................
void loop() {
//idle 
for(pixelNumber = startPixel; pixelNumber <= endPixel; pixelNumber++){  
pixel.setPixelColor(pixelNumber,color);
pixel.setBrightness(100);    
}
pixel.show();
// every minute play line
if (( currentTime - lastMinute ) >60000){
idle();
lastMinute = millis();
}
// in 6 minutes turn off
if (( currentTime - lastMinute ) >300000){
startUp();
lastMinute = millis();
}

// exit idle take and decipher read
if(Serial2.available()){
  int commandId2 = Serial2.read();
  Serial.printf("%i\n",commandId2);
  switch(commandId2){
    case 0:
      track = random(145,148);
      myDFPlayer.play(track);
      delay(4000);
    case 22:
      track = random(139,144);
      myDFPlayer.play(track);
      delay(7000);
      takeRead();
    if(validHeartRate == 0){
      track = random(135,138);
      myDFPlayer.play(track);
      color = violet;
      lightTime(startPixel, endPixel, color);
      delay(4000);
      }
    else{
      if(heartRate < 95 && heartRate > 70){    
        track = random(17,24);
        myDFPlayer.play(track);
        delay(2000);
        track = random(25,32);
      }
        if(heartRate < 70 || heartRate > 95 ){
          color = red;
          lightTime(startPixel, endPixel, color);
          track = random(1,16);
          myDFPlayer.play(track);
          delay(2000);
          track = random(33,45);                  
          myDFPlayer.play(track);
          delay(5000);
        }
    }
    break;
  }
}
}



//...................................................................................
// play random track when in idle mode
void idle(){
color = purple;
lightTime(startPixel, endPixel, color);
track = random(67,134);
myDFPlayer.play(track);
delay(7000);
}

//...................................................................................
//start the code
void startUp(){
while(!Serial2.available()){
color = black;
lightTime(startPixel, endPixel, color);
}
if(Serial2.available()){
  int commandId = Serial2.read();
  Serial.printf("%i\n",commandId);
  if ( commandId == 19){
    track = random(53,66);
    myDFPlayer.play(track);
    color = purple;
    lightTime(startPixel, endPixel, color);
    delay(4000);
    track = random(46,52);
    myDFPlayer.play(track);
    delay(8000);
  }
}
}

//...................................................................................
//change light color
void lightTime(int startPixel, int endPixel, int color){
for(pixelNumber = startPixel; pixelNumber <= endPixel; pixelNumber++){  
pixel.setPixelColor(pixelNumber,color);
pixel.setBrightness(255);    
}
pixel.show();
}

//...................................................................................
// take reads and average them
void takeRead(){
int bufferLength = 100;
uint32_t irBuffer[100]; 
uint32_t redBuffer[100];
for (byte i = 0 ; i < bufferLength ; i++){
    particleSensor.check();
    particleSensor.available(); 
      
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();      
    digitalWrite(readSense, !digitalRead(readSense));
    particleSensor.nextSample();
    Serial.printf("red=%lu \nIR = %lu\n\n",redBuffer[i],irBuffer[i]);   
}
maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
Serial.printf("heart validity: %i heart rate: %lu\n \n oxygen validity: %i oxygen saturation: %lu\n", validHeartRate, heartRate, validSPO2, spo2);
}

//...................................................................................
// set heart sensor
void configureHeartSense(){
byte ledBrightness = 255; 
byte sampleAverage = 8; 
byte ledMode = 2; 
byte sampleRate = 200;
int pulseWidth = 411; 
int adcRange = 16384;
particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}
