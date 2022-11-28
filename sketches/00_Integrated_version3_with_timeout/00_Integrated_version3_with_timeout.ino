/*
    MPU6050 Triple Axis Gyroscope & Accelerometer. Simple Gyroscope Example.
    Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-zyroskop-i-akcelerometr-mpu6050.html
    GIT: https://github.com/jarzebski/Arduino-MPU6050
    Web: http://www.jarzebski.pl
    (c) 2014 by Korneliusz Jarzebski
*/
#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
//Use this for custom transmission port
//Default can be Tx/Rx but have issue while uploading, must unplug these pins
SoftwareSerial MyBlue(10, 9); // TX | RX
String msg;//Transmission buffer
int count_log = -1;//Trigger lock

//Timer check
double startTime=0, endTime=0;
bool checkTrigger = false;
bool countDetect = false;
int timeoutNumber = 7;//Means in 7 seconds, it calls timeout

//Gyroscope initialization
MPU6050 mpu;
bool start_pos=false; //Trigger for repetition
int count = 0;//Repetition count
float a_Offset;//Acceleration offset

//For moving average noise filtering.
#define WINDOW_SIZE 50
int INDEX = 0;
float VALUE = 0;
float SUM = 0;
float READINGS[WINDOW_SIZE];
float AVERAGED = 0;


void setup(){
  //Bluetooth setup baudrate
  MyBlue.begin(9600);;
  Serial.println("Ready to connect\nDefualt password is 1234 or 000");

  // Initializing MPU6050
  Serial.begin(115200);
  Serial.println("Initialize MPU6050");
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  
  // Calibrate gyroscope. The calibration must be at rest.
  // If you don't want calibrate, comment this line.
  mpu.calibrateGyro(); 

  // Gravitationnal acceleration offset.
  a_Offset = accelOffset(100);
  
  // Set threshold sensivty. Default 3.
  // Whats this threshold?
  mpu.setThreshold(3);
}

void loop(){
  // Offest positive, because accelerometer's z-axis is up-side down
  float accelZ = meanAvgAccel();

  count_check(accelZ);
  workoutStart();
  workoutEndChecker();
}

// Returns the mean average horizontal axis linear acceleration with noise smoothing.
float meanAvgAccel() {
  SUM = SUM - READINGS[INDEX];            // Remove the oldest entry from the sum
  Vector normAccel = mpu.readNormalizeAccel();// Read the next sensor value
  VALUE = normAccel.ZAxis;
  READINGS[INDEX] = VALUE;                // Add the newest reading to the window
  SUM = SUM + VALUE;                      // Add the newest reading to the sum
  INDEX = (INDEX+1) % WINDOW_SIZE;        // Increment the index, and wrap to 0 if it exceeds the window size
  return (SUM / WINDOW_SIZE) - a_Offset;      // Divide the sum of the window by the window size for the result
}

// Returns the gravitationnal acceleration fffset using averaging.
// Used function because dependent on location of device.
float accelOffset(int numbAvrgPts) {
  float sum = 0;
  for(int i = 0; i < numbAvrgPts; i++) {
    sum = sum + mpu.readNormalizeAccel().ZAxis;
  }
  return sum/numbAvrgPts;
}

// Only count for linear rep.
// Assuming Z axis control that direction, and is pointing downwards.
void count_check(float accelZ){
  if(accelZ < -3){
    start_pos = true;
  }else if(start_pos == true && accelZ > 3){
    start_pos = false;
    count++;
    MyBlue.println("T");
    Serial.println("T");
    countDetect = true;
  }
}

void workoutStart(){
  if(countDetect){
    //start counter
    startTime = millis();
    countDetect = false;
    checkTrigger=true;
  }
}

bool workoutEndChecker(){
  if(checkTrigger){
    //Update endtime if the start has been called
    endTime = millis();
    if((endTime - startTime)/1000 > timeoutNumber){
      checkTrigger= false;
      MyBlue.println("C");
      Serial.println("C");
      return true;
    }else{
      return false;
    }
  }
  return false;
}
