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

  const static int NUM_CHANNELS = 2;
  bool loop;
  
  AudioSourceImpl():loop(false){}

  void setSourceBuffer(SampleType* buf, size_t numFrames){
    readHead = 0;
    bufferLength = numFrames;
    valueToWrite = buf;
  }
  
  size_t getbufferLength(){
    return bufferLength;
  }

  size_t getSamples(SampleType* outputBuffer, size_t numFramesRequested, int numChannels){
    size_t numFramesWritten = 0;
    for (int i = 0; i < numFramesRequested && readHead < bufferLength * NUM_CHANNELS; i++) {
      for (int chan = 0; chan < numChannels; chan++) {
        outputBuffer[i * numChannels + chan] = valueToWrite[readHead++];
      }
      numFramesWritten++;
      if (readHead >= bufferLength * NUM_CHANNELS) {
        if(loop){
          readHead = 0;
        }
//        if(!loop){
//          break;
//        }
      }
    }
    
    return numFramesWritten;
  }
  
private:

  size_t bufferLength;
  SampleType* valueToWrite;
  int readHead;
  
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
    SampleType* testBuffer = (SampleType*)malloc(kNumFramesInAudioSourceBuffer * kNumChannels * sizeof(SampleType));

    audioSource.setSourceBuffer(testBuffer, kNumFramesInAudioSourceBuffer);
  
    for (int i = 0; i < (64 * 4 ) * kNumChannels; i++ ) {
        testBuffer[i] =  i ; //sin(i / 100.0f);
    }
    auto sourceFramesWrapper = BufferTestWrapper(testBuffer, 100);
    
    TEST_EQ(sourceFramesWrapper, sourceFramesWrapper, "The source frames should equal itself");
  
  
  
  
    // set up destination buffer
    const int kNumFramesInDestinationBuffer = kNumFramesInAudioSourceBuffer * 2;
    SampleType* destinationBuffer = (SampleType*)malloc(sizeof(SampleType) * kNumChannels * kNumFramesInDestinationBuffer );
    auto destinationFramesWrapper = BufferTestWrapper(destinationBuffer, 100);
  
  
    auto wipeDestinationBuffer = [=](){ memset(destinationBuffer, 0, sizeof(SampleType) * kNumChannels * kNumFramesInDestinationBuffer); };
    wipeDestinationBuffer();
  
  
  
    ///////////////////////////////////////
    // Test linear interpolator with AudioSource returning as many samples as requested
    ///////////////////////////////////////
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
  
    audioSource.setSourceBuffer(testBuffer, 64);
  
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.getbufferLength(), "The renderer should render 64 frames");
    TEST_EQ(destinationFramesWrapper, sourceFramesWrapper, "The output frames should be the same as the input frames");

    ///////////////////////////////////////
    // Test linear interpolator with AudioSource returning fewer samples than requested
    ///////////////////////////////////////
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
  
    audioSource.setSourceBuffer(testBuffer, 60);
    
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.getbufferLength(), "The renderer should render 60 frames") ;
  
  
  
    ///////////////////////////////////////
    // Test one octave up. Should return half as many samples as supplied
    ///////////////////////////////////////
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
    renderer.setPitch(2, 2, 0);
  
    audioSource.setSourceBuffer(testBuffer, 60);
    
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.getbufferLength() / 2, "The renderer should render 30 frames") ;
  
  
    ///////////////////////////////////////
    // Test cubic interpolator with AudioSource returning as many samples as requested
    ///////////////////////////////////////
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new CubicInterpolator());
    renderer.setAudioSource(&audioSource);
  

    audioSource.setSourceBuffer(testBuffer, 64);
  
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.getbufferLength(), "The renderer should render 64 frames");
    TEST_EQ(destinationFramesWrapper, sourceFramesWrapper, "The output frames should be the same as the input frames");
  
    
    ///////////////////////////////////////
    // Test hermite interpolator with AudioSource returning as many samples as requested
     ///////////////////////////////////////
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new HermiteInterpolator());
    renderer.setAudioSource(&audioSource);
  

    audioSource.setSourceBuffer(testBuffer, 64);
  
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.getbufferLength(), "The renderer should render 64 frames");
    TEST_EQ(destinationFramesWrapper, sourceFramesWrapper, "The output frames should be the same as the input frames");
  
    ///////////////////////////////////////
    // Test repeated calls to audiosource
    ///////////////////////////////////////
  
    audioSource.loop = false;
    audioSource.setSourceBuffer(testBuffer, 65);
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
    renderer.setPitch(1, 1, 0);
    renderer.render(destinationBuffer, 64);
    TEST_EQ(renderer.render(destinationBuffer, 64), 1, "The second call to render should return only 1 frame.");
  
    ///////////////////////////////////////
    // Test looping audiosource
    ///////////////////////////////////////
  
    {
      const int TEST_BUF_NUM_FRAMES = 65;
      audioSource.loop = true;
      audioSource.setSourceBuffer(testBuffer, TEST_BUF_NUM_FRAMES);
    
      renderer = Renderer(kSampleRate,  kNumChannels, 64);
      renderer.setInterpolator(new LinearInterpolator());
      renderer.setAudioSource(&audioSource);
      renderer.setPitch(1, 1, 0);
      
      renderer.render(destinationBuffer, 64);
    
      auto sourceFramesWrapper = BufferTestWrapper(testBuffer, 100);
    
      TEST_EQ(sourceFramesWrapper, destinationFramesWrapper, "Buffer mismatch");
      
      renderer.render(destinationBuffer, 64);
      
      TEST_EQ(destinationBuffer[0], testBuffer[ 64 * AudioSourceImpl::NUM_CHANNELS ], "Sample mismatch");
      TEST_EQ(destinationBuffer[2], testBuffer[ 0 ], "Sample mismatch");
      
      TEST_EQ(BufferTestWrapper( destinationBuffer + 2, 63), BufferTestWrapper(testBuffer,  63), "Buffer mismatch");
      
    }
  
    ///////////////////////////////////////
    // Test non-looping mock audiosource implemenationation
    ///////////////////////////////////////
  
    const int TEST_BUF_NUM_FRAMES = 65;
    const int BLOCK_SIZE = 64;
    audioSource.loop = false;
    audioSource.setSourceBuffer(testBuffer, TEST_BUF_NUM_FRAMES);
  
    audioSource.getSamples(destinationBuffer, TEST_BUF_NUM_FRAMES, 2);
    TEST_EQ(audioSource.getSamples(destinationBuffer, TEST_BUF_NUM_FRAMES, 2), 0, "Too many frames returned");
    TEST_EQ(audioSource.getSamples(destinationBuffer, TEST_BUF_NUM_FRAMES, 2), 0, "Too many frames returned");
  
  
    ///////////////////////////////////////
    // Test renderer with non-looping audiosource
    ///////////////////////////////////////
  
    audioSource.loop = false;
    audioSource.setSourceBuffer(testBuffer, TEST_BUF_NUM_FRAMES);
  
    renderer = Renderer(kSampleRate,  kNumChannels, BLOCK_SIZE);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
    renderer.setPitch(1, 1, 0);
  
    // make sure the renderer only returns TEST_BUF_NUM_FRAMES frames
    renderer.render(destinationBuffer, BLOCK_SIZE);
    TEST_EQ(renderer.render(destinationBuffer, BLOCK_SIZE), TEST_BUF_NUM_FRAMES - BLOCK_SIZE, "Too many frames returned");
    TEST_EQ(renderer.render(destinationBuffer, BLOCK_SIZE), 0, "Too many frames returned");
  
    // set a frame to non-zero. This should be zeroed by the renderer;
    destinationBuffer[0] = 1;
    TEST_EQ(renderer.render(destinationBuffer, BLOCK_SIZE), 0, "Too many frames returned");
    TEST_EQ(destinationBuffer[0], 0, "The renderer should zero out unrendered samples");
  
    // Reset the audio source. Again, the renderer should render only TEST_BUF_NUM_FRAMES frames
    audioSource.setSourceBuffer(testBuffer, TEST_BUF_NUM_FRAMES);
    TEST_EQ(renderer.render(destinationBuffer, BLOCK_SIZE), BLOCK_SIZE, "Wrong frame count");
    // make sure the samples are correct
    TEST_EQ(BufferTestWrapper( destinationBuffer , BLOCK_SIZE), BufferTestWrapper(testBuffer,  BLOCK_SIZE), "Buffer mismatch");
  
    TEST_EQ(renderer.render(destinationBuffer, BLOCK_SIZE), TEST_BUF_NUM_FRAMES - BLOCK_SIZE, "Wrong frame count");
    TEST_EQ(BufferTestWrapper( destinationBuffer , 1), BufferTestWrapper(testBuffer + kNumChannels * BLOCK_SIZE,  1), "Buffer mismatch");
  
    TEST_EQ(destinationBuffer[2], 0, "Buffer mismatch");
  
  
    ///////////////////////////////////////
    // Test renderer with a source buffer of size > 2 * BLOCK_SIZE
    ///////////////////////////////////////
  
    
    audioSource.loop = false;
    audioSource.setSourceBuffer(testBuffer, BLOCK_SIZE * 2.5);
  
    renderer.render(destinationBuffer, BLOCK_SIZE);
    TEST_EQ(BufferTestWrapper( destinationBuffer , BLOCK_SIZE), BufferTestWrapper(testBuffer ,  BLOCK_SIZE), "Buffer mismatch");
  
    renderer.render(destinationBuffer, BLOCK_SIZE);
    TEST_EQ(BufferTestWrapper( destinationBuffer , BLOCK_SIZE), BufferTestWrapper(testBuffer + kNumChannels * BLOCK_SIZE,  BLOCK_SIZE), "Buffer mismatch");
  
    renderer.render(destinationBuffer, BLOCK_SIZE);
    TEST_EQ(BufferTestWrapper( destinationBuffer , BLOCK_SIZE / 2), BufferTestWrapper(testBuffer + kNumChannels * BLOCK_SIZE * 2,  BLOCK_SIZE / 2), "Buffer mismatch");
  
    renderer.render(destinationBuffer, BLOCK_SIZE);
    renderer.render(destinationBuffer, BLOCK_SIZE);
    renderer.render(destinationBuffer, BLOCK_SIZE);
    
    audioSource.setSourceBuffer(testBuffer, BLOCK_SIZE * 2.5);
    renderer.render(destinationBuffer, BLOCK_SIZE);
    TEST_EQ(BufferTestWrapper( destinationBuffer , BLOCK_SIZE), BufferTestWrapper(testBuffer ,  BLOCK_SIZE), "Buffer mismatch");
  
    
    
    ///////////////////////////////////////
    // Debug hermite interpolator
     ///////////////////////////////////////
  
     
    audioSource.loop = false;
    audioSource.setSourceBuffer(testBuffer, kNumFramesInAudioSourceBuffer);
  
    renderer = Renderer(kSampleRate,  kNumChannels, BLOCK_SIZE);
    renderer.setAudioSource(&audioSource);
    renderer.setInterpolator(new CubicInterpolator());
    renderer.render(destinationBuffer, BLOCK_SIZE);
    renderer.render(destinationBuffer, BLOCK_SIZE);
    renderer.render(destinationBuffer, BLOCK_SIZE);
  
    std::cout << "\n ======== Tests Completed =========== \n\n";
  
    return 0;
}











