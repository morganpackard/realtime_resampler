//
//  RealtimeResampler.cpp
//  Resampler
//
//  Created by Morgan Packard on 2/22/15.
//

#include "RealtimeResampler.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <math.h>

using namespace std;

namespace RealtimeResampler {

 // LogLevel Renderer::sCurrentLogLevel = Renderer::LOG_INFO;
  
  Renderer::Renderer(float sampleRate, int numChannels, size_t sourceBufferLength, size_t maxFramesToRender) :
    mNumChannels(numChannels),
    mCurrentPitch(1),
    mPitchDestination(1),
    mSecondsUntilPitchDestination(0),
    mSourceBufferReadHead(sourceBufferLength),
    mSampleRate(sampleRate),
    mMaxFramesToRender(maxFramesToRender),
    mAlloc(&malloc),
    mDealloc(&free)
  {
    
    mCurrentSourceBuffer = &mSourceBuffer[0];
    mNextSourceBuffer = &mSourceBuffer[1];
  
    size_t sourceBufferBytes = sourceBufferLength * numChannels * sizeof(SampleType);
    for(int i = 0; i < 2; i++){
      mSourceBuffer[i].data = (SampleType*)mAlloc(sourceBufferBytes);
      mSourceBuffer[i].length = 0;
      mSourceBuffer[i].maxLength = sourceBufferLength;
    }
    mPitchBuffer = (float*)mAlloc(maxFramesToRender* sizeof(float));
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
    
    size_t numFramesRendered = 0;
    
    for (int frame = 0; frame < numFramesRequested; frame++) {
    
      // If we've reached the end of the current buffer, swap buffers and load more data
      if (mSourceBufferReadHead >= mCurrentSourceBuffer->maxLength) {
        swapBuffersAndFillNext();
      }
    
      float interpolationPosition = mSourceBufferReadHead;
      
      // The first frame of the interpolated pair
      int frame1 = (int)interpolationPosition;
      
      // The second frame of the interpolated pair.
      int frame2 = frame1 + 1;
      
      // The buffer from which we'll pull data for frame 2.
      // This may be mCurrentBuffer, or mNextBuffer, depending on whether or not we're on the last frame or not
      AudioBuffer* frame2Buffer;
      if (frame2 < mCurrentSourceBuffer->maxLength) {
        frame2Buffer = mCurrentSourceBuffer;
      }else{
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
      mSourceBufferReadHead += mPitchBuffer[frame];
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
    AudioBuffer* newCurrent = mNextSourceBuffer;
    AudioBuffer* newNext = mCurrentSourceBuffer;
    mCurrentSourceBuffer = newCurrent;
    mNextSourceBuffer = newNext;
    mSourceBufferReadHead -= mCurrentSourceBuffer->maxLength;
    if (mCurrentSourceBuffer->length == 0) {
      mCurrentSourceBuffer->length = mAudioSource->getSamples(mCurrentSourceBuffer->data, mCurrentSourceBuffer->maxLength);
    }
    mNextSourceBuffer->length = mAudioSource->getSamples(mNextSourceBuffer->data, mNextSourceBuffer->maxLength);
  }
  
}
