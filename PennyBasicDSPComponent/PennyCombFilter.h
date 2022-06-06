#pragma once

#include <PennyDSP/PennyBasicDSPComponent/PennyBaseDSP.h>
#include <PennyDSP/PennyContainers/PennyAudioBufferView.h>
#include <PennyDSP/PennyBasicDSPComponent/PennyDelayLine.h>

namespace Penny {
	template<typename sT>
	class CombFilter : public BaseDSP<sT> {
	public:
		using SampleType = sT;
	public:
		CombFilter() {}
		CombFilter(int numChannels) : numChannels{ numChannels }, delayLine{ numChannels, maxDelayInSamples } {}
		CombFilter(int numChannels, int maxDelayInSamples) : 
			numChannels{ numChannels }, maxDelayInSamples{ maxDelayInSamples }, delayLine{numChannels, maxDelayInSamples} {}

		void SetDelay(int delayInSamples) {
			jassert(delayInSamples <= maxDelayInSamples);
			this->delayInSamples = delayInSamples;
		}
		void SetGain(float feedbackGain) {
			jassert(feedbackGain < 1.0f && feedbackGain > -1.0f);
			this->feedbackGain = feedbackGain;
		}

		void Prepare(int sampleRate, int samplesPerBlock) {
			feedbackBuffer.setSize(numChannels, samplesPerBlock);
			delayLine.Prepare(sampleRate, samplesPerBlock);
			isReady = true;
		};
		void Process(ProcessContext<sT>& ctx) {
			if (!isReady)
				return;

			AudioBufferView<sT> feedbackBufferView{ feedbackBuffer };

			delayLine.PopSamples(feedbackBufferView, delayInSamples);
			
			feedbackBufferView *= feedbackGain;
			feedbackBufferView += ctx.GetInput();

			delayLine.PushSamples(feedbackBufferView);
			delayLine.PopSamples(ctx.GetOutput(), delayInSamples);
		};
		void Reset() {
			if (!isReady)
				return;

			delayLine.Reset();
		};
	private:
		bool isReady = false;
		int numChannels = 1;
		int maxDelayInSamples = 44110;
		int sampleRate, samplesPerBlock;
		int delayInSamples = 0;
		float feedbackGain = 0.5f;
		DelayLine<sT> delayLine{};
		juce::AudioBuffer<sT> feedbackBuffer{};
	};
}
