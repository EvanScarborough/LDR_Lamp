#include "PinChangeInt.h"
#include "IO.cpp"
#include "Network.cpp"
#include "State.cpp"

ESP wifi;




//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv ONLY EDIT STUFF BELOW THIS!

int nightHour = 9;
int nightMinute = 30;

int dayHour = 4+12;
int dayMinute = 30;

int dayBrightness = 15;
int nightBrightness = 10;
bool clockAtNight = true; // Automatically go to clock mode at night

bool Audio::silent = false; // This is only for making sounds when changing modes

bool Input::katrina = true; // set this to true, KC, it's to account for hardware differences :)

String wifiName = "2G FBI Van";//"ATT621";
String wifiPassword = "mightyocean278";//"9703015988";

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ONLY EDIT STUFF ABOVE THIS!



















State current;
State lastSync;




int retries = 0;

bool isNight = false;
bool brightnessChanged = false;


Adafruit_NeoPixel* Lights::pixels = new Adafruit_NeoPixel();
Color** Lights::strip;
int Lights::brightness = 20;

//std::map<int,bool> Input::buttonLock = std::map<int,bool>();
bool Input::modesPressed = false;
bool Input::centerPressed = false;
bool Input::sendPressed = false;

bool Input::enc1val = true;
bool Input::enc2val = true;
int Input::ENCODER = 0;
bool Input::encLock = false;
unsigned long Input::sinceLastTouch = 2001;

unsigned long secondCounter = 0;

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv STARTUP
void setup() {
  Serial.begin(115200);

  Lights::begin();
  Input::begin();
  Audio::begin();

  PCintPort::attachInterrupt(Input::ENC1, enc1, CHANGE);
  PCintPort::attachInterrupt(Input::ENC2, enc2, CHANGE);

  if(nightHour < 12) nightHour += 12;
  
  for(int i = 0; i < 32; i++){
    Lights::setAtHeight(0,Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0));
    current.hue-=2;
    Lights::streamFromBottom();
    Lights::display();
    delay(10);
  }
  for(int i = 0; i < 32; i++){
    Lights::streamFromBottom();
    Lights::display();
    delay(10);
  }

  if(Input::katrina){
    wifi.WiFiName = wifiName;
    wifi.WiFiPass = wifiPassword;
  }
  wifi.begin();

  secondCounter = millis();
}


int menuTime = 10*100;
int modeSelectTimer = -1;






void loop() {
  unsigned long startTime = millis();
  while(startTime - secondCounter > 1000){
    current.addTime(1);
    secondCounter+=1000;
  }
  Input::update();
  
  if(Input::sinceLastTouch >= 1000){
    Input::sinceLastTouch = 2000;
  }
  if(Input::sinceLastTouch >= 1000 || wifi.querying || wifi.connectionStatus < wifi.CONNECTED){
    State retState = wifi.update();
    if(retState.mode >= 0){
      if(!lastSync.isEqual(retState)){
        Serial.println("GOT NEW STATE:");
        retState.printout();
        current.set(retState);
        lastSync.set(retState);
        lastSync.mode = retState.mode;
        if(!isNight){
          Audio::playTone(Audio::C,50);
          Audio::playTone(Audio::C*4,30);
        }
      }
      else{
        current.hour = retState.hour;
        current.minute = retState.minute;

        if((current.hour > nightHour || (current.hour == nightHour && current.minute > nightMinute)) || (current.hour < dayHour || (current.hour == dayHour && current.minute > dayMinute))){
          if(!isNight){
            Serial.println("Set to Night");
            Lights::setBrightness(nightBrightness);
            if(clockAtNight) current.mode = State::CLOCK_MODE;
            isNight = true;
            wifi.queryTime = 100*60*5;
          }
        }
        else if(isNight){
          Serial.println("Set to Day");
          Lights::setBrightness(dayBrightness);
          if(clockAtNight) current.mode = retState.mode;
          isNight = false;
          wifi.queryTime = 100*30;
        }
        
        Serial.print(current.hour);Serial.print(":");Serial.println(current.minute);
        //current.second = retState.second;
        Serial.println("No update.");
      }
    }
  }

  if(Input::pressed(Input::SEND)){
    // Post current state
    if(wifi.postState(current)){
      lastSync.set(current);
    }
  }

  if(modeSelectTimer >= 0) modeSelectTimer++;
  if(Input::sinceLastTouch >= 500 && modeSelectTimer >= 0){
    modeSelectTimer = -1;Lights::fill(Color(0,0,0));
  }
  
  if(Input::pressed(Input::MODES)){
    current.mode = (current.mode+1)%current.NUM_MODES;
    Audio::modeNote(current.mode);
  }

  if(Input::down(Input::MODES)){
    Lights::brightness += Input::ENCODER;
    if(Lights::brightness < 0) Lights::brightness=0;
    if(Lights::brightness > 100) Lights::brightness=100;
    if(Input::ENCODER != 0) Lights::setBrightness(Lights::brightness);
    if(isNight) nightBrightness = Lights::brightness;
    else dayBrightness = Lights::brightness;
    Input::ENCODER=0;
  }

  if(Input::sinceLastTouch == 1){
    wifi.queryTime = 100*30;
  }

  if(current.mode == State::COLOR_MODE){
    colorMode();
  }
  else if(current.mode == State::CLOCK_MODE){
    clockMode();
  }
  else if(current.mode == State::SPLIT_MODE){
    splitMode();
  }
  else if(current.mode == State::SPINNY_MODE){
    spinnyMode();
  }
  else if(current.mode == State::BLINK_MODE){
    blinkMode(current.temp);
  }
  else if(current.mode == State::FALLING_MODE){
    fallingMode();
  }
  else if(current.mode == State::HEART_MODE){
    heartMode();
  }
  else if(current.mode == State::RAINBOW_MODE){
    rainbowMode();
  }
  
  Lights::display();

  int loopTime = millis() - startTime;
//  Serial.println(loopTime);
  if(loopTime<10)delay(10-loopTime);
}






