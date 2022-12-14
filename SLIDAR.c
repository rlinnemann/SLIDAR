/*

AUTH: Robert Linnemann
File: SLIDAR.c
Course: ECEN 494/495 - University of Nebraska Lincoln
Description:
This project is uses VL53L0X time of flight sensors to determine distance 
for a rail. The distance changes the note that is sent to Ardour to play
music. This begins by assigning different addresses to each sensor on the 
bus in order to gather unique data from each sensor. The pins where the 
XSHUT is connected to is defined. These are used to assign new addresses.
Initilization is done and arrays can be created to store the last note 
played. XSHUT all the pins low and then high to reset. Set the XSHUT for 
each sensor high one at a time. If any of the sensors cant be found then
do not begin the program and return an error message for debuggin purposes.
read_quad_sensors() reads the values from all the sensors and prints them
to the serial monitor. If the distance falls within a certain range, then
played that specified note. There are two notes per rail so an array of size
2 can hold the last note played status. If the first note in a rail is played
then change the zero index to a 1 and set the 1 index to a 0. The opposite
is true when the second note in a rail is played. When distance is determined,
before sending a note, check to see if the note has already been played
by seeing if its approprote array index is 1. If the index is 0 then play it
otherwise dont play it again. If the sensors read a distance that translates 
to the back wall of the rail, then reset both indexs in the array to 0 
to allow for either a repeated note to be played or a unique note. 

 */

#include "Adafruit_VL53L0X.h"
#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

// address we will assign if multiple sensors are present
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31
#define LOX3_ADDRESS 0x32
#define LOX4_ADDRESS 0x34

// set the pins to shutdown
#define SHT_LOX1 1
#define SHT_LOX2 2
#define SHT_LOX3 3
#define SHT_LOX4 0

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox4 = Adafruit_VL53L0X();


USBKeyboard Keyboard;

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;
VL53L0X_RangingMeasurementData_t measure3;
VL53L0X_RangingMeasurementData_t measure4;

int sensorState1[2];
int sensorState2[2];
int sensorState3[2];
int sensorState4[2];

/*
    Reset all sensors by setting all of their XSHUT pins low for delay(10), then set all XSHUT high to bring out of reset
    Keep sensor #1 awake by keeping XSHUT pin high
    Put all other sensors into shutdown by pulling XSHUT pins low
    Initialize sensor #1 with lox.begin(new_i2c_address) Pick any number but 0x29 and it must be under 0x7F. Going with 0x30 to 0x3F is probably OK.
    Keep sensor #1 awake, and now bring sensor #2 out of reset by setting its XSHUT pin high.
    Initialize sensor #2 with lox.begin(new_i2c_address) Pick any number but 0x29 and whatever you set the first sensor to
    CHECK THIS
    ---------------------------------
    SDA => Pin 6 (GP4)
    SCL => Pin 7 (GP5)
    XSHUT1 => Pin 10 (GP7)
    XSHUT2 => Pin 9 (GP6)
    XSHUT3 => Pin 12 (GP9)
 */
void setID() {
  // all reset
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);
  delay(10);

  // all unreset
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, HIGH);
  digitalWrite(SHT_LOX3, HIGH);
  digitalWrite(SHT_LOX4, HIGH);
  delay(10);

  // activating LOX1 and resetting LOX2
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);

  // initing LOX1
  if(!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while(1);
  }
  delay(10);

  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  //initing LOX2
  if(!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while(1);
  }
  delay(10);

  // activating LOX3
  digitalWrite(SHT_LOX3, HIGH);
  delay(10);

  //initing LOX3
  if(!lox3.begin(LOX3_ADDRESS)) {
    Serial.println(F("Failed to boot third VL53L0X"));
    while(1);
  }

  // activating LOX4
  digitalWrite(SHT_LOX4, HIGH);
  delay(10);
  //initing LOX4
  if(!lox4.begin(LOX4_ADDRESS)) {
    Serial.println(F("Failed to boot fourth VL53L0X"));
    while(1);
  }


}

