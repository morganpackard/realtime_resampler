//
//  RealtimeResamplerFilter.cpp
//
//  Created by Morgan Packard on 6/27/15.
//

#include "RealtimeResamplerFilter.h"
#include <math.h>


namespace RealtimeResampler {

  #ifndef PI
  const SampleType PI = 3.14159265358979f;
  #endif

  void Filter::init(float sampleRate, size_t maxBufferFrames, int numChannels){
    mSampleRate = sampleRate;
    mMaxBufferFrames = maxBufferFrames;
    mNumChannels = numChannels;
  }
  
  
  SampleType Filter::pitchFactorToCutoff(SampleType pitchFactor){
      return 0.6 * mSampleRate / ( 2 * std::max(1.0f, pitchFactor) ) ;
  }
  
  IIRFilter::IIRFilter():mQ(5){}

  IIRFilter::Biquad::Biquad(size_t maxBufferFrames, int numChannels):
    mSourceCopy((maxBufferFrames + 2) * numChannels),
    mNumChannels(numChannels)
  {
    mSourceCopy.start = mSourceCopy.mData + numChannels * Renderer::BUFFER_FRONT_PADDING;
    mSourceCopy.mFrontPadding = numChannels * Renderer::BUFFER_FRONT_PADDING;
  }
  
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

  void IIRFilter::Biquad::filter(Buffer* buffer, size_t numFrames){
  
    // Keep a copy of the clean source. An IIR filter builds each sample using a combination of delayed
    // source samples, and delayed feedback samples
    
    // copy the last two frames from the last call to the beginning of the source buffer
    memcpy(mSourceCopy.mData, mSourceCopy.mData + mNumChannels * mSourceCopy.length, Renderer::BUFFER_FRONT_PADDING * mNumChannels * sizeof(SampleType));
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
  
  
  LPF12::LPF12():
    mBiquad(0,1)
  {}
  
  void LPF12::init(float sampleRate, size_t maxBufferFrames, int numChannels){
    Filter::init(sampleRate, maxBufferFrames, numChannels);
    mCutoff = mSampleRate / 2;
    mBiquad = Biquad(maxBufferFrames, numChannels);
    
    bltCoef(0, 0, 1.0f/mQ, 1.0f/mQ, 1, mCutoff, &mBiquad.mCoef[0]);
    
  }
  
  void LPF12::process(Buffer* buffer, float cutoff, size_t numFrames){
    if(cutoff != mCutoff){
      mCutoff = cutoff;
      bltCoef(0, 0, 1.0f/mQ, 1.0f/mQ, 1, mCutoff, &mBiquad.mCoef[0]);
    }
  
    mBiquad.filter(buffer, numFrames);
  }
  
  
  //////////////////////////////////////////
  /// Four-Pole low-pass filter
  //////////////////////////////////////////
  
  LPF24::LPF24():
    mBiquad1(0,1),
    mBiquad2(0,1)
  {}
  
  void LPF24::setCoefficients(){
            // stage 1
        bltCoef(0, 0, 1.0f/mQ, 0.5412f/mQ, 1, mCutoff, &mBiquad1.mCoef[0]);
        
        // stage 2
        bltCoef(0, 0, 1.0f/mQ, 1.3066f/mQ, 1, mCutoff, &mBiquad2.mCoef[0]);
    
  }
  
  void LPF24::init(float sampleRate, size_t maxBufferFrames, int numChannels){
    Filter::init(sampleRate, maxBufferFrames, numChannels);
    mCutoff = mSampleRate / 2;
    mBiquad1 = Biquad(maxBufferFrames, numChannels);
    mBiquad2 = Biquad(maxBufferFrames, numChannels);
    
    setCoefficients();
    
  }
  
  void LPF24::process(Buffer* buffer, float cutoff, size_t numFrames){
    if(cutoff != mCutoff){
      mCutoff = cutoff;
      setCoefficients();
    }
  
    mBiquad1.filter(buffer, numFrames);
    mBiquad2.filter(buffer, numFrames);
  }
  

}
