#pragma once

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "SynthParam.h"
#include "HapticMode.h"

#define LOG_MIDI_NOTES false

#define VOICES 8

#define EFFECTS_MIX 0.0f

class Synth {
	public:
		AudioSynthWaveformDc     pwm[VOICES];
		AudioMixer4              pwmMixer[VOICES];
		AudioSynthWaveformPWM    vco[VOICES];
		AudioSynthWaveformSine   sub[VOICES];
		AudioSynthNoiseWhite     noise[VOICES];
		AudioMixer4              vcoMixer[VOICES];
		AudioSynthWaveformDc     filterEnvelopeAmount[VOICES];
		AudioEffectEnvelope      filterEnvelope[VOICES];
		AudioMixer4              filterFreqMixer[VOICES];
		AudioFilterLadder        filter[VOICES];
		AudioEffectEnvelope      ampEnvelope[VOICES];
		AudioSynthWaveformSine   lfo;

		AudioMixer4              submix1;
		AudioMixer4              submix2;
		AudioMixer4              submix3;

		AudioAmplifier           delayGain;
		AudioMixer4              feedback;
		AudioEffectDelay         delayLeft;
		AudioEffectDelay         delayRight;
		AudioMixer4              delayLeftMix;
		AudioMixer4              delayRightMix;
		AudioEffectFreeverb      reverbLeft;
		AudioEffectFreeverb      reverbRight;
		AudioMixer4              reverbMixLeft;
		AudioMixer4              reverbMixRight;
		AudioOutputI2S           output;

		AudioConnection*         patchCordPPm[VOICES];
		AudioConnection*         patchCordLPm[VOICES];
		AudioConnection*         patchCordPmV[VOICES];
		AudioConnection*         patchCordFeaFe[VOICES];
		AudioConnection*         patchCordFeFfm[VOICES];
		AudioConnection*         patchCordLFfm[VOICES];
		AudioConnection*         patchCordFfmF[VOICES];
		AudioConnection*         patchCordVVm[VOICES];
		AudioConnection*         patchCordSVm[VOICES];
		AudioConnection*         patchCordNVm[VOICES];
		AudioConnection*         patchCordVmF[VOICES];
		AudioConnection*         patchCordFAe[VOICES];
		AudioConnection*         patchCordAeS[VOICES];

		AudioConnection*         patchCord01;
		AudioConnection*         patchCord02;
		AudioConnection*         patchCord03;
		AudioConnection*         patchCord04;
		AudioConnection*         patchCord05;
		AudioConnection*         patchCord06;
		AudioConnection*         patchCord07;
		AudioConnection*         patchCord08;
		AudioConnection*         patchCord09;
		AudioConnection*         patchCord10;
		AudioConnection*         patchCord11;
		AudioConnection*         patchCord12;
		AudioConnection*         patchCord13;
		AudioConnection*         patchCord14;
		AudioConnection*         patchCord15;
		AudioConnection*         patchCord16;
		AudioConnection*         patchCord17;
		AudioConnection*         patchCord18;
		AudioConnection*         patchCord19;

		SynthParam* vcoFrequency;
		SynthParam* vcoPwm;
		SynthParam* vcoPwmLfoAmount;

		SynthParam* mixerSub;
		SynthParam* mixerNoise;

		SynthParam* filterFrequency;
		SynthParam* filterResonance;
		SynthParam* filterDrive;
		SynthParam* filterEGAmount;
		SynthParam* filterLFOAmount;

		SynthParam* egAttack;
		SynthParam* egDecay;
		SynthParam* egSustain;
		SynthParam* egRelease;

		SynthParam* lfoRate;

		SynthParam* selectedParam;
		HAPTIC_MODE hapticMode;

