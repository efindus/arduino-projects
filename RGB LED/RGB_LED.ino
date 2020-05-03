/*
RGB LED Controlled by Bluetooth or Potentiometer
by EFindus AKA. adammickiewicz635

This code allows you to control RGB light by Bluetooth or single potentiometer.

Created on 1 May 2020
by EFindus

Instructable link:
https://www.instructables.com/id/RGB-Led-Controlled-by-Bluetooth-or-Potentiometer/
*/
#include <Wire.h> 
#include <PCF8574.h>
#include <LiquidCrystal_I2C.h>

//RGB LED
#define RED_PIN 10
#define GREEN_PIN 6
#define BLUE_PIN 5

//EXPANDER
#define RAINBOW_LED 4
#define INTERFACE_LED 5
#define LOCK_LED 6

//BUTTONS
#define R_COLOR_BUTTON 4
#define L_COLOR_BUTTON 7
#define LOCK_BUTTON 8
#define RAINBOW_BUTTON 9
#define INTERFACE_BUTTON 11

//POTENTIOMETER
#define POTENTIOMETER A0

//ADDRESSES
#define LCD 0x3F
#define PCF 0x20

//RGB LED TYPE
#define RGB_COMMON 0 //0 - common cathode, 1 - common anode
#if RGB_COMMON == 0
#define MIN 255
#define MAX 0
#elif RGB_COMMON == 1
#define MIN 0
#define MAX 255
#endif

LiquidCrystal_I2C lcd(LCD, 16, 2);
PCF8574 expander;

int interface = 0; //0 - BUTTONS; 1 - BLUETOOTH
int mode = 0; //0 - NORMAL; 1 - RAINBOW; 2 - LOCK
int aRead = 0; // potentiometer value
int color = 0; //0 - RED; 1 - GREEN; 2 - BLUE
int R = 0;
int G = 0;
int B = 0;
String output = ""; //Color display on LCD
int flicker = 0;
bool asyncbutton[] = {false, false, false, false, false}; //R_COLOR; L_COLOR; INTERFACE; RAINBOW; LOCK;

//calibration
int lowest = 0;
int highest = 255;

//RAINBOW
int counter = 0;
int numColors = 255;
int animationDelay = 10;

//BLUETOOTH
char cmd[100];
byte cmdIndex = 0;

void calibratePotentiometer() {
  //this was anoying me in setup so moved here
  lcd.setCursor(0, 0);
  lcd.print("Calibration...  ");
  int index = 0;
  while(index < 3000) { //Expert mode without unnecessary delays
    if(digitalRead(INTERFACE_BUTTON) == LOW) {
      while(digitalRead(INTERFACE_BUTTON) == LOW);
      
      lcd.setCursor(0, 0);
      lcd.print("Lowest:         ");
      
      while(digitalRead(INTERFACE_BUTTON) == HIGH);
      delay(20);
      expander.digitalWrite(INTERFACE_LED, HIGH);
      while(digitalRead(INTERFACE_BUTTON) == LOW);
      delay(20);
      expander.digitalWrite(INTERFACE_LED, LOW);
      
      lowest = analogRead(POTENTIOMETER) / 4 + 2;
      R = lowest;
      G = lowest;
      B = lowest;
      
      lcd.setCursor(0, 0);
      lcd.print("Highest:        ");
      
      while(digitalRead(INTERFACE_BUTTON) == HIGH);
      delay(20);
      expander.digitalWrite(INTERFACE_LED, HIGH);
      while(digitalRead(INTERFACE_BUTTON) == LOW);
      delay(20);
      expander.digitalWrite(INTERFACE_LED, LOW);
      
      highest = analogRead(POTENTIOMETER) / 4 - 4;
      
      lcd.setCursor(0, 0);
      lcd.print("     Thanks!    ");
      lcd.setCursor(0, 1);
      lcd.print("  Turning on... ");
      delay(1000);
      return;
    }
    index++;
    delay(1);
  }
  lcd.setCursor(0, 0);
  lcd.print("Set potentiomet-");
  lcd.setCursor(0, 1);
  lcd.print("er to the lowest");
  delay(5000);
  
  lcd.setCursor(0, 0);
  lcd.print("value and click ");
  lcd.setCursor(0, 1);
  lcd.print("interface button");
  
  while(digitalRead(INTERFACE_BUTTON) == HIGH);
  delay(20);
  expander.digitalWrite(INTERFACE_LED, HIGH);
  while(digitalRead(INTERFACE_BUTTON) == LOW);
  delay(20);
  expander.digitalWrite(INTERFACE_LED, LOW);
  
  lowest = analogRead(POTENTIOMETER) / 4 + 2;
  R = lowest;
  G = lowest;
  B = lowest;
  
  lcd.setCursor(0, 0);
  lcd.print("Set potentiomet-");
  lcd.setCursor(0, 1);
  lcd.print("er to the highe-");
  delay(5000);
  
  lcd.setCursor(0, 0);
  lcd.print("st value and cl-");
  lcd.setCursor(0, 1);
  lcd.print("ick same button.");
  
  while(digitalRead(INTERFACE_BUTTON) == HIGH);
  delay(20);
  expander.digitalWrite(INTERFACE_LED, HIGH);
  while(digitalRead(INTERFACE_BUTTON) == LOW);
  delay(20);
  expander.digitalWrite(INTERFACE_LED, LOW);
  
  highest = analogRead(POTENTIOMETER) / 4 - 4;
  
  lcd.setCursor(0, 0);
  lcd.print("     Thanks!    ");
  lcd.setCursor(0, 1);
  lcd.print("  Turning on... ");
  delay(5000);
}

