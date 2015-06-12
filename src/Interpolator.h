//
//  Interpolator.h
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 6/12/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#ifndef __EliasResamplerDemo__Interpolator__
#define __EliasResamplerDemo__Interpolator__

#include <stdio.h>
#include "RealtimeResampler.h"

namespace RealtimeResampler {


  //////////////////////////////////////////
  /// Abstract Interpolator delegate class.
  //////////////////////////////////////////

  class Interpolator{
  
    friend class Renderer;
  
  public:
  
    /*!
      Interpolate between input frames. It's up to the caller to determine what the output buffer size will be.
    */
    
  
  protected:
    virtual size_t                process(SampleType* outputBuffer,  size_t outputbufferSize, float* pitchScale) = 0;
    void                          fillNextBuffer(); // Access a protected function of Renderer from a subclass.
    Renderer*                     mRenderer;
    AudioBuffer*                  mCurrentSourceBuffer;
    AudioBuffer*                  mNextSourceBuffer;
    size_t                        mMaxSourceBufferLength;
    float                         mSourceBufferReadHead;
    int                           mNumChannels;
  
  };

  //////////////////////////////////////////
  /// Linear Interpolator
  //////////////////////////////////////////

  class LinearInterpolator : public Interpolator{
  protected:
      size_t process(SampleType* outputBuffer, size_t outputbufferSize, float* pitchScale);
  };
  
  //////////////////////////////////////////
  /// Cubic Interpolator
  //////////////////////////////////////////

  class CubicInterpolator : public Interpolator{
  protected:
      size_t process(SampleType* outputBuffer, size_t outputbufferSize, float* pitchScale);
  };

}

#endif /* defined(__EliasResamplerDemo__Interpolator__) */
