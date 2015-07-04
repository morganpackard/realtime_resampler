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
      mSourceCopy((maxBufferFrames + Renderer::BUFFER_FRONT_PADDING) * numChannels),
      mNumChannels(numChannels)
    {}
  
    void IIRFilter::Biquad::filter(Buffer& buffer, size_t numFrames){
        memcpy(mSourceCopy.mData, buffer.mData, buffer.mNumSamples * mNumChannels * sizeof(SampleType));
    }
    
    LPF12::LPF12(float sampleRate, size_t maxBufferFrames, int numChannels):
      Filter(sampleRate, maxBufferFrames, numChannels),
      mBiquad(maxBufferFrames, numChannels)
    {}
  
    
    void LPF12::process(Buffer& buffer, float pitchFactor, size_t numFrames){
      mBiquad.filter(buffer, numFrames);
    }

}
