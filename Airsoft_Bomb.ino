// Airsoft_Bomb.ino
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ctype.h>

// LCD
#define I2C_ADDR 0x27
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
boolean lcdState = HIGH;

// Keypad
const byte rows = 4;
const byte cols = 4;

char keys[rows][cols] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

byte rowPins[rows] = {9,8,7,6};
byte colPins[cols] = {5,4,3,2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

// Bomb Pins
byte armedLEDIndicator = 10;
byte initiator = 11;

byte defusePins[4] = {A0,A1,A2,A3};

byte buzzer = 12;

// Bomb variables
byte defusePinIndex;

boolean armed = false;
byte hours;
byte minutes;
byte seconds;

char code[4];

char empty[] = "                ";

void setup() {
	Serial.begin(9600);

	// Set Up LCD
	lcd.begin(16,2);
	lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
	lcd.setBacklight(HIGH);

	// Set up pins
	pinMode(armedLEDIndicator, OUTPUT);
	digitalWrite(armedLEDIndicator, LOW);

	pinMode(buzzer, OUTPUT);
	digitalWrite(buzzer, LOW);

	pinMode(initiator, INPUT_PULLUP);

	for(int i = 0; i < 4; i++){
		pinMode(defusePins[i], INPUT_PULLUP);
	}

	// Print welcome text
	printHello();
	delay(2000);
}

void loop() {
	if(!armed){
		waitForInitiator();
		setBomb();
	}
	else{
		countdown();
	}
}

void clearLcd(){
	lcd.home();
	lcd.print(empty);
	lcd.setCursor(0,1);
	lcd.print(empty);
	lcd.home();
}

void printHello(){
	clearLcd();
	lcd.setCursor(2,0);
	lcd.print("Airsoft Bomb");
	lcd.setCursor(6,1);
	lcd.print("v0.1");
}

void waitForInitiator(){
	clearLcd();
	lcd.home();
	lcd.print("Connect");
	lcd.setCursor(0,1);
	lcd.print("Primer");

	// Wait for initiator to be connected
	do{
	    delay(1);
	} while (!isInitiatorConnected());
	delay(1000);
}

boolean isInitiatorConnected(){
	if(digitalRead(initiator) == HIGH){
		return false;
	}
	else{
		return true;
	}
}

void setBomb(){
	getTime();
	getCode();
	waitToArm();
}

void getTime(){
	clearLcd();
	// Print Message
	lcd.home();
	lcd.print("Set Time");
	lcd.setCursor(0,1);
	lcd.print("HH:MM:SS");
	lcd.setCursor(0,1);
	lcd.blink();

	boolean timeEntered = false;
	byte cursorPosition = 0;
	//byte

	do{
		// Check if initiator is still connected
		if(!isInitiatorConnected()){
			lcd.noBlink();
			return;
		}

	    char key = getKey();

	    if(key != NO_KEY){
	    	boolean correctValue = false;

	    	if(isdigit(key)){
	    		// Hackish way of converting char into digit
	    		byte digit = key - '0';

	    		// Get Hours
	    		if(cursorPosition == 0){
	    			hours = digit * 10;
	    			correctValue = true;
	    		}
	    		else if(cursorPosition == 1){
	    			hours += digit;
	    			correctValue = true;
	    		}
	    		// Get Minutes
	    		else if(cursorPosition == 3 && (digit >=0 && digit <= 6)){
	    			minutes = digit * 10;
	    			correctValue = true;
	    		}
	    		else if(cursorPosition == 4){
	    			if(minutes < 60){
	    				minutes += digit;
	    				correctValue = true;
	    			}
	    			else if(digit == 0){
	    				correctValue = true;
	    			}
	    		}
	    		//Get Seconds
	    		else if(cursorPosition == 6 && (digit >=0 && digit <= 6)){
	    			seconds = digit * 10;
	    			correctValue = true;
	    		}
	    		else if(cursorPosition == 7){
	    			if(seconds < 60){
	    				seconds += digit;
	    				correctValue = true;
	    			}
	    			else if(digit == 0){
	    				correctValue = true;
	    			}
	    		}

				if(correctValue){
					//Print digit
		    		lcd.print(digit);

		    		// Handle cursor move
		    		cursorPosition++;
		    		if(cursorPosition == 2 || cursorPosition == 5){
				    	cursorPosition++;
				    }
				    lcd.setCursor(cursorPosition, 1);
				}
	    	}
	    	// Clearing values
	    	else if(key == '*'  && cursorPosition > 0){
	    		timeEntered = false;

	    		if(cursorPosition < 4){
	 				hours = 0;
	 				lcd.setCursor(0,1);
	 				lcd.print("HH");
	 				cursorPosition = 0;   			
	    		}
	    		else if(cursorPosition < 7){
		    		minutes = 0;
	 				lcd.setCursor(3,1);
	 				lcd.print("MM");
	 				cursorPosition = 3;   				
	    		}
	    		else if(cursorPosition < 9){
	    			seconds = 0;
	 				lcd.setCursor(6,1);
	 				lcd.print("SS");
	 				cursorPosition = 6;   					
	    		}
	    		lcd.setCursor(cursorPosition,1);
	    	}
	    	else if(key = '#' && cursorPosition == 8){
	    		timeEntered = true;
	    	}
	    }
	} while (!timeEntered);
}

void getCode(){
	clearLcd();
	// Print Message
	lcd.home();
	lcd.print("Set Code");
	lcd.setCursor(0,1);
	lcd.print("____");
	lcd.setCursor(0,1);
	lcd.blink();

	boolean codeEntered = false;
	byte cursorPosition = 0;

	do{
	    // Check if initiator is still connected
		if(!isInitiatorConnected()){
			lcd.noBlink();
			return;
		}

	    char key = getKey();

	    if(key != NO_KEY){
	    	if(isdigit(key) && cursorPosition < 4){
    			code[cursorPosition] = key;	
    			lcd.print(key);
	    		cursorPosition++;
	    		lcd.setCursor(cursorPosition,1);
	    	}
		    else if(key == '*' && cursorPosition > 0){
		    	cursorPosition--;
		    	lcd.setCursor(cursorPosition,1);
		    	lcd.print("_");
		    	lcd.setCursor(cursorPosition,1);
		    }
		    else if(key == '#' && cursorPosition == 4){
		    	codeEntered = true;
		    }
	    }
	} while (!codeEntered);
}

void waitToArm(){
	clearLcd();
	lcd.print("Bomb Online");
	lcd.setCursor(0,1);
	lcd.print("Press \'A\' to arm");

	armed = false;
	do{
	    // Check if initiator is still connected
		if(!isInitiatorConnected()){
			lcd.noBlink();
			return;
		}

		char key = getKey();
	    if(key == 'A'){
	    	// Assing defuse pin
	    	randomSeed(millis());
	    	defusePinIndex = random(0, 4);

	    	armed = true;
	    }
	} while (!armed);
}

void countdown(){
	clearLcd();
	lcd.print("ARMED: ");
	updateTime();

	lcd.setCursor(0,1);
	lcd.print("Enter Code:____");
	lcd.setCursor(11,1);

	// Turn LED on
	digitalWrite(armedLEDIndicator, HIGH);

	// Times
	long lastTick = millis();

	byte cursorPosition = 11;

	char enteredCode[4];
	byte codeCounter = 0;

	do{
	   	if(!isInitiatorConnected()){
			lcd.noBlink();
			detonate();
		}

		// Decrement time
		// Seconds
		long newTick = millis();

		if(seconds > 0){
			if(newTick - lastTick >= 1000){
				seconds--;
				lastTick = newTick;
				updateTime();
			}
		}
		// Minutes
		if(minutes > 0 && seconds <= 0){
			minutes--;
			seconds = 60;
		}
		// Hours
		if(hours > 0 && minutes <= 0){
			hours--;
			minutes = 60;
		}

		// Check if time has elapsed
		if(hours == 0 && minutes == 0 && seconds == 0){
			detonate();
			armed = false;
		}

		// Check wires
		for(byte i = 0; i < 4; i++){
			if(i == defusePinIndex && digitalRead(defusePins[i]) == HIGH){
				defuse();
				armed = false;
			}
			else if(digitalRead(defusePins[i]) == HIGH){
				detonate();
				armed = false;
			}
		}

		// Check keypad
		lcd.setCursor(cursorPosition,1);
		lcd.blink();

		char key = getKey();

		if(key != NO_KEY){
			if(isdigit(key) && cursorPosition < 15){
				lcd.print(key);
				enteredCode[codeCounter] = key;
				cursorPosition++;
				codeCounter++;
			}
			else if(key == '*' && cursorPosition > 11){
		    	cursorPosition--;
		    	codeCounter--;
		    	lcd.setCursor(cursorPosition,1);
		    	lcd.print("_");
		    	lcd.setCursor(cursorPosition,1);
		    }
		    else if(key == '#' && cursorPosition == 15){
		    	boolean correct = true;

		    	for(int i= 0; i < 4; i++){
		    		if(code[i] != enteredCode[i]){
		    			correct = false;
		    			break;
		    		}
		    	}

		    	if(correct){
		    		defuse();
		    		armed = false;
		    	}
		    	else{
		    		cursorPosition = 11;
		    		codeCounter = 0;
		    		lcd.setCursor(cursorPosition,1);
		    		lcd.print("____");
		    		lcd.setCursor(cursorPosition,1);
		    	}
		    }
		}
	} while (armed);
}

void updateTime(){
	lcd.noBlink();

	lcd.setCursor(7,0);
	lcd.print(empty);
	lcd.setCursor(7,0);
	lcd.print(hours);
	lcd.print(":");
	lcd.print(minutes);
	lcd.print(":");
	lcd.print(seconds);
}

void detonate(){
	// LED off
	digitalWrite(armedLEDIndicator, LOW);

	// Show Message
	clearLcd();
	lcd.noBlink();
	lcd.print("DETONATED");
	lcd.setCursor(0,1);
	lcd.print("Reset with key");

	// Buzzer ON
	digitalWrite(buzzer, HIGH);

	delay(10000);

	// Buzzer OFF
	digitalWrite(buzzer, LOW);

	// Wait for reset
	for(;;){
	}
}
void defuse(){
	digitalWrite(armedLEDIndicator, LOW);

	clearLcd();
	lcd.noBlink();
	lcd.print("DEFUSED");
	lcd.setCursor(0,1);
	lcd.print("Press \'A\' to arm");

	boolean armAgain = false;
	do{
	    // Check if initiator is still connected
		if(!isInitiatorConnected()){
			lcd.noBlink();
			return;
		}

		char key = getKey();
	    if(key == 'A'){
	    	armAgain = true;
	    }
	}while(!armAgain);
}

// Keypad helpers
char getKey(){
	char key = keypad.getKey();
	if(key == 'B'){
		lcdState = !lcdState;
		lcd.setBacklight(lcdState);
	}
	return key;
}