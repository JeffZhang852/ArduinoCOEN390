// Test code for UC sensor with Arduino Uno R3

#define echoPin 2
#define triggerPin 3

float distance;
int rep_counter = 0;
bool zero_pos = false, rep_pos = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Arduino Uno with Ultrasonic");
}

void loop() {
  // put your main code here, to run repeatedly:
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
}

//below 6cm and above 30cm counts for 1 rep
// basic logic only, not adapting to real world
bool rep_count(float position) {
  if (position < 7.0) {
    zero_pos = true;
  }
  if (position > 30.0 && zero_pos == true) {
    rep_counter++;
    zero_pos = false;
  }
}