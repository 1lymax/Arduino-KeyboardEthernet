#include "Keyboard.h"
#include "Mouse.h"
//#include "PS2Mouse.h"
//#include "PS2KeyRaw.h"
#include "Ethernet.h"
#include "SPI.h"
//#include "ping.h"
//#include "avr/wdt.h"


#define KEY_DATA 2
#define KEY_CLOCK 3

bool prnDebug = false;
boolean reading = false;
String oldStr;
String myStr;

long tm;
bool keyb_released = false;

int ping_retry=0;

int Asci[] = {0, 202, 0, 198, 196, 194, 195, 205, 0, 203, 201, //0-10
              199, 197, 179, 96, 0, 0, 134, 129, 0, 128, //11-20
              113, 49, 0, 0, 0, 122, 115, 97, 119, 50, //21-30
              0, 0, 99, 120, 100, 101, 52, 51, 0, 0, //31-40
              32, 118, 102, 116, 114, 53, 0, 0, 110, 98, //41-50
              104, 103, 121, 54, 0, 0, 0, 109, 106, 117, //51-60
              55, 56, 0, 0, 44, 107, 105, 111, 48, 57, //61-70
              0, 0, 46, 47, 108, 59, 112, 45, 0, 0, //71-80
              0, 39, 0, 91, 43, 0, 0, 193, 133, 176, //81-90
              93, 0, 0, 0, 0, 0, 0, 0, 0, 0, //91-100
              0, 178, 0, 0, 213, 0, 216, 210, 0, 0, //101-110
              0, 209, 212, 217, 0, 215, 218, 177, 0, 204, //111-120
              0, 214, 0, 0, 211, 0, 0, 0, 0, 0, //121-130
              200, 0, 0, 0, 0, 0, 0, 0, 0, 0, //131-140
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //141-150
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //151-160
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //161-170
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //171-180
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //181-190
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //191-200
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //201-210
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //211-220
              0
             };

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress my_ip     (192, 168, 2, 177);
IPAddress server_ip (192, 168, 2, 100);

EthernetServer server(80);

void(* resetFunc) (void) = 0;

void setup() {
  //wdt_disable();
  tm = 0;

  Keyboard.begin();
  sendDebugInfo("d", "keyboard2 initialized...");
  Mouse.begin();

  Ethernet.init(10);
  Ethernet.begin(mac, my_ip);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  server.begin();
  sendDebugInfo("d", "ArduinoEthernetStarted");
  //wdt_enable(WDTO_8S);
}

void loop() {
  //wdt_reset();
  if (millis() < 1000) {
    tm = 0;
  }
  if (millis()-tm > 10000 and tm !=0) {
    if (keyb_released == false) {
      keyb_released = true;
      Keyboard.releaseAll();
      sendDebugInfo ("k", "released");
    }
  }

end:
  checkForClient();
}


////////////////////////////////////////////
//      Ethernet Section
////////////////////////////////////////////

void checkForClient() {

  EthernetClient client = server.available();
  
  if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;
    myStr = "";
    while (client.connected()) {
      if (client.available()) {

        char c = client.read();

        if (reading && c == ' ') reading = false;
        if (c == '?') reading = true; //found the ?, begin reading the info

        if (reading) {
          //Serial.print(c);
          if (c != '?') {
            myStr += c;
          }

        }

        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    printDebug("Making answer for request "+myStr);
    parseThangs(myStr);
    client.println("HTTP/1.1 200 OK");
    client.println("GET /?n=" + myStr + " HTTP/1.0");
    client.println();
    client.stop(); 
    printDebug("End answer for request");

  }
}

String sendDebugInfo (String n, String v){
   EthernetClient out_client;
   printDebug("Making request: GET /?" + n + "=" + v + " HTTP/1.0");    
  if (out_client.connect(server_ip, 80)) {
    out_client.println("GET /?" + n + "=" + v + " HTTP/1.1");
    out_client.println();
    out_client.stop(); 
    
    printDebug("Client request ended");
  } else {
    printDebug("connection failed");
  }
}

String parseThangs(String str) {
  int startIndex = str.indexOf("r");
  int endIndex = str.indexOf("p");
  String redStr = str.substring(startIndex + 2, endIndex - 1);
  startIndex = str.indexOf("p");
  endIndex = str.indexOf("m");
  String greenStr = str.substring(startIndex + 2, endIndex - 1);
  startIndex = str.indexOf("m");
  endIndex = str.indexOf("e");
  String blueStr = str.substring(startIndex + 2, endIndex - 1);
  sendKey (redStr, greenStr, blueStr);
  //if (oldStr != redStr + " " + greenStr + " " + blueStr) {
    //Serial.println(redStr + " " + greenStr + " " + blueStr);
  //}
  oldStr=redStr + " " + greenStr + " " + blueStr;
  //return("HTTP/1.1 200 OK");
  return "ok";
}


void sendKey (String key, String dlay, String modificator) {
  char charKey;
  //key.toCharArray(charKey, 1);
  if (modificator=="alloff") {
    Keyboard.releaseAll();
  }
  else if (modificator=="ping") {
    tm = millis();
    keyb_released = false;
    
  }
  else if (modificator=="on_dl") {
    sendDebugInfo (modificator, key);
    Keyboard.press(key.toInt());

    //Serial.println("keyon="+key);
    delay(dlay.toInt());
    Keyboard.release(key.toInt());
  }
  else if (modificator=="on") {
    sendDebugInfo (modificator, key);
    Keyboard.press(key.toInt());

      //Serial.println("keyon="+key);
  }
  else if (modificator=="off") {
    sendDebugInfo (modificator, key);
    Keyboard.release(key.toInt());
    //Serial.println("keyoff="+key);

  }
  
  else if (modificator=="mv") {
    if ((key.toInt() != 0) || (dlay.toInt() != 0)) {
      Mouse.move(key.toInt(), dlay.toInt(), 0);
    
    }

  }
  else if (modificator=="b1on") {
    if (!Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.press(MOUSE_LEFT);
    }
  }
  
  else if (modificator=="b1off") {
    if (Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.release(MOUSE_LEFT);
    }
  }
  
  else if (modificator=="b2on") {
    if (!Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.press(MOUSE_RIGHT);
    }
  }
  
  else if (modificator=="b2off") {
    if (Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.release(MOUSE_RIGHT);
    }
  }
  
  else {
       Keyboard.press(key.toInt());   
       delay(dlay.toInt());
       Keyboard.release(key.toInt());    
      //Serial.println("keyelse="+key);
  }
  


 }

void printDebug(String d)
{
   if (prnDebug) {
      Keyboard.println(d);
   }
}
