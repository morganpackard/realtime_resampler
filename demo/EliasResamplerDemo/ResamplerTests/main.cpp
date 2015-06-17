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

using namespace RealtimeResampler;

static const float kSampleRate = 44100;

#define TEST_EQ(a, b, error){ int lineNumber = __LINE__;    \
auto actual = a; \
auto expected = b; \
if(actual != expected){                     \
  std::cout << "Test failed at line " << lineNumber << ". " << error << " Expected " << expected << " got " <<  actual << std::endl; \
}}  

#define TEST_TRUE(value, error){ int lineNumber = __LINE__;    \
if(!(value)){                     \
  std::cout << "Test failed at line " << lineNumber << ". " << error << " Expected value to be true" << std::endl; \
}}                              \

int main(int argc, const char * argv[]) {
  
    static const int kNumChannels = 2;
  
    auto renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
  
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
  
    renderer.setPitch(1, 1, 0);
  
    class AudioSourceImpl : public AudioSource{
    public:
      size_t numFramesToProvide;
      size_t getSamples(SampleType* outputBuffer, size_t numFramesRequested, int numChannels){
          return numFramesToProvide;
      }
    };
  
    AudioSourceImpl audioSource;
  
    SampleType* destinationBuffer = (SampleType*)malloc(sizeof(SampleType) * kNumChannels * 1000 );
  
    renderer.setAudioSource(&audioSource);
  
    audioSource.numFramesToProvide = 64;
  
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide, "The renderer should render 64 frames") ;
  
    audioSource.numFramesToProvide = 60;
    
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide, "The renderer should render 60 frames") ;
  
    return 0;
}











