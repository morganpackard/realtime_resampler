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
      virtual void              process(Buffer* buffer, float pitchFactor, size_t numFrames) = 0;
      virtual void              init(float sampleRate, size_t maxBufferFrames, int numChannels);
    
  };
  
  
  //////////////////////////////////////////
  /// Abstract Infinite Impulse Response filter delegate class.
  //////////////////////////////////////////
  
  class IIRFilter : public virtual Filter{
  
    protected:
    
      class Biquad {
      
      public:
        Biquad(size_t maxBufferFrames, int numChannels);
        int                     mNumChannels;
        Buffer                  mSourceCopy;
        float                   mCoef[5];
        void                    filter(Buffer* buffer, size_t numFrames);

      };
    
  };
  
  //////////////////////////////////////////
  /// Two-Pole low-pass filter
  //////////////////////////////////////////
  
  class LPF12 : public IIRFilter{
  
    public:
    
      LPF12();
    
      void                      init(float sampleRate, size_t maxBufferFrames, int numChannels);
    
    protected:
    
      virtual void              process(Buffer* buffer, float pitchFactor, size_t numFrames);
    
      Biquad                    mBiquad;
  
  };
  

}

#endif /* defined(__EliasResamplerDemo__RealtimeResamplerFilter__) */


