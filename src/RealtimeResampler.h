//
//  RealtimeResampler.h
//  Resampler
//
//  Created by Morgan Packard on 2/22/15.
//
//  Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//


#ifndef __Resampler__RealtimeResampler__
#define __Resampler__RealtimeResampler__

#include <stdio.h>
#include <string>

namespace RealtimeResampler {

      typedef float SampleType;
  
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
        
        virtual size_t                getSamples(SampleType* outputBuffer, size_t numFramesRequested) = 0;
        
      };
    
      //////////////////////////////////////////
      /// Abstract Interpolator delegate class.
      //////////////////////////////////////////
    
      class Interpolator{
      
      public:
      
        /*!
          Interpolate between input frames. It's up to the caller to determine what the output buffer size will be.
        */
        virtual size_t                process(SampleType* inputBuffer, SampleType* outputBuffer, size_t inputbufferSize,  size_t outputbufferSize, float* pitchScale) = 0;
      
      };
    
      //////////////////////////////////////////
      /// Abstract Low Pass Filter delegate class.
      //////////////////////////////////////////
    
      class LPF{
      
      public:
        
          /*!
            Filter a buffer.
          */
        
          virtual void                process(SampleType* inputBuffer, SampleType* outputBuffer, float* pitchScale, size_t inputbufferSize) = 0;
      
      };

      //////////////////////////////////////////
      /// Renderer class.
      //////////////////////////////////////////

      class Renderer{
      
        public:
        
          /*!
           Quality setting. Shortcuts for pre-defined combinations of filter/interpolator
          */
        
          enum                        Quality {LOW, MEDIUM, HIGH};
        
          /*!
            Constructor
          */
        
          Renderer(float sampleRate, int numChannels, size_t sourceBufferLength = 64, size_t maxFramesToRender = 64);
        
          /*!
            Destructor
          */
          
          ~Renderer();
        
          /*!
            Render samples at the current pitch. Returns the actual number of samples written to the output buffer. 
            If the AudioSource has no more data to supply, the number of frames written may be less than the number of frames requested.
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
            Calclulate how many input frames will be needed to render a given number of output frames, starting with the next call to "render". 
            This calculation is based on the currently scheduled pitch and interpolation. One should always call setPitch _before_ getInputFrameCount
            rather than after. Otherwise the value returned by getInputFrameCount will be invalid.
          */
        
          size_t                      getInputFrameCount(size_t outputFrameCount);
        
          /*!
            Calclulate how many output frames may be rendered using a given number of input frames. 
            This calculation is based on the currently scheduled pitch and interpolation. One should always call setPitch _before_ getOutputFrameCount
            rather than after in order, otherwise the value returned by getInputFrameCount will be invalid.
          */
        
          size_t                      getOutputFrameCount(size_t inputFrameCount);
        
        
          /*!
            Get the number of channels the renderer was configured with.
          */
        
          size_t                      getNumChannels();
        
        
          /*!
            Set the AudioSource delegate object. This MUST be called or there will be no data to resample!
          */
        
          void                        setAudioSource(AudioSource* audioSource);
        
          /*!
            Set the course-grained quality of the renderer. This values of the Quality enum will actually be shortcuts to instantiating 
            different interpolator/filter options. For example, LOW quality might mean linear intorpolation and no low pass filtering.
          */
        
          void                        setQuality(Quality qaulity);
        
          /*!
            "Manually" set the interpolator. This should not be called after the first call to rRenderer::render.
          */
        
          void                        setInterpolator(Interpolator* interpolator);
        
          /*!
            "Manually" set the low pass filter. This should not be called after the first call to rRenderer::render.
          */
        
          void                        setLowPassFilter(LPF* interpolator);
        
          /*!
            Option for custom allocator. This should not be called after the first call to Renderer::render.
          */
        
          void                        setAllocator(void* (size_t));
          
          /*!
            Option for custom deallocator.
          */
        
          void                        setDeAllocator(void (void*));
        
        
        private:
        
        
          void                        error(std::string message); // todo -- don't use std::string. Use error codes instead.

          struct AudioBuffer{
              SampleType*             data;
              size_t                  length;
          };

          void                        calculatePitchForNextFrames(size_t numFrames);
          void                        swapBuffersAndFillNext();
        
          size_t                      mNumChannels;
          float                       mSampleRate; // frames per second
          AudioSource*                mAudioSource;
          float                       mCurrentPitch;
          float                       mPitchDestination;
          float                       mSecondsUntilPitchDestination;
          float*                      mPitchBuffer;
          void*                       (*mAlloc)(size_t); // allocator function
          void                        (*mDealloc)(void*); // deallocator function
          AudioBuffer                 mSourceBuffer[2];
          AudioBuffer*                mCurrentSourceBuffer;
          AudioBuffer*                mNextSourceBuffer;
          size_t                      mMaxSourceBufferLength;
          float                       mSourceBufferReadHead;
          Interpolator*               mInterpolator;
          size_t                      mMaxFramesToRender;
      };
  
}

#endif /* defined(__Resampler__RealtimeResampler__) */
