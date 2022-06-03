#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "PennyDelayLine.h"

namespace Penny {
	enum class DryWetMixingType {
		Linear,
		Balanced
	};

	template<typename SampleType>
	class DryWetMixer {
	public:
		/**
		 * Construct a default DryWetMixer, with 2 channels and 44110 max latency.
		 */
		DryWetMixer() : dryDelayLine{ 2, 44110 } { 
			SetDryWetVolume(); 
		}
		/**
		 * Construct a DryWetMixer, with specified number of channels.
		 * Max wet latency is defined to 44110.
		 */
		DryWetMixer(int numChannels) : dryDelayLine{ numChannels, 44110 } {
			SetDryWetVolume(); 
		}
		/**
		 * Construct a DryWetMixer, with specified number of channel and max wet latency.
		 */
		DryWetMixer(int numChannels, int maxWetLatencyInSamples) : 
			dryDelayLine{ numChannels, maxWetLatencyInSamples } {
			SetDryWetVolume();
		}

		/**
		 * Set the current latency, used to compensate with the dry signal.
		 * 
		 * \param wetLatencyInSamples : new latency
		 */
		void SetWetLatency(int wetLatencyInSamples) {
			jassert(wetLatencyInSamples > dryDelayLine.GetMaxDelay())
			this->wetLatencyInSamples = wetLatencyInSamples;
		}

		void SetMixingValue(float mixValue) {
			this->mixValue = mixValue;
			SetDryWetVolume();
		}

		void SetMixingType(DryWetMixingType mixingType) {
			this->mixingType = mixingType;
			SetDryWetVolume();
		}

		/** Push dry samples */
		void PushDrySamples(const juce::AudioBuffer<SampleType>& dryAudioBuffer, int offset, int length) {
			dryDelayLine.PushSamples(dryAudioBuffer, offset, length);
		}

		/** Mix dry and wet signal back in the wet audio buffer. will compensate for any wet latency set with SetWetLantecy. */
		void MixDryWet(juce::AudioBuffer<SampleType>& wetAudioBuffer) {
			for (int i = 0; i < wetAudioBuffer.getNumChannels(); i++) {
				SampleType* channelData = wetAudioBuffer.getWritePointer(i);
				for (int samplesIdx = 0; samplesIdx < wetAudioBuffer.getNumSamples(); samplesIdx++) {
					SampleType drySample = dryDelayLine.GetSample(i, wetAudioBuffer.getNumChannels() + wetLatencyInSamples);
					channelData[samplesIdx] = channelData[samplesIdx] * wetVolume.getCurrentValue() + drySample * dryVolume.getCurrentValue();
				}
			}
		}
	private:
		void SetDryWetVolume() {
			switch (mixingType) {
			case DryWetMixingType::Balanced: {
				dryVolume.setTargetValue(static_cast<SampleType>(2.0) * juce::jmin(static_cast<SampleType>(0.5), static_cast<SampleType>(1.0) - mixValue));
				wetVolume.setTargetValue(static_cast<SampleType>(2.0) * juce::jmin(static_cast<SampleType>(0.5), mixValue));
				break;
			}
			case DryWetMixingType::Linear: {
				dryVolume.setTargetValue(static_cast<SampleType>(1.0) - mixValue);
				wetVolume.setTargetValue(mixValue);
				break;
			}
			default: {
				dryVolume.setTargetValue(juce::jmin(static_cast<SampleType>(0.5), static_cast<SampleType>(1.0) - mixValue));
				wetVolume.setTargetValue(juce::jmin(static_cast<SampleType>(0.5), mixValue));
				break;
			}
			}
		}
	private:
		juce::SmoothedValue<SampleType> dryVolume, wetVolume;
		float mixValue = 0.5f;
		DryWetMixingType mixingType = DryWetMixingType::Balanced;
		int wetLatencyInSamples = 0;
		DelayLine<SampleType> dryDelayLine{};
	};
}
