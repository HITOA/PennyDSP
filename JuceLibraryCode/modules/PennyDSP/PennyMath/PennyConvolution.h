#pragma once

#include <PennyDSP/PennyContainers/PennyAudioBufferView.h>

namespace Penny {
	template<typename sT>
	class Convolution {
	public:
		/**
		 * Convolve input and h.
		 * 
		 * \param input : input to be convolved with h.
		 * \param h : inpulse response (h).
		 * \param output : accumulator.
		 */
		static void Convolve(const AudioBufferView<sT>& input, const AudioBufferView<sT>& h, AudioBufferView<sT>& output) {
			jassert(output.GetNumSamples() >= input.GetNumSamples() + h.GetNumSamples() - 1);
			jassert(output.GetNumChannels() == input.GetNumChannels() && h.GetNumChannels() == input.GetNumChannels());
			for (int channel = 0; channel < input.GetNumChannels(); channel++) {
				const sT* inputData = input.GetConstChannelPtr(channel);
				const sT* hdata = h.GetConstChannelPtr(channel);
				sT* outputData = output.GetChannelPtr(channel);
				for (int i = 0; i < input.GetNumSamples(); i++) {
					for (int j = 0; j < h.GetNumSamples(); j++) {
						outputData[i + j] += inputData[i] * hdata[j];
					}
				}
			}
		}
	};
}
