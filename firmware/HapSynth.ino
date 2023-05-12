#include "Synth.h"
#include "SynthParam.h"
#include "HapticMode.h"
#include <EEPROM.h>

// From Hap
#define I2C_ADDRESS 0x20

#define IODIRA		0x00
#define IODIRB		0x01

#define IPOLA		0x02
#define IPOLB		0x03

#define GPINTENA	0x04
#define GPINTENB	0x05

#define DEFVALA		0x06
#define DEFVALB		0x07

#define INTCONA		0x08
#define INTCONB		0x09

#define IOCON		0x0A

#define GPPUA		0x0C
#define GPPUB		0x0D

#define INTFA		0x0E
#define INTFB		0x0F

#define INTCAPA		0x10
#define INTCAPB		0x11

#define GPIOA		0x12
#define GPIOB		0x13

#define EXPANDER_INTERRUPT_PIN 8
#define EXPANDER_INTERRUPT attachInterrupt(digitalPinToInterrupt(EXPANDER_INTERRUPT_PIN), expanderPressed, FALLING);

#define MOTOR_IN1 4
#define MOTOR_IN2 3
#define MOTOR_PWM 2
#define SLIDER    A0

#define BUTTON_1 12
#define BUTTON_2 11
#define BUTTON_3 10
#define BUTTON_4 9
#define BUTTON_5 6
#define BUTTON_6 5
#define BUTTON_SHIFT 15

#define EEPROM_SLIDER_MIN  0
#define EEPROM_SLIDER_MAX  2

void(* resetFunc) (void) = 0;

Synth* synth;
uint32_t synthParamChanged = 0;

int sliderMin = 0;
int sliderMax = 1023;
bool shiftPressed = false;
bool calibrateMin = false;
bool calibrateMax = false;

bool wasTouching = false;

void serialLogPrefix() {
	float mils = millis() / 1000.0f;
	int time = floor(mils);
	mils = round((mils - time) * 1000);
	int seconds = time % 60;
	int minutes = (time - seconds) / 60;

	Serial.print(minutes);
	Serial.print(":");
	if(seconds < 10) {
		Serial.print("0");
	}
	Serial.print(seconds);
	Serial.print(".");
	if(mils < 100) {
		Serial.print("0");
	}
	if(mils < 10) {
		Serial.print("0");
	}
	Serial.print(mils, 0);
	Serial.print(" | ");
}

void serialLog(const char msg[]) {
	serialLogPrefix();
	Serial.println(msg);
}

void serialLogParamName() {
	serialLogPrefix();
	synth->selectedParam->printName();
	Serial.println();
}

void serialLogValuesForThesis() {
	serialLogPrefix();
	
	Serial.print("Cutoff: ");
	Serial.print(String(synth->filterFrequency->rawValue / 1023.0f).c_str());
	
	Serial.print(", Attack: ");
	Serial.print(String(synth->egAttack->rawValue / 1023.0f).c_str());

	Serial.print(", Rate: ");
	Serial.print(String(synth->lfoRate->rawValue / 1023.0f).c_str());

	Serial.print(", LFO amount to PWM: ");
	Serial.print(String(synth->vcoPwmLfoAmount->rawValue / 1023.0f).c_str());

	Serial.print(", Mode: ");
	Serial.print(synth->hapticMode + 1);

	Serial.print(", Param: ");
	synth->selectedParam->printName();

	Serial.println();
}

void initExpander() {
	Wire.beginTransmission(I2C_ADDRESS);
	uint8_t result = Wire.endTransmission();
	if(result == 0) {
		Serial.print("Connected: ");
		Serial.println(I2C_ADDRESS, HEX);
	} else {
		Serial.print("Couldn't connect, please reset...");
	}

	// Reverse input polarity
	transmit(IPOLA, 0xFF);
	transmit(IPOLB, 0xFF);

	// Enable interrupts
	transmit(GPINTENA, 0xFF);
	transmit(GPINTENB, 0xFF);

	// Set interrupt default value
	transmit(DEFVALA, 0x00);
	transmit(DEFVALB, 0x00);

	// Compare interrupts to last value
	transmit(INTCONA, 0x00);
	transmit(INTCONB, 0x00);

	// Mirror INT pins
	transmit(IOCON, 0b01000001);
	
	// Set internal pull-ups to on
	transmit(GPPUA, 0xFF);
	transmit(GPPUB, 0xFF);

	// Reset interrupts
	transmit(INTCAPA);
	Wire.requestFrom(I2C_ADDRESS, 2);
	Wire.read();
	Wire.read();
}

