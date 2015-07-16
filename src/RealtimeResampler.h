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
#include <string> // TODO remove this
#include <cstdlib>


namespace RealtimeResampler {

      typedef float SampleType;
      class Interpolator;
      class Filter;
  
      // allocator / deallocator are malloc and free by default, but can be overridden
      extern void* (*mallocFn)(size_t);
      extern void (*freeFn)(void*);
  
      /*!
        Memory-managed audio buffer class class. Handles allocation and deallocation. 
       
        Note frontPadding and backPadding. They are used when you want to permit "peaking" ahead and behind the
        boundaries of the buffer. In ther words, you can keep a few frames from the previous buffer before postion
        zero, and a few frames from the next buffer after "official" end of the buffer. This allows us to pass the same
        data to different interpolation functions which may have different needs with regard to looking back past the two 
        samples being interpolated.
        
        Also used as a general-purpose heap-allocated float array.
        
        Copy and assignment constructors do NOT copy audio data. Data must be explicitly copied.
        
        I'll admit it. This class is a bit ugly.
       
      */
  
      struct Buffer{
        // constructor
        Buffer(size_t numSamples, size_t frontPadding=0, size_t backPadding=0):
          mNumSamples(frontPadding + numSamples + backPadding),
          mFrontPadding(frontPadding),
          length(0)
        {
          init();
        }
        
        // copy constructor
        Buffer(const Buffer &other):
          mNumSamples(other.mNumSamples),
          mFrontPadding(other.mFrontPadding),
          length(other.length)
        {
          init();
        }
        
        // assignment operator
        Buffer& operator= (const Buffer& other){
          mNumSamples = other.mNumSamples;
          length = other.length;
          mFrontPadding = other.mFrontPadding;
          init();
          return *this;
        }
        ~Buffer(){  freeFn(mData); }
        
        // The current length of the "actual" data, in frames. This doesn't include padding.
        // The buffer doesn't know if it's stereo or not, so this may or may not equal mNumSamples
        size_t                  length;
        
        // the number of samples the allocated space will hold, minus the padding
        size_t                  mNumSamples;
        
        size_t                  mFrontPadding;
        
        // mData, offset by frontPadding. The "official" start of the buffer
        SampleType*             start;
        
        void                    init(){
          size_t bytes = (mNumSamples + mFrontPadding) * sizeof(SampleType);
          mData = (SampleType*)(*mallocFn)(bytes);
          start = mData + mFrontPadding;
          memset(mData, 0, bytes);
        }
      
        // All of the memory owned by this object
        SampleType*             mData;
      };
  
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
           Quality setting. Shortcuts for pre-defined combinations of filter/interpolator
          */
        
          enum                        Quality {LOW, MEDIUM, HIGH};
        
          /*!
            Constructor
          */
        
          // TODO -- consider creating an initializer struct instead of all these arguments;
        
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
            Set the course-grained quality of the renderer. The values of the Quality enum will actually be shortcuts to instantiating 
            different interpolator/filter options. For example, LOW quality might mean linear intorpolation and no low pass filtering.
          */
          // TODO -- implement or remove this
          void                        setQuality(Quality qaulity);
        
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
          void                        error(std::string message); // todo -- don't use std::string. Use error codes instead.
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
