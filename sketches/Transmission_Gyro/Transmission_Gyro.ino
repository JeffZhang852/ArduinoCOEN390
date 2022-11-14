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
SoftwareSerial MyBlue(10, 9); // RX | TX
String msg;

int count_log = -1;

MPU6050 mpu;
bool start_pos=false;
int count = 0;
float a_Offset;

// For moving average noise filtering.
#define WINDOW_SIZE 50
int INDEX = 0;
float VALUE = 0;
float SUM = 0;
float READINGS[WINDOW_SIZE];
float AVERAGED = 0;


void setup(){

  //Bluetooth pin related
//  pinMode(echoPin, INPUT);
//  pinMode(triggerPin, OUTPUT);
  //Transmission related
//  Serial.begin(9600);
//  MyBlue.begin(9600);
  MyBlue.begin(115200);
  Serial.println("Ready to connect\nDefualt password is 1234 or 000");

  
  //Sensor related
  Serial.begin(115200);
  // Initializing MPU6050
  Serial.println("Initialize MPU6050");
  // MPU6050_SCALE_2000DPS  :  data per second
  // MPU6050_RANGE_2G       :  Accelerometer range in g's
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  
  // Gyroscope offsets:
  // mpu.setGyroOffsetX(155);
  // mpu.setGyroOffsetY(15);
  // mpu.setGyroOffsetZ(15);
  
  // Calibrate gyroscope. The calibration must be at rest.
  // If you don't want calibrate, comment this line.
  mpu.calibrateGyro(); 

  // Gravitationnal acceleration offset.
  a_Offset = accelOffset(100);
  
  // Set threshold sensivty. Default 3.
  // Whats this threshold?
  mpu.setThreshold(3);
  
  // Check settings
//  checkSettings();
}

// Useless function for now.
//void checkSettings(){
//  Serial.println();
//  
//  Serial.print(" * Sleep Mode:        ");
//  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
//  
//  Serial.print(" * Clock Source:      ");
//  switch(mpu.getClockSource())
//  {
//    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
//    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
//    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
//    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
//    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
//    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
//    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
//  }
//  
//  Serial.println(" * Gyroscope:         ");
//  switch(mpu.getScale()) {
//    case MPU6050_SCALE_2000DPS:        Serial.println("2000 dps"); break;
//    case MPU6050_SCALE_1000DPS:        Serial.println("1000 dps"); break;
//    case MPU6050_SCALE_500DPS:         Serial.println("500 dps"); break;
//    case MPU6050_SCALE_250DPS:         Serial.println("250 dps"); break;
//  } 
//}

void loop(){
//  Serial.print("\t\tOffseted acceleration: ");
//  Serial.println(getAccel() - a_Offset, 10);
//  Serial.println(meanAvgAccel(), 10);

  // Offest positive, because accelerometer's z-axis is up-side down
  float accelZ = meanAvgAccel();

//  Serial.print(accelZ);
  count_check(accelZ);
//  MyBlue.print("Z Accel: ");
//  MyBlue.println(accelZ);//For Z info transmission
  

  if(count_log!= count){
    MyBlue.print("Rep counter: ");//Keep this on for now to have clearer display.
    MyBlue.println(count);
    count_log=count;
  }
 
// delay(1);
}

// Returns the horizontal axis linear acceleration, respective to the z-axis of the accelerometer. 
// The z-axis of the accelerometer is pointing downwards while performing data analysis in sprint 2.
float getAccel() {
  Vector normAccel = mpu.readNormalizeAccel();

  // Using the DMP. Won't use here.
  //  mpu.dmpGetQuaternion(&q, fifoBuffer);
  //  mpu.dmpGetAccel(&aa, fifoBuffer);
  //  mpu.dmpGetGravity(&gravity, &q);
  //  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  //  return aaReal.z;

  return normAccel.ZAxis;

}

// Returns the mean verage horizontal axis linear acceleration with noise smoothing.
float meanAvgAccel() {
  SUM = SUM - READINGS[INDEX];            // Remove the oldest entry from the sum
  Vector normAccel = mpu.readNormalizeAccel();// Read the next sensor value
  VALUE = normAccel.ZAxis;
  READINGS[INDEX] = VALUE;                // Add the newest reading to the window
  SUM = SUM + VALUE;                      // Add the newest reading to the sum
  INDEX = (INDEX+1) % WINDOW_SIZE;        // Increment the index, and wrap to 0 if it exceeds the window size

  return (SUM / WINDOW_SIZE) - a_Offset;      // Divide the sum of the window by the window size for the result
//  delay(25); 
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
  Serial.print("\t\tAccel: ");
  Serial.print(accelZ, 10);
  Serial.print("\t\tRep count: ");
  Serial.println(count);
  
  if(accelZ < -2){
    start_pos = true;
  }else if(start_pos == true && accelZ > 2){
    start_pos = false;
    count++;
  }
}
