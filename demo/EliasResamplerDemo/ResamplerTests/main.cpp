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
using namespace std;

static const float kSampleRate = 44100;

#define TEST_EQ(a, b, error){ int lineNumber = __LINE__;    \
if(a != b){                     \
  cout << "Test failed at line " << lineNumber << ". " << error << " Expected " << b << " got " <<  a << endl; \
}}                               \

int main(int argc, const char * argv[]) {
  
  
    auto renderer = RealtimeResampler::Renderer(kSampleRate,  2, kSampleRate * 10);
  
  
    renderer.setPitch(1, 1, 0);
  
    TEST_EQ( renderer.getInputFrameCount(100), 100 , "InputFramecount at pitch == 1 failed")
  
    
    renderer.setPitch(2, 2, 0);
  
    TEST_EQ( renderer.getInputFrameCount(100), 200 , "InputFramecount at pitch == 2 failed")
  
  
    renderer.setPitch(1, 2, 0);
  
    TEST_EQ( renderer.getInputFrameCount(100), 200 , "InputFramecount at pitch == 2 with instant glide failed")
  
    renderer.setPitch(1, 2, 1);
    
    TEST_EQ( renderer.getInputFrameCount(kSampleRate), kSampleRate * 1.5 , "InputFramecount at pitch == 2 with one second glide failed")
  
  
    renderer.setPitch(1, 2, 1);
    
    TEST_EQ( renderer.getInputFrameCount(kSampleRate  + kSampleRate ), kSampleRate * 1.5 + kSampleRate * 2 , "InputFramecount at pitch == 2 with one second glide two second render failed")
      
    renderer.setPitch(1, 3, 2);

    TEST_EQ( renderer.getInputFrameCount(kSampleRate), kSampleRate * 1.5 , "InputFramecount at pitch == 2 with one second glide failed")
  
    return 0;
}
