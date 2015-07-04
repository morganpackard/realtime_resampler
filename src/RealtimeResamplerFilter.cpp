//
//  RealtimeResamplerFilter.cpp
//
//  Created by Morgan Packard on 6/27/15.
//

#include "RealtimeResamplerFilter.h"

namespace RealtimeResampler {

  
    Filter::Filter(float sampleRate, size_t maxBufferFrames, int numChannels):
      mSampleRate(sampleRate),
      mMaxBufferFrames(maxBufferFrames),
      mNumChannels(numChannels),
      mBufferFrontPadding(2)
    {}
  
    IIRFilter::Biquad::Biquad(size_t maxBufferFrames, int numChannels):
      mSourceCopy(maxBufferFrames * numChannels),
      mNumChannels(numChannels)
    {}
  
    void IIRFilter::Biquad::filter(SampleType* buffer, size_t numFrames){
      
    }
    
    LPF12::LPF12(float sampleRate, size_t maxBufferFrames, int numChannels):
      Filter(sampleRate, maxBufferFrames, numChannels),
      mBiquad(maxBufferFrames, numChannels)
    {}
  
    
    void LPF12::process(SampleType* buffer, float pitchFactor, size_t numFrames){
      mBiquad.filter(buffer, numFrames);
    }

}
