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


  
  
  //////////////////////////////////////////
  /// Linear Interpolator
  //////////////////////////////////////////
  
  
  void LinearInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop){
    
    for (int i = 0; i < numFrames; i++) {
    
      int integerPartOfInterpolationBuffer = (int)interpolationBuffer[i];
      SampleType interpolationCoefficient = interpolationBuffer[i] - integerPartOfInterpolationBuffer;
    
      // The first frame of the interpolated pair
      int sampleIndex1 = integerPartOfInterpolationBuffer * hop;
      
      // The second frame of the interpolated pair.
      int sampleIndex2 = sampleIndex1 + hop;
      
      SampleType sample1 = inputBuffer[sampleIndex1];
      SampleType sample2 = inputBuffer[sampleIndex2];
      
      outputBuffer[i * hop] = sample1 + (sample2 - sample1) * interpolationCoefficient ;
      
    }
  }
  


  void WattTrilinearInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop){
    
    SampleType frame0Sample, frame1Sample, frame2Sample, frame3Sample;
    
    for (int i = 0; i < numFrames; i++) {
    
      int interpPosition = (int)interpolationBuffer[i];
      SampleType t = interpolationBuffer[i] - interpPosition;
    
      frame0Sample = inputBuffer[ (interpPosition - 1) * hop];
      frame1Sample = inputBuffer[ (interpPosition)  * hop];
      frame2Sample = inputBuffer[ (interpPosition + 1) * hop];
      frame3Sample = inputBuffer[ (interpPosition + 2) * hop];
  
      // 4-point, 2nd-order Watte tri-linear (x-form)
      float ym1py2 = frame0Sample + frame3Sample;
      float c0 = frame1Sample;
      float c1 = 3/2.0*frame2Sample - 1/2.0*(frame1Sample+ym1py2);
      float c2 = 1/2.0*(ym1py2-frame1Sample-frame2Sample);
      outputBuffer[i * hop] = (c2*t+c1)*t+c0;

    }
  }
  
  //////////////////////////////////////////
  /// Cubic Interpolator
  //////////////////////////////////////////
  
  

  
  void CubicInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop){
    
    SampleType frame0Sample, frame1Sample, frame2Sample, frame3Sample;
    
    
    for (int i = 0; i < numFrames; i++) {
    
      int interpPosition = (int)interpolationBuffer[i];
      SampleType t = interpolationBuffer[i] - interpPosition;
    
      frame0Sample = inputBuffer[ (interpPosition - 1) * hop];
      frame1Sample = inputBuffer[ (interpPosition)  * hop];
      frame2Sample = inputBuffer[ (interpPosition + 1) * hop];
      frame3Sample = inputBuffer[ (interpPosition + 2) * hop];
    
      SampleType a0, a1, a2, a3;
    
      a0 = frame3Sample - frame2Sample - frame0Sample + frame1Sample;
      a1 = frame0Sample - frame1Sample - a0;
      a2 = frame2Sample - frame0Sample;
      a3 = frame1Sample;
      
      outputBuffer[i * hop] = (a0 * (t * t * t)) + (a1 * (t * t)) + (a2 * t) + (a3);
    }
  }
  
  void HermiteInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop){
  
    SampleType frame0Sample, frame1Sample, frame2Sample, frame3Sample;
    
    for (int i = 0; i < numFrames; i++) {
    
      int interpPosition = (int)interpolationBuffer[i];
      SampleType t = interpolationBuffer[i] - interpPosition;
      
      frame0Sample = inputBuffer[ (interpPosition - 1) * hop];
      frame1Sample = inputBuffer[ (interpPosition)  * hop];
      frame2Sample = inputBuffer[ (interpPosition + 1) * hop];
      frame3Sample = inputBuffer[ (interpPosition + 2) * hop];
    
      float c0 = frame1Sample;
      float c1 = .5F * (frame2Sample - frame0Sample);
      float c2 = frame0Sample - (2.5F * frame1Sample) + (2 * frame2Sample) - (.5F * frame3Sample);
      float c3 = (.5F * (frame3Sample - frame0Sample)) + (1.5F * (frame1Sample - frame2Sample));
      outputBuffer[i * hop] = (((((c3 * t) + c2) * t) + c1) * t) + c0;

    }
  }

  
  
  /*

  size_t  LinearInterpolator::process(SampleType* outputBuffer,  size_t outputbufferSize, SampleType* pitchScale){

    size_t numFramesRendered = 0;
    
    for (int frame = 0; frame < outputbufferSize; frame++) {
    
      // If we've reached the end of the current buffer, swap buffers and load more data
      if (mSourceBufferReadHead >= mMaxSourceBufferLength) {
        fillNextBuffer();
      }else if ((int)mSourceBufferReadHead >= mCurrentSourceBuffer->length){
        break;
      }
    
      float interpolationPosition = mSourceBufferReadHead;
      
      // The first frame of the interpolated pair
      int frame1 = (int)interpolationPosition;
      
      // The second frame of the interpolated pair.
      int frame2 = frame1 + 1;
      
      // The buffer from which we'll pull data for frame 2.
      // This may be mCurrentBuffer, or mNextBuffer, depending on whether or not we're on the last frame or not
      Buffer* frame2Buffer;
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
  
  
  //////////////////////////////////////////
  /// Abstract Interpolator Using Four Frames
  //////////////////////////////////////////


  size_t FourFrameInterpolator::process(SampleType* outputBuffer, size_t outputbufferSize, SampleType* pitchScale){
    size_t numFramesRendered = 0;
    
    for (int frame = 0; frame < outputbufferSize; frame++) {
    
      // If we've reached the end of the current buffer, swap buffers and load more data
      if (mSourceBufferReadHead - 1 >= mMaxSourceBufferLength) {
        fillNextBuffer();
      }else if (mCurrentSourceBuffer->length < mMaxSourceBufferLength && (int)mSourceBufferReadHead >= mCurrentSourceBuffer->length){
        break;
      }
    
      float interpolationPosition = mSourceBufferReadHead;
      
      int frame0 = mSourceBufferReadHead == 0 ? 0 : (int)interpolationPosition - 1;
      int frame1 = (int)interpolationPosition;
      int frame2 = frame1 + 1;
      int frame3 = frame2 + 1;
      
      Buffer* frame0Buffer;
      Buffer* frame1Buffer;
      Buffer* frame2Buffer;
      Buffer* frame3Buffer;
      
      if (frame0 < mMaxSourceBufferLength) {
        frame0Buffer = mCurrentSourceBuffer;
      }else{
        frame0 = frame0 % mMaxSourceBufferLength;
        frame0Buffer = mNextSourceBuffer;
      }
      
      if (frame1 < mMaxSourceBufferLength) {
        frame1Buffer = mCurrentSourceBuffer;
      }else{
        frame1 = frame1 % mMaxSourceBufferLength;
        frame1Buffer = mNextSourceBuffer;
      }
      
      if (frame2 < mMaxSourceBufferLength) {
        frame2Buffer = mCurrentSourceBuffer;
      }else{
        frame2 = frame2 % mMaxSourceBufferLength;
        frame2Buffer = mNextSourceBuffer;
      }
      
      if (frame3 < mMaxSourceBufferLength) {
        frame3Buffer = mCurrentSourceBuffer;
      }else{
        frame3 = frame3 % mMaxSourceBufferLength;
        frame3Buffer = mNextSourceBuffer;
      }
    
      float t = interpolationPosition - (int)interpolationPosition;
    
      for (int channel = 0; channel < mNumChannels;  channel++) {
        // Get the data for frames we're interpolatining between
        
        float frame0Sample = frame0Buffer->data[frame0 * mNumChannels + channel];
        float frame1Sample = frame1Buffer->data[frame1 * mNumChannels + channel];
        float frame2Sample = frame2Buffer->data[frame2 * mNumChannels + channel];
        float frame3Sample = frame3Buffer->data[frame3 * mNumChannels + channel];

        outputBuffer[frame * mNumChannels + channel] = buildSample(t, frame0Sample, frame1Sample, frame2Sample, frame3Sample);
        
      }
      numFramesRendered++;
      mSourceBufferReadHead += pitchScale[frame];
    }
     
    return numFramesRendered;
   
  }
  
  //////////////////////////////////////////
  /// Cubic Interpolator
  //////////////////////////////////////////


  SampleType CubicInterpolator::buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample){
    float a0, a1, a2, a3;
    a0 = frame3Sample - frame2Sample - frame0Sample + frame1Sample;
    a1 = frame0Sample - frame1Sample - a0;
    a2 = frame2Sample - frame0Sample;
    a3 = frame1Sample;
    return (a0 * (t * t * t)) + (a1 * (t * t)) + (a2 * t) + (a3);
  }
  
  //////////////////////////////////////////
  /// Hermite Interpolator
  //////////////////////////////////////////

  SampleType HermiteInterpolator::buildSample(float t, SampleType frame0Sample, SampleType frame1Sample, SampleType frame2Sample, SampleType frame3Sample){
    float c0 = frame1Sample;
    float c1 = .5F * (frame2Sample - frame0Sample);
    float c2 = frame0Sample - (2.5F * frame1Sample) + (2 * frame2Sample) - (.5F * frame3Sample);
    float c3 = (.5F * (frame3Sample - frame0Sample)) + (1.5F * (frame1Sample - frame2Sample));
    return (((((c3 * t) + c2) * t) + c1) * t) + c0;
  }
   */


}