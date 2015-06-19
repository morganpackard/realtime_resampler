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
#include <cassert>
#include <stdio.h> // TODO remove this
#include <iostream>// TODO remove this

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
    mSourceBufferLength(sourceBufferLength),
    mIsInitialRender(true),
    mSourceBuffer1(sourceBufferLength * numChannels * sizeof(SampleType), allocFn, freeFn),
    mSourceBuffer2(sourceBufferLength * numChannels * sizeof(SampleType), allocFn, freeFn),
    mCurrentSourceBuffer(&mSourceBuffer1),
    mNextSourceBuffer(&mSourceBuffer2),
    mSourceBufferReadHead(sourceBufferLength),
    mPitchBuffer(maxFramesToRender * sizeof(SampleType), allocFn, freeFn),
    mInterpolationPositionBuffer(maxFramesToRender * sizeof(SampleType), allocFn, freeFn)
  {
  
  }
  
  Renderer::~Renderer(){
  }
  
  void Renderer::error(std::string message){
    printf(message.c_str());
  }

  size_t Renderer::getNumChannels(){
    return mNumChannels;
  }
  
  size_t Renderer::render(SampleType* outputBuffer, size_t numFramesRequested){
    
    assert(numFramesRequested <= mMaxFramesToRender);
    
    size_t numFramesRendered = 0;
    
    calculatePitchForNextFrames(numFramesRequested);
    
    while (numFramesRendered < numFramesRequested) {
        
          // load the source data if necessary
          if(mSourceBufferReadHead >= mCurrentSourceBuffer->length){
            swapBuffersAndFillNext();
          }
        
          // how many frames to render in this pass
          size_t interpolatedFramesToRender = 0;
      
          // last calculated interpolation position. Initalize it to be the last position read
          // this may be in the middle of a source buffer
          float interpPosition = mSourceBufferReadHead;
          
          // build the interpolation position buffer
          //  - keep track of how big it is
          //  - make sure we're not using more frames than are currently in the source buffer
          //  - make sure we're not rendering more frames than requested
          while(
            interpPosition < mCurrentSourceBuffer->length
            && ((interpolatedFramesToRender + numFramesRendered) < numFramesRequested)
          ){
            size_t pitchBufferPosition = numFramesRendered + interpolatedFramesToRender;
            mInterpolationPositionBuffer.data[interpolatedFramesToRender] = interpPosition;
            interpPosition += mPitchBuffer.data[pitchBufferPosition];
            interpolatedFramesToRender++;
          }
          
          // render the interpolated data
          
          // start where we left off
          SampleType* writeHead = outputBuffer + numFramesRendered * mNumChannels;
          SampleType* readHead = mCurrentSourceBuffer->data + (int)mSourceBufferReadHead;
          
          // interpolate [interpolatedFramesToRender] frames starting at readHead, writing to writehead
          // and using interpolationBuffer for frame position and interpolation coefficient
          /*
          for(int channel = 0; i < mNumChannels; channel++){
            interpolate(readHead + channel, writeHead + channel, interpolationBuffer, interpolatedFramesToRender, mNumChannels);
          }
          */

          memcpy(writeHead, readHead, interpolatedFramesToRender * sizeof(SampleType) * mNumChannels);
      
          // increment our total frame count
          numFramesRendered += interpolatedFramesToRender;
          
          // update the source buffer read head
          mSourceBufferReadHead = interpPosition;
      
          // if the current sourceBuffer is not full, that means the audiosource didn't supply enough samples
          if (mCurrentSourceBuffer->length < mSourceBufferLength) {
            break;
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
  
  size_t Renderer::getInputFrameCount(size_t outputFrameCount){
    double duration = outputFrameCount / mSampleRate;
    double glideDuration = duration < mSecondsUntilPitchDestination ? duration : mSecondsUntilPitchDestination;
    double nonGlideDuration = duration - glideDuration;
    nonGlideDuration = nonGlideDuration > 0 ? nonGlideDuration : 0;
    double glideSourceTime = 0;
    if (glideDuration > 0) {
      double slope = (mPitchDestination - mCurrentPitch) / mSecondsUntilPitchDestination;
      double endPitch = mCurrentPitch + slope * glideDuration;
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
    mInterpolator->mSourceBufferReadHead = mSourceBufferLength;
    mInterpolator->mMaxSourceBufferLength = mSourceBufferLength;
    mInterpolator->mNumChannels = mNumChannels;
//    mInterpolator->mCurrentSourceBuffer = &mSourceBuffer1.audioBuffer;
//    mInterpolator->mNextSourceBuffer = &mSourceBuffer2.audioBuffer;
  }
  
  
  void Renderer::calculatePitchForNextFrames(size_t numFrames){
    double pitchChangePerFrame;
    double interpPosition = 0;
    if (mSecondsUntilPitchDestination > 0) {
      pitchChangePerFrame = (mPitchDestination - mCurrentPitch) / (mSecondsUntilPitchDestination * mSampleRate);
    }else{
      pitchChangePerFrame = 0;
    }
    double secondsPerFrame = 1.0 / mSampleRate;
    for (int frame = 0; frame < numFrames; frame++) {
      mSecondsUntilPitchDestination -= secondsPerFrame;
      mCurrentPitch += pitchChangePerFrame;
      if (mSecondsUntilPitchDestination <=0) {
        mSecondsUntilPitchDestination = 0;
        pitchChangePerFrame = 0;
        mCurrentPitch = mPitchDestination;
      }
      mPitchBuffer.data[frame] = mCurrentPitch;
    }
  }
  
  void Renderer::swapBuffersAndFillNext(){
    mSourceBufferReadHead -= mSourceBufferLength;
    Buffer* newCurrent = mNextSourceBuffer;
    Buffer* newNext = mCurrentSourceBuffer;
    mCurrentSourceBuffer = newCurrent;
    mNextSourceBuffer = newNext;
    if (mCurrentSourceBuffer->length == 0) {
      mCurrentSourceBuffer->length = mAudioSource->getSamples(mCurrentSourceBuffer->data, mSourceBufferLength, mNumChannels);
    }
    newNext->length = mAudioSource->getSamples(newNext->data, mSourceBufferLength, mNumChannels);
  }
  
  size_t Renderer::fillSourceBuffer(Buffer* buffer){
    return mAudioSource->getSamples(buffer->data, mInterpolator->mMaxSourceBufferLength, mNumChannels);
  }
  
}
