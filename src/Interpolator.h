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
  /// Abstract Interpolator Using Four Frames
  //////////////////////////////////////////
  
  /*!
    More sophisticated interpolation techniques (Cubic, Hermite) rely on more than two frames for interpolation.
  */

  class FourFrameInterpolator : public Interpolator{
  protected:
      size_t process(SampleType* outputBuffer, size_t outputbufferSize, float* pitchScale);
      virtual SampleType buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample)=0;
  };
  
  //////////////////////////////////////////
  /// Cubic Interpolator
  //////////////////////////////////////////

  class CubicInterpolator : public FourFrameInterpolator{
  protected:
      SampleType buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample);
  };
  
  //////////////////////////////////////////
  /// Hermite Interpolator
  //////////////////////////////////////////

  class HermiteInterpolator : public FourFrameInterpolator{
  protected:
      SampleType buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample);
  };

}

#endif /* defined(__EliasResamplerDemo__Interpolator__) */
