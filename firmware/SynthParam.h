#pragma once

#include "HapticMode.h"

class Synth;

class SynthParam {
	public:
		HapticMode* hapticModes[6];
		int rawValue = 512;
		int defaultValue = 512;

		SynthParam(Synth* synth) {
			this->synth = synth;

			hapticModes[POSITION] = new Position();
			hapticModes[ELASTICITY] = new Elasticity(&rawValue);
			hapticModes[DETENTS] = new Detents(5);
			hapticModes[TEXTURE] = new Texture();
			hapticModes[OSCILLATION] = new Oscillation(&rawValue);
			hapticModes[FRICTION] = new Friction();
		}
		virtual void update(int position);
		
		void reset() {
			rawValue = defaultValue;
		}

		virtual void printName() { Serial.print("__param__"); }
	
	protected:
		Synth* synth;
};

//
// VCO
//
class VCOFrequency : public SynthParam {
	public:
		VCOFrequency(Synth* synth) : SynthParam(synth) {
			hapticModes[DETENTS] = new Detents(3);
		}
		void update(int position) override;
		void printName() override { Serial.print("VCO Frequency"); }
};
class VCOPWM : public SynthParam {
	public:
		VCOPWM(Synth* synth) : SynthParam(synth) { }
		void update(int position) override;
		void printName() override { Serial.print("VCO PWM"); }
};
class VCOPWMLFOAmount : public SynthParam {
	public:
		VCOPWMLFOAmount(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
		}
		void update(int position) override;
		void printName() override { Serial.print("VCO PWM LFO Amount"); }
};

//
// Mixer
//
class MixerSub : public SynthParam {
	public:
		MixerSub(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
		}
		void update(int position) override;
		void printName() override { Serial.print("Mixer Sub"); }
};
class MixerNoise : public SynthParam {
	public:
		MixerNoise(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
		}
		void update(int position) override;
		void printName() override { Serial.print("Mixer Noise"); }
};

//
// Filter
//
class FilterFrequency : public SynthParam {
	public:
		FilterFrequency(Synth* synth) : SynthParam(synth) {
			defaultValue = 1023;
		}
		void update(int position) override;
		void printName() override { Serial.print("Filter Frequency"); }
};
class FilterResonance : public SynthParam {
	public:
		FilterResonance(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
			hapticModes[DETENTS] = new Detents(4);
		}
		void update(int position) override;
		void printName() override { Serial.print("Filter Resonance"); }
};
class FilterDrive : public SynthParam {
	public:
		FilterDrive(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
		}
		void update(int position) override;
		void printName() override { Serial.print("Filter Drive"); }
};
class FilterEGAmount : public SynthParam {
	public:
		FilterEGAmount(Synth* synth) : SynthParam(synth) {
			defaultValue = 512;
		}
		void update(int position) override;
		void printName() override { Serial.print("Filter EG Amount"); }
};
class FilterLFOAmount : public SynthParam {
	public:
		FilterLFOAmount(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
		}
		void update(int position) override;
		void printName() override { Serial.print("Filter LFO Amount"); }
};

// Envelope generator
class EGAttack : public SynthParam {
	public:
		EGAttack(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
			hapticModes[DETENTS] = new Detents(9);
		}
		void update(int position) override;
		void printName() override { Serial.print("EG Attack"); }
};
class EGDecay : public SynthParam {
	public:
		EGDecay(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
			hapticModes[DETENTS] = new Detents(9);
		}
		void update(int position) override;
		void printName() override { Serial.print("EG Decay"); }
};
class EGSustain : public SynthParam {
	public:
		EGSustain(Synth* synth) : SynthParam(synth) {
			defaultValue = 1023;
		}
		void update(int position) override;
		void printName() override { Serial.print("EG Sustain"); }
};
class EGRelease : public SynthParam {
	public:
		EGRelease(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
			hapticModes[DETENTS] = new Detents(9);
		}
		void update(int position) override;
		void printName() override { Serial.print("EG Release"); }
};

//
// LFO
//
class LFO : public SynthParam {
	public:
		LFO(Synth* synth) : SynthParam(synth) {
			defaultValue = 0;
			hapticModes[DETENTS] = new Detents(9);
		}
		void update(int position) override;
		void printName() override { Serial.print("LFO Rate"); }
};