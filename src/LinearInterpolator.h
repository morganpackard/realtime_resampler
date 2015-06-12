//
//  LinearInterpolator.h
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 6/12/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#ifndef __EliasResamplerDemo__LinearInterpolator__
#define __EliasResamplerDemo__LinearInterpolator__

#include <stdio.h>
#include "RealtimeResampler.h"


namespace RealtimeResampler {

  class LinearInterpolator : public Interpolator{
      size_t process(SampleType* inputBuffer, SampleType* outputBuffer, size_t inputbufferSize,  size_t outputbufferSize, float* pitchScale);
  };

}

#endif /* defined(__EliasResamplerDemo__LinearInterpolator__) */
