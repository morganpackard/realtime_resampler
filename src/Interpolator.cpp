//
//  Interpolator.cpp
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 6/12/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#include "Interpolator.h"

namespace RealtimeResampler{

  //////////////////////////////////////////
  /// Abstract Interpolator delegate class.
  //////////////////////////////////////////

  void Interpolator::fillNextBuffer(){
    mRenderer->swapBuffersAndFillNext();
  }
  
  
  //////////////////////////////////////////
  /// Linear Interpolator
  //////////////////////////////////////////

  size_t  LinearInterpolator::process(SampleType* outputBuffer,  size_t outputbufferSize, float* pitchScale){

    size_t numFramesRendered = 0;
    
    for (int frame = 0; frame < outputbufferSize; frame++) {
    
      // If we've reached the end of the current buffer, swap buffers and load more data
      if (mSourceBufferReadHead >= mMaxSourceBufferLength) {
        fillNextBuffer();
      }
    
      float interpolationPosition = mSourceBufferReadHead;
      
      // The first frame of the interpolated pair
      int frame1 = (int)interpolationPosition;
      
      // The second frame of the interpolated pair.
      int frame2 = frame1 + 1;
      
      // The buffer from which we'll pull data for frame 2.
      // This may be mCurrentBuffer, or mNextBuffer, depending on whether or not we're on the last frame or not
      AudioBuffer* frame2Buffer;
      if (frame2 < mMaxSourceBufferLength) {
        frame2Buffer = mCurrentSourceBuffer;
      }else{
        frame2 = frame2 % mMaxSourceBufferLength;
        frame2Buffer = mNextSourceBuffer;
      }
    
      // Fill in the data for each channel
      for (int channel = 0; channel < mNumChannels;  channel++) {
        // Get the data for the two frames we're interpolatining between
        float inputframe1 = mCurrentSourceBuffer->data[frame1 * mNumChannels + channel];
        float inputframe2 = frame2Buffer->data[frame2 * mNumChannels + channel];
        
        // interpolate between the two points
        outputBuffer[frame * mNumChannels + channel] = inputframe1 + (inputframe2 - inputframe1) * (interpolationPosition - frame1) ;
      }
      
      numFramesRendered++;
      mSourceBufferReadHead += pitchScale[frame];
    }
    
    return numFramesRendered;

  }


}