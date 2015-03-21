//
//  PitchablePitchableBufferPlayer.cpp
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 3/21/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#include "PitchableBufferPlayer.h"

namespace Tonic { namespace Tonic_{
  
  PitchableBufferPlayer_::PitchableBufferPlayer_() : resampler( 0 ), currentSample(0), isFinished_(true), playbackRateIsOne(true){
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
  }
  
  inline void PitchableBufferPlayer_::computeSynthesisBlock(const SynthesisContext_ &context){

    bool doesLoop = doesLoop_.tick(context).value;
    bool trigger = trigger_.tick(context).triggered;
    float startPosition = startPosition_.tick(context).value;
    
    if(trigger){
      isFinished_ = false;
      currentSample = startPosition * sampleRate() * buffer_.channels();
    }
    
    if(isFinished_){
      outputFrames_.clear();
    }else if(playbackRateIsOne){
      int samplesLeftInBuf = (int)buffer_.size() - currentSample;
      int samplesToCopy = min(samplesPerSynthesisBlock, samplesLeftInBuf);
      copySamplesToOutputBuffer(currentSample, samplesToCopy);
      if (samplesToCopy < samplesPerSynthesisBlock) {
        if(doesLoop){
          currentSample = startPosition * sampleRate() * buffer_.channels();
        }else{
          isFinished_ = true;
          finishedTrigger_.trigger();
        }
      }else{
        currentSample += samplesPerSynthesisBlock;
      }
    }else{
      int playbackRateHop = playbackRateFrames_.channels(); // ignore all but the first channel
      int outputChannels = outputFrames_.channels();
      playbackRate_.tick(playbackRateFrames_, context);
      TonicFloat* playbackRateData = &playbackRateFrames_[0];
      TonicFloat* inData = buffer_.dataPointer();
      TonicFloat* outData = &outputFrames_[0];
      size_t bufferFrameCount = buffer_.frames();
      for(int i = 0; i < kSynthesisBlockSize; i++){
        for (int chan = 0; chan < outputChannels; chan++) {
          outData[i * outputChannels + chan] = inData[ ((int)currentSample) * outputChannels + chan ];
        }
        currentSample += (playbackRateData[i * playbackRateHop]);
        if(currentSample >= bufferFrameCount ){
          currentSample -= bufferFrameCount;
        }
      }
    }
  }


} // Namespace Tonic_
  
  
  
} // Namespace Tonic