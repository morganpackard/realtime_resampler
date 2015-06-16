//
//  PitchablePitchableBufferPlayer.h
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 3/21/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#ifndef __EliasResamplerDemo__PitchableBufferPlayer__
#define __EliasResamplerDemo__PitchableBufferPlayer__

#include "Tonic/TonicFrames.h"
#include "Tonic/Generator.h"
#include "Tonic/FixedValue.h"
#include "Tonic/SampleTable.h"
#include "Tonic/ControlTrigger.h"
#include "RealtimeResampler.h"

namespace Tonic {
  
  namespace Tonic_ {

    class PitchableBufferPlayer_ : public Generator_, public RealtimeResampler::AudioSource{
      
    protected:

        SampleTable buffer_;
        int testVar;
        int currentFrame;
        int samplesPerSynthesisBlock;
        ControlGenerator doesLoop_;
        ControlGenerator trigger_;
        ControlGenerator startPosition_;
        ControlTrigger finishedTrigger_;
        bool isFinished_;
        ControlGenerator playbackRate_;
        bool playbackRateIsOne;
        RealtimeResampler::Renderer* resampler;
        bool mDoesLoop;
        int mStartSecs;
        size_t calculateFramesLeftInBuffer();

    public:
        PitchableBufferPlayer_();
        ~PitchableBufferPlayer_();
        void computeSynthesisBlock(const SynthesisContext_ &context);

        void setBuffer(SampleTable sampleTable);
        void setDoesLoop(ControlGenerator doesLoop){ doesLoop_ = doesLoop; }
        void setTrigger(ControlGenerator trigger){ trigger_ = trigger; }
        void setStartPosition(ControlGenerator startPosition){ startPosition_ = startPosition; }
        bool isFinished(){ return isFinished_; }
        bool isStereoOutput(){ return buffer_.channels() == 2; }
        ControlGenerator finishedTrigger(){ return finishedTrigger_; };
        void setPlaybackRate(ControlGenerator playbackRate){
          playbackRate_ = playbackRate;
          playbackRateIsOne = false;
        }
        size_t getSamples(RealtimeResampler::SampleType* outputBuffer, size_t numFramesRequested, int numChannels);
      

    };



  }
  
  /*!
    Simply plays back a buffer. "loop" parameter works, but doesn't wrap between ticks, so mostly likely you'll wind up with a few zeroes at the end of 
    the last buffer if you're looping. In other words, buffer lengths are rounded up to the nearest kSynthesisBlockSize 
   
    Usage:
    
    SampleTable buffer = loadAudioFile("/Users/morganpackard/Desktop/trashme/2013.6.5.mp3");
    bPlayer.setBuffer(buffer).loop(false).trigger(ControlMetro().bpm(100));
   
  */
  
  class PitchableBufferPlayer : public TemplatedGenerator<Tonic_::PitchableBufferPlayer_>{
    
  public:
  
    PitchableBufferPlayer& setBuffer(SampleTable buffer){
      gen()->setBuffer(buffer);
      return *this;
    };

    bool isFinished(){ return gen()->isFinished(); }

    //! returns a ControlGenerator that emits a trigger message when the buffer reaches the end
    ControlGenerator finishedTrigger(){ return gen()->finishedTrigger(); }; 

    TONIC_MAKE_CTRL_GEN_SETTERS(PitchableBufferPlayer, loop, setDoesLoop)
    TONIC_MAKE_CTRL_GEN_SETTERS(PitchableBufferPlayer, trigger, setTrigger)
    TONIC_MAKE_CTRL_GEN_SETTERS(PitchableBufferPlayer, startPosition, setStartPosition)
    TONIC_MAKE_CTRL_GEN_SETTERS(PitchableBufferPlayer, playbackRate, setPlaybackRate)

  };
}

#endif /* defined(__EliasResamplerDemo__PitchablePitchableBufferPlayer__) */