void read_quad_sensors() {

  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!
  lox3.rangingTest(&measure3, false); // pass in 'true' to get debug data printout!
  lox4.rangingTest(&measure4, false); // pass in 'true' to get debug data printout!

  // print sensor one reading
  Serial.print(F("1: "));
  if(measure1.RangeStatus != 4) {     // if not out of range
    Serial.print(measure1.RangeMilliMeter);
    Serial.print("\n");
    if(measure1.RangeMilliMeter < 54){
      if(sensorState1[0] == 1){
        // do nothing
      }
      else{
        Keyboard.printf("jjjjjjjjjjj");
        sensorState1[0] = 1;
        sensorState1[1] = 0;
      }
    }
    else if(measure1.RangeMilliMeter > 54 && measure1.RangeMilliMeter  < 105){
      if (sensorState1[1] == 1){
        // do nothing
      }
      else{
        Keyboard.printf("kkkkkkkkkkk");
        sensorState1[0] = 0;
        sensorState1[1] = 1;
      }
    }
    else if (measure1.RangeMilliMeter > 110){
      sensorState1[0] = 0;
      sensorState1[1] = 0;
    }
  }
  else {
    Serial.print(F("Out of range"));
  }
  Serial.print(F(" "));

  // print sensor two reading
  Serial.print(F("2: "));
  if(measure2.RangeStatus != 4) {
    Serial.print(measure2.RangeMilliMeter);
    Serial.print("\n");
    if(measure2.RangeMilliMeter < 87){
      if (sensorState2[0] == 1){
        //do nothing
      }
      else{
        Keyboard.printf("ggggggggggg");
        sensorState2[0] = 1;
        sensorState2[1] = 0;
      }
    }
    else if(measure2.RangeMilliMeter > 87 && measure2.RangeMilliMeter  < 130){
      if (sensorState2[1] == 1){
        //do nothing
      }
      else{
        Keyboard.printf("hhhhhhhhhhh");
        sensorState2[0] = 0;
        sensorState2[1] = 1;
      }
    }
    else if (measure2.RangeMilliMeter > 140){
      sensorState2[0] = 0;
      sensorState2[1] = 0;
    }
  }
  else {
    Serial.print(F("Out of range"));
  }
  Serial.print(F(" "));

  //print sensor 3 reading
  Serial.print(F("3: "));
  if(measure3.RangeStatus != 4) {
    Serial.print(measure3.RangeMilliMeter);
    Serial.print("\n");
    if(measure3.RangeMilliMeter < 89){
      if (sensorState3[0] == 1){
        //do nothing
      }
      else{
        Keyboard.printf("ddddddddddd");
        sensorState3[0] = 1;
        sensorState3[1] = 0;
      }
    }
    else if(measure3.RangeMilliMeter > 89 && measure3.RangeMilliMeter  < 138){
      if (sensorState3[1] == 1){
        //do nothing
      }
      else{
        Keyboard.printf("fffffffffff");
        //f is a problem
        sensorState3[0] = 0;
        sensorState3[1] = 1;
      }
    }
    else if(measure3.RangeMilliMeter > 140){
      sensorState3[0] = 0;
      sensorState3[1] = 0;
    }
  }
  else {
    Serial.print(F("Out of range"));
  }
  Serial.print(F(" "));

  //print sensor 4 reading
  Serial.print(F("4: "));
  if(measure4.RangeStatus != 4) {
    Serial.print(measure4.RangeMilliMeter);
    Serial.print("\n");
    if(measure4.RangeMilliMeter < 82){
      if(sensorState4[0] == 1){
        //do nothing
      }
      else{
        Keyboard.printf("aaaaaaaaaaa");
        sensorState4[0] = 1;
        sensorState4[1] = 0;
      }
    }
    else if(measure4.RangeMilliMeter > 82 && measure4.RangeMilliMeter  < 105){
      if (sensorState4[1] == 1){
        //do nothing
      }
      else{
        //s is a problem
        Keyboard.printf("sssssssssss");
        sensorState4[0] = 0;
        sensorState4[1] = 1;
      }
    }
    else if(measure4.RangeMilliMeter > 110){
      sensorState4[0] = 0;
      sensorState4[1] = 0;
    }
  }
  else {
    Serial.print(F("Out of range"));
  }
}

void setup() {
  Serial.begin(115200);

  // wait until serial port opens for native USB devices (MAKES YOU NEED PUTTY)
  while (! Serial) { delay(1); }

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  pinMode(SHT_LOX3, OUTPUT);
  pinMode(SHT_LOX4, OUTPUT);

  pinMode(29, INPUT);

  Serial.println(F("Shutdown pins inited..."));

  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  digitalWrite(SHT_LOX3, LOW);
  digitalWrite(SHT_LOX4, LOW);

  Serial.println(F("All in reset mode...(pins are low)"));

  Serial.println(F("Starting..."));
  setID();

}

void loop() {

  read_quad_sensors();
  delay(10);

}
