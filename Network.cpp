#ifndef NETWORK
#define NETWORK

#include <Arduino.h>
#include "State.cpp"
#include "IO.cpp"
using namespace std;

#define esp Serial3

class OpenWeather{
public:
  String key = "3a16ecd6392e5a256ae039113d340d74";

  bool query(String zipcode, State& into){
    Serial.println("Getting weather at " + zipcode);
    esp.println("AT+CIPSTART=\"TCP\",\"api.openweathermap.org\",80");
    if (esp.find("Error")) return;
    String cmd = "GET /data/2.5/weather?zip="+zipcode+",us&APPID="+key+" HTTP/1.0\r\nHost: api.openweathermap.org\r\n\r\n";
    esp.print("AT+CIPSEND=");
    esp.println(cmd.length());
    int timeout = 0;
    while(!esp.available()){timeout++;if(timeout>1000){Serial.println("Timeout");return false;}delay(1);}
    String resp = esp.readString();
    if(resp.indexOf(">")<0){
      Serial.print("Unknown Response: ");Serial.println(resp);
      return false;
    }
    esp.print(cmd);


    int tt = 0;
    while(true){
      tt++;
      if(esp.available()){
        String m = esp.readString();
        Serial.println(m);
        if(m.indexOf("CLOSE") >= 0){
          if(m.indexOf("OK") >= 0){
            Serial.println("Success!");
            into.putWeather(m);
            return true;
          }
        }
      }
      if(tt >= 800){
        Serial.println("Timeout");
        return false;
      }
      delay(10);
    }
    return true;
  }
};



class JSONbin{
public:
  String binName = "5b8611fad6fe677c48d1fffe";
  String key = "$2a$10$Rr2eQZ4MEuZO6mc2iR/iiup8KCiB.VL8zNtrJMcQs.NXBnA/nes1G";

  bool startQuery(){
    esp.println("AT+CIPSTART=\"TCP\",\"api.jsonbin.io\",80");
    if(esp.find("Error")) return false;
    String cmd = "GET /b/"+binName+"/latest HTTP/1.0\r\nsecret-key: "+key+"\r\nHost: api.jsonbin.io\r\n\r\n";
    esp.print("AT+CIPSEND=");
    esp.println(cmd.length());
    return true;
  }
  bool sendQuery(){
    Serial.println("send query");
    String cmd = "GET /b/"+binName+"/latest HTTP/1.0\r\nsecret-key: "+key+"\r\nHost: api.jsonbin.io\r\n\r\n";
    esp.print(cmd);
  }


  bool postState(State state){
    Serial.println("Posting...");

    Lights::fill(Color::getFromHSL(state.hue,(float)state.sat/100.0,(float)state.lum/100.0));
    Lights::setAtHeight(0,Color::getFromHSL(state.hue+180,(float)state.sat/100.0,(float)state.lum/100.0));
    Lights::setAtHeight(31,Color::getFromHSL(state.hue+180,(float)state.sat/100.0,(float)state.lum/100.0));
    for(int i = 0; i < 60; i++){
      Lights::spin();
      Lights::display();
      delay(10);
    }
    
    esp.println("AT+CIPSTART=\"TCP\",\"api.jsonbin.io\",80");
//    Serial.println("AT+CIPSTART=\"TCP\",\"api.jsonbin.io\",80");
    if(esp.find("Error")) return;
  
    String data = "{";
    data += "\"temp\": "+String(state.temp)+",";
    data += "\"weather\": "+String(state.weather)+",";
    data += "\"feeling\": "+String(state.feeling)+",";
    data += "\"lum\": "+String(state.lum)+",";
    data += "\"sat\": "+String(state.sat)+",";
    data += "\"hue\": "+String(state.hue)+",";
    data += "\"mode\": "+String(state.mode);
    data += "}";
    
    String cmd = "PUT /b/"+binName+" HTTP/1.0\r\n";
    cmd += "secret-key: "+key+"\r\n";
    cmd += "Content-Type: application/json\r\n";
    cmd += "Content-Length: "+String(data.length())+"\r\n";
    cmd += "versioning: false\r\n";
    cmd += "Host: api.jsonbin.io\r\n\r\n";
    cmd += data+"\r\n";
    cmd += "\r\n";
    
    esp.print("AT+CIPSEND=");
    esp.println(cmd.length());
//    Serial.print("AT+CIPSEND=");
//    Serial.println(cmd.length());
    delay(200);
    
    esp.flush();
    String resp = esp.readString();
    if(resp.indexOf(">")<=0){
      Serial.print("Unexpected result: ");Serial.println(resp);
      return false;
    }
    else{
      Serial.println("Sending");
    }
    esp.print(cmd);

    int tt = 0;
    while(true){
      tt++;
      if(esp.available()){
        String m = esp.readString();
        Serial.println(m);
        if(m.indexOf("CLOSE") >= 0){
          if(m.indexOf("OK") >= 0){
            Serial.println("Success!");
            Lights::fill(Color::getFromHSL(state.hue,(float)state.sat/100.0,(float)state.lum/100.0));
            Lights::flashColor(Color(0,255,0));
            return true;
          }
        }
      }
      if(tt >= 800){
        Serial.println("Timeout");
        Lights::fill(Color::getFromHSL(state.hue,(float)state.sat/100.0,(float)state.lum/100.0));
        Lights::flashColor(Color(255,0,0));
        return false;
      }
      Lights::spin();
      Lights::display();
      delay(10);
    }
    
    return false;
  }
  
};






