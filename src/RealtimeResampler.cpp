//
//  RealtimeResampler.h
//  Resampler
//
//  Created by Morgan Packard with encouragement and guidance from Philip Bennefall on 2/22/15.
//
//  Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include "RealtimeResampler.h"
#include <cstdlib>
#include <algorithm>
#include <math.h>
#include "RealtimeResamplerInterpolator.h"
#include "RealtimeResamplerFilter.h"
#include <cassert>

#ifndef REALTIME_RESAMPLER_BUFFER_BACK_PADDING
  #define REALTIME_RESAMPLER_BUFFER_BACK_PADDING 2
#endif

#ifndef REALTIME_RESAMPLER_BUFFER_FRONT_PADDING
  #define REALTIME_RESAMPLER_BUFFER_FRONT_PADDING 2
#endif

namespace RealtimeResampler {

  void* (*mallocFn)(size_t) = malloc;
  void (*freeFn)(void*) = free;
  
  
  
  const int Renderer::BUFFER_BACK_PADDING = REALTIME_RESAMPLER_BUFFER_BACK_PADDING;
  const int Renderer::BUFFER_FRONT_PADDING = REALTIME_RESAMPLER_BUFFER_FRONT_PADDING;
  
  Renderer::Renderer(float sampleRate, int numChannels, size_t sourceBufferLength, size_t maxFramesToRender ) :
    mNumChannels(numChannels),
    mCurrentPitch(1),
    mPitchDestination(1),
    mSecondsUntilPitchDestination(0),
    mSampleRate(sampleRate),
    mMaxFramesToRender(maxFramesToRender),
    mSourceBufferLength(sourceBufferLength),
    mSourceBuffer1( sourceBufferLength, numChannels, BUFFER_FRONT_PADDING, BUFFER_BACK_PADDING),
    mSourceBuffer2( sourceBufferLength, numChannels, BUFFER_FRONT_PADDING, BUFFER_BACK_PADDING),
    mBufferSwapState(0),
    mSourceBufferReadHead(sourceBufferLength),
    mPitchBuffer(maxFramesToRender, 1),
    mInterpolationPositionBuffer(maxFramesToRender, 1),
    mLpfCount(0)
  {
  }
  
  Renderer::~Renderer(){
  }
  
  
  void Renderer::reset(){
      mSourceBuffer1.length = 0;
      mSourceBuffer2.length = 0;
      for(int i = 0; i < mLpfCount; i++){
        mLPF[i]->reset();
      }
  }

  size_t Renderer::getNumChannels(){
    return mNumChannels;
  }
  
