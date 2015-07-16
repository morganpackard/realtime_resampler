//
//  RealtimeResamplerBuffer.cpp
//
//  Created by Morgan Packard on 7/16/15.
//

#include "RealtimeResamplerBuffer.h"
#include "RealtimeResampler.h"

namespace RealtimeResampler{

        Buffer::Buffer(size_t numSamples, size_t frontPadding, size_t backPadding):
          mNumSamples(frontPadding + numSamples + backPadding),
          mFrontPadding(frontPadding),
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


}
