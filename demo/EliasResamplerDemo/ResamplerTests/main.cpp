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
  
    
  
    renderer.setPitch(1, 1, 0);
  
    class AudioSourceImpl : public AudioSource{
    public:
      size_t numFramesToProvide;
      SampleType* valueToWrite;
      int readHead;
      size_t getSamples(SampleType* outputBuffer, size_t numFramesRequested, int numChannels){
        for (int i = 0; i < numFramesToProvide * numChannels; i++) {
            outputBuffer[i] = valueToWrite[readHead++];
        }
        return numFramesToProvide;
      }
    };
  
    const int kNumFramesInAudioSourceBuffer = 1000;
    AudioSourceImpl audioSource;
    audioSource.valueToWrite = (SampleType*)malloc(kNumFramesInAudioSourceBuffer * kNumChannels * sizeof(SampleType));
  
    SampleType* destinationBuffer = (SampleType*)malloc(sizeof(SampleType) * kNumChannels * 1000 );
  
    // Test linear interpolator with AudioSource returning as many samples as requested
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
  
    audioSource.numFramesToProvide = 64;
  
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide, "The renderer should render 64 frames") ;
  
    // Test linear interpolator with AudioSource returning fewer samples than requested
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new LinearInterpolator());
    renderer.setAudioSource(&audioSource);
  
    audioSource.numFramesToProvide = 60;
    
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide, "The renderer should render 60 frames") ;
  
    // Test cubic interpolator with AudioSource returning fewer samples than requested
  
    renderer = Renderer(kSampleRate,  kNumChannels, 64);
    renderer.setInterpolator(new CubicInterpolator());
    renderer.setAudioSource(&audioSource);
  
    audioSource.numFramesToProvide = 60;
  
    for(int i = 0; i < kNumFramesInAudioSourceBuffer; i++){
      audioSource.valueToWrite[i * kNumChannels] = (i % (64 * 2) < 64) ? 0 : 1;
    }
    
    TEST_EQ(renderer.render(destinationBuffer, 64), audioSource.numFramesToProvide, "The renderer should render 60 frames") ;
  
  
    std::cout << "\n ======== Tests Completed =========== \n\n";
  
    return 0;
}











