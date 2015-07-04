//
//  RealtimeResamplerInterpolator.h
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
    
  private:
  
    // Interpolators need to look in to the next buffer, and sometimes in to the previous buffer.
    // These values control how many of these "next and previous" frames they are supplied.
    const int mBufferFrontPadding;
    const int mBufferBackPadding;
  
  public:
  
  Interpolator():mBufferFrontPadding(1), mBufferBackPadding(2){}
  
  protected:
  
    /*!
      Interpolate between input frames. It's up to the caller to determine what the output buffer size will be. The interpolator may look
      as far ahead of the inputBuffer pointer as inputBuffer[-(mBufferFrontPadding*hop)]
    */
  
    virtual void process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop) = 0;
    
    int getBufferFrontPadding(){return mBufferFrontPadding;}
    
    int getBufferBackPadding(){return mBufferBackPadding;}
    
  };

  //////////////////////////////////////////
  /// Linear Interpolator
  //////////////////////////////////////////

  class LinearInterpolator : public Interpolator{
  protected:
    void process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop);
  };
  
  
  /*
    According to the elephant paper:
    
    http://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf
    
     4-point Hermite, and Watte tri-linear are the best interpolators for un-oversampled data 
 
    "it shall only be commented that using polynomial interpolators
    with unoversampled input is a choice that can only be made when the quality is not
    that important but speed is essential, the most useful interpolators in that case being
    linear and 4-point Hermite, and Watte tri-linear, which is somewhere between those
    two in both quality and computational complexity."
 
  */
  
  //////////////////////////////////////////
  /// Watte tri-linear Interpolator
  //////////////////////////////////////////
  
  class WatteTrilinearInterpolator : public Interpolator{
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
 
}

#endif /* defined(__EliasResamplerDemo__Interpolator__) */
