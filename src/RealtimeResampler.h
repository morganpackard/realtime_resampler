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


#ifndef __Resampler__RealtimeResampler__
#define __Resampler__RealtimeResampler__

#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "RealtimeResamplerBuffer.h"
#include "RealtimeResamplerCommon.h"

namespace RealtimeResampler {

      class Interpolator;
      class Filter;
  
      // allocator / deallocator are malloc and free by default, but can be overridden
      extern void* (*mallocFn)(size_t);
      extern void (*freeFn)(void*);
  
      //////////////////////////////////////////
      /// Abstract AudioSource delegate class.
      //////////////////////////////////////////
    
      /*!
        This is the object that delivers the original samples to the renderer. 
        You must create an instance  subclass and pass it to the renderer using Renderer::setAudioSource
      */
  
      class AudioSource{
      
        public:
        
        /*!
         The renderer calls this method when it needs additional samples in order to complete the render. 
         getSamples must return the actual number of frames written to the buffer, which may be less than numFramesRequested.
        */
        
        virtual size_t                getSamples(SampleType* outputBuffer, size_t numFramesRequested, int numChannels) = 0;
        
      };

      //////////////////////////////////////////
      /// Renderer class.
      //////////////////////////////////////////

      class Renderer{
      friend class Interpolator;
        public:
        
          /*!
            Constructor
          */

          Renderer(
            float sampleRate,
            int numChannels,
            size_t sourceBufferLength = 64,
            size_t maxFramesToRender = 64
          );
        
          /*!
            Destructor
          */
          
          ~Renderer();
        
          /*!
            Render samples at the current pitch. Returns the actual number of samples written to the output buffer. 
            If the AudioSource has no more data to supply, the number of frames written may be less than the number of frames requested.
            numFramesRequested MUST NOT exceed maxFramesToRender (64 by default)
          */
        
          size_t                      render(SampleType* outputBuffer, size_t numFramesRequested);
        
          /*!
            Set the pitch ratio to render at. A pitch of 1 means no change in pitch. Pitch scale of 2 means the output will
            be, twice the speed and an octave higher. A pitch of 0.5 will reduce the speed by half and lower the pitch an octave.
            The next rendered frame will be at a pitch of "start". The pitch at "glideDuration" seconds from the next rendered frame 
            will be at the pitch of "end". Linear interpolation will be used to determine the pitches of the frames in between.
          */
        
          void                        setPitch(float start, float end, float glideDuration);
        
          /*!
            Returns the pitch (with 1 being same pitch, 2 being double pitch, 0.5 being half pitch) of the last rendered frame.
          */
        
          float                       getCurrentPitch();
        
          /*!
            Get the number of channels the renderer was configured with.
          */
        
          size_t                      getNumChannels();
        
        
          /*!
            Set the AudioSource delegate object. This MUST be called or there will be no data to resample!
          */
        
          void                        setAudioSource(AudioSource* audioSource);
        
          /*!
            "Manually" set the interpolator. This should not be called after the first call to rRenderer::render.
          */
        
          void                        setInterpolator(Interpolator* interpolator);
        
          /*!
            "Manually" set the low pass filter. This should not be called after the first call to rRenderer::render.
          */
        
          void                        addLowPassFilter(Filter* filter);
        
          /*!
            Remove all low-pass filters.
          */
        
          void                        clearLowPassfilters();
        
          
          /*!
            Clear the internal buffers.
          */
        
          void                        reset();
        
          const static int            BUFFER_BACK_PADDING; //we need to copy the first bit of the next buffer on to the end of the current buffer
          const static int            BUFFER_FRONT_PADDING; //we need to copy the last bit of the previous buffer on to the end of the current buffer

        private:
        
          //                          -methods-
          void                        calculatePitchForNextFrames(size_t numFrames);
          void                        swapBuffersAndFillNext();
          void                        fillSourceBuffer(Buffer* buf);
          void                        filterBuffer(Buffer* buf);
        
          //                          -variables-
          int                         mNumChannels;
          float                       mSampleRate; // frames per second
          AudioSource*                mAudioSource;
          float                       mCurrentPitch;
          float                       mPitchDestination;
          float                       mSecondsUntilPitchDestination;
          Buffer                      mPitchBuffer; // The pitch at each frame. In other words, the factor by which to advance the source buffer read head
          Buffer                      mInterpolationPositionBuffer; // The pitch at each frame. In other words, the factor by which to advance the source buffer read head
          bool                        mBufferSwapState;
          double                      mSourceBufferReadHead;
          Buffer                      mSourceBuffer1;
          Buffer                      mSourceBuffer2;
          float                       mCurrentSourceBufferReadHead;
          size_t                      mSourceBufferLength;
          Interpolator*               mInterpolator;
          size_t                      mMaxFramesToRender;
          Filter*                     mLPF[10];
          int                         mLpfCount;

      };
  
  
}

#endif /* defined(__Resampler__RealtimeResampler__) */
