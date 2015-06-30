//
//  RealtimeResamplerFilter.h
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 6/27/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#ifndef __EliasResamplerDemo__RealtimeResamplerFilter__
#define __EliasResamplerDemo__RealtimeResamplerFilter__

#include <stdio.h>
#include "RealtimeResampler.h"

#ifdef blahblah

namespace RealtimeResampler {

  class Filter{
  
    friend class Renderer;
  
    /*!
      Filter a buffer to remove frequencies that will be above 1/2 the sample rate
    */
    
    protected:
      float sampleRate;
      virtual void process(SampleType* inputBuffer, SampleType* outputBuffer, float pitchFactor, size_t numFrames, int hop) = 0;
  };
  
  
  
  //! Biquad_ is an IIR biquad filter which provides a base object on which to build more advanced filters
  class Biquad {
    
  protected:
    
    float coef_[5];
    float inputVec_;
    float outputVec_;
    
  public:
    
    Biquad();
    
    void setIsStereo(bool stereo){
      // resize vectors to match number of channels
      inputVec_.resize(kSynthesisBlockSize + 4, stereo ? 2 : 1, 0);
      outputVec_.resize(kSynthesisBlockSize + 4, stereo ? 2 : 1, 0);
    }
    
    //! Set the coefficients for the filtering operation.
    /*
             b0 + b1*z^-1 + b2*z^-2
     H(z) = ------------------------
             1 + a1*z^-1 + a2*z^-2
     */
    
    void setCoefficients( float *newCoef );
    
    void filter( TonicFrames &inFrames, TonicFrames &outFrames );
  };
  

  
  void Biquad::setCoefficients(TonicFloat *newCoef){
    memcpy(coef_, newCoef, 5 * sizeof(TonicFloat));
  }
  
  void Biquad::filter( TonicFrames &inFrames, TonicFrames &outFrames ){
    
    // initialize vectors
    memcpy(&inputVec_[0], &inputVec_(kSynthesisBlockSize, 0), 2 * inputVec_.channels() * sizeof(TonicFloat));
    memcpy(&inputVec_(2, 0), &inFrames[0], inFrames.size() * sizeof(TonicFloat));
    memcpy(&outputVec_[0], &outputVec_(kSynthesisBlockSize, 0), 2 * outputVec_.channels() * sizeof(TonicFloat));
    
    // perform IIR filter
    
    unsigned int stride = inFrames.channels();
    
    for (unsigned int c=0; c<stride; c++){
      
      TonicFloat* in = &inputVec_(2, c);
      TonicFloat* out = &outputVec_(2, c);
      
      for (unsigned int i=0; i<kSynthesisBlockSize; i++){
        *out = *(in)*coef_[0] + *(in-stride)*coef_[1] + *(in-2*stride)*coef_[2] - *(out-stride)*coef_[3] - *(out-2*stride)*coef_[4];
        in += stride;
        out += stride;
      }
      
    }


    // copy to synthesis block
    memcpy(&outFrames[0], &outputVec_(2,0), kSynthesisBlockSize * stride * sizeof(TonicFloat));
  }
  
  
  

}

#endif

#endif /* defined(__EliasResamplerDemo__RealtimeResamplerFilter__) */