class ESP{
public:
  int connectionStatus = NONE;

  // ************************************************************************************************
//  String WiFiName = "ATT621";
//  String WiFiPass = "9703015988";
  String WiFiName = "2G FBI Van";
  String WiFiPass = "mightyocean278";
  String zipcode = "77019";

  OpenWeather openWeather;
  JSONbin jsonBin;
  
  ESP(){}

  void begin(){
    Serial.println("Begin");
    esp.begin(115200);
  }

  bool checkChip(){
    Serial.println("Check Chip");
    esp.println("AT+RST");
    delay(1000);
    esp.println("AT");
    delay(10);
    if(esp.available()){
      String response = esp.readString();
      Serial.println(response);
      if(response.indexOf("OK") >= 0){
        connectionStatus = CHIPFOUND;
      }
      else{
        connectionStatus = NOCHIP;
        return false;
      }
    }
    else{
      Serial.println("Nothing Found");
      connectionStatus = NOCHIP;
      return false;
    }
    return true;
  }

  bool connectWiFi(){
    Serial.print("\nAttempting to connect to ");
    Serial.println(WiFiName);
    String cmd = "AT+CWJAP=\"" + WiFiName + "\",\"" + WiFiPass + "\"";
    //String cmd = "AT+CIPSTATUS";
    esp.println(cmd);
    connectionStatus = TRYINGCONNECT;
    return true;
  }


  String message = "";
  bool readSerial(){
    bool ret = false;
    if(esp.available()){
    }
    while(esp.available()){
      message += ((char)esp.read());
      ret = true;
    }
    return ret;
  }


  int lastTimer = 1000;
  int timer = 0;
  void setTimer(int t){if(t>100*60*60*24*10)t=-1; timer = t; lastTimer = t;}


  int queryTime = 4*15*100;
  int queryTimer = 300;
  bool querying = false;
  int queryType = 0;
  
  State update(){
    if(querying){return updateQuery();}
    
    if(timer < 0) return State::empty();
    if(timer > 0) timer--;

    bool gotMessage = readSerial();
    if(gotMessage){
//      Serial.println();
//      Serial.print(message);
    }
    
    if(timer==0 && (connectionStatus == NONE || connectionStatus == NOCHIP || connectionStatus == DISCONNECTED)){
      checkChip();
      Serial.println(connectionStatus);
      if(connectionStatus == NOCHIP){setTimer(lastTimer*2);return State::empty();}
      setTimer(100);
      return State::empty();
    }
    if(timer == 0 && connectionStatus == CHIPFOUND){
      connectWiFi();
      timer = 500;
    }
    if(timer==0 && connectionStatus == TRYINGCONNECT){
      setTimer(lastTimer*2);
      connectionStatus = CHIPFOUND;
      Serial.println("Timeout... Trying again.");
      message = "";
    }
    if(gotMessage && connectionStatus == TRYINGCONNECT){
      if(message.indexOf("GOT") >= 0){
        Serial.println("Connected!");
        connectionStatus = CONNECTED;
        message = "";
      }
      else if(message.indexOf("FAIL") >= 0){
        setTimer(lastTimer*2);
        connectionStatus = CHIPFOUND;
        Serial.println("Failed... Trying again.");
        message = "";
      }
    }


    if(connectionStatus == CONNECTED){
      if(queryTimer > -1){queryTimer--;}
//      if(queryTimer%100==0&&queryTimer>0)Serial.println(queryTimer);

      if(queryTimer == 0){
        querying = true;
        //jsonBin.startQuery();
        queryTimer = queryTime;
        ttt = 0;
        tryCount = 0;
      }

      
    }
    
    return State::empty();
  }

  int ttt = 0;
  int tryCount = 0;
  State updateQuery(){
    if(esp.available()){
      String m = esp.readString();
      if(m.indexOf("CLOSE") >= 0){
        querying = false;
        return State::fromJSON(m);
      }
    }
    ttt++;
    if(ttt==50){
      if(tryCount >= 1){
        querying = false;
        Serial.println("Timeout");
        return State::empty();
      }
      jsonBin.startQuery();
    }
    if(ttt==100){
      jsonBin.sendQuery();
      tryCount++;
      ttt = 0;
    }
    if(ttt>=12000){
      querying = false;
      Serial.println("Timeout");
      return State::empty();
    }
    return State::empty();
  }