  size_t Renderer::render(SampleType* outputBuffer, size_t numFramesRequested){
  
    memset(outputBuffer, 0, numFramesRequested * mNumChannels * sizeof(SampleType));
    
    assert(numFramesRequested <= mMaxFramesToRender);
    
    size_t numFramesRendered = 0;
    
    calculatePitchForNextFrames(numFramesRequested);
    
    Buffer* currentBuffer = mBufferSwapState ? &mSourceBuffer1 : &mSourceBuffer2;
     
    while (numFramesRendered < numFramesRequested) {
        
      // load the source data if necessary
      if(mSourceBufferReadHead >= currentBuffer->length){
        swapBuffersAndFillNext();
        currentBuffer = mBufferSwapState ? &mSourceBuffer1 : &mSourceBuffer2;
      }
    
      // how many frames to render in this pass
      size_t interpolatedFramesToRender = 0;
  
      // last calculated interpolation position. Initalize it to be the last position read
      // this may be in the middle of a source buffer
      float interpPosition = mSourceBufferReadHead;
     
      int interpPositionOffset = (int)mSourceBufferReadHead;
      
      // build the interpolation position buffer
      //  - keep track of how big it is
      //  - make sure we're not using more frames than are currently in the source buffer
      //  - make sure we're not rendering more frames than requested
      //  - when we build the mInterpolationPositionBuffer, compensate for the fact that readhead has an offset. this could be optimized better.
      while(
        (interpPosition) < currentBuffer->length
        && ((interpolatedFramesToRender + numFramesRendered) < numFramesRequested)
      ){
        size_t pitchBufferPosition = numFramesRendered + interpolatedFramesToRender;
        mInterpolationPositionBuffer.getStartPtr()[interpolatedFramesToRender] = interpPosition - interpPositionOffset;
        interpPosition += mPitchBuffer.getStartPtr()[pitchBufferPosition];
        interpolatedFramesToRender++;
      }
      
      // render the interpolated data
      
      // start where we left off
      SampleType* writeHead = outputBuffer + numFramesRendered * mNumChannels;
      SampleType* readHead = currentBuffer->getStartPtr() + ((int)mSourceBufferReadHead) * mNumChannels;
      
      // no need to interpolate if the pitch is zero
      if( mCurrentPitch == 1 && mPitchDestination == 1){
        memcpy(writeHead, readHead, interpolatedFramesToRender * mNumChannels * sizeof(SampleType));
      }else{
        // otherwise, use the interpolator
        // interpolate [interpolatedFramesToRender] frames starting at readHead, writing to writehead
        // and using interpolationBuffer for frame position and interpolation coefficient
        for(int channel = 0; channel < mNumChannels; channel++){
          mInterpolator->process(readHead + channel, writeHead + channel, mInterpolationPositionBuffer.getStartPtr(), interpolatedFramesToRender, mNumChannels);
        }
      }
      

  
      // increment our total frame count
      numFramesRendered += interpolatedFramesToRender;
      
     // printf("interpolatedFramesToRender: %i\n", interpolatedFramesToRender);
      
      // update the source buffer read head
      mSourceBufferReadHead = interpPosition;
  
      // if the current sourceBuffer is not full, that means the audiosource didn't supply enough samples
      if (currentBuffer->length < mSourceBufferLength) {
        reset();
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
  
  void Renderer::calculatePitchForNextFrames(size_t numFrames){
    double pitchChangePerFrame;
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
      mPitchBuffer.getStartPtr()[frame] = mCurrentPitch;
    }
  }
  
  
  void Renderer::fillSourceBuffer(Buffer* buf){
    buf->length = mAudioSource->getSamples(buf->getStartPtr(), mSourceBufferLength, mNumChannels);
  }
  
  void Renderer::filterBuffer(Buffer* buf){
    // There's no need to anti-alias if we're pitching down
    if(mCurrentPitch > 1){
      // Attenuate frequencies above nyquist. Use the start of the pitch buffer for
      // convenience. There will be some error in the case of wild pitch bends,
      // but it is assumed that this approach is good enough.
      for(int i = 0; i < mLpfCount; i++){
        mLPF[i]->process(buf, mLPF[i]->pitchFactorToCutoff(*mPitchBuffer.getStartPtr()));
      }
    }
  }
  
  void Renderer::swapBuffersAndFillNext(){
  
    mSourceBufferReadHead = fmax(0, mSourceBufferReadHead - mSourceBufferLength);
    mBufferSwapState = !mBufferSwapState;
    Buffer* currentBuffer = mBufferSwapState ? &mSourceBuffer1 : &mSourceBuffer2;
    Buffer* nextBuffer =  !mBufferSwapState ? &mSourceBuffer1 : &mSourceBuffer2;
    if (currentBuffer->length == 0) {
      fillSourceBuffer(currentBuffer);
      // run the buffer through the anti-aliasing filter
      filterBuffer(currentBuffer);
    }
    
    fillSourceBuffer(nextBuffer);
    
    // zero out the end of the buffer in case it's shorter than we requested
    memset(nextBuffer->getStartPtr() + nextBuffer->length * mNumChannels, 0, (mSourceBufferLength - nextBuffer->length) * mNumChannels * sizeof(SampleType) );
    
    // copy the end of the current buffer into the "hidden" frames below index zero of the next buffer
    int bufferFrontSampleCount = BUFFER_FRONT_PADDING * mNumChannels;
    void* nextBufferHiddenFramesStart = nextBuffer->getStartPtr() - bufferFrontSampleCount;
    void* currentBufCopyStartPoint = currentBuffer->getStartPtr() + currentBuffer->length * mNumChannels - bufferFrontSampleCount;
    memcpy(nextBufferHiddenFramesStart, currentBufCopyStartPoint, bufferFrontSampleCount * sizeof(SampleType));
    
    filterBuffer(nextBuffer);
    
    // copy the (filtered) first couple frames of the next buffer on to the end of the current buffer to handle interpolation between the end of the
    // current and the beginning of the next.
    memcpy(currentBuffer->getStartPtr() + currentBuffer->length * mNumChannels, nextBuffer->getStartPtr(), BUFFER_BACK_PADDING * mNumChannels * sizeof(SampleType));
    
  }
  
  void Renderer::setInterpolator(RealtimeResampler::Interpolator *interpolator){
    mInterpolator = interpolator;
    mSourceBuffer1 = Buffer( mSourceBufferLength * mNumChannels, BUFFER_FRONT_PADDING * mNumChannels, BUFFER_BACK_PADDING * mNumChannels);
    mSourceBuffer2= Buffer( mSourceBufferLength * mNumChannels, BUFFER_FRONT_PADDING * mNumChannels, BUFFER_BACK_PADDING * mNumChannels);
  }
  
  void Renderer::addLowPassFilter(Filter* filter){
    // check to make sure the filter wasn't already added
    for(int i = 0; i < mLpfCount; i++){
      if (mLPF[i] == filter) {
        printf("ERROR in Renderer::addLowPassFilter. You must not add the same filter more than once.");
        return;
      }
    }
    if (mLpfCount < 10) {
      mLPF[mLpfCount++] = filter;
      filter->init(mSampleRate, mSourceBufferLength, mNumChannels);
    }
  }
  
  void Renderer::clearLowPassfilters(){
      mLpfCount = 0;
  }

  
}
