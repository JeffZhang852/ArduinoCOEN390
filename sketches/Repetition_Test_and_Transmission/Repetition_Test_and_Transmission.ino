#include <SoftwareSerial.h>
//Use this for custom transmission port
//Default can be Tx/Rx but have issue while uploading, must unplug these pins
SoftwareSerial MyBlue(10, 9); // RX | TX
String msg;
//Pins for US sensor
#define echoPin 2
#define triggerPin 3
float distance;
int rep_counter = 0;
bool zero_pos = false, rep_pos = false;

void setup()
{
  //US related
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  //Transmission related
  Serial.begin(9600);
  MyBlue.begin(9600);
  Serial.println("Ready to connect\nDefualt password is 1234 or 000");
}
void loop()
{
//Reading from US sensor
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  distance = (pulseIn(echoPin, HIGH) * .0343) / 2;
  Serial.print("Distance: ");
  Serial.print(distance);
  delayMicroseconds(2);
  rep_count(distance);
  Serial.print("\tRep counter: ");
  Serial.println(rep_counter);
  
  //msg = Serial.readString();
  MyBlue.print("Distance: ");
  MyBlue.println(distance);
  MyBlue.print("Rep counter: ");
  MyBlue.println(rep_counter);
  delay(100);
}


//below 6cm and above 30cm counts for 1 rep
//basic logic only, not adapting to real world
bool rep_count(float position) {
  if (position < 7.0) {
    zero_pos = true;
  }
  if (position > 30.0 && zero_pos == true) {
    rep_counter++;
    zero_pos = false;
  }
}
