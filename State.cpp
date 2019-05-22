#ifndef STATE
#define STATE

#include <Arduino.h>
using namespace std;


class State{
public:
  int mode = COLOR_MODE;
  int hue = 381;
  int sat = 100;
  int lum = 50;
  int weather = CLEAR;
  int feeling = 0;
  int temp = 80;

  int hour = 0;
  int minute = 0;
  int second = 0;
  void addTime(int numsecs){
    second+=numsecs;
    while(second >= 60){
      minute++;
      second-=60;
    }
    while(minute >= 60){
      hour++;
      minute-=60;
    }
    while(hour >= 24){
      hour-=24;
    }
  }
  void subTime(int numsecs){
    second-=numsecs;
    while(second < 0){
      minute--;
      second+=60;
    }
    while(minute < 0){
      hour--;
      minute+=60;
    }
    while(hour < 0){
      hour+=24;
    }
  }
  
  bool sleep = false;




  bool isEqual(State s){
    if(hue!=s.hue)return false;
    if(sat!=s.sat)return false;
    if(mode!=s.mode)return false;
    if(lum!=s.lum)return false;
    if(weather!=s.weather)return false;
    if(feeling!=s.feeling)return false;
    if(temp!=s.temp)return false;
    return true;
  }
  void set(State s){
    hue = s.hue;
    sat = s.sat;
    lum = s.lum;
    weather = s.weather;
    feeling = s.feeling;
    temp = s.temp;
    if(mode != CLOCK_MODE){mode=s.mode;}
//    if(s.mode==FEELING_MODE || s.mode == WEATHER_MODE){
//      mode = s.mode;
//    }
  }





  static int nextInt(String& s, int i, int def){
    int r = 0;
    bool found = false;
//    Serial.println();
    for(int j = i; j < s.length(); j++){
//      Serial.print(s.charAt(j));
      if(isdigit(s.charAt(j))){
        r = r*10+((int)s.charAt(j) - 48);
        found = true;
      }
      else if(found){
        break;
      }
    }
    if(!found) return def;
    return r;
  }
  static int nextInt(String& s, String ss, int def){
    int i = s.indexOf(ss);
    if(i<0)return def;
    return nextInt(s,i,def);
  }

  void setTimestamp(String& s){
    int i = s.indexOf("Date");
    int spaceCount = 0;
    while(i<s.length()){
      if(s.charAt(i)==' ')spaceCount++;
      i++;
      if(spaceCount == 5) break;
    }
    int h = nextInt(s,i,0) - 5;
    if(h<0)h+=24;
    i+=2;
    int m = nextInt(s,i,0);
    i+=2;
    int sec = nextInt(s,i,0);
//    Serial.print("TIME: ");
//    Serial.print(h);
//    Serial.print(":");
//    Serial.println(m);
    hour = h;
    minute = m;
    //second = sec;
  }

  static State fromJSON(String& s){
//    Serial.println(s);
    State ret;
    ret.hue = nextInt(s,"hue",0);
    ret.sat = nextInt(s,"sat",100);
    ret.lum = nextInt(s,"lum",50);
    ret.mode = nextInt(s,"mode",COLOR_MODE);
    ret.feeling = nextInt(s,"feeling",0);
    ret.weather = nextInt(s,"weather",CLEAR);
    ret.temp = nextInt(s,"temp",80);
    ret.setTimestamp(s);
    return ret;
  }

  void putWeather(String& s){
    Serial.println(s);
  }

  static State empty(){
    State ret;
    ret.mode = -5;
    return ret;
  }



  


  static const int NUM_MODES = 8;

  static const int COLOR_MODE = 0;
  static const int CLOCK_MODE = 1;
  static const int SPLIT_MODE = 2;
  static const int SPINNY_MODE = 3;
  static const int BLINK_MODE = 4;
  static const int FALLING_MODE = 5;
  static const int HEART_MODE = 6;
  static const int RAINBOW_MODE = 7;

  static const int CLEAR = 0;
  static const int PART_CLOUDY = 1;
  static const int CLOUDY = 2;
  static const int RAIN = 3;
  static const int SNOW = 4;

  void printout(){
    Serial.print(hour);Serial.print(":");Serial.print(minute);Serial.print(":");Serial.println(second);
    Serial.println(mode);
    Serial.println(hue);
    Serial.println(sat);
    Serial.println(lum);
    Serial.println(feeling);
    Serial.println(weather);
    Serial.println(temp);
  }
};



#endif
