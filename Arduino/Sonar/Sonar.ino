/*
**   _   __               _   _                _      _            
**  | | / /              | | (_)              | |    | |           
**  | |/ / _ __ ___  __ _| |_ ___   ____ _  __| | ___| | __ _ _ __ 
**  |    \| '__/ _ \/ _` | __| \ \ / / _` |/ _` |/ _ \ |/ _` | '__|
**  | |\  \ | |  __/ (_| | |_| |\ V / (_| | (_| |  __/ | (_| | |   
**  \_| \_/_|  \___|\__,_|\__|_| \_/ \__,_|\__,_|\___|_|\__,_|_|   
**                                                                 
**
**  ZeroPi distance sensing with ultrasonic sensor,
**  showing information on serial TFT screen 
**  from Kreativadelar.se, Sweden
**
**  This code is written "quick & dirty" and should not be as a guide
**  in how to program an Arduino. Feel free to change code as you like
**  and share with your friends.
**
**  If you want to share your code changes, please e-mail them to
**  info@kreativadelar.se and we will put them on our web for other
**  customers to download.
** 
** Connection:
**
** Ultrasonic sensor pin 6 -> ZeroPi pin A0
**                   pin 4 -> ZeroPi pin 5V
**                   pin 3 -> ZeroPi pin GND
**                    
** TFT Screen        pin 3 -> ZeroPi pin GND 
**                   pin 4 -> ZeroPi pin 5V 
**                   pin 5 -> ZeroPi pin Tx1>1 
**                
** Switch            pin 1 -> ZeroPi pin GND 
**                   pin 2 -> ZeroPi pin A1                  
**
**  (C) Kreativadelar.se 2015, Sweden, Patrik
**  http://www.kreativadelar.se
**
**  Version 1.0, Initial public release, February 2016
**
**  This example code is in the public domain.
**
*/
#include "zeropi.h"

ZeroPiStepper step1(SLOT1);
int i;
int count;
bool bHomingDone = false;

int centerPosX = 160;
int centerPosY = 239;
int axisLength = 100;

void setup() {
  pinMode(A0,OUTPUT);
  pinMode(A1,INPUT_PULLUP);
  
  SerialUSB.begin(9600);
  Serial1.begin(9600);

  SerialUSB .println("Starting up");
  setupStepper();
  Serial1.print("CLS(0);");
  Serial1.println("DS64(30,90,'Kreativadelar.se',7);");
  delay(1000);
  homing();
  delay(1000);
  Serial1.print("DS64(50,90,'Homing done',0);");
  Serial1.print("DS24(80,5,'Kreativadelar.se',0);");
  Serial1.print("CIR(160,240,2,2);");
  Serial1.print("BOXF(150,0,170,11,0);");
  Serial1.print("DS16(0,50,'Angle:',1);");
  Serial1.print("DS16(0,66,'Distance:',1);");
  Serial1.println("DS24(80,5,'Kreativadelar.se',7);");
}

void loop() {
  sweep();
}


