#include "Synth.h"
#include "SynthParam.h"

void SynthParam::update(int position) {
	if(synth->hapticMode != ELASTICITY && synth->hapticMode != OSCILLATION) {
		rawValue = position;
	}
}

//
// VCO
//
void VCOFrequency::update(int position) {
	SynthParam::update(position);

	float octave = (position - 512) / 512.0f;
	float roundOctave = roundf(octave);

	if (abs(octave - roundOctave) < 0.05) {
		octave = roundOctave;
	}
	synth->Octave(octave);
}
void VCOPWM::update(int position) {
	SynthParam::update(position);

	float amount = (position - 512) / 512.0f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->pwm[i].amplitude(amount);
	}
}
void VCOPWMLFOAmount::update(int position) {
	SynthParam::update(position);

	float amount = position / 1023.0f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->pwmMixer[i].gain(1, amount);
	}
}

//
// Mixer
//
void MixerSub::update(int position) {
	SynthParam::update(position);

	float amount = position / 1023.0f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->vcoMixer[i].gain(1, amount);
	}
}
void MixerNoise::update(int position) {
	SynthParam::update(position);

	float amount = position / 1023.0f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->vcoMixer[i].gain(2, amount);
	}
}

//
// Filter
//
void FilterFrequency::update(int position) {
	SynthParam::update(position);

	float freq = powf(sqrtf(20000.0f - 40.0f) * position / 1023.0f, 2.0f) + 40.0f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->filter[i].frequency(freq);
	}
}
void FilterResonance::update(int position) {
	SynthParam::update(position);

	float reso = position / 1023.0f * 1.5f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->filter[i].resonance(reso);
	}
}
void FilterDrive::update(int position) {
	SynthParam::update(position);

	float drive = position / 1023.0f * 1.5f + 0.5f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->filter[i].inputDrive(drive);
	}
}
void FilterEGAmount::update(int position) {
	SynthParam::update(position);

	float amount = (position - 512) / 512.0f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->filterEnvelopeAmount[i].amplitude(amount);
	}
}
void FilterLFOAmount::update(int position) {
	SynthParam::update(position);

	float amount = position / 1023.0f;
	for(uint8_t i = 0; i < VOICES; i++) {
		synth->filterFreqMixer[i].gain(1, amount);
	}
}

//
// Envelope generator
//
void EGAttack::update(int position) {
	SynthParam::update(position);

	float time = powf(2.0f, position / 1023.0f * 8.0f + 5.0f);
	if(time < 35) time = 0.0f;

	for(uint8_t i = 0; i < VOICES; i++) {
		synth->ampEnvelope[i].attack(time);
		synth->filterEnvelope[i].attack(time);
	}
}
void EGDecay::update(int position) {
	SynthParam::update(position);

	float time = powf(2.0f, position / 1023.0f * 8.0f + 5.0f);
	if(time < 35) time = 0.0f;

	for(uint8_t i = 0; i < VOICES; i++) {
		synth->ampEnvelope[i].decay(time);
		synth->filterEnvelope[i].decay(time);
	}
}
void EGSustain::update(int position) {
	SynthParam::update(position);

	float sustain = position / 1023.0f;

	for(uint8_t i = 0; i < VOICES; i++) {
		synth->ampEnvelope[i].sustain(sustain);
		synth->filterEnvelope[i].sustain(sustain);
	}
}
void EGRelease::update(int position) {
	SynthParam::update(position);
	
	float time = powf(2.0f, position / 1023.0f * 8.0f + 5.0f);
	if(time < 35) time = 0.0f;

	for(uint8_t i = 0; i < VOICES; i++) {
		synth->ampEnvelope[i].release(time);
		synth->filterEnvelope[i].release(time);
	}
}

//
// LFO
//
void LFO::update(int position) {
	SynthParam::update(position);

	float freq = powf(2.0f, (position - 512) / 512.0f * 4.0f);
	synth->lfo.frequency(freq);
}