#ifndef IO
#define IO

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

using namespace std;




class Color{
public:
  int r = 0;
  int g = 0;
  int b = 0;
  Color(){}
  Color(int rr, int gg, int bb){
    while(rr > 255) rr-=255;
    while(rr < 0) rr+=255;
    while(gg > 255) gg-=255;
    while(gg < 0) gg+=255;
    while(bb > 255) bb-=255;
    while(bb < 0) bb+=255;
    r = rr; g = gg; b = bb;
  }
  void set(Color c){
    r = c.r;
    g = c.g;
    b = c.b;
  }
  static float floatAbs(float f){
    if(f < 0) f *= -1;
    return f;
  }
  static float floatMod(float f, float m){
    if(f < 0) f *= -1;
    return f;
  }
  static Color getFromHSL(int h, float s, float l){
    while(h<0)h+=360;
    while(h>=360)h-=360;
//    Serial.print(h);Serial.print("\t");
//    Serial.print(s);Serial.print("\t");
//    Serial.print(l);Serial.print("\t");
    float c = (1.0 - floatAbs(2.0*l - 1.0)) * s;
//    Serial.print(c);Serial.print("\t");
    float x = c * (1.0 - floatAbs(float(fmod((float(h)/60.0),2.0)) - 1.0));
//    Serial.print(h/60);Serial.print("\t");
//    Serial.print(x);Serial.print("\t");
    float m = l-c/2.0;
//    Serial.print(m);Serial.print("\t");
    float rp = 0; float gp = 0; float bp = 0;
    if(h<60){rp=c;gp=x;}
    else if(h>=60&&h<120){rp=x;gp=c;}
    else if(h>=120&&h<180){gp=c;bp=x;}
    else if(h>=180&&h<240){gp=x;bp=c;}
    else if(h>=240&&h<300){rp=x;bp=c;}
    else{rp=c;bp=x;}
//    Serial.print(rp);Serial.print("\t");
//    Serial.print(gp);Serial.print("\t");
//    Serial.print(bp);Serial.print("\t");
//
//    Serial.print((rp+m)*255);Serial.print("\t");
//    Serial.print((gp+m)*255);Serial.print("\t");
//    Serial.print((bp+m)*255);Serial.print("\n");
    
    return Color((rp+m)*255,(gp+m)*255,(bp+m)*255);
  }
};







class Input{
public:
  static const int MODES = 2;
  static const int CENTER = 3;
  static const int SEND = 4;

  static const int ENC1 = 10;
  static const int ENC2 = 11;

  static bool katrina;

  static bool enc1val;
  static bool enc2val;
  static int ENCODER;
  static bool encLock;

  static unsigned long sinceLastTouch;

//  static std::map<int,bool> buttonLock;
  static bool modesPressed;
  static bool centerPressed;
  static bool sendPressed;

  static void update(){
    sinceLastTouch++;
    if(down(MODES) || down(SEND) || down(CENTER)) sinceLastTouch=0;
    if(!down(MODES))modesPressed=false;
    if(!down(CENTER))centerPressed=false;
    if(!down(SEND))sendPressed=false;
//    for(auto a:buttonLock){
//      if(!digitalRead(a.first)){
//        //buttonLock.insert(a.first,false);
//        buttonLock[a.first] = false;
//      }
//    }
  }

  static void begin(){
    pinMode(MODES,INPUT);
    pinMode(SEND,INPUT);
    pinMode(CENTER,INPUT);
  
    pinMode(ENC1,INPUT);
    pinMode(ENC2,INPUT);

    // Attach interrupts
  }

  static bool down(int b){
    if(!katrina && b==CENTER) return !digitalRead(b);
    return digitalRead(b);
  }

  static bool pressed(int b){
    if(b == MODES){
      if(down(b) && !modesPressed){
        modesPressed = true;
        return true;
      }
    }
    if(b == CENTER){
      if(down(b) && !centerPressed){
        centerPressed = true;
        return true;
      }
    }
    if(b == SEND){
      if(down(b) && !sendPressed){
        sendPressed = true;
        return true;
      }
    }
//    if(buttonLock.find(b) == buttonLock.end()){
//      buttonLock[b] = false;
//    }
//    if(digitalRead(b)){
//      bool ret = digitalRead(b) && !buttonLock[b];
//      buttonLock[b] = true;
//      return ret;
//    }
    return false;
  }
  
};











class Lights{
public:
  static const int pixelPin = A2;
  static const int numPixels = 64;
  
  static Adafruit_NeoPixel* pixels;
  
  static int brightness;
  static void setBrightness(int b){
    if(b < 0) b = 0;
    if(b > 255) b = 255;
    brightness = b;
    pixels->setBrightness(b);
  }
  
  static Color** strip;


  static void begin(){
    pixels = new Adafruit_NeoPixel(numPixels, pixelPin, NEO_GRB + NEO_KHZ800);
    pixels->begin();
    setBrightness(brightness);
    
    strip = new Color*[2];
    for(int i = 0; i < 2; i++){
      strip[i] = new Color[32];
    }
    
  }

