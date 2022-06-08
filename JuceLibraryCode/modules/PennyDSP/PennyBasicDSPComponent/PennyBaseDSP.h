#pragma once

#include "PennyProcessContext.h"

namespace Penny {
	template<typename sT>
	class BaseDSP {
	public:
		virtual void Prepare(int sampleRate, int samplesPerBlock) = 0;
		virtual void Process(ProcessContext<sT>& ctx) = 0;
		virtual void Reset() = 0;
	};
}