int colorInputType = 0;
void colorInput(){
  if(colorInputType == 0){ // hue
    current.hue += Input::ENCODER*2;
    while(current.hue < 0) current.hue+=360;
    while(current.hue >= 360) current.hue-=360;
    Input::ENCODER=0;
  }
  if(colorInputType == 1){ // secondary
    current.feeling += Input::ENCODER*2;
    while(current.feeling < 0) current.feeling+=360;
    while(current.feeling >= 360) current.feeling-=360;
    Input::ENCODER=0;
  }
  else if(colorInputType == 2){ // sat
    current.sat -= Input::ENCODER*2;
    if(current.sat < 0) current.sat=0;
    if(current.sat >= 100) current.sat=100;
    Input::ENCODER=0;
  }
  else{ // lum
    current.lum -= Input::ENCODER*2;
    if(current.lum < 0) current.lum=0;
    if(current.lum >= 100) current.lum=100;
    Input::ENCODER=0;
  }
  if(Input::pressed(Input::CENTER)){colorInputType++;colorInputType=colorInputType%4;
    if(colorInputType == 1){
      if(current.mode == current.COLOR_MODE){colorInputType++;colorInputType=colorInputType%4;}
      else Lights::fill(Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0));
    }
  }
  if(Input::sinceLastTouch >= 500)colorInputType=0;
}
void displayInputMode(){
  if(colorInputType == 2){ // saturation
    for(int i = 0; i < 64; i++){
      if(current.sat >= float(i)*100.0/64.0 && current.sat < float(i+1)*100.0/64.0){
        Lights::setPixelInLine(i,Color::getFromHSL(current.hue+180,1,0.5));
      }
      else{
        float satAt = float(i)*100.0/64.0; if(satAt > 100) satAt = 100; if(satAt < 0) satAt = 0;
        Lights::setPixelInLine(i,Color::getFromHSL(current.hue,satAt/100.0,(float)current.lum/100.0));
      }
    }
  }
  if(colorInputType == 3){ // lum
    for(int i = 0; i < 64; i++){
      if(current.lum >= float(i)*100.0/64.0 && current.lum < float(i+1)*100.0/64.0){
        Lights::setPixelInLine(i,Color::getFromHSL(current.hue+180,1,0.5));
      }
      else{
        float lumAt = float(i)*100.0/64.0; if(lumAt > 100) lumAt = 100; if(lumAt < 0) lumAt = 0;
        Lights::setPixelInLine(i,Color::getFromHSL(current.hue,(float)current.sat/100.0,lumAt/100.0));
      }
    }
  }
}






