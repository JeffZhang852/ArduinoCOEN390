#include <SoftwareSerial.h> 
//Use this for custom transmission port
//Default can be Tx/Rx but have issue while uploading, must unplug these pins
SoftwareSerial MyBlue(10, 9); // RX | TX 
String msg;

void setup() 
{   
 Serial.begin(9600); 
 MyBlue.begin(9600); 
 Serial.println("Ready to connect\nDefualt password is 1234 or 000"); 
} 
void loop() 
{
  msg = Serial.readString();
  MyBlue.println(msg);
  delay(100);
}  
