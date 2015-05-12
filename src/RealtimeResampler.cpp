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

using namespace std;

namespace RealtimeResampler {

 // LogLevel Renderer::sCurrentLogLevel = Renderer::LOG_INFO;
  
  Renderer::Renderer(float sampleRate, int numChannels, size_t sourceBufferLength, size_t maxFramesToRender) :
    mNumChannels(numChannels),
    mCurrentPitch(1),
    mPitchDestination(1),
    mSecondsUntilPitchDestination(0),
    mSourceBufferReadHead(0),
    mSourceBufferLength(sourceBufferLength),
    mSampleRate(sampleRate),
    mMaxFramesToRender(maxFramesToRender),
    mAlloc(&malloc),
    mDealloc(&free)
  {
    size_t sourceBufferBytes = sourceBufferLength * numChannels * sizeof(SampleType);
    mSourceBuffer = (SampleType*)mAlloc(sourceBufferBytes);
    mPitchBuffer = (float*)mAlloc(maxFramesToRender* sizeof(float));
  }
  
  
  Renderer::~Renderer(){
    mDealloc(mSourceBuffer);
    mDealloc(mPitchBuffer);
  }

  size_t Renderer::getNumChannels(){
    return mNumChannels;
  }
  
  size_t Renderer::render(SampleType* outputBuffer, size_t numFramesRequested){
    
    if (numFramesRequested > mMaxFramesToRender) {
      printf("Error in Renderer::render: numFramesRequested is larger than mMaxFramesToRender.");
    }
    
    calculatePitchForNextFrames(numFramesRequested);
    
    size_t sourceFramesRemainingToResample = getInputFrameCount(numFramesRequested);
    size_t numFramesRendered = 0;
    SampleType* writeHead = outputBuffer;
    
    while (sourceFramesRemainingToResample > 0) {
        size_t sourceFramesToFetch = std::min<size_t>(mSourceBufferLength, sourceFramesRemainingToResample);
        size_t sourceFramesDelivered = mAudioSource->getSamples(mSourceBuffer, sourceFramesToFetch);
        if (sourceFramesDelivered == sourceFramesToFetch) {
            sourceFramesRemainingToResample -= sourceFramesDelivered;
        }else{
          sourceFramesRemainingToResample = 0;
        }
        size_t framesToRender = getOutputFrameCount(sourceFramesDelivered);
        mInterpolator->process(mSourceBuffer, writeHead, sourceFramesDelivered, framesToRender, mPitchBuffer);
        
    }
    
  
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
  
//  void Renderer::log(std::string text, LogLevel level){
//    
//    string levelString;
//    
//    switch (level) {
//      case LOG_DEBUG:
//        levelString = "DEBUG";
//        break;
//
//      default:
//        break;
//    }
//    
//    cout << text << endl;
//  }
  
}
