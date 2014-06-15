#include <IRremote.h>

// Adding the following define will stop keyboard and mouse
// support and add debugging messages through the COM port
#define DEBUGFLAG

// The following defines has been set based on my
// own remote from my LG TV which uses the NEC protocol
// http://wiki.altium.com/display/ADOH/NEC+Infrared+Transmission+Protocol
#define IR_UP 0xEF1002FD				// Menu up
#define IR_DOWN 0xEF10827D				// Menu down
#define IR_LEFT 0xEF10E01F				// Menu left
#define IR_RIGHT 0xEF10609F				// Menu right
#define IR_ENTER 0xEF1022DD				// Menu centre OK
#define IR_MOUSEUP 0xEF1048B7			// Keypad 2
#define IR_MOUSEDOWN 0xEF1018E7			// Keypad 8
#define IR_MOUSELEFT 0xEF1028D7			// Keypad 4
#define IR_MOUSERIGHT 0xEF106897		// Keypad 6
#define IR_MOUSELEFTCLICK 0xEF108877	// Keypad 1
#define IR_MOUSERIGHTCLICK 0xEF10C837	// Keypad 3
#define IR_ACTIVATE 0xEF1008F7			// Keypad 0
#define IR_DEACTIVATE 0xEF10D02F		// Input button
#define IR_LOGIN 0xEF10A857				// Keypad 5
#define IR_REPEAT 0xFFFFFFFF


static const char IR_RX_PIN = 6;		// Digital pin 6 is T0/OC4D/ADC10
static const char ACTIVE_PIN = 8;		// Digital pin 8 is PCINT/ADC1
//char ACTIVE_PIN_STATE;
char previousState;
char active;
byte fastRepeatFlag;
//unsigned long starTime;
//unsigned long endTime;
unsigned long currentTime;
unsigned long lastCommandTime;
IRrecv irrecv(IR_RX_PIN);
decode_results results;


namespace controls {
	static const char up = 0;
	static const char down = 1;
	static const char left = 2;
	static const char right = 3;
	static const char enter = 4;
	static const char back = 5;
	static const char mouseUp = 6;
	static const char mouseDown = 7;
	static const char mouseLeft = 8;
	static const char mouseRight = 9;
	static const char mouseLeftClick = 10;
	static const char mouseRightClick = 11;
	static const char repeat = 12;
	static const char login = 13;
	static const char activate = 14;
	static const char deactivate = 15;
	static const char unknown = 16;
};


void setup() {
	pinMode(ACTIVE_PIN, OUTPUT); 
	irrecv.enableIRIn(); // Start the receiver
  
	// Initialise global variables
	//ACTIVE_PIN_STATE = 0;
	previousState = controls::unknown;
	fastRepeatFlag = 0;
	//starTime = millis();
	currentTime =  millis();
	//endTime = starTime + 5000;
	lastCommandTime = currentTime;
	
	// initialize control over the keyboard & mouse or COM port
	#ifdef DEBUGFLAG
		Serial.begin(115200);
		// wait for serial port to connect. Needed for Leonardo only
		while (!Serial) { ; } 
		Serial.println("Sending decoded data!:");
	#else
		Keyboard.begin();
		Mouse.begin();
	#endif
	

}


