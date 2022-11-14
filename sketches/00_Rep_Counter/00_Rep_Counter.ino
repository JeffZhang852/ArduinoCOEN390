/*
    MPU6050 Triple Axis Gyroscope & Accelerometer. Simple Gyroscope Example.
    Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-zyroskop-i-akcelerometr-mpu6050.html
    GIT: https://github.com/jarzebski/Arduino-MPU6050
    Web: http://www.jarzebski.pl
    (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;
bool start_pos=false;
int count = 0;
float a_Offset;

// Variables for moving average noise filtering.
#define WINDOW_SIZE 50
int INDEX = 0;
float VALUE = 0;
float SUM = 0;
float READINGS[WINDOW_SIZE];
float AVERAGED = 0;

// Variables for integration.
float init_accel;
float init_velocity;
float init_position;
unsigned long init_time;


void setup(){
  Serial.begin(115200);

  // Initializing MPU6050
  Serial.println("Initialize MPU6050");
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // Set threshold sensivty. Default 3.
  // Whats this threshold?
  mpu.setThreshold(3);
  
  // Gravitationnal acceleration offset.
  a_Offset = accelOffset(100);

  // Filling moving average buffer.
  for(int i = 0; i < WINDOW_SIZE; i++) {
    meanAvgAccel();
  }
  
  
  // Check settings
  checkSettings();
  
  // These initial values should be respective to the weights being at rest.
  init_accel = meanAvgAccel();
  init_velocity = 0;
  init_position = 0;
  init_time = micros();
}

void checkSettings(){
  Serial.println();
  
  Serial.print(" * Sleep Mode:        ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
  
  Serial.print(" * Clock Source:      ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.println(" * Gyroscope:         ");
  switch(mpu.getScale()) {
    case MPU6050_SCALE_2000DPS:        Serial.println("2000 dps"); break;
    case MPU6050_SCALE_1000DPS:        Serial.println("1000 dps"); break;
    case MPU6050_SCALE_500DPS:         Serial.println("500 dps"); break;
    case MPU6050_SCALE_250DPS:         Serial.println("250 dps"); break;
  } 
}

void loop(){
//  Serial.print("\t\tOffseted acceleration: ");
//  Serial.println(getAccel() - a_Offset, 10);
//  Serial.println(meanAvgAccel(), 10);

  // Offest positive, because accelerometer's z-axis is up-side down
  float accelZ = meanAvgAccel();
  Serial.println(accelZ);

  // TODO.
//  float posZ = doubleIntegration(accelZ);
//  float velocityZ = meanAvgIntegration();

//  count_check(accelZ);
 
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

// Double integration to get positon from acceleration. Must account for drift and error in averaging approximation.
// TODO: Describe function
float doubleIntegration(float new_accel) {
  unsigned long new_time = micros();
  // Code them together to reduce number of instruction, and therefore drift?
  float new_velocity = init_velocity + (((init_accel + new_accel) / 2) * (new_time - init_time));
  float new_position = init_position + (((init_velocity + new_velocity) / 2) * (new_time - init_time));
//  Serial.println("Z-velocity: " + (String) (new_velocity));
  Serial.println("Z-position: " + (String) (new_position));

  float init_accel = new_accel;
  float init_velocity = new_velocity;
  float init_position = new_position;
  float init_time = new_time;
  
  delay(10);
  return new_velocity;  

}


// Using averaged integral values. (unfinished function)
float meanAvgIntegration() {
  unsigned long new_time = micros();
  SUM = SUM - READINGS[INDEX];            // Remove the oldest entry from the sum
  Vector normAccel = mpu.readNormalizeAccel();// Read the next sensor value
  VALUE = normAccel.ZAxis;
  READINGS[INDEX] = VALUE;                // Add the newest reading to the window
  SUM = SUM + VALUE;                      // Add the newest reading to the sum
  INDEX = (INDEX+1) % WINDOW_SIZE;        // Increment the index, and wrap to 0 if it exceeds the window size

  AVERAGED = (SUM / WINDOW_SIZE) - a_Offset;      // Divide the sum of the window by the window size for the result

//  Serial.print(VALUE - a_Offset);
//  Serial.print(",");
<<<<<<< HEAD
  Serial.println(AVERAGED, 5);
=======
  Serial.println(AVERAGED - a_Offset);
>>>>>>> d26e946c0a9a84b63d82edf5ad0d07a953745770

  float new_velocity = init_velocity + (((init_accel + AVERAGED) / 2) * (new_time - init_time));
  float new_position = init_position + (((init_velocity + new_velocity) / 2) * (new_time - init_time));

  float init_accel = AVERAGED;
  float init_velocity = new_velocity;
  float init_position = new_position;
  float init_time = new_time;

  Serial.println("Z-position: " + (String)(new_position));
//  Serial.println("Z-position: " + (String) (new_position));
  return AVERAGED;
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
  
  if(accelZ < -3){
<<<<<<< HEAD
    Serial.println(-3);
    start_pos = true;
  }else if(start_pos == true && accelZ > 3){
    Serial.println(3);
=======
    start_pos = true;
  }else if(start_pos == true && accelZ > 3){
>>>>>>> d26e946c0a9a84b63d82edf5ad0d07a953745770
    start_pos = false;
    count++;
  }
}