void button1Pressed() {
	if(!shiftPressed) {
		serialLog("Position");
		synth->hapticMode = POSITION;
	} else {
		calibrateMin = true;
	}
}
void button2Pressed() {
	if(!shiftPressed) {
		serialLog("Detents");
		synth->hapticMode = DETENTS;
	} else {
		calibrateMax = true;
	}
}
void button3Pressed() {
	serialLog("Texture");
	synth->hapticMode = TEXTURE;
}
void button4Pressed() {
	serialLog("Friction");
	synth->hapticMode = FRICTION;
}
void button5Pressed() {
	serialLog("Elasticity");
	synth->hapticMode = ELASTICITY;
}
void button6Pressed() {
	if(!shiftPressed) {
		serialLog("Oscillation");
		synth->hapticMode = OSCILLATION;
	} else {
		serialLog("Resetting...");
		synth->reset();
		synthParamChanged = millis();
	}
}
void shiftChanged() {
	int value = digitalRead(BUTTON_SHIFT);
	
	if(value == 0) {
		serialLog("Shift on");
		shiftPressed = true;
	} else {
		serialLog("Shift off");
		shiftPressed = false;
	}
}

void expanderPressed() {
	noInterrupts();
	delayMicroseconds(1000);
	detachInterrupt(digitalPinToInterrupt(EXPANDER_INTERRUPT_PIN));
	interrupts();

	transmit(INTFA);
	Wire.requestFrom(I2C_ADDRESS, 2);
	int flagA = Wire.read();
	int flagB = Wire.read();
	 
	if(flagA != 0) {
		handleInputs(INTCAPA, flagA);
	} else {
		handleInputs(INTCAPB, flagB);
	}

	EXPANDER_INTERRUPT;
}

void handlePreset(uint8_t number) {
	synth->reset();

	switch (number)
	{
		case 1:
			serialLog("Preset 01");
			synth->filterFrequency->update(round(0.6f * 1023));
			break;
		case 2:
			serialLog("Preset 02");
			synth->egAttack->update(round(0.25f * 1023));
			break;
		case 3:
			serialLog("Preset 03");
			synth->lfoRate->update(round(0.4f * 1023));
			synth->vcoPwmLfoAmount->update(round(0.75f * 1023));
			break;
			
		case 4:
			serialLog("Preset 04");
			synth->filterFrequency->update(round(0.4f * 1023));
			break;
		case 5:
			serialLog("Preset 05");
			synth->egAttack->update(round(0.6f * 1023));
			break;
		case 6:
			serialLog("Preset 06");
			synth->lfoRate->update(round(0.75f * 1023));
			synth->vcoPwmLfoAmount->update(round(0.25f * 1023));
			break;
			
		case 7:
			serialLog("Preset 07");
			synth->filterFrequency->update(round(0.25f * 1023));
			break;
		case 8:
			serialLog("Preset 08");
			synth->egAttack->update(round(0.75f * 1023));
			break;
		case 9:
			serialLog("Preset 09");
			synth->lfoRate->update(round(0.6f * 1023));
			synth->vcoPwmLfoAmount->update(round(0.4f * 1023));
			break;
			
		case 10:
			serialLog("Preset 10");
			synth->filterFrequency->update(round(0.75f * 1023));
			break;
		case 11:
			serialLog("Preset 11");
			synth->egAttack->update(round(0.4f * 1023));
			break;
		case 12:
			serialLog("Preset 12");
			synth->lfoRate->update(round(0.25f * 1023));
			synth->vcoPwmLfoAmount->update(round(0.6f * 1023));
			break;
		default:
			break;
	}

	synthParamChanged = millis();
}