void loop() {
	char currentState;
	
	// Check for active state timeout
	currentTime = millis(); 
	//if( (currentTime>endTime) || (currentTime<starTime)) {
		//ACTIVE_PIN_STATE = 0;
	//	currentState = controls::deactivate;
	//	previousState = controls::unknown;
	//	digitalWrite(ACTIVE_PIN, LOW);
	//}
	
	// Decode when IR signal received 
	if (irrecv.decode(&results)) {
		// Check if is inactive and the active button pressed
		//if( (ACTIVE_PIN_STATE == 0) && (results.value == IR_ACTIVATE) ) {
		//	// In this case activate and loop again from top
		//	ACTIVE_PIN_STATE = 1;
		//	digitalWrite(ACTIVE_PIN, HIGH);
		//	currentState = controls::activate;
		//	previousState = controls::unknown;
		//	starTime = currentTime;
		//	endTime = starTime + 60000;
		//}
		// Otherwise check if the remote is in active state
		//else if(ACTIVE_PIN_STATE>0) {
			// Set current state based on key pressed
			if(results.value == IR_UP) {
				currentState = controls::up;
			} else if(results.value == IR_DOWN) {
				currentState = controls::down;
			} else if(results.value == IR_LEFT) {
				currentState = controls::left;
			} else if(results.value == IR_RIGHT) {
				currentState = controls::right;
			} else if(results.value == IR_ENTER) {
				currentState = controls::enter;
			} else if(results.value == IR_MOUSEUP) {
				currentState = controls::mouseUp;
				fastRepeatFlag++;
			} else if(results.value == IR_MOUSEDOWN) {
				currentState = controls::mouseDown;
				fastRepeatFlag++;
			} else if(results.value == IR_MOUSELEFT) {
				currentState = controls::mouseLeft;
				fastRepeatFlag++;
			} else if(results.value == IR_MOUSERIGHT) {
				currentState = controls::mouseRight;
				fastRepeatFlag++;
			} else if(results.value == IR_MOUSELEFTCLICK) {
				currentState = controls::mouseLeftClick;
			} else if(results.value == IR_MOUSERIGHTCLICK) {
				currentState = controls::mouseRightClick;
			} else if(results.value == IR_LOGIN) {
				currentState = controls::login;
			} else if(results.value == IR_REPEAT) {
				currentState = previousState;
				if( (currentState == controls::mouseLeft)
					|| (currentState == controls::mouseRight)
					|| (currentState == controls::mouseUp)
					|| (currentState == controls::mouseDown) ) {
					fastRepeatFlag++;
				}
				#ifdef DEBUGFLAG
				else { Serial.print("Repeated "); }
				#endif
			//} else if(results.value == IR_DEACTIVATE) {
			//	// Deactivate
			//	ACTIVE_PIN_STATE = 0;
			//	digitalWrite(ACTIVE_PIN, LOW);
			//	currentState = controls::deactivate;
			//	previousState = controls::unknown;
			} else {
				currentState = controls::unknown;
			}    

			// Print the code if not in use
			if(currentState == controls::unknown) {
				#ifdef DEBUGFLAG
					Serial.print("Ignored unknown code: ");
					Serial.println(results.value, HEX);
				#endif
			// Otherwise run command if >350ms since last command run
			} else if( (currentTime>lastCommandTime+350)
				|| (currentTime<lastCommandTime)
				|| (fastRepeatFlag>0) ) {
					if(currentTime>lastCommandTime+400) {
						fastRepeatFlag = 0;
					}
					// print the current control signal
					performAction(currentState);
					lastCommandTime = currentTime;
			}
			
			previousState = currentState;
			
			// Reset active timeout timer for another minute
			//StarTime = currentTime;
			//endTime = currentTime + 60000;
		//}
		irrecv.resume(); // Receive the next value
	}
}


