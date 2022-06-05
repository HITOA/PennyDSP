#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace Penny {
	template<typename sT>
	struct AudioBufferView_Impl {
	public:
		static void Add(sT* dst, sT* buffer, int size) {
			for (int i = 0; i < dsize; i++)
				dst[i] += buffer[i];
		}
		static void Sub(sT* dst, sT* buffer, int size) {
			for (int i = 0; i < dsize; i++)
				dst[i] -= buffer[i];
		}
		static void Mul(sT* dst, sT* buffer, int size) {
			for (int i = 0; i < dsize; i++)
				dst[i] *= buffer[i];
		}
		static void Div(sT* dst, sT* buffer, int size) {
			for (int i = 0; i < dsize; i++)
				dst[i] /= buffer[i];
		}
	};

	template<>
	struct AudioBufferView_Impl<float> {
	public:
		static void Add(float* dst, float* buffer, int size) {
			__m256* vDst = (__m256*)dst;
			__m256* vBuffer = (__m256*)buffer;
			int qsize = (size / 8);
			for (int i = 0; i < qsize; i++) {
				vDst[i] = _mm256_add_ps(vDst[i], vBuffer[i]);
			}
			if (qsize * 8 != size) {
				for (int i = qsize * 8; i < size; i++) {
					dst[i] += buffer[i];
				}
			}
		}
		static void Sub(float* dst, float* buffer, int size) {
			__m256* vDst = (__m256*)dst;
			__m256* vBuffer = (__m256*)buffer;
			int qsize = (size / 8);
			for (int i = 0; i < qsize; i++) {
				vDst[i] = _mm256_sub_ps(vDst[i], vBuffer[i]);
			}
			if (qsize * 8 != size) {
				for (int i = qsize * 8; i < size; i++) {
					dst[i] -= buffer[i];
				}
			}
		}
		static void Mul(float* dst, float* buffer, int size) {
			__m256* vDst = (__m256*)dst;
			__m256* vBuffer = (__m256*)buffer;
			int qsize = (size / 8);
			for (int i = 0; i < qsize; i++) {
				vDst[i] = _mm256_mul_ps(vDst[i], vBuffer[i]);
			}
			if (qsize * 8 != size) {
				for (int i = qsize * 8; i < size; i++) {
					dst[i] *= buffer[i];
				}
			}
		}
		static void Div(float* dst, float* buffer, int size) {
			__m256* vDst = (__m256*)dst;
			__m256* vBuffer = (__m256*)buffer;
			int qsize = (size / 8);
			for (int i = 0; i < qsize; i++) {
				vDst[i] = _mm256_div_ps(vDst[i], vBuffer[i]);
			}
			if (qsize * 8 != size) {
				for (int i = qsize * 8; i < size; i++) {
					dst[i] /= buffer[i];
				}
			}
		}
	};

	template<typename sT>
	class AudioBufferView {
	public:
		using SampleType = sT;
	public:
		AudioBufferView() = delete;
		/** Create an audio buffer view, viewing all the data containing by the audio buffer. */
		explicit AudioBufferView(juce::AudioBuffer<SampleType>& buffer) noexcept {
			numChannels = buffer.getNumChannels();
			size = buffer.getNumSamples();
			channels = buffer.getArrayOfWritePointers();
			offset = 0;
		}
		/** Create an audio buffer view from a ptr to channels. */
		explicit AudioBufferView(SampleType** channels, int numChannels, int size) noexcept {
			this->channels = channels;
			this->numChannels = numChannels;
			this->size = size;
			offset = 0;
		}
		/** Create an audio buffer view, viewing a part of the data containing by the audio buffer. */
		explicit AudioBufferView(juce::AudioBuffer<SampleType>& buffer, int offset, int length) {
			jassert(buffer.getNumSamples() >= offset + length);
			numChannels = buffer.getNumChannels();
			size = length;
			channels = buffer.getArrayOfWritePointers();
			this->offset = offset;
		}
		/** Create an audio buffer view from a ptr to channels. */
		explicit AudioBufferView(SampleType** channels, int numChannels, int offset, int length) noexcept {
			this->channels = channels;
			this->numChannels = numChannels;
			this->size = length;
			this->offset = offset;
		}

		/** Get channels number. */
		inline int GetNumChannels() const noexcept { return numChannels; }
		/** Get samples number. */
		inline int GetNumSamples() const noexcept { return size; }

		/** Get raw ptr to the channel. */
		inline SampleType* GetChannelPtr(int channel) {
			jassert(channel < numChannels && channel > -1);
			return channels[channel] + offset;
		}
		/** Get raw const ptr to the channel. */
		inline const SampleType* GetConstChannelPtr(int channel) const {
			jassert(channel < numChannels&& channel > -1);
			return channels[channel] + offset;
		}
		/** Get new audio buffer view, viewing a single channel. */
		inline AudioBufferView GetChannelView(int channel) {
			jassert(channel < numChannels && channel > -1);
			return AudioBufferView{ channels + channel, 1, offset, size };
		}
		/** Get new audio buffer view, viewing multiple channels. */
		inline AudioBufferView GetChannelsView(int startChannel, int numChannels) {
			jassert(startChannel < this->numChannels && startChannel > -1);
			jassert(numChannels > -1 && startChannel + numChannels < this->numChannels);
			return AudioBufferView{ channels + startChannel, numChannels, offset, size };
		}

		/** Get sample from specified channel. */
		inline SampleType GetSample(int channel, int idx) const {
			jassert(channel < numChannels&& channel > -1);
			jassert(idx > 0 && idx < size);
			return channels[channel][offset + idx];
		}
		/** Set sample in specified channel. */
		inline void SetSample(int channel, int idx, SampleType sample) {
			jassert(channel < numChannels&& channel > -1);
			jassert(idx > 0 && idx < size);
			channels[channel][offset + idx] = sample;
		}
		/** Copy sample in src audio buffer view specified channel to this audio buffer view specified channel. */
		void CopyFrom(int channel, int startOffset, const AudioBufferView<SampleType>& src, int srcChannel, int srcStartOffset, int length) {
			jassert(channel < numChannels && channel > -1);
			jassert(startOffset > -1 && startOffset + length <= size);
			jassert(srcChannel < src.GetNumChannels() && srcChannel > -1);
			jassert(srcStartOffset > -1 && srcStartOffset + length < src.GetNumChannels());

			const SampleType* channelData = src.GetConstChannelPtr(srcChannel);
			memcpy(channels[channel] + startOffset, channelData + srcStartOffset, sizeof(SampleType) * length);
		}
		/** Copy sample in src ptr to this audio buffer view specified channel. */
		void CopyFrom(int channel, int startOffset, const SampleType* src, int length) {
			jassert(channel < numChannels&& channel > -1);
			jassert(startOffset > -1 && startOffset + length <= size);
			
			memcpy(channels[channel] + startOffset, src, sizeof(SampleType) * length);
		}

		void operator+=(SampleType value) {
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				for (int j = 0; j < size; j++) {
					data[offset + j] += value;
				}
			}
		}
		void operator+=(const AudioBufferView<SampleType>& src) {
			jassert(src.GetNumChannels() >= numChannels);
			jassert(src.GetNumSamples() >= size);
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				const SampleType* __restrict srcData = src.GetConstChannelPtr(i);
				/*for (int j = 0; j < size; j++) {
					data[offset + j] += srcData[src.offset + j];
				}*/
				AudioBufferView_Impl<sT>::Add(data, const_cast<SampleType*>(srcData + src.offset), size);
			}
		}
		void operator-=(SampleType value) {
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				for (int j = 0; j < size; j++) {
					data[offset + j] -= value;
				}
			}
		}
		void operator-=(const AudioBufferView<SampleType>& src) {
			jassert(src.GetNumChannels() >= numChannels);
			jassert(src.GetNumSamples() >= size);
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				const SampleType* __restrict srcData = src.GetConstChannelPtr(i);
				/*for (int j = 0; j < size; j++) {
					data[offset + j] += srcData[src.offset + j];
				}*/
				AudioBufferView_Impl<sT>::Sub(data, const_cast<SampleType*>(srcData + src.offset), size);
			}
		}
		void operator*=(SampleType value) {
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				for (int j = 0; j < size; j++) {
					data[offset + j] *= value;
				}
			}
		}
		void operator*=(const AudioBufferView<SampleType>& src) {
			jassert(src.GetNumChannels() >= numChannels);
			jassert(src.GetNumSamples() >= size);
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				const SampleType* __restrict srcData = src.GetConstChannelPtr(i);
				/*for (int j = 0; j < size; j++) {
					data[offset + j] += srcData[src.offset + j];
				}*/
				AudioBufferView_Impl<sT>::Mul(data, const_cast<SampleType*>(srcData + src.offset), size);
			}
		}
		void operator/=(SampleType value) {
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				for (int j = 0; j < size; j++) {
					data[offset + j] /= value;
				}
			}
		}
		void operator/=(const AudioBufferView<SampleType>& src) {
			jassert(src.GetNumChannels() >= numChannels);
			jassert(src.GetNumSamples() >= size);
			for (int i = 0; i < numChannels; i++) {
				SampleType* __restrict data = channels[i];
				const SampleType* __restrict srcData = src.GetConstChannelPtr(i);
				/*for (int j = 0; j < size; j++) {
					data[offset + j] += srcData[src.offset + j];
				}*/
				AudioBufferView_Impl<sT>::Div(data, const_cast<SampleType*>(srcData + src.offset), size);
			}
		}
	private:
		int numChannels, size, offset;
		SampleType** channels;
	};
}