void handleInputs(char bank, int flag) {
	transmit(bank);
	Wire.requestFrom(I2C_ADDRESS, 1);
	int cap = Wire.read();

	int value = flag & cap;

	if(value != 0) {
		int n = 0;

		if((flag & B00000001) != 0) n = 1;
		if((flag & B00000010) != 0) n = 2;
		if((flag & B00000100) != 0) n = 3;
		if((flag & B00001000) != 0) n = 4;
		if((flag & B00010000) != 0) n = 5;
		if((flag & B00100000) != 0) n = 6;
		if((flag & B01000000) != 0) n = 7;
		if((flag & B10000000) != 0) n = 8;

		switch(bank) {
			case INTCAPA:
				switch (n) {
					case 1:
						if (!shiftPressed) {
							synth->selectedParam = synth->egDecay;
						} else {
							handlePreset(6);
						}
						break;
					case 2:
						if (!shiftPressed) {
							synth->selectedParam = synth->egAttack;
						} else {
							handlePreset(3);
						}
						break;
					case 3:
						if (!shiftPressed) {
							synth->selectedParam = synth->filterResonance;
						} else {
							handlePreset(5);
						}
						break;
					case 4:
						if (!shiftPressed) {
						synth->selectedParam = synth->filterFrequency;
						} else {
							handlePreset(2);
						}
						break;
					case 5:
						if (!shiftPressed) {
							synth->selectedParam = synth->vcoPwmLfoAmount;
						} else {
							handlePreset(7);
						}
						break;
					case 6:
						if (!shiftPressed) {
							synth->selectedParam = synth->vcoPwm;
						} else {
							handlePreset(4);
						}
						break;
					case 7:
						if (!shiftPressed) {
							synth->selectedParam = synth->vcoFrequency;
						} else {
							handlePreset(1);
						}
						break;
					case 8:
						break;
					default:
						break;
				}
				break;
			case INTCAPB:
				switch (n) {
					case 1:
						if (!shiftPressed) {
							synth->selectedParam = synth->mixerSub;
						} else {
							handlePreset(10);
						}
						break;
					case 2:
						synth->selectedParam = synth->mixerNoise;
						break;
					case 3:
						if (!shiftPressed) {
							synth->selectedParam = synth->filterDrive;
						} else {
							handlePreset(8);
						}
						break;
					case 4:
						if (!shiftPressed) {
							synth->selectedParam = synth->filterEGAmount;
						} else {
							handlePreset(11);
						}
						break;
					case 5:
						synth->selectedParam = synth->filterLFOAmount;
						break;
					case 6:
						if (!shiftPressed) {
							synth->selectedParam = synth->egSustain;
						} else {
							handlePreset(9);
						}
						break;
					case 7:
						if (!shiftPressed) {
							synth->selectedParam = synth->egRelease;
						} else {
							handlePreset(12);
						}
						break;
					case 8:
						synth->selectedParam = synth->lfoRate;
						break;
					default:
						break;
				}
				break;
			default:
				break;

		}

		if(n != 0  && !shiftPressed) {
			serialLogParamName();
			synthParamChanged = millis();
		}
	}
}

void transmit(int reg) {
	Wire.beginTransmission(I2C_ADDRESS);
	Wire.write(reg);
	Wire.endTransmission();
}

void transmit(int reg, int data) {
	Wire.beginTransmission(I2C_ADDRESS);
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission();
}

void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
	synth->onNoteOn(channel, note, velocity);
}

void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
	synth->onNoteOff(channel, note, velocity);
}

void OnPitchChange(uint8_t channel, int pitch) {
	synth->onPitchChange(channel, pitch);
}

void OnProgramChange(uint8_t channel, uint8_t program) {
	serialLog("Program Change Received");
	handlePreset(program);
}

