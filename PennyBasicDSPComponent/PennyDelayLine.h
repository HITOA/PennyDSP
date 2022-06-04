#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <PennyDSP/PennyBasicDSPComponent/PennyBaseDSP.h>

namespace Penny {
	template<typename sT>
	class DelayLine : public BaseDSP<sT> {
	public:
		using SampleType = sT;
	public:
		/** Construct a delayline with 1 channel and 44110 max delayed samples */
		DelayLine() {}
		/** Construct a delayline with specified number of channels and 44110 max delayed samples */
		DelayLine(int numChannels) : numChannels{ numChannels } {}
		/** Construct a delayline with specified number of channels and max delayed samples */
		DelayLine(int numChannels, int maxDelayInSamples) : numChannels{ numChannels }, maxDelayInSamples{ maxDelayInSamples } {}

		/** Set channels number, will reset the delay line. */
		void SetChannelsNumber(int numChannels) {
			this->numChannels = numChannels;
			Reset();
		}
		int GetChannelsNumber() {
			return numChannels;
		}

		/** Set max delay, will reset the delay line. */
		void SetMaxDelay(int maxDelayInSamples) {
			this->maxDelayInSamples = maxDelayInSamples;
			Reset();
		}
		int GetMaxDelay() {
			return maxDelayInSamples;
		}

		/** Set current delay used for processing */
		void SetDelay(int delayInSamples) {
			jassert(delayInSamples <= maxDelayInSamples);
			this->delayInSamples = delayInSamples;
		}
		int GetDelay() {
			return delayInSamples;
		}

		/** Push samples in the delay line. */
		void PushSamples(const AudioBufferView<sT>& src) {
			jassert(isReady);
			jassert(src.GetNumChannels() >= numChannels);
			jassert(src.GetNumSamples() <= samplesPerBlock);

			for (int i = 0; i < numChannels; i++) {
				const SampleType* data = src.GetConstChannelPtr(i);
				if (delayBufferPosition + src.GetNumSamples() < delayBuffer.getNumSamples()) {
					delayBuffer.copyFrom(i, delayBufferPosition, data, src.GetNumSamples());
				}
				else {
					delayBuffer.copyFrom(i, delayBufferPosition, data, delayBuffer.getNumSamples() - delayBufferPosition);
					delayBuffer.copyFrom(i, 0, data + (delayBuffer.getNumSamples() - delayBufferPosition), 
						src.GetNumSamples() - (delayBuffer.getNumSamples() - delayBufferPosition));
				}
			}

			delayBufferPosition = (delayBufferPosition + src.GetNumSamples()) % delayBuffer.getNumSamples();
		}
		/** Pop samples from the delay line. */
		void PopSamples(AudioBufferView<sT>& dst, int delayInSamples) {
			jassert(isReady);
			jassert(delayInSamples <= maxDelayInSamples);
			jassert(dst.GetNumSamples() <= samplesPerBlock);
			jassert(delayBuffer.getNumChannels() >= dst.GetNumChannels());

			int bufferDelayedPosition = (delayBufferPosition + delayBuffer.getNumSamples() - delayInSamples - dst.GetNumSamples()) % delayBuffer.getNumSamples();

			for (int i = 0; i < dst.GetNumChannels(); i++) {
				const SampleType* data = delayBuffer.getReadPointer(i);
				if (bufferDelayedPosition + dst.GetNumSamples() < delayBuffer.getNumSamples()) {
					dst.CopyFrom(i, 0, data + bufferDelayedPosition, dst.GetNumSamples());
				}
				else {
					dst.CopyFrom(i, 0, data + bufferDelayedPosition, delayBuffer.getNumSamples() - bufferDelayedPosition);
					dst.CopyFrom(i, delayBuffer.getNumSamples() - bufferDelayedPosition, data, 
						dst.GetNumSamples() - (delayBuffer.getNumSamples() - bufferDelayedPosition));
				}
			}
		}

		void Prepare(int sampleRate, int samplesPerBlock) {
			this->sampleRate = sampleRate;
			this->samplesPerBlock = samplesPerBlock;
			Reset();
			isReady = true;
		}

		/** Push sample from input, and pop in output. */
		void Process(ProcessContext<sT>& ctx) {
			jassert(isReady);
			PushSamples(ctx.GetInput());
			PopSamples(ctx.GetOutput(), delayInSamples);
		}

		void Reset() {
			delayBuffer.setSize(numChannels, maxDelayInSamples + samplesPerBlock);
			delayBuffer.clear();
			delayBufferPosition = 0;
		}
	private:
		bool isReady = false;
		int numChannels = 1;
		int maxDelayInSamples = 44110;
		int sampleRate, samplesPerBlock;
		int delayBufferPosition = 0;
		int delayInSamples = 0;
		juce::AudioBuffer<sT> delayBuffer{};
	};
}
