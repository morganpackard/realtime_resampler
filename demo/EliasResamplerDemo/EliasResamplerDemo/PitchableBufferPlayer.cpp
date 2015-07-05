//
//  PitchablePitchableBufferPlayer.cpp
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 3/21/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#include "PitchableBufferPlayer.h"
#include "RealtimeResamplerInterpolator.h"
#include "RealtimeResamplerFilter.h"

using namespace RealtimeResampler;

namespace Tonic { namespace Tonic_{
  
  PitchableBufferPlayer_::PitchableBufferPlayer_() : resampler( 0 ), currentFrame(0), isFinished_(true), playbackRateIsOne(true){
    doesLoop_ = ControlValue(false);
    trigger_ = ControlTrigger();
    startPosition_ = ControlValue(0);
  }
  
  PitchableBufferPlayer_::~PitchableBufferPlayer_(){
    delete resampler;
  }
  
  void  PitchableBufferPlayer_::setBuffer(SampleTable buffer){
    buffer_ = buffer;
    setIsStereoOutput(buffer.channels() == 2);
    samplesPerSynthesisBlock = kSynthesisBlockSize * buffer_.channels();
    if(resampler != 0){
      delete resampler;
    }
    resampler = new RealtimeResampler::Renderer(kSynthesisBlockSize, buffer_.channels(), 1000, 1000);
    resampler->setInterpolator(new WatteTrilinearInterpolator());
    resampler->setLowPassFilter(new LPF12(Tonic::sampleRate(), kSynthesisBlockSize, buffer.channels()));
    resampler->setAudioSource(this);
    printf("PitchableBufferPlayer_::setBuffer buffer size: %zu\n", buffer_.frames());
  }
  
  inline void PitchableBufferPlayer_::computeSynthesisBlock(const SynthesisContext_ &context){
    mDoesLoop = doesLoop_.tick(context).value;
    bool trigger = trigger_.tick(context).triggered;
    mStartSecs = startPosition_.tick(context).value;
    float pitch = playbackRate_.tick(context).value;
    resampler->setPitch(pitch, pitch, 0);
    
    if(trigger){
      isFinished_ = false;
      currentFrame = mStartSecs * sampleRate() * buffer_.channels();
    }
    
    if(isFinished_){
      outputFrames_.clear();
    }else{
      size_t framesRendered = resampler->render(&outputFrames_[0], kSynthesisBlockSize);
      if(framesRendered < kSynthesisBlockSize){
        
        isFinished_ = true;
        finishedTrigger_.trigger();

        void* start = &outputFrames_[0] + (isStereoOutput() ? 1 : 2) * (kSynthesisBlockSize - framesRendered);
        size_t bytes = (kSynthesisBlockSize - framesRendered) * sizeof(TonicFloat);
        memset(start, 0, bytes);
      }
    }
    
  }
  
  size_t PitchableBufferPlayer_::calculateFramesLeftInBuffer(){
    return buffer_.frames() - currentFrame;
  }
  
  size_t PitchableBufferPlayer_::getSamples(RealtimeResampler::SampleType* outputBuffer, size_t numFramesRequested, int numChannels){
  
    memset(outputBuffer, 0, numFramesRequested * numChannels * sizeof(TonicFloat));
  
    size_t totalFramesCopied = 0;
    
    while (totalFramesCopied < numFramesRequested) {
      size_t framesLeftInBuffer = calculateFramesLeftInBuffer();
      size_t copyOverage = max(0, numFramesRequested - framesLeftInBuffer);
      size_t framesToCopy = numFramesRequested - totalFramesCopied - copyOverage;
      size_t bytesToCopy = framesToCopy * buffer_.channels() * sizeof(TonicFloat);
      memcpy(outputBuffer, &buffer_.dataPointer()[currentFrame * buffer_.channels()], bytesToCopy);
      
      
//      printf("PitchableBufferPlayer_::getSamples copying %lu frames starting at %lu\n", framesToCopy, currentFrame);
      
      if(totalFramesCopied > 0){
          printf("PitchableBufferPlayer_::getSamples totalFramesCopied: %lu. framesToCopy: %lu\n", totalFramesCopied, framesToCopy);
      }
      
      totalFramesCopied += framesToCopy;
      currentFrame +=framesToCopy;
      outputBuffer += framesToCopy;
      if (currentFrame >= buffer_.frames()) {
        if(mDoesLoop){
          currentFrame = mStartSecs * sampleRate() * buffer_.channels();
        }else{
          break;
        }
      }
    }
    
    if(totalFramesCopied < numFramesRequested){
      printf("PitchableBufferPlayer_::getSamples returning fewer frames than requested\n");
    }
    
    return totalFramesCopied;

  }


} // Namespace Tonic_
  
  
  
} // Namespace Tonic