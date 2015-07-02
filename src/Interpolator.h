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
    virtual void process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop) = 0;
  };

  //////////////////////////////////////////
  /// Linear Interpolator
  //////////////////////////////////////////

  class LinearInterpolator : public Interpolator{
  protected:
    void process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop);
  };
  
  
      /*
  
    according to the elephant paper:
    
    http://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf
    
     4-point Hermite, and Watte tri-linear are the best interpolators for un-oversampled data 
     
     It is outside the scope of this paper to make guesses of the most profitable oversampling
ratio. Also, it shall only be commented that using polynomial interpolators
with unoversampled input is a choice that can only be made when the quality is not
that important but speed is essential, the most useful interpolators in that case being
linear and 4-point Hermite, and Watte tri-linear, which is somewhere between those
two in both quality and computational complexity.
  
  */
  
  
  
  //////////////////////////////////////////
  /// Watte tri-linear Interpolator
  //////////////////////////////////////////
  
  class WattTrilinearInterpolator : public Interpolator{
  protected:
    void process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop);
  };
  
 
  //////////////////////////////////////////
  /// Cubic Interpolator
  //////////////////////////////////////////

  class CubicInterpolator : public Interpolator{
  protected:
    void process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop);
  };
 
  //////////////////////////////////////////
  /// Hermite Interpolator
  //////////////////////////////////////////

  class HermiteInterpolator : public Interpolator{
  protected:
    void process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop);
  };
  
  
//  
//  //////////////////////////////////////////
//  /// Abstract Interpolator Using Four Frames
//  //////////////////////////////////////////
//  
//  /*!
//    More sophisticated interpolation techniques (Cubic, Hermite) rely on more than two frames for interpolation.
//  */
//
//  class FourFrameInterpolator : public Interpolator{
//  protected:
//      size_t process(SampleType* outputBuffer, size_t outputbufferSize, SampleType* pitchScale);
//      virtual SampleType buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample)=0;
//  };
//  
//  //////////////////////////////////////////
//  /// Cubic Interpolator
//  //////////////////////////////////////////
//
//  class CubicInterpolator : public FourFrameInterpolator{
//  protected:
//      SampleType buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample);
//  };
//  
//  //////////////////////////////////////////
//  /// Hermite Interpolator
//  //////////////////////////////////////////
//
//  class HermiteInterpolator : public FourFrameInterpolator{
//  protected:
//      SampleType buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample);
//  };

}

#endif /* defined(__EliasResamplerDemo__Interpolator__) */
