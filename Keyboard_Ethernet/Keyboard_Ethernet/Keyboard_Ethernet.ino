#include "Keyboard.h"
#include "Mouse.h"
#include "PS2Mouse.h"
#include "PS2KeyRaw.h"
#include "Ethernet.h"
#include "SPI.h"

#define KEY_DATA 2
#define KEY_CLOCK 3

#define MOUSE_DATA 8
#define MOUSE_CLOCK 9

bool MouseLeftPressed=false;
bool MouseRightPressed=false;

bool RunMacros=false;
bool KeyPressed=false;

boolean reading = false;
boolean client_readed=false;
String myStr;
String oldStr;

int MacrosKey = 205;  // F12
int prevKeyNumber=-1;
char prevKey;
char keys[6]={'q','e','r','t','g','f'};
  
int Asci[]={0,202,0,198,196,194,195,205,0,203,201, //0-10
    199,197,179,96,0,0,134,129,0,128,    //11-20
    113,49,0,0,0,122,115,97,119,50,  //21-30
    0,0,99,120,100,101,52,51,0,0,    //31-40
    32,118,102,116,114,53,0,0,110,98,    //41-50
    104,103,121,54,0,0,0,109,106,117,    //51-60
    55,56,0,0,44,107,105,111,48,57,    //61-70
    0,0,46,47,108,59,112,45,0,0,    //71-80
    0,39,0,91,43,0,0,193,133,176,    //81-90
    93,0,0,0,0,0,0,0,0,0,    //91-100
    0,178,0,0,213,0,216,210,0,0,    //101-110
    0,209,212,217,0,215,218,177,0,204,    //111-120
    0,214,0,0,211,0,0,0,0,0,    //121-130
    200,0,0,0,0,0,0,0,0,0,    //131-140
    0,0,0,0,0,0,0,0,0,0,    //141-150
    0,0,0,0,0,0,0,0,0,0,    //151-160
    0,0,0,0,0,0,0,0,0,0,    //161-170
    0,0,0,0,0,0,0,0,0,0,    //171-180
    0,0,0,0,0,0,0,0,0,0,    //181-190
    0,0,0,0,0,0,0,0,0,0,    //191-200
    0,0,0,0,0,0,0,0,0,0,    //201-210
    0,0,0,0,0,0,0,0,0,0,    //211-220
    0
    };

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 2, 177);

EthernetServer server(80);

PS2KeyRaw keyboard;
//PS2Mouse mouse(MOUSE_CLOCK, MOUSE_DATA, REMOTE);

void setup() {
  Ethernet.init(10); 
  Serial.begin( 9600 );  
  while (!Serial) {
    Serial.println( "Test of PS2 Keyboard codes" );
  }
    
  //mouse.initialize();
  Serial.println("mouse initialized...");
  keyboard.begin( KEY_DATA, KEY_CLOCK );
  Serial.println("keyboard initialized...");

  Keyboard.begin();
  Serial.println("keyboard2 initialized...");
  //Mouse.begin();
  //Serial.println("mouse2 initialized...");

  Ethernet.begin(mac, ip);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
}

void loop() {
  int c=0;
  int pos=-1;
  //Serial.print(".c=-,");
  checkForClient();

  if ( keyboard.available() ) {
     int c = keyboard.read();
     
     if ( Asci[c] == MacrosKey ) {
     Serial.print("."+String(c)+"run macros check--,");
       if (RunMacros){
         RunMacros=false;
       }else{
         RunMacros=true;
       }
    }
     Serial.print(".c="+String(c)+"-,");
//-----------------------
  if ( c==224 ) goto end;
  if ( c==240 ) { // 240 - байт, говорящий об отжатии клавиши.
    while ( true ){
      
      if ( c==240 or c==255 ){
        c = keyboard.read(); 
         
      }else{
        break;
      }
    
    }
    
    //Serial.print('.'+String(c)+"--,");
     
    Keyboard.release(Asci[c]);
    switch (c){
      case 20: case 18: case 17: case 89:
        Keyboard.release(Asci[c]);
        c = keyboard.read();

        break;      
    }
    //Serial.print('.'+String(c)+"---,");
    
    
  }else{
    Keyboard.press(Asci[c]);
  }
  }
  end:
  ///Serial.print("point 1");

// ---------------

//-----------------------     

      //Keyboard.press(Asci[c]);
      //delay(random(50,100));
      //Keyboard.release(Asci[c])  
  
  checkMouse();

  if (RunMacros){
    int MacroCase=random(1,100);
    if (MacroCase<3){ // 10% вероятности - пауза на нек время.
      sendRandomDelay();
    }else{
      sendRandomKey();
    }
  }
}

