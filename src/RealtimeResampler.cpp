//
//  RealtimeResampler.cpp
//  Resampler
//
//  Created by Morgan Packard on 2/22/15.
//

#include "RealtimeResampler.h"
#include <cstdlib>
#include <algorithm>
#include <math.h>
#include "Interpolator.h"

namespace RealtimeResampler {
  
  Renderer::Renderer(float sampleRate, int numChannels, size_t sourceBufferLength, size_t maxFramesToRender, void* (*allocFn)(size_t), void (*freeFn)(void*) ) :
    mNumChannels(numChannels),
    mCurrentPitch(1),
    mPitchDestination(1),
    mSecondsUntilPitchDestination(0),
    mSampleRate(sampleRate),
    mMaxFramesToRender(maxFramesToRender),
    mMalloc(allocFn),
    mDealloc(freeFn),
    mSourceBufferLength(sourceBufferLength)
  {
    
    size_t sourceBufferBytes = sourceBufferLength * numChannels * sizeof(SampleType);
    for(int i = 0; i < 2; i++){
      mSourceBuffer[i].data = (SampleType*)mMalloc(sourceBufferBytes);
      mSourceBuffer[i].length = 0;
    }
    mPitchBuffer = (float*)mMalloc(maxFramesToRender* sizeof(float));
  }
  
  
  Renderer::~Renderer(){
    for(int i = 0; i < 2; i++){
      mDealloc(mSourceBuffer[i].data);
    }
    mDealloc(mPitchBuffer);
  }
  
  void Renderer::error(std::string message){
    printf(message.c_str());
  }

  size_t Renderer::getNumChannels(){
    return mNumChannels;
  }
  
  size_t Renderer::render(SampleType* outputBuffer, size_t numFramesRequested){
    
    if (numFramesRequested > mMaxFramesToRender) {
      error("Error in Renderer::render: numFramesRequested is larger than mMaxFramesToRender.");
    }
    
    calculatePitchForNextFrames(numFramesRequested);
    return mInterpolator->process(outputBuffer, numFramesRequested, mPitchBuffer);
    
  }
  
  void Renderer::setAudioSource(AudioSource* audioSource){
    mAudioSource = audioSource;
  }
  
  void Renderer::setPitch(float start, float end, float glideDuration){
      mCurrentPitch = start;
      mPitchDestination = end;
      mSecondsUntilPitchDestination = glideDuration;
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
    return ceil( (glideSourceTime + nonGlideSourceTime) * mSampleRate );
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
      float outputFramesForGlide = mSecondsUntilPitchDestination * mSampleRate;
      size_t inputFramesForSustain = inputFrameCount - inputFramesForGlide;
      size_t outputFramesForSustain = inputFramesForSustain / mPitchDestination;
      return outputFramesForSustain + outputFramesForGlide;
    }
  }
  
  void Renderer::setInterpolator(RealtimeResampler::Interpolator *interpolator){
    mInterpolator = interpolator;
    mInterpolator->mRenderer = this;
    mInterpolator->mSourceBufferReadHead = mSourceBufferLength; // Initialize this to one past maximum to force initial data load
    mInterpolator->mMaxSourceBufferLength = mSourceBufferLength;
    mInterpolator->mNumChannels = mNumChannels;
    mInterpolator->mCurrentSourceBuffer = &mSourceBuffer[0];
    mInterpolator->mNextSourceBuffer = &mSourceBuffer[1];
  }
  
  
  void Renderer::calculatePitchForNextFrames(size_t numFrames){
    float pitchChangePerFrame;
    if (mSecondsUntilPitchDestination > 0) {
      pitchChangePerFrame = (mPitchDestination - mCurrentPitch) / (mSecondsUntilPitchDestination * mSampleRate);
    }else{
      pitchChangePerFrame = 0;
    }
    float secondsPerFrame = 1.0 / mSampleRate;
    for (int frame = 0; frame < numFrames; frame++) {
      mSecondsUntilPitchDestination -= secondsPerFrame;
      mCurrentPitch += pitchChangePerFrame;
      if (mSecondsUntilPitchDestination <=0) {
        mSecondsUntilPitchDestination = 0;
        pitchChangePerFrame = 0;
        mCurrentPitch = mPitchDestination;
      }
      mPitchBuffer[frame] = mCurrentPitch;
    }
  }
  
  void Renderer::swapBuffersAndFillNext(){
    AudioBuffer* newCurrent = mInterpolator->mNextSourceBuffer;
    AudioBuffer* newNext = mInterpolator->mCurrentSourceBuffer;
    mInterpolator->mCurrentSourceBuffer = newCurrent;
    mInterpolator->mNextSourceBuffer = newNext;
    mInterpolator->mSourceBufferReadHead -= mInterpolator->mMaxSourceBufferLength;
    if (newCurrent->length == 0) {
      newCurrent->length = mAudioSource->getSamples(newCurrent->data, mInterpolator->mMaxSourceBufferLength, mNumChannels);
    }
    newNext->length = mAudioSource->getSamples(newNext->data, mInterpolator->mMaxSourceBufferLength, mNumChannels);
    if(newNext->length < mInterpolator->mMaxSourceBufferLength){
      printf("Renderer::swapBuffersAndFillNext newNext->length is less than number requested\n");
    }
  }
  
  size_t Renderer::fillSourceBuffer(AudioBuffer* buffer){
    return mAudioSource->getSamples(buffer->data, mInterpolator->mMaxSourceBufferLength, mNumChannels);
  }
  
}
