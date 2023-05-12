#pragma once

#define MOTOR_IN1 4
#define MOTOR_IN2 3
#define MOTOR_PWM 2

enum HAPTIC_MODE {
	POSITION = 0,
	DETENTS = 1,
	TEXTURE = 2,
	FRICTION = 3,
	ELASTICITY = 4,
	OSCILLATION = 5
};

class HapticMode {
	public:
		virtual void update(int position, bool touching);
};

class Position : public HapticMode {
	public:
		Position() { }
		void update(int position, bool touching) override {
			analogWrite(MOTOR_PWM, 0);
		}
};

class Elasticity : public HapticMode {
	public:
		Elasticity(int* center) {
			this->center = center;
		}

		void update(int position, bool touching) override {
			position -= *center;

			if(position > 8) {
				digitalWrite(MOTOR_IN1, LOW);
				digitalWrite(MOTOR_IN2, HIGH);
				analogWrite(MOTOR_PWM, map(position, 0, 1023 - *center, 64, 256));
			} else if (position < -8) {
				digitalWrite(MOTOR_IN1, HIGH);
				digitalWrite(MOTOR_IN2, LOW);
				analogWrite(MOTOR_PWM, map(-position, 0, *center, 64, 256));
			} else {
				analogWrite(MOTOR_PWM, 0);
			}
		}
	
	private:
		int* center;
};

class Detents : public HapticMode {
	public:
		Detents(int detents) {
			this->detentSize = 1024.0f / (detents - 1);
			this->halfDetent = this->detentSize / 2.0f;
		}

		void update(int position, bool touching) override {
			float notch = round(position / this->detentSize) * this->detentSize;
			float min = constrain(notch - this->halfDetent, 0, 1023);
			float max = constrain(notch + this->halfDetent, 0, 1023);

			if(position > notch + 16) {
				digitalWrite(MOTOR_IN1, LOW);
				digitalWrite(MOTOR_IN2, HIGH);
				analogWrite(MOTOR_PWM, map(position, notch, max, 192, 64));
			} else if(position < notch - 16) {
				digitalWrite(MOTOR_IN1, HIGH);
				digitalWrite(MOTOR_IN2, LOW);
				analogWrite(MOTOR_PWM, map(position, min, notch, 64, 192));
			} else {
				analogWrite(MOTOR_PWM, 0);
			}
		}

	private:
		float detentSize;
		float halfDetent;
};

class Texture : public HapticMode {
	public:
		Texture() { }

		void update(int position, bool touching) override {
			int direction = ((int)roundf(millis() / 5.0f)) % 2;
			int distance = abs(position - previousPosition);
			previousPosition = previousPosition * 0.99f + position * 0.01f;

			if(distance > 1.0f) {
				float vibration = constrain(distance * 4, 32, 255);
				if(direction > 0) {
					digitalWrite(MOTOR_IN1, LOW);
					digitalWrite(MOTOR_IN2, HIGH);
					analogWrite(MOTOR_PWM, vibration);
				} else {
					digitalWrite(MOTOR_IN1, HIGH);
					digitalWrite(MOTOR_IN2, LOW);
					analogWrite(MOTOR_PWM, vibration);
				}
			} else {
				analogWrite(MOTOR_PWM, 0);
			}
		}

	private:
		float previousPosition = 0;
};

class Oscillation : public HapticMode {
	public:
		Oscillation(int* center) {
			this->center = center;
		}

		void update(int position, bool touching) override {
			if(touching) {
				displacement = abs(position - *center) * damping;
				direction = position > *center ? 1 : -1;
				analogWrite(MOTOR_PWM, 0);
			} else {
				if(displacement > 8) {
					if(direction > 0) {
						if(position < *center - displacement || position < 8) {
							direction = -1;
							displacement *= damping;
						}
					} else if(direction < 0) {
						if(position > *center + displacement || position > 1023 - 8) {
							direction = 1;
							displacement *= damping;
						}
					}
					
					int speed = map(displacement, 0, 1023, 64, 255);

					if(direction > 0) {
						digitalWrite(MOTOR_IN1, LOW);
						digitalWrite(MOTOR_IN2, HIGH);
						analogWrite(MOTOR_PWM, speed);
					} else if (direction < 0) {
						digitalWrite(MOTOR_IN1, HIGH);
						digitalWrite(MOTOR_IN2, LOW);
						analogWrite(MOTOR_PWM, speed);
					}
				} else {
					analogWrite(MOTOR_PWM, 0);
				}
			}
		}

	private:
		int* center;

		int8_t direction = 0;
		float displacement = 0.0f;
		float damping = 0.75f;
};

class Friction : public HapticMode {
	public:
		Friction() { }

		void update(int position, bool touching) override {
			int direction = position - previousPosition;
			previousPosition = previousPosition * 0.99f + position * 0.01f;
			
			int val = map(min(abs(direction), 255), 0, 255, 255, 0);

			if(touching) {
				if(direction > 2) {
					digitalWrite(MOTOR_IN1, LOW);
					digitalWrite(MOTOR_IN2, HIGH);
					analogWrite(MOTOR_PWM, val);
				} else if(direction < -2) {
					digitalWrite(MOTOR_IN1, HIGH);
					digitalWrite(MOTOR_IN2, LOW);
					analogWrite(MOTOR_PWM, val);
				} else {
					analogWrite(MOTOR_PWM, 0);
				}
			} else {	
				analogWrite(MOTOR_PWM, 0);
			}
		}

	private:
		float previousPosition = 0;
		uint32_t previousTime = 0;
};