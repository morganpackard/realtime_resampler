//
//  LinearInterpolator.h
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 5/7/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#ifndef __EliasResamplerDemo__LinearInterpolator__
#define __EliasResamplerDemo__LinearInterpolator__

#include <stdio.h>
#include "RealtimeResampler.h"

namespace RealtimeResampler {

  class LinearInterpolator : public Interpolator{
    public:
      LinearInterpolator(int numChannels);
      size_t              process(SampleType* inputBuffer, SampleType* outputBuffer, size_t inputbufferSize,  size_t outputbufferSize, float* pitchScale);
    private:
      int                 mNumChannels;
  };

}

#endif /* defined(__EliasResamplerDemo__LinearInterpolator__) */
