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

#ifndef __EliasResamplerDemo__RealtimeResamplerBuffer__
#define __EliasResamplerDemo__RealtimeResamplerBuffer__

#include <stdio.h>
#include "RealtimeResamplerCommon.h"

namespace RealtimeResampler {

        /*!
        Memory-managed audio buffer class class. Handles allocation and deallocation. 
       
        Note frontPadding and backPadding. They are used when you want to permit "peaking" ahead and behind the
        boundaries of the buffer. In ther words, you can keep a few frames from the previous buffer before postion
        zero, and a few frames from the next buffer after "official" end of the buffer. This allows us to pass the same
        data to different interpolation functions which may have different needs with regard to looking back past the two 
        samples being interpolated.
        
        Also used as a general-purpose heap-allocated float array.
        
        Copy and assignment constructors do NOT copy audio data. Data must be explicitly copied.
       
      */
  
      class Buffer{
      public:
        // Constructor. Front padding and back padding arguments are in frames
        Buffer(size_t numFrames, size_t numChannels, size_t frontPadding=0, size_t backPadding=0);
        
        ~Buffer();
        
        // copy constructor
        Buffer(const Buffer &other);
        
        // assignment operator
        Buffer& operator= (const Buffer& other);
        
        SampleType*             getDataPtr();
        
        SampleType*             getStartPtr();
        
        // The current length of the "actual" data, in frames. This doesn't include padding.
        // This is not set by the buffer. It's up to whoever's using the buffer, filling it with data, etc,
        // to set the length appropriately
        size_t                  length;
        
        //  set all the samples to zero
        void                    clear();
        
      protected:
      
        
        // the number of samples the allocated space will hold, minus the padding
        size_t                  mNumSamples;
        
        // the number of SAMPLES of front padding. Frames * numChannels
        size_t                  mFrontPadding;
      
        void                    init();
        // All of the memory owned by this object
        SampleType*             mData;
        
        // mData, offset by frontPadding. The "official" start of the buffer
        SampleType*             start;
        
      };

}

#endif /* defined(__EliasResamplerDemo__RealtimeResamplerBuffer__) */
