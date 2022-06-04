#pragma once

#include "PennyDSP/PennyContainers/PennyAudioBufferView.h"

namespace Penny {
	template<typename sT>
	struct ProcessContext {
	public:
		using SampleType = sT;
	public:
		ProcessContext() = delete;
		ProcessContext(AudioBufferView<sT>& inout) noexcept : input{ inout }, output{ inout }, isInout{ true } {}
		ProcessContext(const AudioBufferView<sT>& in, AudioBufferView<sT>& out) noexcept : input{ in }, output{ out }, isInout{ false } {}
	public:
		const AudioBufferView<sT>& GetInput() const noexcept { return input; }
		AudioBufferView<sT>& GetOutput() noexcept { return output; }
		bool IsInout() const noexcept { return isInout; }
	private:
		const AudioBufferView<sT>& input;
		AudioBufferView<sT>& output;
		bool isInout;
	};
}
