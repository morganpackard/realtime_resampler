//
//  PitchablePitchableBufferPlayer.cpp
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 3/21/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#include "PitchableBufferPlayer.h"
#include "Interpolator.h"

using namespace RealtimeResampler;

namespace Tonic { namespace Tonic_{
  
  PitchableBufferPlayer_::PitchableBufferPlayer_() : resampler( 0 ), currentFrame(0), isFinished_(true), playbackRateIsOne(true){
    doesLoop_ = ControlValue(false);
    trigger_ = ControlTrigger();
    startPosition_ = ControlValue(0);
  }
  
  PitchableBufferPlayer_::~PitchableBufferPlayer_(){
    
  }
  
  void  PitchableBufferPlayer_::setBuffer(SampleTable buffer){
    buffer_ = buffer;
    setIsStereoOutput(buffer.channels() == 2);
    samplesPerSynthesisBlock = kSynthesisBlockSize * buffer_.channels();
    if(resampler != 0){
      delete resampler;
    }
    resampler = new RealtimeResampler::Renderer(kSynthesisBlockSize, buffer_.channels());
    resampler->setInterpolator(new CubicInterpolator());
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
      
      size_t framesRendered = resampler->render(&outputFrames_[0], min(kSynthesisBlockSize, calculateFramesLeftInBuffer()));
//      if(framesRendered < kSynthesisBlockSize){
//        printf("PitchableBufferPlayer_::computeSynthesisBlock frames rendered: %zu\n", framesRendered);
//      }
    }
    
  }
  
  size_t PitchableBufferPlayer_::calculateFramesLeftInBuffer(){
    return buffer_.frames() - currentFrame;
  }
  
  size_t PitchableBufferPlayer_::getSamples(RealtimeResampler::SampleType* outputBuffer, size_t numFramesRequested, int numChannels){
  
    size_t totalFramesCopied = 0;
    
    while (totalFramesCopied < numFramesRequested) {
      size_t framesLeftInBuffer = calculateFramesLeftInBuffer();
      size_t copyOverage = max(0, numFramesRequested - framesLeftInBuffer);
      size_t framesToCopy = numFramesRequested - copyOverage;
      size_t bytesToCopy = framesToCopy * buffer_.channels() * sizeof(TonicFloat);
      memcpy(outputBuffer, &buffer_.dataPointer()[currentFrame * buffer_.channels()], bytesToCopy);
      totalFramesCopied += framesToCopy;
      currentFrame +=framesToCopy;
      outputBuffer += framesToCopy;
      if (currentFrame >= buffer_.frames()) {
        if(mDoesLoop){
          currentFrame = mStartSecs * sampleRate() * buffer_.channels();
        }else{
          isFinished_ = true;
          finishedTrigger_.trigger();
          break;
        }
      }
    }
    
    return totalFramesCopied;

  }


} // Namespace Tonic_
  
  
  
} // Namespace Tonic