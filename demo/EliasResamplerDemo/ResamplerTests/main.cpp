//
//  main.cpp
//  ResamplerTests
//
//  Created by Morgan Packard on 4/5/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#include <iostream>
#include "RealtimeResampler.h"
#include <stdio.h>
#include "Interpolator.h"
#include <cmath>
#include <iomanip>

using namespace RealtimeResampler;
using namespace std;

static const float kSampleRate = 44100;

#define TEST_EQ(a, b, error){ int lineNumber = __LINE__;    \
auto actual = a; \
auto expected = b; \
if(!(actual == expected)){                     \
  std::cout << "Test failed at line " << lineNumber << ". " << error << " Expected " << expected << " got " <<  actual << std::endl; \
}}  

#define TEST_TRUE(value, error){ int lineNumber = __LINE__;    \
if(!(value)){                     \
  std::cout << "Test failed at line " << lineNumber << ". " << error << " Expected value to be true" << std::endl; \
}}  


// Audio source for test data

class AudioSourceImpl : public AudioSource{
public:
  size_t numFramesToProvide;
  size_t valueToWriteLength;
  SampleType* valueToWrite;
  int readHead;
  size_t getSamples(SampleType* outputBuffer, size_t numFramesRequested, int numChannels){
    for (int i = 0; i < numFramesToProvide * numChannels; i++) {
        outputBuffer[i] = valueToWrite[readHead++];
}
    if (readHead > valueToWriteLength) {
      printf("AudioSourceImpl error -- eadHead > valueToWriteLength)\n");
    }
    return numFramesToProvide;
  }
};

// Wrapper object to test equality with audio buffers

class BufferTestWrapper{
public:
  BufferTestWrapper(SampleType* buffer, size_t samples):mBuffer(buffer),mSamples(samples){};
  bool operator==(const BufferTestWrapper& rhs) const{
     return (mSamples == rhs.mSamples)
     && ( memcmp(mBuffer, rhs.mBuffer, mSamples * sizeof(SampleType)) == 0 );
  }

  size_t mSamples;
  SampleType* mBuffer;
};

std::ostream& operator<<(std::ostream &strm, const BufferTestWrapper& wrapper) {
  for (int i = 0; i < wrapper.mSamples; i++) {
      if (i % 8 == 0) {
        strm << std::endl;
      }
      strm  << std::setprecision(8) << wrapper.mBuffer[i] << " ";

  }
  return strm << std::endl;
}


static void debugFreeFn(void* ptr){
  std::cout << "debugFreeFn on ptr: " << ptr << std::endl;
  free(ptr);
};

static void* debugMallocFn(size_t size){
  void* ptr = malloc(size);
  std::cout << "debugMallocFn of ptr: " << ptr << std::endl;
  return ptr;
};



