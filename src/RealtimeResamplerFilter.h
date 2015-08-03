//
//  RealtimeResampler.h
//  Resampler
//
//  Created by Morgan Packard with encouragement and guidance from Philip Bennefall on 2/22/15.
//
//  Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
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
    
      Filter();
      virtual ~Filter(){};
    
      /*!
        Different filters have steeper and shallower rolloffs. Depending in the filter, it may be necessary to set the cutoff
        significantly below the nyquist frequency in order to achieve an acceptable degree of anti-aliasing. One should feel
        free to set this as often as desired. For example, the value could be set to 1 for pitch factors close to one, but 0.6 for high pitch 
        factors, and interpolated in between.
      */
    
      void                      setCutoffToNyquistRatio(float);
    
    protected:
      virtual void              process(Buffer* buffer, float cutoff) = 0;
      virtual void              init(float sampleRate, size_t maxBufferFrames, int numChannels);
    
      /*!
        Reset any state saved by the filter. In particular, zero out any delay lines.
      */
    
      virtual void              reset(){};
      
      float                     mSampleRate;
      size_t                    mMaxBufferFrames;
      int                       mNumChannels;
    
      /*!
        Given a pitch factor, where 1 is constant, and 2 is twice the speed, calculate a cutoff frequency that
        sufficiently attenuates frequencies above nyquist (sample rate * 0.5)
      */
    
      virtual SampleType        pitchFactorToCutoff(SampleType pitchFactor);
    

    
      float                     mCutoffToNyquistRatio;
    
  };
  
  
  //////////////////////////////////////////
  /// Abstract Infinite Impulse Response filter delegate class.
  //////////////////////////////////////////
  
  class IIRFilter : public virtual Filter{
  
    public:
    
      IIRFilter();
  
      static const float        Q_MIN;
  
    protected:
    
      class Biquad {
      
      public:
        Biquad(size_t maxBufferFrames, int numChannels);
        int                     mNumChannels;
        Buffer                  mSourceCopy;
        Buffer                  mWorkspace;
        float                   mCoef[5];
        void                    filter(Buffer* buffer);

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
    
      void                      process(Buffer* buffer, float cutoff);
      virtual void              reset();
    
      Biquad                    mBiquad;
      SampleType                mCutoff;
  
  };
 
  

}

#endif /* defined(__EliasResamplerDemo__RealtimeResamplerFilter__) */


