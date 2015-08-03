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
  
    virtual ~Interpolator(){};
  
  protected:
  
    /*!
      Interpolate between input frames. It's up to the caller to determine what the output buffer size will be. The interpolator may look
      as far ahead of the inputBuffer pointer as inputBuffer[-(mBufferFrontPadding*hop)].
      
      The interpolationBuffer variable is a list of positions we need to interpolate. It is the sample length as the outputBuffer number
      of frames. For example, for a four-frame mono input sped up for a factor of 1.5, the contents of the interpolationBuffer would be [0, 1.5, 3]
     
    */
  
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