void writeToEeprom(int address, int value) {
	EEPROM.write(address, value >> 8);
	EEPROM.write(address + 1, value & 0xFF);
}
int readFromEeprom(int address) {
	return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void setup() {
	Serial.begin(9600);
	Wire.begin();

	// Set pin modes
	pinMode(MOTOR_IN1, OUTPUT);
	pinMode(MOTOR_IN2, OUTPUT);
	pinMode(MOTOR_PWM, OUTPUT);

	pinMode(BUTTON_1, INPUT_PULLUP);
	pinMode(BUTTON_2, INPUT_PULLUP);
	pinMode(BUTTON_3, INPUT_PULLUP);
	pinMode(BUTTON_4, INPUT_PULLUP);
	pinMode(BUTTON_5, INPUT_PULLUP);
	pinMode(BUTTON_6, INPUT_PULLUP);
	pinMode(BUTTON_SHIFT, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(BUTTON_1), button1Pressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON_2), button2Pressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON_3), button3Pressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON_4), button4Pressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON_5), button5Pressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON_6), button6Pressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON_SHIFT), shiftChanged, CHANGE);

	pinMode(EXPANDER_INTERRUPT_PIN, INPUT);
	EXPANDER_INTERRUPT;
	
	initExpander();

	synth = new Synth();

	usbMIDI.setHandleNoteOff(OnNoteOff);
	usbMIDI.setHandleNoteOn(OnNoteOn);
	usbMIDI.setHandlePitchChange(OnPitchChange);
	usbMIDI.setHandleProgramChange(OnProgramChange);

	pinMode(LED_BUILTIN,OUTPUT);
	synthParamChanged = millis();

	sliderMin = readFromEeprom(EEPROM_SLIDER_MIN);
	sliderMax = readFromEeprom(EEPROM_SLIDER_MAX);
}

void loop() {
	usbMIDI.read();

	int value = analogRead(SLIDER);

	if(calibrateMin) {
		serialLog("Calibrating min...");
		digitalWrite(LED_BUILTIN, HIGH);

		int values = 0;
		for(int i = 0; i < 10; i++) {
			values += analogRead(SLIDER);
		}
		sliderMin = values / 10;
		writeToEeprom(EEPROM_SLIDER_MIN, sliderMin);

		digitalWrite(LED_BUILTIN, LOW);
		Serial.print("Calibration done, new min: ");
		Serial.println(sliderMin);

		calibrateMin = false;
	} else if(calibrateMax) {
		serialLog("Calibrating max...");
		digitalWrite(LED_BUILTIN, HIGH);

		int values = 0;
		for(int i = 0; i < 10; i++) {
			values += analogRead(SLIDER);
		}
		sliderMax = values / 10;
		writeToEeprom(EEPROM_SLIDER_MAX, sliderMax);

		digitalWrite(LED_BUILTIN, LOW);
		Serial.print("Calibration done, new max: ");
		Serial.println(sliderMax);

		calibrateMax = false;
	} else {
		value = map(value, sliderMin, sliderMax, 0, 1023);

		if(millis() - synthParamChanged > 100) {
			bool touching = digitalRead(22);

			synth->selectedParam->update(value);
			synth->selectedParam->hapticModes[synth->hapticMode]->update(value, touching);
			
			
			if (wasTouching && !touching) {
				serialLogValuesForThesis();
			}
			wasTouching = touching;
			
		} else {
			value -= synth->selectedParam->rawValue;

			if(value > 8) {
				digitalWrite(MOTOR_IN1, LOW);
				digitalWrite(MOTOR_IN2, HIGH);
				analogWrite(MOTOR_PWM, map(value, 0, 1023 - synth->selectedParam->rawValue, 128, 256));
			} else if (value < -8) {
				digitalWrite(MOTOR_IN1, HIGH);
				digitalWrite(MOTOR_IN2, LOW);
				analogWrite(MOTOR_PWM, map(-value, 0, synth->selectedParam->rawValue, 128, 256));
			} else {
				analogWrite(MOTOR_PWM, 0);
			}
		}
	}
}

