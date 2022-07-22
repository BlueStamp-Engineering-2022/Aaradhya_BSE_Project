#include <SoftwareSerial.h>
#include <PS2X_lib.h>  
#include <HampelFilter.h>


int motor1pin1 = 5;
int motor1pin2 = 4;
int motor1EN = 3;

int motor2pin1 = 8;

int motor2pin2 = 7;
int motor2EN = 6;

int motor3pin1 = 9;
int motor3pin2 = 10;
int motor3EN = 11;

int motorSpeed = 0;
double lJoyXD = 0.0;
double lJoyYD = 0.0;
int motor1Speed = 0;
int motor2Speed = 0;
int motor3Speed = 0;

boolean greenLight = false;
boolean bluetoothMode = false;

boolean forward = false;
boolean backwards = false;
boolean moveRight = false;
boolean moveLeft = false;
boolean rRight = false;
boolean rLeft = false;
boolean stopped = false;

HampelFilter leftX = HampelFilter(0.00, 3, 100);
HampelFilter leftY = HampelFilter(0.00, 3, 100);

HampelFilter rightX = HampelFilter(0.00, 3, 120);
HampelFilter rightY = HampelFilter(0.00, 3, 120);

SoftwareSerial bluetooth (8,12);
char command = 'h';
char bluetoothStatus = 'p';
char text = 'q';

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you conect the controller, 
//or call config_gamepad(pins) again after connecting the controller.
int error = 0; 
byte type = 0;
byte vibrate = 0;

void setup() 
{
  // put your setup code here, to run once:
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(motor3pin1, OUTPUT);
  pinMode(motor3pin2, OUTPUT);

  pinMode(motor1EN, OUTPUT);
  pinMode(motor2EN, OUTPUT);
  pinMode(motor3EN, OUTPUT);

 Serial.begin(9600);
 bluetooth.begin(9600);
  
 error = ps2x.config_gamepad(A3,A1,A2,A0, false, true);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error 
     
 // Check for error    
    
  if(error == 0){      
    Serial.println("Found Controller, configured successful");      
  }      
  else if(error == 1)      
    Serial.println("No controller found, check wiring or reset the Arduino");      
  else if(error == 2)      
    Serial.println("Controller found but not accepting commands");         
  else if(error == 3)      
    Serial.println("Controller refusing to enter Pressures mode, may not support it.");     
     
  // Check for the type of controller       
  type = ps2x.readType();       
  switch(type) {      
    case 0:      
      Serial.println("Unknown Controller type");      
      break;      
    case 1:      
      Serial.println("DualShock Controller Found");      
      break;      
    case 2:      
      Serial.println("GuitarHero Controller Found");      
      break;
  }
}