void colorMode(){
  colorInput();
  Lights::setAtHeight(0,Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0));
  Lights::streamFromBottom();
  displayInputMode();
}





void clockMode(){
  if(Input::ENCODER>0)current.addTime(Input::ENCODER*60);
  if(Input::ENCODER<0)current.subTime(Input::ENCODER*60);
  Input::ENCODER=0;
  Lights::fill(Color(0,0,0));
  Lights::bigHand(current.hour,Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0));
  Lights::littleHand(current.minute,Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0));
  Lights::littleHand(current.second,Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0));
}




void weatherMode(){
//  if(Input::pressed(Input::CENTER)){
//    wifi.openWeather.query(
//  }
}



void splitMode(){
  colorInput();
  Lights::setAtHeight(0,Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0));
  Lights::streamFromBottom();
  Lights::fadeAll(Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0),0.07);
  displayInputMode();
}


int spinnyModeCounter = 0;
int spinnySpeed = 8;
void spinnyMode(){
  spinnyModeCounter++;
  if(spinnyModeCounter >= 64*spinnySpeed) spinnyModeCounter = 0;
  colorInput();
  if(spinnyModeCounter%spinnySpeed==0){
    Lights::spin();
    Lights::setPixelInLine(0,Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0));
    Lights::fadeTo(0,Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0),float(abs(32*spinnySpeed - spinnyModeCounter))/float(spinnySpeed*32));
  }
  displayInputMode();
}


int blinkModeCounter = 0;
void blinkMode(int blinkSpeed){
  blinkModeCounter++;
  if(blinkModeCounter >= blinkSpeed*2) blinkModeCounter = 0;
//  colorInput();
  current.temp += Input::ENCODER*2;
  if(current.temp < 10) current.temp=10;
  while(current.temp >= 360) current.temp=360;
  Input::ENCODER=0;
  
  float fval = -1 + (float(abs(blinkSpeed - blinkModeCounter))/float(blinkSpeed))*3;
  if(fval < 0) fval = 0; if(fval > 1) fval = 1;
  Lights::fill(Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0));
  Lights::fadeAll(Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0),fval);
  
//  displayInputMode();
}

int fallingModeCounter = 0;
void fallingMode(){
  fallingModeCounter++;
  colorInput();
  if(fallingModeCounter%2==0){
    Lights::streamFromTop();
    Lights::fadeAtHeight(31,Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0),0.1);
  }
  if(fallingModeCounter == 32){
    Lights::setAtHeight(31,Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0));
    fallingModeCounter=0;
  }
  displayInputMode();
}


int heartModeCounter = 0;
void heartMode(){
  heartModeCounter++;
  if(heartModeCounter >= 100) heartModeCounter = 0;
  colorInput();

  if(heartModeCounter == 50){
    Lights::fill(Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0));
    Lights::fadeAll(Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0),0.3);
  }
  if(heartModeCounter == 77){
    Lights::fill(Color::getFromHSL(current.feeling,(float)current.sat/100.0,(float)current.lum/100.0));
  }
  Lights::fadeAll(Color::getFromHSL(current.hue,(float)current.sat/100.0,(float)current.lum/100.0),0.03);
  
  displayInputMode();
}


int rainbowModeCounter = 0;
void rainbowMode(){
  rainbowModeCounter++;
  if(rainbowModeCounter >= 360) rainbowModeCounter = 0;
  Lights::spin();
  Lights::setPixelInLine(0,Color::getFromHSL(rainbowModeCounter,1,0.5));
}





void enc1(){
  if(!Input::encLock && !Input::enc1val && !Input::enc2val){
    Input::ENCODER--;
//    Serial.println("minus");
    Input::sinceLastTouch = 0;
    Input::encLock = true;
  }
  Input::enc1val = digitalRead(Input::ENC1);
  if(Input::enc1val && Input::enc2val) Input::encLock = false;
}
void enc2(){
  if(!Input::encLock && !Input::enc1val && !Input::enc2val){
    Input::ENCODER++;
//    Serial.println("plus");
    Input::sinceLastTouch = 0;
    Input::encLock = true;
  }
  Input::enc2val = digitalRead(Input::ENC2);
  if(Input::enc1val && Input::enc2val) Input::encLock = false;
}