int main(int argc, const char * argv[]) {
  
//    mallocFn = debugMallocFn;
//    freeFn = debugFreeFn;
  
    static const int kNumChannels = 2;
  
    auto renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
  
    Renderer testRenderer(kSampleRate,  kNumChannels, 64);
  
    testRenderer = renderer;
  
    ///////////////////////////////////////
    // Test Buffer class
    ///////////////////////////////////////
  
    const size_t kTestBufferSize = 132;
  
    Buffer buf(kTestBufferSize);
  
    buf.data[kTestBufferSize -1 ] = 0;
  
    ///////////////////////////////////////
    // Test Renderer::getInputFrameCount
    ///////////////////////////////////////
  
    renderer.setPitch(1, 1, 0);
  
    TEST_TRUE( abs(renderer.getInputFrameCount(100) - 100.0f) <= 1 , "InputFramecount at pitch == 1 failed")
  
    
    renderer.setPitch(2, 2, 0);
  
    TEST_TRUE( abs(renderer.getInputFrameCount(100) - 200.0f) <= 1 , "InputFramecount at pitch == 2 failed")
  
    renderer.setPitch(1, 2, 0);
  
    TEST_TRUE( abs(renderer.getInputFrameCount(100) - 200.0f) <= 1 , "InputFramecount at pitch == 2 with instant glide failed")
  
    renderer.setPitch(1, 2, 1);
    
    TEST_EQ( renderer.getInputFrameCount(kSampleRate), kSampleRate * 1.5 , "InputFramecount at pitch == 2 with one second glide failed")
  
  
    renderer.setPitch(1, 2, 1);
    
    TEST_EQ( renderer.getInputFrameCount(kSampleRate  + kSampleRate ), kSampleRate * 1.5 + kSampleRate * 2 , "InputFramecount at pitch == 2 with one second glide two second render failed")
      
    renderer.setPitch(1, 3, 2);

    TEST_EQ( renderer.getInputFrameCount(kSampleRate), kSampleRate * 1.5 , "InputFramecount at pitch == 3 with two second glide failed")
  
  
    ///////////////////////////////////////
    // Test Renderer::getOutputFrameCount
    ///////////////////////////////////////
  
    renderer.setPitch(1, 1, 0);
  
    TEST_EQ( renderer.getOutputFrameCount(kSampleRate), kSampleRate  , "getOutputFrameCount at pitch == 1 with no glide failed")
  
    renderer.setPitch(1, 2, 0);
  
    TEST_EQ( renderer.getOutputFrameCount(200), 100 , "getOutputFrameCount at pitch == 2 with instant glide failed")
  
  
    renderer.setPitch(1, 2, 1);
  
    TEST_EQ( renderer.getOutputFrameCount(kSampleRate * 1.5), kSampleRate   , "getOutputFrameCount at pitch == 2 with one second glide failed")
  
    renderer.setPitch(1, 2, 1);
    
    TEST_EQ( renderer.getOutputFrameCount( kSampleRate * 1.5 + kSampleRate * 2   ), kSampleRate  + kSampleRate , "getOutputFrameCount at pitch == 2 with one second glide two second render failed")
  
  
    renderer.setPitch(1, 3, 2);

    TEST_EQ( renderer.getOutputFrameCount(kSampleRate * 1.5), kSampleRate  , "getOutputFrameCount at pitch == 3 with two second glide failed")
  
  
    ///////////////////////////////////////
    // Test Renderer returns fewer frames when audio source provides fewer
    ///////////////////////////////////////
  
  
    // set up source for test audio data
    AudioSourceImpl audioSource;
    const int kNumFramesInAudioSourceBuffer = 1000;
    audioSource.valueToWrite = (SampleType*)malloc(kNumFramesInAudioSourceBuffer * kNumChannels * sizeof(SampleType));
    for (int i = 0; i < (64 * 4 ) * kNumChannels; i++ ) {
        audioSource.valueToWrite[i] =  i ; //sin(i / 100.0f);
    }
    audioSource.valueToWriteLength = kNumFramesInAudioSourceBuffer * kNumChannels;
    auto sourceFramesWrapper = BufferTestWrapper(audioSource.valueToWrite, 100);
    
    TEST_EQ(sourceFramesWrapper, sourceFramesWrapper, "The source frames should equal itself");
  
  
  
  
    // set up destination buffer
    const int kNumFramesInDestinationBuffer = kNumFramesInAudioSourceBuffer * 2;
    SampleType* destinationBuffer = (SampleType*)malloc(sizeof(SampleType) * kNumChannels * kNumFramesInDestinationBuffer );
    auto destinationFramesWrapper = BufferTestWrapper(destinationBuffer, 100);
  
  
    auto wipeDestinationBuffer = [=](){ memset(destinationBuffer, 0, sizeof(SampleType) * kNumChannels * kNumFramesInDestinationBuffer); };
    wipeDestinationBuffer();
  
  
  
  
    // Test linear interpolator with AudioSource returning as many samples as requested
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
  
    audioSource.readHead = 0;
    audioSource.numFramesToProvide = 64;
  
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide, "The renderer should render 64 frames");
    TEST_EQ(destinationFramesWrapper, sourceFramesWrapper, "The output frames should be the same as the input frames");
  
    // Test linear interpolator with AudioSource returning fewer samples than requested
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
  
    audioSource.numFramesToProvide = 60;
    
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide, "The renderer should render 60 frames") ;
  
  
  
  
    // Test one octave up. Should return half as many samples as supplied
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
    renderer.setPitch(2, 2, 0);
  
    audioSource.numFramesToProvide = 60;
    
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide / 2, "The renderer should render 30 frames") ;
  
    std::cout << "\n ======== Tests Completed =========== \n\n";
  
    return 0;
}











