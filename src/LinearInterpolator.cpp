//
//  LinearInterpolator.cpp
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 5/7/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#include "LinearInterpolator.h"

namespace RealtimeResampler {

  LinearInterpolator::LinearInterpolator(int numChannels){
    mNumChannels = numChannels;
  }
  
  /*
    Getting a little hung up on here. Obviously, the interpolator is going to need to have some sense of history.
  
  */

  size_t LinearInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, size_t inputbufferSize, size_t outputbufferSize, float* pitchScale){
    float inputFrame = 0;
    for (int frame = 0; frame < outputbufferSize; frame++) {
      for (int channel = 0; channel < mNumChannels;  channel++) {
        int frame0 = (int)inputFrame;
        int frame1 = frame0 + 1;
        float inputFrame0 = inputBuffer[frame0 * mNumChannels + channel];
        float inputFrame1 = inputBuffer[frame1 * mNumChannels + channel];
      
        outputBuffer[frame * mNumChannels + channel] =  inputFrame0 + (inputFrame1 - inputFrame0) * (inputFrame - frame0) ;
      }
      inputFrame += pitchScale[frame];
    }
    return 0;
  }
  

}