void setup() {
  Serial1.begin(9600);
  
  expander.begin(PCF);
  
  lcd.init();
  lcd.backlight();
  
  pinMode(POTENTIOMETER, INPUT);
  pinMode(R_COLOR_BUTTON, INPUT_PULLUP);
  pinMode(L_COLOR_BUTTON, INPUT_PULLUP);
  pinMode(LOCK_BUTTON, INPUT_PULLUP);
  pinMode(RAINBOW_BUTTON, INPUT_PULLUP);
  pinMode(INTERFACE_BUTTON, INPUT_PULLUP);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  expander.pinMode(RAINBOW_LED, OUTPUT);
  expander.pinMode(INTERFACE_LED, OUTPUT);
  expander.pinMode(LOCK_LED, OUTPUT);

  expander.digitalWrite(RAINBOW_LED, HIGH);
  expander.digitalWrite(INTERFACE_LED, LOW);
  expander.digitalWrite(LOCK_LED, HIGH);

  digitalWrite(RED_PIN, MIN == 255 ? HIGH : LOW);
  digitalWrite(GREEN_PIN, MIN == 255 ? HIGH : LOW);
  digitalWrite(BLUE_PIN, MIN == 255 ? HIGH : LOW);

  calibratePotentiometer();
  
  lcd.setCursor(0, 0);
  lcd.print("Color Mixer     ");
  lcd.setCursor(0, 1);
  lcd.print("Working...      ");
}

void printwrite() { //Utility function to print to lcd and write vars to RGB LED
  
  int _R = map(R, 0, 255, MIN, MAX);
  int _G = map(G, 0, 255, MIN, MAX);
  int _B = map(B, 0, 255, MIN, MAX);
  
  if(mode != 1 && interface == 0) {
    _R = map(R, lowest, highest, MIN, MAX);
    _G = map(G, lowest, highest, MIN, MAX);
    _B = map(B, lowest, highest, MIN, MAX);
  }

  if(_R > 255) _R = 255;
  if(_R < 0) _R = 0;
  if(_G > 255) _G = 255;
  if(_G < 0) _G = 0;
  if(_B > 255) _B = 255;
  if(_B < 0) _B = 0;
  
  output = "R";
  output += map(_R, MIN, MAX, 0, 255);
  output += " G";
  output += map(_G, MIN, MAX, 0, 255);
  output += " B";
  output += map(_B, MIN, MAX, 0, 255);

  for(int i = output.length(); i < 16; i++) {
    output += " ";
  }

  if(mode == 0 && interface == 0) {
    char Colors[] = {'R', 'G', 'B'};
    flicker++;
    if(flicker > 35) {
      if(flicker == 70) flicker = 0;
      for(int i = 0; i < 16; i++) {
        if(output[i] == Colors[color]) {
          output[i] = ' ';
        }
      }
    }
  }

  lcd.setCursor(0, 1);
  lcd.print(output);

  analogWrite(RED_PIN, _R);
  analogWrite(GREEN_PIN, _G);
  analogWrite(BLUE_PIN, _B);
}

String cmdConvert() { //Utility function that returns commmand string from cmd char array
  String cmdOut = "";
  for(int i = 0; i < cmdIndex; i++) {
    cmdOut += cmd[i];
  }
  return cmdOut;
}

bool asyncButton(int pin, int buttonID) {
  if(digitalRead(pin) == LOW && asyncbutton[buttonID] == false) {
    delay(20);
    asyncbutton[buttonID] = true;
    return true;
  } else if(digitalRead(pin) == HIGH && asyncbutton[buttonID] == true) {
    delay(20);
    asyncbutton[buttonID] = false;
  }
  return false;
}