void loop() 
{
  // put your main code here, to run repeatedly.
  
  error = ps2x.config_gamepad(A3,A1,A2,A0, false, true);
    
  if (error == 1)
  {
    analogWrite(motor1EN, 0);
    analogWrite(motor2EN, 0);
    analogWrite(motor3EN, 0);
    return;
  }
  else 
  {
    ps2x.read_gamepad(false, vibrate);

    //Serial.println(PSB_R1);

    if (bluetooth.available())
    {
       command = bluetooth.read();
        
      if (command == 'c')
        bluetoothMode = true;
      else if (command == 'n')
        bluetoothMode = false;

      if (bluetoothMode == true)
      {
        Serial.println(command);
        if (command == 'f')
          forward = true;
        else if (command == 'b')
          backwards = true;
        else if (command == 'l')
          moveLeft = true;
        else if (command == 'r')
          moveRight = true;
        else if (command == 'u')
          rLeft = true;
        else if (command == 'd')
          rRight = true;
        
        if (command == 's')
        {
          stopped = true;
          forward = false;
          backwards = false;
          moveLeft = false;
          moveRight = false;
          rLeft = false;
          rRight = false;
        }
      }
    }

    // Serial.println(bluetoothStatus);
    
    /*if (ps2x.Button(PSB_R1))
    {
      bluetoothMode = true;
     // Serial.println("YO");
    }
    
    if (ps2x.Button(PSB_L1))
    {
      bluetoothMode = false;
      Serial.println("HI");
    }*/
   
    int rJoyX = ps2x.Analog(PSS_RX); // read x-joystick
    int rJoyY = ps2x.Analog(PSS_RY); // read y-joystick

    int lJoyX = ps2x.Analog(PSS_LX);
    int lJoyY = ps2x.Analog(PSS_LY);

    rJoyX = map(rJoyX, 0, 255, -255, 255);
    rJoyY = map(rJoyY, 0, 255, 255, -255);
    lJoyX = map(lJoyX, 0, 255, -255, 255);
    lJoyY = map(lJoyY, 0, 255, 255, -255);

    //Serial.println(lJoyX);
    //Serial.println(lJoyY);

    //TODO: deadzone

    if ((lJoyX > 0 && lJoyX < 8) || bluetoothMode == true)
      lJoyX = 0;
    if ((lJoyY > -3 && lJoyY < 5) || bluetoothMode == true)
      lJoyY = 0;

    if ((rJoyX > -4 && rJoyX < 4) || bluetoothMode == true)
      rJoyX = 0;
    if ((rJoyY > -4 && rJoyY < 4) || bluetoothMode == true)
      rJoyY = 0;

   leftY.write(lJoyY);
   leftX.write(lJoyX);

   rightY.write(rJoyY);
   rightX.write(rJoyX);

   if (leftY.checkIfOutlier(lJoyY) == true || leftX.checkIfOutlier(lJoyX) == true || 
      rightX.checkIfOutlier(rJoyX) == true || rightY.checkIfOutlier(rJoyY) == true)
   {
      greenLight = false;
   }
   else
      greenLight = true;
   
    float L = (sqrt( ( (float) lJoyY * (float) lJoyY) + ((float) lJoyX * (float) lJoyX) ));
    L = L*0.66666666666;
    float radian = (atan2((float) lJoyY, (float) lJoyX));

    motor1Speed = (int) (L * cos( (60*(M_PI/180)) - radian ));
    motor2Speed = (int) (L * cos( (300*(M_PI/180)) - radian ));
    motor3Speed = (int) (L * cos( (180*(M_PI/180)) - radian ));

   //Serial.println(radian);
    

    if (rJoyX == 0 && rJoyY == 0)
    {
      if (greenLight == true)
      {
        if (motor1Speed < 0) 
        {
           motor1Speed = motor1Speed*-1;
           digitalWrite(motor1pin1, HIGH);
           digitalWrite(motor1pin2, LOW);
    
           if (lJoyX == 0 && motor1Speed == 147)
           {
              motor1Speed = 255;
           }
    
           if (motor1Speed < 0)
           {
              motor1Speed = 0;
           }
           
           analogWrite(motor1EN, motor1Speed);
        } 
        else if (motor1Speed > 0) 
        {  
           digitalWrite(motor1pin1, LOW);
           digitalWrite(motor1pin2, HIGH);
    
           if (lJoyX == 0 && motor1Speed == 147)
           { 
              motor1Speed = 255;  
           }
    
           if (motor1Speed < 0)
           {
              motor1Speed = 0;
           }
    
           analogWrite(motor1EN, motor1Speed);
        }
        else
          analogWrite(motor1EN, 0);
       
        if (motor2Speed < 0)
        {
           motor2Speed = motor2Speed*-1;
 
           digitalWrite(motor2pin1, HIGH);
           digitalWrite(motor2pin2, LOW);
    
           if (lJoyX == 0 && motor2Speed == 147)
              motor2Speed = 255;
           
           if (lJoyY != 0 && lJoyX != 0)
           {
              motor2Speed = motor2Speed + 25;
           }
           else if (lJoyY == 0 && lJoyX < 0)
           {
              
              
              motor2Speed = motor2Speed + 10;
           }
    
           if (motor2Speed > 255)
              motor2Speed = 255;
             
           analogWrite(motor2EN, motor2Speed);
        }
        else if (motor2Speed > 0)
        {
           digitalWrite(motor2pin1, LOW);
           digitalWrite(motor2pin2, HIGH);
           
           if (lJoyX == 0 && motor2Speed == 147)
              motor2Speed = 255;
              
           if (lJoyY != 0 && lJoyX != 0)
           {
              motor2Speed = motor2Speed + 5;
           }
           else if (lJoyY == 0 && lJoyX > 0)
           {
              motor2Speed = motor2Speed - 10;
           }
           else if (radian >= 2.00 && radian <= 2.59)
           {
              motor2Speed = motor2Speed + 150;
           }
    
           if (motor2Speed > 255)
              motor2Speed = 255;
    
           analogWrite(motor2EN, motor2Speed);
        }
        else
        {
          analogWrite(motor2EN, 0);
        }
        
        if (motor3Speed < 0)
        {
           motor3Speed = motor3Speed*-1;
    
           digitalWrite(motor3pin1, HIGH);
           digitalWrite(motor3pin2, LOW);
    
           analogWrite(motor3EN, motor3Speed);
        }
        else if (motor3Speed > 0)
        {
           digitalWrite(motor3pin1, LOW);
           digitalWrite(motor3pin2, HIGH);

           if (radian >= 2.00 && radian <= 2.59)
           {
              motor3Speed = motor3Speed - 50;
           }
    
           analogWrite(motor3EN, motor3Speed);
        }
        else
          analogWrite(motor3EN, 0);
      }
    }

    if ((lJoyX == 0 && lJoyY == 0) || bluetoothMode == true)
    {
      if ((rJoyX < 0 && rJoyX >= -255) || rLeft == true && stopped == false)
      {
        digitalWrite(motor1pin1, HIGH);
        digitalWrite(motor1pin2, LOW);
    
        digitalWrite(motor2pin1, HIGH);
        digitalWrite(motor2pin2, LOW);
    
        digitalWrite(motor3pin1, HIGH);
        digitalWrite(motor3pin2, LOW);

        if (rLeft == false)
          motorSpeed = rJoyX*-1;
        else if (rLeft == true)
          motorSpeed = 150;
 
        if (motorSpeed <= 120)
          motorSpeed = 120;
    
        analogWrite(motor1EN, motorSpeed);
        analogWrite(motor2EN, motorSpeed);
        analogWrite(motor3EN, motorSpeed);
      }
      else if ((rJoyX > 0 && rJoyX <= 255) || rRight == true && stopped == false)
      {
        digitalWrite(motor1pin1, LOW);
        digitalWrite(motor1pin2, HIGH);
    
        digitalWrite(motor2pin1, LOW);
        digitalWrite(motor2pin2, HIGH);
    
        digitalWrite(motor3pin1, LOW);
        digitalWrite(motor3pin2, HIGH);

        if (rRight == false)
          motorSpeed = rJoyX;
        else if (rRight == true)
          motorSpeed = 150;
          
        if (motorSpeed <= 120)
          motorSpeed = 120;
    
        analogWrite(motor1EN, motorSpeed);
        analogWrite(motor2EN, motorSpeed);
        analogWrite(motor3EN, motorSpeed);
      }
      else if (stopped == true && bluetoothMode == true)
      {
        rRight = false;
        rLeft = false;
        stopped = false;
        
        analogWrite(motor1EN, 0);
        analogWrite(motor2EN, 0);
        analogWrite(motor3EN, 0);
      }
      else
      {
        if (bluetoothMode == false)
        {
          analogWrite(motor1EN, 0);
          analogWrite(motor2EN, 0);
          analogWrite(motor3EN, 0);
        }
      }
    }

    if (bluetoothMode == true)
    {
      if (stopped == false)
      {
        if (forward == true)
        {
          digitalWrite(motor1pin1, LOW);
          digitalWrite(motor1pin2, HIGH);
      
          digitalWrite(motor2pin1, HIGH);
          digitalWrite(motor2pin2, LOW);
      
          analogWrite(motor1EN, 255);
          analogWrite(motor2EN, 255);
          analogWrite(motor3EN, 0);

          //Serial.println(forward);
        }
        else if (backwards == true)
        {
          digitalWrite(motor1pin1, HIGH);
          digitalWrite(motor1pin2, LOW);
      
          digitalWrite(motor2pin1, LOW);
          digitalWrite(motor2pin2, HIGH);
          
          analogWrite(motor1EN, 255);
          analogWrite(motor2EN, 255);
          analogWrite(motor3EN, 0);
        }
        else if (moveLeft == true)
        {
          digitalWrite(motor1pin1, HIGH);
          digitalWrite(motor1pin2, LOW);
      
          digitalWrite(motor2pin1, HIGH);
          digitalWrite(motor2pin2, LOW);
      
          digitalWrite(motor3pin1, LOW);
          digitalWrite(motor3pin2, HIGH);
          
          analogWrite(motor1EN, 85);
          analogWrite(motor2EN, 95);
          analogWrite(motor3EN, 170);
        }
        else if (moveRight == true)
        {
          digitalWrite(motor1pin1, LOW);
          digitalWrite(motor1pin2, HIGH);
      
          digitalWrite(motor2pin1, LOW);
          digitalWrite(motor2pin2, HIGH);
      
          digitalWrite(motor3pin1, HIGH);
          digitalWrite(motor3pin2, LOW);
          
          analogWrite(motor1EN, 84);
          analogWrite(motor2EN, 83);
          analogWrite(motor3EN, 170);
        }
      }
      else if (stopped == true)
      {
        forward = false;
        backwards = false;
        moveLeft = false;
        moveRight = false;
        stopped = false;
        
        analogWrite(motor1EN, 0);
        analogWrite(motor2EN, 0);
        analogWrite(motor3EN, 0);
      }
    } 

    if (lJoyX == 0 && lJoyY == 0 && rJoyX == 0 && rJoyY == 0 && bluetoothMode == false)
    {
      if (ps2x.Button(PSB_PAD_UP))
      {
        digitalWrite(motor1pin1, LOW);
        digitalWrite(motor1pin2, HIGH);
    
        digitalWrite(motor2pin1, HIGH);
        digitalWrite(motor2pin2, LOW);
    
        analogWrite(motor1EN, 255);
        analogWrite(motor2EN, 255);
        analogWrite(motor3EN, 0);
      }
      else if (ps2x.Button(PSB_PAD_DOWN))
      {
        digitalWrite(motor1pin1, HIGH);
        digitalWrite(motor1pin2, LOW);
    
        digitalWrite(motor2pin1, LOW);
        digitalWrite(motor2pin2, HIGH);
        
        analogWrite(motor1EN, 255);
        analogWrite(motor2EN, 255);
        analogWrite(motor3EN, 0);
      }
      else if (ps2x.Button(PSB_PAD_LEFT))
      {
        digitalWrite(motor1pin1, HIGH);
        digitalWrite(motor1pin2, LOW);
    
        digitalWrite(motor2pin1, HIGH);
        digitalWrite(motor2pin2, LOW);
    
        digitalWrite(motor3pin1, LOW);
        digitalWrite(motor3pin2, HIGH);
        
        analogWrite(motor1EN, 85);
        analogWrite(motor2EN, 90);
        analogWrite(motor3EN, 170);
      }
      else if (ps2x.Button(PSB_PAD_RIGHT))
      {
        digitalWrite(motor1pin1, LOW);
        digitalWrite(motor1pin2, HIGH);
    
        digitalWrite(motor2pin1, LOW);
        digitalWrite(motor2pin2, HIGH);
    
        digitalWrite(motor3pin1, HIGH);
        digitalWrite(motor3pin2, LOW);
        
        analogWrite(motor1EN, 84);
        analogWrite(motor2EN, 90);
        analogWrite(motor3EN, 170);
      }
      else
      {
        analogWrite(motor1EN, 0);
        analogWrite(motor2EN, 0);
        analogWrite(motor3EN, 0);
      }
    } 
    
    delay(100);
   }
}
