#pragma once

#include <PennyDSP/PennyBasicDSPComponent/PennyBaseDSP.h>
#include <PennyDSP/PennyBasicDSPComponent/PennyCombFilter.h>
#include <PennyDSP/PennyBasicDSPComponent/PennyProcessContext.h>

namespace Penny {
	template<typename sT>
	class AllPassFilter : public BaseDSP<sT> {
	public:
		using SampleType = sT;
	public:
		AllPassFilter() {}
		AllPassFilter(int numChannels) : numChannels{ numChannels }, combFilter{ numChannels, maxDelayInSamples } {}
		AllPassFilter(int numChannels, int maxDelayInSamples) :
			numChannels{ numChannels }, maxDelayInSamples{ maxDelayInSamples }, combFilter{ numChannels, maxDelayInSamples } {}

		void SetDelay(int delayInSamples) {
			jassert(delayInSamples <= maxDelayInSamples);
			this->delayInSamples = delayInSamples;
			combFilter.SetDelay(delayInSamples);
		}
		void SetGain(float feedbackGain) {
			jassert(feedbackGain <= 1.0f && feedbackGain >= -1.0f);
			this->feedbackGain = feedbackGain;
			combFilter.SetGain(feedbackGain);
		}

		void Prepare(int sampleRate, int samplesPerBlock) {
			audioBuffer.setSize(numChannels, samplesPerBlock);
			audioBuffer.clear();
			combFilter.Prepare(sampleRate, samplesPerBlock);
			isReady = true;
		}
		void Process(ProcessContext<sT>& ctx) {
			if (!isReady)
				return;

			AudioBufferView<sT> audioBufferView{ audioBuffer };
			AudioBufferView<sT> inputView = ctx.GetInput();
			AudioBufferView<sT> outputView = ctx.GetOutput();

			for (int i = 0; i < inputView.GetNumChannels(); i++) {
				audioBufferView.CopyFrom(i, 0, inputView, i, 0, audioBufferView.GetNumSamples());
			}

			ProcessContext<sT> combCtx{ inputView };
			combFilter.Process(combCtx);

			inputView *= 1 - feedbackGain * feedbackGain;

			for (int i = 0; i < inputView.GetNumChannels(); i++) {
				outputView.CopyFrom(i, 0, inputView, i, 0, outputView.GetNumSamples());
			}

			outputView *= -feedbackGain;
			outputView += audioBufferView;
		}
		void Reset() {
			if (!isReady)
				return;

			combFilter.Reset();
			audioBuffer.clear();
		}
	private:
		bool isReady = false;
		int numChannels = 1;
		int maxDelayInSamples = 44110;
		int delayInSamples = 0;
		float feedbackGain = 0.5f;
		CombFilter<sT> combFilter{};
		juce::AudioBuffer<sT> audioBuffer{};
	};
}
