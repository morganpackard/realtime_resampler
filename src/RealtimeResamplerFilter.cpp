//
//  RealtimeResamplerFilter.cpp
//
//  Created by Morgan Packard with encouragement and guidance from Philip Bennefall on 6/27/15.
//

#include "RealtimeResamplerFilter.h"
#include <cmath>
#include <algorithm>

namespace RealtimeResampler {

  #ifndef PI
  const SampleType PI = 3.14159265358979f;
  #endif
  
  Filter::Filter():
    mCutoffToNyquistRatio(0.6)
  {}

  void Filter::init(float sampleRate, size_t maxBufferFrames, int numChannels){
    mSampleRate = sampleRate;
    mMaxBufferFrames = maxBufferFrames;
    mNumChannels = numChannels;
  }
  
  void Filter::setCutoffToNyquistRatio(float ratio){
    mCutoffToNyquistRatio = ratio;
  }
  
  SampleType Filter::pitchFactorToCutoff(SampleType pitchFactor){
      return mCutoffToNyquistRatio * mSampleRate / ( 2 * std::max(1.0f, pitchFactor) ) ;
  }
  
  const float IIRFilter::Q_MIN = 0.7071;
  
  IIRFilter::IIRFilter():mQ(Q_MIN){}

  IIRFilter::Biquad::Biquad(size_t maxBufferFrames, int numChannels):
    mSourceCopy(maxBufferFrames, numChannels, Renderer::BUFFER_FRONT_PADDING),
    mWorkspace(maxBufferFrames, numChannels, Renderer::BUFFER_FRONT_PADDING),
    mNumChannels(numChannels)
  {}
  
  void IIRFilter::bltCoef( SampleType b2, SampleType b1, SampleType b0, SampleType a1, SampleType a0, SampleType fc, SampleType *coef_out)
  {
      SampleType sf = 1.0f/tanf(PI*fc/mSampleRate);
      SampleType sfsq = sf*sf;
      SampleType norm = a0 + a1*sf + sfsq;
      coef_out[0] = (b0 + b1*sf + b2*sfsq)/norm;
      coef_out[1] = 2.0f * (b0 - b2*sfsq)/norm;
      coef_out[2] = (b0 - b1*sf + b2*sfsq)/norm;
      coef_out[3] = 2.0f * (a0 - sfsq)/norm;
      coef_out[4] = (a0 - a1*sf + sfsq)/norm;
  }

  void IIRFilter::Biquad::filter(Buffer* buffer){
  
    size_t bufferLengthBytes = buffer->length * mNumChannels * sizeof(SampleType);
  
    // Keep a copy of the clean source. An IIR filter builds each sample using a combination of delayed
    // source samples, and delayed feedback samples
    
    // copy the last two frames from the last call to the beginning of the source buffer
    memcpy(mSourceCopy.getDataPtr(), mSourceCopy.getDataPtr() + mNumChannels * mSourceCopy.length, Renderer::BUFFER_FRONT_PADDING * mNumChannels * sizeof(SampleType));
    // copy in the incoming data
    memcpy(mSourceCopy.getStartPtr(), buffer->getStartPtr(), bufferLengthBytes);
    // set the mSource buffer length to match the incoming data length
    mSourceCopy.length = buffer->length;
    
    // copy the last two frames of the workspace on to the beginning of the workspace
    memcpy(mWorkspace.getDataPtr(), mWorkspace.getDataPtr() + mNumChannels * mWorkspace.length, Renderer::BUFFER_FRONT_PADDING * mNumChannels * sizeof(SampleType));
    mWorkspace.length = buffer->length;
    
    
    for (int chan = 0; chan < mNumChannels; chan++) {
      SampleType* in = mSourceCopy.getStartPtr() + chan;
      SampleType* out = mWorkspace.getStartPtr() + chan;
    
      for (int i = 0; i < buffer->length ; i++) {
        *out = *(in)*mCoef[0] + *(in-mNumChannels)*mCoef[1] + *(in-2*mNumChannels)*mCoef[2] - *(out-mNumChannels)*mCoef[3] - *(out-2*mNumChannels)*mCoef[4];
        in += mNumChannels;
        out += mNumChannels;
      }
    }
    
    memcpy(buffer->getStartPtr(), mWorkspace.getStartPtr(), bufferLengthBytes);
   
  }

  //////////////////////////////////////////
  /// Two-Pole low-pass filter
  //////////////////////////////////////////
  
  
  LPF12::LPF12():
    mBiquad(0,1)
  {}
  
  void LPF12::init(float sampleRate, size_t maxBufferFrames, int numChannels){
    Filter::init(sampleRate, maxBufferFrames, numChannels);
    mCutoff = mSampleRate / 2;
    mBiquad = Biquad(maxBufferFrames, numChannels);
    
    bltCoef(0, 0, 1, 1.0f/mQ, 1, mCutoff, &mBiquad.mCoef[0]);
    
  }
  
  void LPF12::process(Buffer* buffer, float cutoff){
    if(cutoff != mCutoff){
      mCutoff = cutoff;
      bltCoef(0, 0, 1, 1.0f/mQ, 1, mCutoff, &mBiquad.mCoef[0]);
    }
  
    mBiquad.filter(buffer);
  }
  
  void LPF12::reset(){
    mBiquad.mSourceCopy.clear();
    mBiquad.mWorkspace.clear();
  }

}