		Synth() {
			for(uint8_t i = 0; i < VOICES; i++) {
				patchCordPPm[i] = new AudioConnection(pwm[i], 0, pwmMixer[i], 0);
				patchCordLPm[i] = new AudioConnection(lfo, 0, pwmMixer[i], 1);
				patchCordPmV[i] = new AudioConnection(pwmMixer[i], vco[i]);
				patchCordFeaFe[i] = new AudioConnection(filterEnvelopeAmount[i], filterEnvelope[i]);
				patchCordFeFfm[i] = new AudioConnection(filterEnvelope[i], 0, filterFreqMixer[i], 0);
				patchCordLFfm[i] = new AudioConnection(lfo, 0, filterFreqMixer[i], 1);
				patchCordFfmF[i] = new AudioConnection(filterFreqMixer[i], 0, filter[i], 1);
				patchCordVVm[i] = new AudioConnection(vco[i], 0, vcoMixer[i], 0);
				patchCordSVm[i] = new AudioConnection(sub[i], 0, vcoMixer[i], 1);
				patchCordNVm[i] = new AudioConnection(noise[i], 0, vcoMixer[i], 2);
				patchCordVmF[i] = new AudioConnection(vcoMixer[i], 0, filter[i], 0);
				patchCordFAe[i] = new AudioConnection(filter[i], ampEnvelope[i]);
				if(i < 4) {
					patchCordAeS[i] = new AudioConnection(ampEnvelope[i], 0, submix1, i);
				} else {
					patchCordAeS[i] = new AudioConnection(ampEnvelope[i], 0, submix2, i-4);
				}
			}
			patchCord01 = new AudioConnection(submix1, 0, submix3, 0);
			patchCord02 = new AudioConnection(submix2, 0, submix3, 1);
			patchCord03 = new AudioConnection(submix3, 0, delayLeftMix, 0);
			patchCord04 = new AudioConnection(submix3, 0, delayRightMix, 0);
			patchCord05 = new AudioConnection(submix3, 0, feedback, 0);
			patchCord06 = new AudioConnection(delayGain, delayRight);
			patchCord07 = new AudioConnection(feedback, delayLeft);
			patchCord08 = new AudioConnection(delayLeft, 0, delayLeftMix, 1);
			patchCord09 = new AudioConnection(delayLeft, 0, delayGain, 0);
			patchCord10 = new AudioConnection(delayRight, 0, delayRightMix, 1);
			patchCord11 = new AudioConnection(delayRight, 0, feedback, 1);
			patchCord12 = new AudioConnection(delayLeftMix, 0, reverbMixLeft, 0);
			patchCord13 = new AudioConnection(delayLeftMix, reverbLeft);
			patchCord14 = new AudioConnection(delayRightMix, reverbRight);
			patchCord15 = new AudioConnection(delayRightMix, 0, reverbMixRight, 0);
			patchCord16 = new AudioConnection(reverbLeft, 0, reverbMixLeft, 1);
			patchCord17 = new AudioConnection(reverbRight, 0, reverbMixRight, 1);
			patchCord18 = new AudioConnection(reverbMixLeft, 0, output, 0);
			patchCord19 = new AudioConnection(reverbMixRight, 0, output, 1);

			AudioMemory(1024);

			vcoFrequency = new VCOFrequency(this);
			vcoPwm = new VCOPWM(this);
			vcoPwmLfoAmount = new VCOPWMLFOAmount(this);

			mixerSub = new MixerSub(this);
			mixerNoise = new MixerNoise(this);

			filterFrequency = new FilterFrequency(this);
			filterResonance = new FilterResonance(this);
			filterDrive = new FilterDrive(this);
			filterEGAmount = new FilterEGAmount(this);
			filterLFOAmount = new FilterLFOAmount(this);

			egAttack = new EGAttack(this);
			egDecay = new EGDecay(this);
			egSustain = new EGSustain(this);
			egRelease = new EGRelease(this);

			lfoRate = new LFO(this);

			selectedParam = vcoFrequency;
			hapticMode = POSITION;

			reset();
		}

