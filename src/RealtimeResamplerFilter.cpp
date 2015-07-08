//
//  RealtimeResamplerFilter.cpp
//
//  Created by Morgan Packard on 6/27/15.
//

#include "RealtimeResamplerFilter.h"

namespace RealtimeResampler {


  void Filter::init(float sampleRate, size_t maxBufferFrames, int numChannels){
    mSampleRate = sampleRate;
    mMaxBufferFrames = maxBufferFrames;
    mNumChannels = numChannels;
  }

  IIRFilter::Biquad::Biquad(size_t maxBufferFrames, int numChannels):
    mSourceCopy((maxBufferFrames + 2) * numChannels),
    mNumChannels(numChannels)
  {
    mSourceCopy.start = mSourceCopy.mData + numChannels * Renderer::BUFFER_FRONT_PADDING;
  }

  void IIRFilter::Biquad::filter(Buffer* buffer, size_t numFrames){
  
    // Keep a copy of the clean source. An IIR filter builds each sample using a combination of delayed
    // source samples, and delayed feedback samples
    
    // copy the last two frames from the last call to the beginning of the source buffer
    memcpy(mSourceCopy.mData, mSourceCopy.mData + mNumChannels * mSourceCopy.length, 2 * mNumChannels * sizeof(SampleType));
    // copy in the incoming data
    memcpy(mSourceCopy.start, buffer->start, buffer->length * mNumChannels * sizeof(SampleType));
    // set the mSource buffer length to match the incoming data length
    mSourceCopy.length = buffer->length;
    
    for (int chan = 0; chan < mNumChannels; chan++) {
      SampleType* in = mSourceCopy.start + chan;
      SampleType* out = buffer->start + chan;
    
      for (int i = 0; i < buffer->length ; i++) {
        *out = *(in)*mCoef[0] + *(in-mNumChannels)*mCoef[1] + *(in-2*mNumChannels)*mCoef[2] - *(out-mNumChannels)*mCoef[3] - *(out-2*mNumChannels)*mCoef[4];
        in += mNumChannels;
        out += mNumChannels;
      }
    }
   
  }

  //////////////////////////////////////////
  /// Two-Pole low-pass filter
  //////////////////////////////////////////
  
  
  LPF12::LPF12(): mBiquad(0,1){}
  
  void LPF12::init(float sampleRate, size_t maxBufferFrames, int numChannels){
    Filter::init(sampleRate, maxBufferFrames, numChannels);
    mBiquad = Biquad(maxBufferFrames, numChannels);
    
    mBiquad.mCoef[0] = 1;
    mBiquad.mCoef[1] = 0;
    mBiquad.mCoef[2] = 0;
    mBiquad.mCoef[3] = 0;
    mBiquad.mCoef[4] = 0;
  }
  
  void LPF12::process(Buffer* buffer, float pitchFactor, size_t numFrames){
    mBiquad.filter(buffer, numFrames);
  }

}
