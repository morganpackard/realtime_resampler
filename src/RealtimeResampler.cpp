//
//  RealtimeResampler.cpp
//  Resampler
//
//  Created by Morgan Packard on 2/22/15.
//  Copyright (c) 2015 Morgan Packard. All rights reserved.
//

#include "RealtimeResampler.h"


namespace RealtimeResampler {
  
  Renderer::Renderer(float sampleRate, int numChannels) : mNumChannels(numChannels){
  
  }

  size_t Renderer::getNumChannels(){
    return mNumChannels;
  }

  size_t Renderer::render(SampleType* outputBuffer, size_t numFramesRequested){
    return mAudioSource->getSamples(outputBuffer, numFramesRequested);
  }
  
  void Renderer::setAudioSource(AudioSource* audioSource){
    mAudioSource = audioSource;
  }

}