void RAINBOW() {
  float colorNumber = counter > numColors ? counter - numColors: counter;
      
  float saturation = 1; // Between 0 and 1 (0 = gray, 1 = full color)
  float brightness = 1; // Between 0 and 1 (0 = dark, 1 is full brightness)
  float hue = (colorNumber / float(numColors)) * 360;
  long color = HSBtoRGB(hue, saturation, brightness);
  
  R = color >> 16 & 255;
  G = color >> 8 & 255;
  B = color & 255;
  
  counter = (counter + 1) % (numColors * 2);
  
  delay(animationDelay);
}

void exeCmd() { //Utility function to execute bluetooth commands
  String command = cmdConvert();
  if(command == "RAINBOW") {
    if(mode == 0) mode = 1;
    else if(mode == 1) mode = 0;
    
  } else if(command == "LOCK") {
    if(mode == 0) mode = 2;
    else if(mode == 2) mode = 0;
    
  } else if(command[0] == 'R' && mode == 0) {
    R = atof(cmd + 2);
  } else if(command[0] == 'G' && mode == 0) {
    G = atof(cmd + 2);
  } else if(command[0] == 'B' && mode == 0) {
    B = atof(cmd + 2);
  }
}

void loop() {
  if(interface == 0) { //BUTTONS
    expander.digitalWrite(INTERFACE_LED, LOW);
    
    if(asyncButton(R_COLOR_BUTTON, 0) == true) {
      if(mode == 0) {
         color++;
        if(color == 3) color = 0;
      }
    }
    if(asyncButton(L_COLOR_BUTTON, 1) == true) {
      if(mode == 0) {
        color--;
        if(color == -1) color = 2;
      }
    }
    if(asyncButton(INTERFACE_BUTTON, 2) == true) {
      if(mode == 0) interface++;
    }
    if(asyncButton(RAINBOW_BUTTON, 3) == true) {
      if(mode == 1) mode = 0;
      else if(mode == 0) mode = 1;
    }
    if(asyncButton(LOCK_BUTTON, 4) == true) {
      if(mode == 2) mode = 0;
      else if(mode == 0) mode = 2;
    }

    if(mode == 0) { 
      aRead = analogRead(POTENTIOMETER);
      aRead /= 4;
      if(aRead < 0) aRead = 0;
      if(aRead > 255) aRead = 255;
      
      if(color == 0) R = aRead;
      else if(color == 1) G = aRead;
      else B = aRead;
    }
    
  } else { //BLUETOOTH
    expander.digitalWrite(INTERFACE_LED, HIGH);
    
    if(asyncButton(INTERFACE_BUTTON, 2) == true) {
      if(mode == 0) interface--;
    }

    if (Serial1.available() > 0) {
      char c = (char)Serial1.read();
      if (c == '\n') {
        cmd[cmdIndex] = 0;
        exeCmd();
        cmdIndex = 0;
      } else {
        cmd[cmdIndex] = c;
        if (cmdIndex < 99) cmdIndex++;
      }
    }
  }

  if(mode == 0) {
    expander.digitalWrite(RAINBOW_LED, HIGH);
    expander.digitalWrite(LOCK_LED, HIGH);
  }
  
  if(mode == 1) {
    expander.digitalWrite(RAINBOW_LED, LOW);
    
    RAINBOW();
  }
  
  if(mode == 2) {
    expander.digitalWrite(LOCK_LED, LOW);
  }
  
  printwrite();
}

long HSBtoRGB(float _hue, float _sat, float _brightness) { //Utility converting function
   float red = 0.0;
   float green = 0.0;
   float blue = 0.0;
   
   if (_sat == 0.0) {
       red = _brightness;
       green = _brightness;
       blue = _brightness;
   } else {
       if (_hue == 360.0) {
           _hue = 0;
       }

       int slice = _hue / 60.0;
       float hue_frac = (_hue / 60.0) - slice;

       float aa = _brightness * (1.0 - _sat);
       float bb = _brightness * (1.0 - _sat * hue_frac);
       float cc = _brightness * (1.0 - _sat * (1.0 - hue_frac));
       
       switch(slice) {
           case 0:
               red = _brightness;
               green = cc;
               blue = aa;
               break;
           case 1:
               red = bb;
               green = _brightness;
               blue = aa;
               break;
           case 2:
               red = aa;
               green = _brightness;
               blue = cc;
               break;
           case 3:
               red = aa;
               green = bb;
               blue = _brightness;
               break;
           case 4:
               red = cc;
               green = aa;
               blue = _brightness;
               break;
           case 5:
               red = _brightness;
               green = aa;
               blue = bb;
               break;
           default:
               red = 0.0;
               green = 0.0;
               blue = 0.0;
               break;
       }
   }

   long ired = red * 255.0;
   long igreen = green * 255.0;
   long iblue = blue * 255.0;
   
   return long((ired << 16) | (igreen << 8) | (iblue));
}
