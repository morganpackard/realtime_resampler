//
//  RealtimeResampler.cpp
//  Resampler
//
//  Created by Morgan Packard on 2/22/15.
//

#include "RealtimeResampler.h"
#include <cstdlib>

namespace RealtimeResampler {
  
  Renderer::Renderer(float sampleRate, int numChannels, size_t sourceBufferLength) :
    mNumChannels(numChannels),
    mCurrentPitch(1),
    mPitchDestination(1),
    mSecondsUntilPitchDestination(0),
    mSourceBufferReadHead(0),
    mSourceFramesLastDelivered(0),
    mSampleRate(sampleRate),
    mAlloc(&malloc),
    mDealloc(&free)
  {
    mSourceBuffer = (SampleType*)mAlloc(sourceBufferLength * numChannels * sizeof(SampleType));
  }
  
  
  Renderer::~Renderer(){
    mDealloc(mSourceBuffer);
  }

  size_t Renderer::getNumChannels(){
    return mNumChannels;
  }
  
  size_t Renderer::render(SampleType* outputBuffer, size_t numFramesRequested){
  
    size_t numFramesRendered = 0;
    
    size_t sourceFrameRequestCount = getInputFrameCount(numFramesRequested);
    size_t sourceFrameReturnCount = mAudioSource->getSamples(mSourceBuffer, sourceFrameRequestCount);
  
//    for (size_t frame; frame < numFramesRequested; frame++) {
//      
//      // if source read position is greater than length of last source buffer minus 1
//        // subtract length of last source buffer from source read position
//        // mSourceFramesLastDelivered = mAudioSource->getSamples
//      // if mSourceFramesLastDelivered > 0 and source read position is less than length of last source buffer minus 1
//        // calculate output sample, copy value to output buffer
//        // increment source read position by current pitch value
//        // increment numFramesRendered
//      // else
//        // write zero to output buffer
//        // mSourceBufferReadHead = 0
//      // recalculate pitch
//      
//      if(mSourceBufferReadHead > mSourceFramesLastDelivered -1){
//      
//        mSourceBufferReadHead -= mSourceFramesLastDelivered;
//        mSourceFramesLastDelivered = mAudioSource->getSamples(mSourceBuffer, mSourceBufferLength);
//        
//      }
//      if(mSourceFramesLastDelivered > 0 && mSourceBufferReadHead < mSourceFramesLastDelivered - 1){
//      
//        for (int channel; channel < mNumChannels; channel++) {
//            outputBuffer[ frame * mNumChannels + channel] = mSourceBuffer[(int)mSourceBufferReadHead * mNumChannels + channel];
//        }
//        
//        mSourceBufferReadHead += mCurrentPitch;
//        numFramesRendered++;
//     
//       }else{
//      
//        for (int channel; channel < mNumChannels; channel++) {
//          outputBuffer[ frame * mNumChannels + channel] =0;
//        }
//        
//      }
//    }
    
    return numFramesRendered;
    
  }
  
  void Renderer::setAudioSource(AudioSource* audioSource){
    mAudioSource = audioSource;
  }
  
  void Renderer::setPitch(float start, float end, float glideDuration){
      mCurrentPitch = start;
      mPitchDestination = end;
      mSecondsUntilPitchDestination = glideDuration;
  }
  
  void Renderer::setAllocator( void* (*alloc)(size_t) ){
    mAlloc = alloc;
  }
  
  void Renderer::setDeAllocator( void (*deallocArg)(void*) ){
    mDealloc = deallocArg;
  }
  
  size_t Renderer::getInputFrameCount(size_t outputFrameCount){
    double duration = outputFrameCount / mSampleRate;
    double glideDuration = duration < mSecondsUntilPitchDestination ? duration : mSecondsUntilPitchDestination;
    double nonGlideDuration = duration - glideDuration;
    nonGlideDuration = nonGlideDuration > 0 ? nonGlideDuration : 0;
    double glideSourceTime = 0;
    if (glideDuration > 0) {
      float slope = (mPitchDestination - mCurrentPitch) / mSecondsUntilPitchDestination;
      float endPitch = mCurrentPitch + slope * glideDuration;
      glideSourceTime = glideDuration *  (mCurrentPitch +  endPitch) / 2 ;
    }
    double nonGlideSourceTime = nonGlideDuration * mPitchDestination;
    return (glideSourceTime + nonGlideSourceTime) * mSampleRate;
  }
  
  
  size_t Renderer::getOutputFrameCount(size_t inputFrameCount){
    // First, calculate the total number of input frames required to render the entire pitch glide
    float inputFramesForGlide = mSampleRate * mSecondsUntilPitchDestination * (mCurrentPitch + mPitchDestination) / 2;
    // If that total is less than inputFrameCount, calculate the duration of glide needed to "use up" the input frames.
    if (inputFrameCount < inputFramesForGlide) {
      /*
      
        f(x) = mx + b;
        area under f(x) = x * b + mx/2
        area = x(b + m/2)
       
          area
        --------  =  x
        b + m/2
      
      */
      
      float slope = (mPitchDestination - mCurrentPitch) / mSecondsUntilPitchDestination;
      return inputFrameCount / (mCurrentPitch + slope / 2);
    }else{
      size_t outputFramesForGlide = mSecondsUntilPitchDestination * mSampleRate;
      size_t inputFramesForSustain = inputFrameCount - inputFramesForGlide;
      size_t outputFramesForSustain = inputFramesForSustain / mPitchDestination;
      return outputFramesForSustain + outputFramesForGlide;
    }
  }
  
}
