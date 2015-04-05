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
    mAlloc(&malloc)
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
  
    for (size_t frame; frame < numFramesRequested; frame++) {
      
      // if source read position is greater than length of last source buffer minus 1
        // subtract length of last source buffer from source read position
        // mSourceFramesLastDelivered = mAudioSource->getSamples
      // if mSourceFramesLastDelivered > 0 and source read position is less than length of last source buffer minus 1
        // calculate output sample, copy value to output buffer
        // increment source read position by current pitch value
        // increment numFramesRendered
      // else
        // write zero to output buffer
        // mSourceBufferReadHead = 0
      // recalculate pitch
      
      if(mSourceBufferReadHead > mSourceFramesLastDelivered -1){
      
        mSourceBufferReadHead -= mSourceFramesLastDelivered;
        mSourceFramesLastDelivered = mAudioSource->getSamples(mSourceBuffer, mSourceBufferLength);
        
      }
      if(mSourceFramesLastDelivered > 0 && mSourceBufferReadHead < mSourceFramesLastDelivered - 1){
      
        for (int channel; channel < mNumChannels; channel++) {
            outputBuffer[ frame * mNumChannels + channel] = mSourceBuffer[(int)mSourceBufferReadHead * mNumChannels + channel];
        }
        
        mSourceBufferReadHead += mCurrentPitch;
        numFramesRendered++;
     
       }else{
      
        for (int channel; channel < mNumChannels; channel++) {
          outputBuffer[ frame * mNumChannels + channel] =0;
        }
        
      }
    }
    
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
    float slope = ( mPitchDestination - mCurrentPitch ) / glideDuration;
    float pitchAtEndOfGlide = mCurrentPitch + slope * glideDuration;
    size_t glideFrames = (mCurrentPitch + pitchAtEndOfGlide) / glideDuration;
    size_t nonGlideFrames = nonGlideDuration * mPitchDestination;
    return glideFrames + nonGlideFrames;
  }
  
}