		void reset() {
			for(uint8_t i = 0; i < VOICES; i++) {
				vco[i].amplitude(volume);
				pwmMixer[i].gain(0, 1.0f);
				pwmMixer[i].gain(1, 0.0f);
				sub[i].amplitude(volume);
				noise[i].amplitude(1.0f);
				vcoMixer[i].gain(0, 1.0f);
				vcoMixer[i].gain(1, 0.0f);
				vcoMixer[i].gain(2, 0.0f);

				filter[i].frequency(20000);
				filter[i].octaveControl(7);

				ampEnvelope[i].attack(0.0f);
				ampEnvelope[i].hold(0.0f);
				ampEnvelope[i].decay(0.0f);
				ampEnvelope[i].sustain(1.0f);
				ampEnvelope[i].release(0.0f);

				filterEnvelopeAmount[i].amplitude(0.0f);
				filterEnvelope[i].attack(0.0f);
				filterEnvelope[i].hold(0.0f);
				filterEnvelope[i].decay(0.0f);
				filterEnvelope[i].sustain(1.0f);
				filterEnvelope[i].release(0.0f);
				filterFreqMixer[i].gain(0, 1.0f);
				filterFreqMixer[i].gain(1, 0.0f);
			}
			float gain = 1.0f / sqrtf(VOICES);
			for(uint8_t i = 0; i < 4; i++) {
				submix1.gain(i, gain);
				submix2.gain(i, gain);
				submix3.gain(i, 1.0f);
			}

			lfo.amplitude(1.0f);
			lfo.frequency(0.0f);

			delayLeft.delay(0, 1000);
			delayRight.delay(0, 1000);
			feedback.gain(0, 0.5f);
			feedback.gain(1, 0.5f);
			delayGain.gain(0.5f);
			delayLeftMix.gain(0, 1.0f - EFFECTS_MIX);
			delayLeftMix.gain(1, EFFECTS_MIX);
			delayRightMix.gain(0, 1.0f - EFFECTS_MIX);
			delayRightMix.gain(1, EFFECTS_MIX);

			reverbLeft.roomsize(0.75f);
			reverbLeft.damping(1.0f);
			reverbMixLeft.gain(0, 1.0f - EFFECTS_MIX);
			reverbMixLeft.gain(1, EFFECTS_MIX);
			reverbRight.roomsize(0.75f);
			reverbRight.damping(1.0f);
			reverbMixRight.gain(0, 1.0f - EFFECTS_MIX);
			reverbMixRight.gain(1, EFFECTS_MIX);

			vcoFrequency->reset();
			vcoPwm->reset();
			vcoPwmLfoAmount->reset();

			mixerSub->reset();
			mixerNoise->reset();

			filterFrequency->reset();
			filterResonance->reset();
			filterDrive->reset();
			filterEGAmount->reset();
			filterLFOAmount->reset();

			egAttack->reset();
			egDecay->reset();
			egSustain->reset();
			egRelease->reset();

			lfoRate->reset();

			octave = 0.0f;
			updateVCOFrequency();
		}

		void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
			digitalWrite(LED_BUILTIN, HIGH);

			int8_t voiceIndex = -1;
			int8_t highest = -1;
			int8_t backup = 0;
			for(uint8_t i = 0; i < VOICES; i++) {
				if(ampEnvelope[i].isActive()) {
					if(voices[i]) {
						if(notes[i] > notes[backup]) {
							backup = i;
						}
					} else {
						if(highest < 0 || notes[i] > notes[highest]) {
							highest = i;
						}
					}
				} else {
					voiceIndex = i;
					break;
				}
			}

			if(voiceIndex < 0) {
				voiceIndex = highest > -1 ? highest : backup;
			}

			Note(note, voiceIndex);

			ampEnvelope[voiceIndex].noteOn();
			filterEnvelope[voiceIndex].noteOn();
		}

		void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
			digitalWrite(LED_BUILTIN, LOW);

			for(uint8_t i = 0; i < VOICES; i++) {
				if(note == notes[i]) {
					voices[i] = false;
					ampEnvelope[i].noteOff();
					filterEnvelope[i].noteOff();
				}
			}
		}

		void onPitchChange(uint8_t channel, int pitch) {
			PitchBend(pitch);
		}

		//
		// VCO
		//
		void Note(uint8_t note, uint8_t voice) {
			voices[voice] = true;
			notes[voice] = note;
			updateVCOFrequency(voice);
		}
		void PitchBend(int pitch) {
			pitchBend = pitch;
			updateVCOFrequency();
		}
		void Octave(float octave) {
			this->octave = octave;
			updateVCOFrequency();
		}
		void updateVCOFrequency(void) {
			for(uint8_t i = 0; i < VOICES; i++) {
				updateVCOFrequency(i);
			}
		}
		void updateVCOFrequency(uint8_t  voice) {
			// https://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies
			// float freq = 440.0f * pow(2, (note - 69) / 12.0f);

			float freq = 440.0f * powf(2, ((notes[voice] + pitchBend / 8192.0f * 2.0f + octave * 12.0f) - 69) / 12.0f);
			vco[voice].frequency(freq);
			sub[voice].frequency(freq / 2.0f);
		}
	
	private:
		float volume = 1.0f;
		boolean voices[VOICES];
		uint8_t notes[VOICES];
		int pitchBend = 0;
		float octave = 0.0f;
};