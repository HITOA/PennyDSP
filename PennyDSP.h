/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 PennyDSP
  vendor:             Penny
  version:            0.0.1
  name:               Penny Digital Signal Processing Module
  description:        All of the base DSP component of any Penny audio plugin.
  website:            https://github.com/HITOA/PennyDSP
  license:            BSD3
  minimumCppStandard: 14

  dependencies:       juce_audio_basics
  OSXFrameworks:      Cocoa Foundation IOKit
  iOSFrameworks:      Foundation
  linuxLibs:          rt dl pthread
  mingwLibs:          uuid wsock32 wininet version ole32 ws2_32 oleaut32 imm32 comdlg32 shlwapi rpcrt4 winmm

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#include "PennyContainers/PennyAudioBufferView.h"

#include "PennyMath/PennyConvolution.h"
#include "PennyMath/PennyFFTConvolution.h"

#include "PennyBasicDSPComponent/PennyBaseDSP.h"
#include "PennyBasicDSPComponent/PennyProcessContext.h"
#include "PennyBasicDSPComponent/PennyDelayLine.h"
#include "PennyBasicDSPComponent/PennyDryWetMixer.h"
