#pragma once

#include <PennyDSP/PennyBasicDSPComponent/PennyBaseDSP.h>

namespace Penny {
    template<typename SampleType>
    class DelayLine {
    public:
        DelayLine() : audioBuffer{ 0, 0 } {}

        explicit DelayLine(int numChannels) : audioBuffer{ numChannels, 0 }, currentPosition{ 0 } {
            audioBuffer.clear();
        };

        explicit DelayLine(int numChannels, int maxDelayInSamples) : audioBuffer{ numChannels, maxDelayInSamples }, currentPosition{ 0 }  {
            audioBuffer.clear();
        }

        /**
         * Push samples in the dellay line audio buffer. 
         * samples audio buffer must have the same number of channels as the delay audio buffer.
         * 
         * \param samples : Audio buffer containing the samples to be pushed.
         * \param offset : Offset of the sample in the samples audio buffer to be added.
         * \param length : Number of samples (in the samples audio buffer) to be pushed. (must be less than the delay audio buffer size)
         */
        void PushSamples(const juce::AudioBuffer<SampleType>& samples, int offset = 0, int length = 1) {
            jassert(samples.getNumChannels() == audioBuffer.getNumChannels() && 
                (offset + length) <= samples.getNumSamples() && length <= audioBuffer.getNumSamples());
            
            for (int i = 0; i < audioBuffer.getNumChannels(); i++) {
                if (currentPosition + length < audioBuffer.getNumSamples()) {
                    audioBuffer.copyFrom(i, currentPosition, samples, i, offset, length);
                }
                else {
                    audioBuffer.copyFrom(i, currentPosition, samples, i, offset, audioBuffer.getNumSamples() - currentPosition);
                    audioBuffer.copyFrom(i, 0, samples, i, offset + audioBuffer.getNumSamples() - currentPosition, 
                        length - audioBuffer.getNumSamples() + currentPosition);
                }
            }

            currentPosition = (currentPosition + length) % audioBuffer.getNumSamples();
        }

        /**
         * Pop samples in the provided buffer.
         * Provided buffer must have the same number of channels as the delay audio buffer.
         * 
         * \param delayInSamples : delay in samples, 0 is no delay. max is GetMaxDelayInSamples
         * \param buffer : Buffer wich will receive the samples
         * \param offset : Buffer offset
         * \param length : Samples number to be popped
         */
        void PopSamples(int delayInSamples, juce::AudioBuffer<SampleType> buffer, int offset = 0, int length = 1) {
            jassert(delayInSamples >= 0 && delayInSamples <= audioBuffer.getNumSamples() &&
                (offset + length) <= buffer.getNumSamples() && length <= audioBuffer.getNumSamples() &&
                audioBuffer.getNumChannels() == buffer.getNumChannels());

            int startPosition = (currentPosition + audioBuffer.getNumSamples() - delayInSamples) % audioBuffer.getNumSamples();

            for (int i = 0; i < audioBuffer.getNumChannels(); i++) {
                if (startPosition + length < audioBuffer.getNumSamples()) {
                    buffer.copyFrom(i, offset, audioBuffer, i, startPosition, length);
                }
                else {
                    buffer.copyFrom(i, offset, audioBuffer, i, startPosition, audioBuffer.getNumSamples() - startPosition);
                    buffer.copyFrom(i, offset + audioBuffer.getNumSamples() - startPosition, audioBuffer,
                        i, 0, length - audioBuffer.getNumSamples() + startPosition);
                }
            }
        }

        /** Get one sample */
        SampleType GetSample(int channel, int delayInSamples) {
            jassert(channel >= 0 && channel < audioBuffer.getNumChannels() &&
                delayInSamples >= 0 && delayInSamples <= audioBuffer.getNumSamples());
            return audioBuffer.getSample(channel, (currentPosition + audioBuffer.getNumSamples() - delayInSamples) % audioBuffer.getNumSamples());
        }

        /** Get max delay of the delay line. */
        int GetMaxDelay() {
            return audioBuffer.getNumSamples();
        }

        /** Set max delay, will rebuild the audio buffer. */
        void SetMaxDelay(int maxDelayInSamples) {
            audioBuffer.setSize(audioBuffer.numChannels, maxDelayInSamples);
            audioBuffer.clear();
        }
    private:
        int currentPosition;
        juce::AudioBuffer<SampleType> audioBuffer;
    };
}