void performAction(char docommand) {
	#ifdef DEBUGFLAG
		debugSerial(docommand);
	#else
		switch(docommand) {
			case controls::up:
				Keyboard.write(KEY_UP_ARROW);
				blinkCommand();
			break;
			case controls::down:
				Keyboard.write(KEY_DOWN_ARROW);
				blinkCommand();
			break;
			case controls::left:
				Keyboard.write(KEY_LEFT_ARROW);
				blinkCommand();
			break;
			case controls::right:
				Keyboard.write(KEY_RIGHT_ARROW);
				blinkCommand();
			break;
			case controls::enter:
				Keyboard.write(KEY_RETURN);
				blinkCommand();
			break;
			case controls::mouseUp:
				Mouse.move(0, (-1*((int)fastRepeatFlag)), 0);
				blinkCommand();
			break;
			case controls::mouseDown:
				Mouse.move(0, ((int)fastRepeatFlag), 0);
				blinkCommand();
			break;
			case controls::mouseLeft:
				Mouse.move((-1*((int)fastRepeatFlag)), 0, 0);
				blinkCommand();
			break;
			case controls::mouseRight:
				Mouse.move(((int)fastRepeatFlag), 0, 0);
				blinkCommand();
			break;
			case controls::mouseLeftClick:
				Mouse.click(MOUSE_LEFT);
				blinkCommand();
			break;
			case controls::mouseRightClick:
				Mouse.click(MOUSE_RIGHT);
				blinkCommand();
			break;
			case controls::login:
				blinkCommand();
				Keyboard.print("wasd");
				Keyboard.write(KEY_RETURN);
				//Keyboard.println("");
				delay(7000);
				// Show desktop
				Keyboard.press(KEY_LEFT_GUI);
				Keyboard.press('d');
				Keyboard.releaseAll();
				// XBMC is the second windows taksbar icon
				Keyboard.press(KEY_LEFT_GUI);
				Keyboard.press('2');
				Keyboard.releaseAll();
			break;
			case controls::repeat: break;
			case controls::activate: break;
			case controls::deactivate: break;
			case controls::unknown: break;
		}
	#endif
}


void debugSerial(char printcommand) {
	switch(printcommand) {
		case controls::up:
			Serial.print("Up: ");
			Serial.println(IR_UP, HEX); 
		break;
		case controls::down: 
			Serial.print("Down: ");
			Serial.println(IR_DOWN, HEX); 
		break;
		case controls::left: 
			Serial.print("Left: ");
			Serial.println(IR_LEFT, HEX); 
		break;
		case controls::right: 
			Serial.print("Right: ");
			Serial.println(IR_RIGHT, HEX); 
		break;
		case controls::enter: 
			Serial.print("Enter: ");
			Serial.println(IR_ENTER, HEX); 
		break;
		case controls::mouseUp: 
			Serial.print("Mouse Up(");
			Serial.print(fastRepeatFlag);
			Serial.print("): ");
			Serial.println(IR_MOUSEUP, HEX); 
		break;
		case controls::mouseDown: 
			Serial.print("Mouse Down(");
			Serial.print(fastRepeatFlag);
			Serial.print("): ");
			Serial.println(IR_MOUSEDOWN, HEX); 
		break;
		case controls::mouseLeft: 
			Serial.print("Mouse Left(");
			Serial.print(fastRepeatFlag);
			Serial.print("): ");
			Serial.println(IR_MOUSELEFT, HEX); 
		break;
		case controls::mouseRight: 
			Serial.print("Mouse Right(");
			Serial.print(fastRepeatFlag);
			Serial.print("): ");
			Serial.println(IR_MOUSERIGHT, HEX); 
		break;
		case controls::mouseLeftClick: 
			Serial.print("Mouse Left Click: ");
			Serial.println(IR_LEFT, HEX); 
		break;
		case controls::mouseRightClick: 
			Serial.print("Mouse Right Click: ");
			Serial.println(IR_MOUSERIGHT, HEX); 
		break;
		case controls::repeat:
			Serial.print("Repeat: ");
			Serial.println(IR_REPEAT, HEX); 
		break;
		case controls::login:
			Serial.print("Login: ");
			Serial.println(IR_LOGIN, HEX);
		case controls::activate:
			Serial.print("Activate: ");
			Serial.println(IR_ACTIVATE, HEX); 
		break;
		case controls::deactivate:
			Serial.print("Deactivate: ");
			Serial.println(IR_DEACTIVATE, HEX); 
		break;
		case controls::unknown:
			Serial.println("Unknown");
		break;
	}
	blinkCommand();
}


void blinkCommand() {
	digitalWrite(ACTIVE_PIN, HIGH);
	delay(70);
	digitalWrite(ACTIVE_PIN, LOW);
}