void checkMouse(){
    
    /*
    int data[3];
    mouse.report(data);
    int lngth=String(data[0],BIN).length();
    String last=String(data[0],BIN).substring(lngth-1,lngth); // Последние 2 символа - нажатие клавиши
    String lasttwo=String(data[0],BIN).substring(lngth-2,lngth-1); // Последние 2 символа - нажатие клавиши
    Mouse.move(data[1],-data[2]);

    if (!MouseLeftPressed && last=="1") { // 01 - код нажатия левой клавиши
      MouseLeftPressed=true;
      Mouse.press(MOUSE_LEFT);
    }
    if (MouseLeftPressed && last!="1") {
      MouseLeftPressed=false;
      Mouse.release(MOUSE_LEFT);
    }

    if (!MouseRightPressed && lasttwo=="1") { // 10 - код нажатия правой клавиши
      MouseRightPressed=true;
      Mouse.press(MOUSE_RIGHT);
    }
    if (MouseRightPressed && lasttwo!="1") { // 10 - код нажатия правой клавиши
      MouseRightPressed=false;
      Mouse.release(MOUSE_RIGHT);
    }*/

}

void sendRandomKey () {
  
  delay(random(50,200));
  int pressedKey=random(0,6);
  switch (pressedKey){
    case 0:
    case 1:
      Keyboard.press(keys[pressedKey]);
      delay(random(50,120));
      Keyboard.release(keys[pressedKey]);
      //Mouse.move(random(15,40),random(15,40));
      break;
    case 2 : case 3: case 4 : case 5:
      if (prevKeyNumber!=pressedKey){
        Keyboard.press(keys[pressedKey]);
        delay(random(50,120));
        Keyboard.release(keys[pressedKey]);
      }
      break;
  }
  prevKeyNumber=pressedKey;

}
void sendRandomDelay(){

    int delayCase=random(1,100);
    int delayPeriod=0;
    if (delayCase<85) { // 70% вероятностью задержка до 3 секунд
      delayPeriod=random(3000,6000);
    }else if (delayCase>=85 && delayCase<95){ //15% задержка до 20 сек
      delayPeriod=random(6000,20000);
    }else if (delayCase>=95 && delayCase<98){ //10% задержка от 20 сек до 40 сек
      delayPeriod=random(20000,40000);
    }else if (delayCase>98){ //5% задержка от 40 сек до 1 мин 30 сек
      delayPeriod=random(40000,90000);
    }
    Keyboard.println(delayPeriod);
    delay(delayPeriod);

}


void sendKey (String key, String dlay, String modificator){
  char charKey;
  key.toCharArray(charKey,1);
  Keyboard.press(charKey);
  delay(dlay.toInt());
  Keyboard.release(charKey);

}

////////////////////////////////////////////
//      Ethernet Section
////////////////////////////////////////////

void checkForClient(){

  EthernetClient client = server.available();

  if (client) {
    //Serial.println("point 1");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;
    myStr = "";
    while (client.connected()) {
      if (client.available()) {

        char c = client.read();

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true; //found the ?, begin reading the info

        if(reading){
          //Serial.print(c);
          if (c!='?') {
            myStr += c;
          }

        }

        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        }else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    client.print(parseThangs(myStr));
    client.stop(); // close the connection:
  } 
}

String parseThangs(String str) {
  int startIndex = str.indexOf("r");
  int endIndex = str.indexOf("g");
  String redStr = str.substring(startIndex + 2, endIndex - 1);
  startIndex = str.indexOf("g");
  endIndex = str.indexOf("b");
  String greenStr = str.substring(startIndex + 2, endIndex -1);
  startIndex = str.indexOf("b");
  endIndex = str.indexOf("e");
  String blueStr = str.substring(startIndex + 2, endIndex -1);
  if (oldStr!=redStr + " " + greenStr + " " + blueStr){
    sendKey (redStr, greenStr, blueStr);
    Serial.println(redStr + " " + greenStr + " " + blueStr);
    oldStr= redStr + " " + greenStr + " " + blueStr;
  }
  return "ok";
}