  static void display(){
    if(Input::katrina){
      for(int i = 56; i < 64; i++){
        Color c = strip[1][i-56];
        pixels->setPixelColor(i,pixels->Color(c.r,c.g,c.b));
      }
      for(int i = 0; i < 32-8; i++){
        Color c = strip[1][i+8];
        pixels->setPixelColor(i,pixels->Color(c.r,c.g,c.b));
      }
      for(int i = 0; i < 32; i++){
        Color c = strip[0][i];
        pixels->setPixelColor(55-i,pixels->Color(c.r,c.g,c.b));
      }
    }
    else{
      for(int i = 0; i < 32; i++){
        Color c = strip[1][i];
        pixels->setPixelColor(i,pixels->Color(c.r,c.g,c.b));
        c = strip[0][i];
        pixels->setPixelColor(63-i,pixels->Color(c.r,c.g,c.b));
      }
    }
    pixels->show();
  }



  static void fill(Color c){
    for(int i = 0; i < 2; i++){
      for(int j = 0; j < 32; j++){
        strip[i][j] = c;
      }
    }
  }
  static void setPixelInLine(int n, Color c){
    while(n>=64)n-=64;
    while(n<0)n+=64;
    if(n<=23) strip[1][n+8]=c;
    else if(n<=55) strip[0][55-n]=c;
    else strip[1][n-56]=c;
  }
  static Color getPixelInLine(int n){
    while(n>=64)n-=64;
    while(n<0)n+=64;
    if(n<=23) return strip[1][n+8];
    else if(n<=55) return strip[0][55-n];
    return strip[1][n-56];
  }
  static void setSide(int s, Color c){
    int start = 32 + s*16;
    for(int i = start; i < start+16; i++){
      setPixelInLine(i,c);
    }
  }
  static void fadeTo(int n, Color c, float factor){
    if(factor<0)factor=0;if(factor>1)factor=1;
    Color current = getPixelInLine(n);
    setPixelInLine(n,Color((float)current.r+factor*(float)(c.r-current.r),(float)current.g+factor*(float)(c.g-current.g),(float)current.b+factor*(float)(c.b-current.b)));
  }
  static void fadeAtHeight(int h, Color c, float factor){
    if(h < 0) h=0; if(h>31) h=31;
    Color current = strip[0][h];
    strip[0][h] = Color((float)current.r+factor*(float)(c.r-current.r),(float)current.g+factor*(float)(c.g-current.g),(float)current.b+factor*(float)(c.b-current.b));
    current = strip[1][h];
    strip[1][h] = Color((float)current.r+factor*(float)(c.r-current.r),(float)current.g+factor*(float)(c.g-current.g),(float)current.b+factor*(float)(c.b-current.b));
  }
  static void fadeAll(Color c, float factor){
    for(int i = 0; i < 64; i++){
      fadeTo(i,c,factor);
    }
  }

  static void streamFromBottom(){
    for(int i = 31; i > 0; i--){
      strip[0][i] = strip[0][i-1];
      strip[1][i] = strip[1][i-1];
    }
  }
  static void streamFromTop(){
    for(int i = 0; i < 31; i++){
      strip[0][i] = strip[0][i+1];
      strip[1][i] = strip[1][i+1];
    }
  }
  static void setAtHeight(int h, Color c){
    if(h < 0) h=0; if(h>31) h=31;
    strip[0][h] = c;
    strip[1][h] = c;
  }
  static void spin(){
    Color temp = strip[1][0];
    for(int i = 0; i < 31; i++){
      strip[1][i] = strip[1][i+1];
    }
    strip[1][31] = strip[0][31];
    for(int i = 31; i > 0; i--){
      strip[0][i] = strip[0][i-1];
    }
    strip[0][0] = temp;
  }


  static void flashColor(Color c){
    Lights::setAtHeight(0,c);
    for(int i = 0; i < 32; i++){
      Lights::streamFromBottom();
      Lights::display();
      delay(10);
    }
  }

  static void littleHand(int t, Color c){
    int pn = 24;
    for(int i = 0; i < t; i++){
      pn--;
      if(pn==23 || pn==7 || pn==39 || pn==55) pn--;
      while(pn<0)pn+=64;
    }
    setPixelInLine(pn,c);
    if(pn==24 || pn==8 || pn==40 || pn==56) setPixelInLine(pn-1,c);
  }
  static void bigHand(int t, Color c){
    littleHand(t*5,c);
    littleHand(t*5+1,c);
//    littleHand(t*5-1,c);
  }
  
};

















class Audio{
public:
  static const int BUZZER = 7;

  static bool silent;

  static void begin(){
    pinMode(BUZZER,OUTPUT);
  }

  static void playTone(double note, int t){
    tone(BUZZER,note);
    delay(t);
    noTone(BUZZER);
  }

  static void modeNote(int mode){
    if(silent) return;
    int notetime = 50;
    if(mode == 0) playTone(C,notetime);
    if(mode == 1) playTone(D,notetime);
    if(mode == 2) playTone(E,notetime);
    if(mode == 3) playTone(F,notetime);
    if(mode == 4) playTone(G,notetime);
    if(mode == 5) playTone(A*2,notetime);
    if(mode == 6) playTone(B*2,notetime);
    if(mode == 7) playTone(C*2,notetime);
  }
  
  
  static const double Gs = 415.305;
  static const double G = 391.995;
  static const double Fs = 369.994;
  static const double F = 349.228;
  static const double E = 329.628;
  static const double Ds = 311.127;
  static const double D = 293.665;
  static const double Cs = 277.183;
  static const double C = 261.626;
  static const double B = 246.942;
  static const double As = 233.082;
  static const double A = 220.000;
};








#endif
