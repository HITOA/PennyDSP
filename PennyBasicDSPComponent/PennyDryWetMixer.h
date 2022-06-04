#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "PennyBaseDSP.h"
#include "PennyDelayLine.h"

namespace Penny {
	enum class DryWetMixingType {
		Linear,
		Balanced
	};

	template<typename sT>
	class DryWetMixer : public BaseDSP<sT> {
	public:
		using SampleType = sT;
	public:
		/** Construct a drywet mixer with 1 channel and 44110 max dry latency */
		DryWetMixer() : dryDelayedBuffer{ numChannels, maxDryLatency }, dryBuffer{ numChannels, maxDryLatency } {}
		/** Construct a drywet mixer with specified number of channels and 44110 max dry latency */
		DryWetMixer(int numChannels) : numChannels{ numChannels }, dryDelayedBuffer{ numChannels, maxDryLatency }, dryBuffer{ numChannels, maxDryLatency } {}
		/** Construct a drywet mixer with specified number of channels and max dry latency */
		DryWetMixer(int numChannels, int maxDryLatency) : 
			numChannels{ numChannels }, maxDryLatency{ maxDryLatency }, dryDelayedBuffer{ numChannels, maxDryLatency }, dryBuffer{ numChannels, maxDryLatency } {}

		void SetDryLatency(int dryLatencyInSamples) {
			jassert(dryLatencyInSamples <= maxDryLatency);
			this->dryLatencyInSamples = dryLatencyInSamples;
		}
		int GetDryLatency() {
			return dryLatencyInSamples;
		}

		void SetMixingType(DryWetMixingType mixingType) {
			this->mixingType = mixingType;
			CalculateVolume();
		}

		void SetMixingRatio(float mixingRatio) {
			this->mixingRatio = mixingRatio;
			CalculateVolume();
		}

		void PushDrySamples(const AudioBufferView<sT>& src) {
			dryDelayedBuffer.PushSamples(src);
		}

		void DryWetMixing(AudioBufferView<sT>& wet, int dryLatencyInSamples) {
			AudioBufferView<sT> dryBufferView{ dryBuffer, 0, wet.GetNumSamples() };
			dryDelayedBuffer.PopSamples(dryBufferView, dryLatencyInSamples);
			dryBufferView *= dryVolume;
			wet *= wetVolume;
			wet += dryBufferView;
		}

		void Prepare(int sampleRate, int samplesPerBlock) {
			this->sampleRate = sampleRate;
			this->samplesPerBlock = samplesPerBlock;
			isReady = true;

			dryDelayedBuffer.Prepare(sampleRate, samplesPerBlock);
			dryBuffer.setSize(numChannels, samplesPerBlock);
		}

		void Process(ProcessContext<sT>& ctx) {
			jassert(isReady);
			PushDrySamples(ctx.GetInput());
			DryWetMixing(ctx.GetOutput(), dryLatencyInSamples);
		}

		void Reset() {
			if (!isReady)
				return;

			dryDelayedBuffer.Reset();
		}
	private:
		void CalculateVolume() {
			switch (mixingType) {
			case DryWetMixingType::Balanced: 
				dryVolume = 2.0f * juce::jmin(0.5f, 1.0f - mixingRatio);
				wetVolume = 2.0f * juce::jmin(0.5f, mixingRatio);
				break;
			case DryWetMixingType::Linear:
				dryVolume = 1.0f - mixingRatio;
				wetVolume = mixingRatio;
				break;
			default:
				dryVolume = 0.5f;
				wetVolume = 0.5f;
				break;
			}
		}
	private:
		bool isReady = false;
		int numChannels = 1;
		int maxDryLatency = 44110;
		int dryLatencyInSamples = 0;
		int sampleRate, samplesPerBlock;
		float mixingRatio = 0.5f;
		DryWetMixingType mixingType = DryWetMixingType::Linear;
		float dryVolume{ 0.5f }, wetVolume{ 0.5f };
		DelayLine<sT> dryDelayedBuffer{};
		juce::AudioBuffer<sT> dryBuffer{};
	};
}