void updateValuesOnScreen(double angle, double distance){
  static double lastDistance = 0.0;
  static double lastAngle = -0.0;
  int X = 0;
  int Y = 0;
  int lastX = 0;
  int lastY = 0;
  String strTemp = "";

  axisLength = distance*1.5;

  if(axisLength == 0 || axisLength > 160) axisLength = 160;

  if(angle > 0 && angle < 90){
    X = centerPosX-(axisLength*cos(radians(angle)));
    Y = centerPosY-(axisLength*sin(radians(angle)));
    lastX = centerPosX-(160*cos(radians(angle)));
    lastY = centerPosY-(160*sin(radians(angle)));
  }else if(angle == 90){
    X = centerPosX;
    Y = centerPosY-axisLength;
    lastX = centerPosX;
    lastY = centerPosY - 160;
  }else if(angle > 90 && angle < 180){
    X = centerPosX+(axisLength*sin(radians(angle-90)));
    Y = centerPosY-(axisLength*cos(radians(angle-90)));
    lastX = centerPosX+(160*sin(radians(angle-90)));
    lastY = centerPosY-(160*cos(radians(angle-90)));
  }else if(angle == 0){
    X = centerPosX - axisLength;
    Y = centerPosY;
    lastX = centerPosX - 160;
    lastY = centerPosY;
  }else if(angle == 180){
    X = centerPosX + axisLength;
    Y = centerPosY;
    lastX = centerPosX + 160;
    lastY = centerPosY;
  }

  
  if(angle != lastAngle){

    Serial1.print("DS16(48,50,'");
    Serial1.print(lastAngle);
    Serial1.print("',0);");
    
    Serial1.print("DS16(48,50,'");
    Serial1.print(angle);
    Serial1.println("',1);"); 
  }

  if(distance != lastDistance){
    Serial1.print("DS16(72,66,'");
    Serial1.print(lastDistance);
    Serial1.print("',0);");
    
    Serial1.print("DS16(72,66,'");
    Serial1.print(distance);
    Serial1.println("',1);");
  }

    strTemp = "";
    strTemp.concat("PL(");
    strTemp.concat(centerPosX);
    strTemp.concat(",");
    strTemp.concat(centerPosY);
    strTemp.concat(",");
    strTemp.concat(lastX);
    strTemp.concat(",");
    strTemp.concat(lastY);
    strTemp.concat(",0);");

    Serial1.println(strTemp);

    strTemp = "";
    strTemp.concat("PL(");
    strTemp.concat(centerPosX);
    strTemp.concat(",");
    strTemp.concat(centerPosY);
    strTemp.concat(",");
    strTemp.concat(X);
    strTemp.concat(",");
    strTemp.concat(Y);
    strTemp.concat(",5);");

    Serial1.println(strTemp);

  lastX = X;
  lastY = Y;
  lastDistance = distance;
  lastAngle = angle;
}

void homing(){
  if(!bHomingDone){
    SerialUSB .println("Homing..");
    Serial1.print("CLS(0);"); // clear the screen with c color 
    Serial1.print("DS24(80,5,'Kreativadelar.se',7);");
    Serial1.println("DS64(50,90,'Homing...',1);");
    step1.setDirection(0);
    for(i = 0;i<1600;i++)
    {
      step1.step();
      if(digitalRead(A1) == LOW){
        bHomingDone = true;
        break;
      }
      delayMicroseconds(1000);
    }

    if(bHomingDone){
      step1.setDirection(1);
      for(i = 0;i<1630;i++)
      {
        step1.step();
        delayMicroseconds(100);
      }
      SerialUSB .println("Homing done");
      Serial1.print("DS64(50,90,'Homing...',0);");
      Serial1.println("DS64(50,90,'Homing done',1);");
    }
    else
    {
      SerialUSB .println("Homing faild...");
      Serial1.print("DS64(50,90,'Homing...',0);");
      for(i=0; i<10; i++)
      {
        Serial1.println("DS64(40,90,'Homing faild!!!',1);");
        delay(1000);
        Serial1.println("DS64(40,90,'Homing faild!!!',0);");
        delay(1000);
      }
      bHomingDone = false;
    }
  }
}

void setupStepper(){
  step1.setResolution(16,DRV8825);
  step1.enable();
}

void sweep(){
  step1.setDirection(0);
  count=0;
  for(i = 0;i<1600;i++)
  {
    step1.step();
    if(count>=20){
      
      checkSensor((0.1125*i));
      count=0;
    }
    count++;
    delayMicroseconds(20000);
  }

  step1.setDirection(1);
  count=0;
  for(i = 1600;i>0;i--)
  {
    step1.step();
    if(count>=20){
      checkSensor((0.1125*i));
      count=0;
    }
    count++;
    delayMicroseconds(20000);
  }
}

void checkSensor(double angle)
{
  double value = distanceCm();
  updateValuesOnScreen(angle,value);
}


double distanceCm()
{
    long distance = measure(400 * 55 + 200);
    return (double)distance / 58.0;
}

long measure(unsigned long timeout)
{
    long duration;
    pinMode(A0,OUTPUT);
    digitalWrite(A0,LOW);
    delayMicroseconds(2);
    digitalWrite(A0,HIGH);
    delayMicroseconds(10);
    digitalWrite(A0,LOW);
    pinMode(A0, INPUT);
    duration = pulseIn(A0, HIGH, timeout);
    return duration;
}
