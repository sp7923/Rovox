/******************************************************************************
TB6612.cpp
TB6612FNG H-Bridge Motor Driver Example code
Michelle @ SparkFun Electronics
8/20/16
https://github.com/sparkfun/SparkFun_TB6612FNG_Arduino_Library

Uses 2 motors to show examples of the functions in the library.  This causes
a robot to do a little 'jig'.  Each movement has an equal and opposite movement
so assuming your motors are balanced the bot should end up at the same place it
started.

Resources:
TB6612 SparkFun Library

Development environment specifics:
Developed on Arduino 1.6.4
Developed with ROB-9457

Modified by Anupkumar Maliyackel (anupkma@gmail.com)- 22nd May 2022 for ESP32
This is for Old TB6612 boards without STBY pin
******************************************************************************/

#include "SparkFun_TB6612.h"
#include <Arduino.h>

Motor::Motor(int In1pin, int In2pin, int PWMpin, int offset,int PWMC,int PWMFre, int PWMRes)
{
  In1 = In1pin;
  In2 = In2pin;
  PWM = PWMpin;
  Offset = offset;

  PWMChannel=PWMC;
  PWMFrequency=PWMFre;
  PWMResolution=PWMRes;
  
  pinMode(In1, OUTPUT);
  pinMode(In2, OUTPUT);
  pinMode(PWM, OUTPUT);
  

  ledcSetup(PWMChannel, PWMFrequency, PWMResolution);
  ledcAttachPin(PWM, PWMChannel);
}

void Motor::drive(int speed)
{
  speed = speed * Offset;
  if (speed>=0) fwd(speed);
  else rev(-speed);
}
void Motor::drive(int speed, int duration)
{
  drive(speed);
  delay(duration);
}

void Motor::fwd(int speed)
{
   digitalWrite(In1, HIGH);
   digitalWrite(In2, LOW);
   ledcWrite(PWMChannel, speed);
   //Serial.println("Inside Speakfun lib fwd");

}

void Motor::rev(int speed)
{
   digitalWrite(In1, LOW);
   digitalWrite(In2, HIGH);
   ledcWrite(PWMChannel, speed);
}

void Motor::brake()
{
   digitalWrite(In1, HIGH);
   digitalWrite(In2, HIGH);
   ledcWrite(PWMChannel, 0);
}

void Motor::switchOff()
{
   digitalWrite(In1, LOW);
   digitalWrite(In2, LOW);
   ledcWrite(PWMChannel, 0);

}

void forward(Motor motor1, Motor motor2, int speed)
{
	motor1.drive(speed);
	motor2.drive(speed);
}
void forward(Motor motor1, Motor motor2)
{
	motor1.drive(DEFAULTSPEED);
	motor2.drive(DEFAULTSPEED);
}


void back(Motor motor1, Motor motor2, int speed)
{
	int temp = abs(speed);
	motor1.drive(-temp);
	motor2.drive(-temp);
}
void back(Motor motor1, Motor motor2)
{
	motor1.drive(-DEFAULTSPEED);
	motor2.drive(-DEFAULTSPEED);
}
void left(Motor left, Motor right, int speed)
{
	int temp = abs(speed)/2;
	left.drive(-temp);
	right.drive(temp);
	
}


void right(Motor left, Motor right, int speed)
{
	int temp = abs(speed)/2;
	left.drive(temp);
	right.drive(-temp);
	
}
void brake(Motor motor1, Motor motor2)
{
	motor1.brake();
	motor2.brake();
}

void switchOff(Motor motor1, Motor motor2)
{
	motor1.switchOff();
	motor2.switchOff();
}