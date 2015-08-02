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

#include "RealtimeResamplerBuffer.h"
#include "RealtimeResampler.h"

namespace RealtimeResampler{

        Buffer::Buffer(size_t numFrames, size_t numChannels, size_t frontPadding, size_t backPadding):
          mNumSamples((frontPadding + numFrames + backPadding) * numChannels ),
          mFrontPadding(frontPadding * numChannels),
          length(0)
        {
          init();
        }
  
        Buffer::Buffer(const Buffer &other):
          mNumSamples(other.mNumSamples),
          mFrontPadding(other.mFrontPadding),
          length(other.length)
        {
          init();
        }
  
        Buffer& Buffer::operator= (const Buffer& other){
          mNumSamples = other.mNumSamples;
          length = other.length;
          mFrontPadding = other.mFrontPadding;
          init();
          return *this;
        }
  
         Buffer::~Buffer(){  freeFn(mData); }
  
        void Buffer::init(){
          size_t bytes = (mNumSamples + mFrontPadding) * sizeof(SampleType);
          mData = (SampleType*)(*mallocFn)(bytes);
          start = mData + mFrontPadding;
          memset(mData, 0, bytes);
        }
  
        void Buffer::clear(){
          memset(mData, 0, mNumSamples * sizeof(SampleType));
        }

        SampleType* Buffer::getDataPtr(){
          return mData;
        }
  
        SampleType* Buffer::getStartPtr(){
          return start;
        }

}
