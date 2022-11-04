#include <Arduino.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <PinButton.h>

PinButton LeftB (8); //pin para boton izquierdo
PinButton RightB (16); //pin para boton derecho

char shftkey = KEY_LEFT_SHIFT; // Pan
char homekey = KEY_HOME; // home view
char altkey = KEY_LEFT_ALT; //
char f11key = KEY_F11;  // ALT + F11 shortcut for full screen freecad

struct Joystick {
	const uint8_t Xpin;
	const uint8_t Ypin;
	const uint8_t SWpin;
};

Joystick joy = {A0, A1, 10}; // VCC is throwaway, it's connected directly to VCC PIN

const int calibration = 220;	// adjust speed, lower value means faster movement
#define THRESHOLD 5
int yOffset, xOffset;
int yValue, xValue;
bool swState, swStatePrev = HIGH;
bool isPanning = false;

struct Encoder {
	const uint8_t CLK;
	const uint8_t DT;
};

Encoder enc = {2, 4}; // CLK and SW needs to be on 2 and 3, for attach interrupt

int currentStateCLK; 
int lastStateCLK;

void encoder_scroll() {
	currentStateCLK = digitalRead(enc.CLK);

	if (currentStateCLK != lastStateCLK){
		if (digitalRead(enc.DT) != currentStateCLK) {
		//	Serial.println("scroll up");
			Mouse.move(0, 0, 1);
		}
		else {
		//	Serial.println("scroll down");
			Mouse.move(0, 0, -1);
		}
	}

	lastStateCLK = currentStateCLK;
}


void setup() {
 Serial.begin(115200);
// put your setup code here, to run once:
  lastStateCLK = currentStateCLK; 
  pinMode(joy.Xpin, INPUT);
  pinMode(joy.Ypin, INPUT);
  pinMode(joy.SWpin, INPUT_PULLUP);

  pinMode (enc.CLK, INPUT);
  pinMode (enc.DT, INPUT);

  attachInterrupt(digitalPinToInterrupt(enc.CLK), encoder_scroll, CHANGE);

  delay(10);

  yOffset = analogRead(joy.Ypin);		// read center values
  xOffset = analogRead(joy.Xpin);

  Mouse.begin();
  Keyboard.begin();
}

void loop() {

  // put your main code here, to run repeatedly:

  LeftB.update();
  RightB.update();

  //LEFT BUTTON
  //Single click left button
  if (LeftB.isSingleClick()) {
    // ZOOM TO FIT FREECAD
    Keyboard.write('v');
    Keyboard.write('f');  
    delay(100);
    // Serial.println("Write V,F");
    // Serial.println("SINGLE");
  }
  //Doble click left button 
  if (LeftB.isDoubleClick()) {
    // ISOMETRIC VIEW FREECAD
    Keyboard.write('0');
    delay(100);
    // Serial.println("Press 0");
    // Serial.println("DOUBLE"); 
  }
  if (LeftB.isLongClick()) {    
    // FULL SCREEN FREECAD
    Keyboard.press(altkey);
    Keyboard.press(f11key); 
    // Serial.println("Press alt+F11");
    delay(100);
    Keyboard.releaseAll();	  
  }

  // RIGHT BUTTON
  //Single click right button
  if (RightB.isSingleClick()) {
    // SHOW-HIDE CONSTRAINS FREECAD (CUSTOM)
    Keyboard.write('9');
    delay(100);
    // Serial.println("Write 9");
    // Serial.println("SINGLE");    
  }
  //Double click right button
  if (RightB.isDoubleClick()) {
    // AS IS FREECAD
    Keyboard.print("v1");
    // Keyboard.write('1'); 
    delay(10);
    // Serial.println("ESC V,1");
    // Serial.println("DOUBLE");
  }
  
  if (RightB.isLongClick()) {    
    // Wrireframe FREECAD
    Keyboard.print("v3");
    // Keyboard.write('3'); 
    // Serial.println("Press alt+F11");
    delay(10);
  }
    
	xValue = analogRead(joy.Ypin) - yOffset;
	yValue = analogRead(joy.Xpin) - xOffset;

//	Serial.println(digitalRead(joy.SWpin));
//	Serial.print(xValue);
//	Serial.print(",");
//	Serial.println(yValue);

	swState = digitalRead(joy.SWpin);
	if(swState == HIGH && swStatePrev == LOW) {
		// Serial.println("joystick click");
		isPanning = !isPanning;		// toggle panning / orbiting
	}
	swStatePrev = swState;

	if (xValue > THRESHOLD || xValue < -THRESHOLD) {
		if(!isPanning) Keyboard.press(KEY_LEFT_SHIFT);	// orbiting
		Mouse.press(MOUSE_MIDDLE);
		Mouse.move(xValue/calibration, 0, 0);
	}

	if (yValue > THRESHOLD || yValue < -THRESHOLD) {
		if(!isPanning) Keyboard.press(KEY_LEFT_SHIFT);
		Mouse.press(MOUSE_MIDDLE);
		Mouse.move(0, -yValue/calibration, 0);
	}

	if (yValue <= THRESHOLD && yValue >= -THRESHOLD &&
		 xValue <= THRESHOLD && xValue >= -THRESHOLD) {
		Keyboard.releaseAll();
		Mouse.release(MOUSE_MIDDLE);
	}

	delay(10);
}