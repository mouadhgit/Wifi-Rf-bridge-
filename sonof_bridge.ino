/**
  ******************************************************************************

  Project: Wifi Rf Bridge 
  Author:   Tp2-ing2: Mouadh Dahech   
  Updated:  05/28/2021

  Implimentation Board: Doit Esp32 Devkit V1
  ******************************************************************************
  */

// add library
#include <ThingSpeak.h>               
#include <ESP8266WebServer.h>
#include <RCSwitch.h>
#include <EEPROM.h>

#define green 4
#define red   5
#define debug 0

RCSwitch mySwitch = RCSwitch();
WiFiClient  client;
unsigned long counterChannelNumber = 1346655;                // Channel ID
const char * myCounterReadAPIKey = "F5QSO9WDP7PUIJJT";
const char * myCounterwriteAPIKey = "YCOT87MIZOLZGYJX";

bool read_rf = 1;                              // The field you wish to read
const int FieldNumber2 = 3;                                 // The field you wish to read
int state  = 0;
bool state2 = 1;
uint32_t RXval1;
uint32_t  lastrxval = 0;
uint32_t  val0,val1,val2,val3; 
int j =0; 
int address = 0;
uint32_t adjTbl[] = {0,0,0,0}; 
String lastS;

void setup()
{
  pinMode(green,OUTPUT);
  pinMode(red,OUTPUT);
  Serial.println("\nTesting EEPROM Library\n");
  if (!EEPROM.begin(1000)) 
  {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart(); 
  }
  mySwitch.enableTransmit(13);
  mySwitch.enableReceive(2);
  
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);
  Serial.begin(115200);
  Serial.println();
  WiFi.begin("Orange-BF3F","G2THJ6DTF2B");                 // write wifi name & password           
  Serial.print("Connecting");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);
}

void loop() 
{
 
 state = ThingSpeak.readLongField(counterChannelNumber, FieldNumber2, myCounterReadAPIKey);
 while(!state){
   digitalWrite(green,LOW);
   digitalWrite(red,HIGH); 
   if (mySwitch.available()&& read_rf) {
     RXval1 = mySwitch.getReceivedValue();
      if(RXval1 !=lastrxval)
      { 
       adjTbl[j]= RXval1;
       digitalWrite(red,LOW);
       delay(500);
       digitalWrite(red,HIGH);
       if (debug == 1)
       { 
         Serial.println(j);
         Serial.println(adjTbl[j]);
       }   
       j++;
      }   

     if(j==4)
      read_rf = 0;
      
     lastrxval=RXval1;   
    }
    
   if(!read_rf)
   {
    while(state2){
    for(int i = 0; i<4; i++){
     EEPROM.writeULong(address+(i*4),adjTbl[i]);
    }
     EEPROM.commit();
     state2 = 0;
    }
   }
   state = ThingSpeak.readLongField(counterChannelNumber, FieldNumber2, myCounterReadAPIKey);  
  }
    
 while(state){

//  Serial.println("Command Mode..");
  digitalWrite(red,LOW); 
  digitalWrite(green,HIGH);
  state = ThingSpeak.readLongField(counterChannelNumber, FieldNumber2, myCounterReadAPIKey);
  String S = ThingSpeak.readCreatedAt(counterChannelNumber,myCounterReadAPIKey);
  S.remove(0,11);
  state = ThingSpeak.readLongField(counterChannelNumber, FieldNumber2, myCounterReadAPIKey);
   val0 =  EEPROM.readULong(address);
   val1 =  EEPROM.readULong(address+4);
   val2 =  EEPROM.readULong(address+8);
   val3 =  EEPROM.readULong(address+12);

   if(S !=lastS){
   state = ThingSpeak.readLongField(counterChannelNumber, FieldNumber2, myCounterReadAPIKey); 
     if(state == 2){
      ThingSpeak.writeField(counterChannelNumber, FieldNumber2, 6,myCounterwriteAPIKey);
      delay(300);
      
      digitalWrite(green,LOW);
      delay(500);
      digitalWrite(green,HIGH);
      mySwitch.send(val0, 24);
      delay(1000);
     }
   
     if(state == 3){
      digitalWrite(green,LOW);
      delay(500);
      digitalWrite(green,HIGH);
      mySwitch.send(val1, 24);
      delay(1000);  
    }

   if(state == 4){
    digitalWrite(green,LOW);
    delay(500);
    digitalWrite(green,HIGH);
    mySwitch.send(val2, 24); 
    delay(1000);
     
   }

    if(state == 5){
      digitalWrite(green,LOW);
      delay(500);
      digitalWrite(green,HIGH);
      mySwitch.send(val3, 24); 
      delay(1000); 
    }
   lastS = S;
   }
   state = ThingSpeak.readLongField(counterChannelNumber, FieldNumber2, myCounterReadAPIKey);
  if (debug == 1)
  {
    Serial.print("val0= ");
    Serial.println(val0);
    Serial.print("val1= ");
    Serial.println(val1);
    Serial.print("val2= ");
    Serial.println(val2);
    Serial.print("val3= ");
    Serial.println(val3);
  }
   if(state == 0)
  {
    read_rf = 1;
    state2 = 1;
    j=0;
  } 
 } 
}