  bool postState(State state){
    return jsonBin.postState(state);
  }

  
  static const int NONE = 0;
  static const int NOCHIP = -100;
  static const int CHIPFOUND = 1;
  static const int TRYINGCONNECT = 2;
  static const int CONNECTED = 3;
  static const int DISCONNECTED = -1;

  static const int JSON = 0;
  static const int WEATHER = 1;
};






/*
#define esp Serial3
class ESP{
public:
  static String WiFiName;
  static String WiFiPass;

  static int retryTimer;
  static int connectionStatus;

  static String response;
  static String nextSend;

  static String update(){
    if(esp.available()){
      response += esp.readString();
    }
    if(response.indexOf(">") >= 0 && nextSend.length() > 0){ // time to send something
      esp.println(nextSend);
      nextSend = "";
      response = "";
    }
    if(response.indexOf("CLOSED") >= 0){
      return response;
      response = "";
    }
    return "";
  }

  static bool begin(){
    esp.begin(115200);
    if(!contactChip()) return false;
    if(!connectWiFi()) return false;
    return true;
  }
  static bool contactChip(){
    Serial.println("Contacting ESP chip...");
    esp.println("AT");
    delay(50);
    if(esp.available()){
      String response = esp.readString();
      if(response.indexOf("OK") >= 0){
        Serial.println("ESP chip responded!");
      }
      else{
        Serial.print("Failed... Unexpected response: ");
        Serial.println(response);
        return false;
      }
    }
    else{
      Serial.print("Failed... No ESP response");
      connectionStatus = -1;
      retryTimer = -1;
      return false;
    }
    connectionStatus=2;
    esp.println("AT+RST");
    esp.println("AT+CWMODE=3");
    return true;
  }
  static bool connectWiFi(){
    Serial.print("\nAttempting to connect to ");
    Serial.print(WiFiName);
    String cmd = "AT+CWJAP=\"" + WiFiName + "\",\"" + WiFiPass + "\"";
    esp.println(cmd);
    esp.readString();
    delay(100);
    int tries = 0;
    while(!esp.available()){tries++;Serial.print(".");delay(100);
      if(tries>300){Serial.println();Serial.println("Failed... Took too long.");}}
    Serial.println();
    delay(1000);
    String response = esp.readString();
    Serial.println(response);
    if(response.indexOf(" CONN") >= 0){
      Serial.println("Success!\n\n");
    }
    else{
      Serial.println("Fail... Maybe the password was wrong?");
      return false;
    }
    connectionStatus=3;
    return true;
  }
};





class OpenWeather{
public:
  static String key;
  static const int houstonZip = 77019;
  static const int madisonZip = 53703;

  
  String getWeather(String zipcode, State& state){
    esp.println("AT+CIPSTART=\"TCP\",\"api.openweathermap.org\",80");
    Serial.println("AT+CIPSTART=\"TCP\",\"api.openweathermap.org\",80");
    if (esp.find("Error")) return;
    //String cmd = "GET /data/2.5/weather?id=";
    //cmd += "2925533";
    //cmd += " HTTP/1.0\r\nHost: api.openweathermap.org\r\n\r\n";
    String cmd = "GET /data/2.5/weather?zip="+zipcode+",us&APPID="+key+" HTTP/1.0\r\nHost: api.openweathermap.org\r\n\r\n";
    esp.print("AT+CIPSEND=");
    esp.println(cmd.length());
    Serial.print("AT+CIPSEND=");
    Serial.println(cmd.length());
    delay(200);
    esp.flush();
    String resp = esp.readString();
    if(resp.indexOf(">")>=0){
      Serial.print(">");
    }
    
    esp.print(cmd);
    Serial.print(cmd);
    return "";
  }
};






class JSONbin{
public:
  static String bin;
  static String key;

  static bool requestState(){
    esp.println("AT+CIPSTART=\"TCP\",\"api.jsonbin.io\",80");
    //Serial.println("AT+CIPSTART=\"TCP\",\"api.jsonbin.io\",80");
    if(esp.find("Error")) return false;
    String cmd = "GET /b/"+bin+"/latest HTTP/1.0\r\nsecret-key: "+key+"\r\nHost: api.jsonbin.io\r\n\r\n";
    esp.print("AT+CIPSEND=");
    esp.println(cmd.length());
    ESP::nextSend = cmd;
//    Serial.print("AT+CIPSEND=");
//    Serial.println(cmd.length());
//    delay(20);
//    esp.flush();
//    String resp = esp.readString();
//    if(resp.indexOf(">")>=0){
////      Serial.print(">");
//    }
//    esp.print(cmd);
//    Serial.print(cmd);
    return true;
  }
};

*/




#endif
