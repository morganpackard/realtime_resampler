//
//  RealtimeResamplerFilter.h
//
//  Created by Morgan Packard on 6/27/15.
//

#ifndef __EliasResamplerDemo__RealtimeResamplerFilter__
#define __EliasResamplerDemo__RealtimeResamplerFilter__

#include <stdio.h>
#include "RealtimeResampler.h"


namespace RealtimeResampler {

  //////////////////////////////////////////
  /// Abstract Filter delegate class.
  //////////////////////////////////////////

  class Filter{
  
    friend class Renderer;
    
    public:
    
    protected:
      float                     mSampleRate;
      size_t                    mMaxBufferFrames;
      int                       mNumChannels;
      // TODO -- remove numFrames. That should be determined by the buffer object itself.
      virtual void              process(Buffer* buffer, float cutoff, size_t numFrames) = 0;
      virtual void              init(float sampleRate, size_t maxBufferFrames, int numChannels);
    
      /*!
        Given a pitch factor, where 1 is constant, and 2 is twice the speed, calculate a cutoff frequency that
        sufficiently attenuates frequencies above nyquist (sample rate * 0.5)
      */
      virtual SampleType        pitchFactorToCutoff(SampleType pitchFactor);
    
  };
  
  
  //////////////////////////////////////////
  /// Abstract Infinite Impulse Response filter delegate class.
  //////////////////////////////////////////
  
  class IIRFilter : public virtual Filter{
  
    public:
    
      IIRFilter();
  
    protected:
    
      class Biquad {
      
      public:
        Biquad(size_t maxBufferFrames, int numChannels);
        int                     mNumChannels;
        Buffer                  mSourceCopy;
        float                   mCoef[5];
        void                    filter(Buffer* buffer, size_t numFrames);

      };
    
      void                      bltCoef( SampleType b2, SampleType b1, SampleType b0, SampleType a1, SampleType a0, SampleType fc, SampleType *coef_out);
      SampleType                mQ;
    
  };
  
  //////////////////////////////////////////
  /// Two-Pole low-pass filter
  //////////////////////////////////////////
  
  class LPF12 : public IIRFilter{
  
    public:
    
      LPF12();
    
      void                      init(float sampleRate, size_t maxBufferFrames, int numChannels);
    
    protected:
    
      void                      process(Buffer* buffer, float cutoff, size_t numFrames);
    
      Biquad                    mBiquad;
      SampleType                mCutoff;
  
  };
  
  
  
  //////////////////////////////////////////
  /// Four-Pole low-pass filter
  //////////////////////////////////////////
  
  class LPF24 : public IIRFilter{
  
    public:
    
      LPF24();
    
      void                      init(float sampleRate, size_t maxBufferFrames, int numChannels);
    
    protected:
    
      void                      process(Buffer* buffer, float cutoff, size_t numFrames);
      void                      setCoefficients();
    
      Biquad                    mBiquad1;
      Biquad                    mBiquad2;
      SampleType                mCutoff;
  
  };
  

}

#endif /* defined(__EliasResamplerDemo__RealtimeResamplerFilter__) */


