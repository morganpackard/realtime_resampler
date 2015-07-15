//
//  RealtimeResamplerInterpolator.cpp
//
//  Created by Morgan Packard with encouragement and guidance from Philip Bennefall on 6/12/15.
//

#include "RealtimeResamplerInterpolator.h"

namespace RealtimeResampler{
  
  //////////////////////////////////////////
  /// Linear Interpolator
  //////////////////////////////////////////
  
  
  void LinearInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop){
    
    for (int i = 0; i < numFrames; i++) {
    
      int integerPartOfInterpolationBuffer = (int)interpolationBuffer[i];
      SampleType interpolationCoefficient = interpolationBuffer[i] - integerPartOfInterpolationBuffer;
    
      // The first frame of the interpolated pair
      int sampleIndex1 = integerPartOfInterpolationBuffer * hop;
      
      // The second frame of the interpolated pair.
      int sampleIndex2 = sampleIndex1 + hop;
      
      SampleType sample1 = inputBuffer[sampleIndex1];
      SampleType sample2 = inputBuffer[sampleIndex2];
      
      outputBuffer[i * hop] = sample1 + (sample2 - sample1) * interpolationCoefficient ;
      
    }
  }
 
  //////////////////////////////////////////
  /// Watte tri-linear Interpolator
  //////////////////////////////////////////
 
  void WatteTrilinearInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop){
    
    SampleType frame0Sample, frame1Sample, frame2Sample, frame3Sample;
    
    for (int i = 0; i < numFrames; i++) {
    
      int interpPosition = (int)interpolationBuffer[i];
      SampleType t = interpolationBuffer[i] - interpPosition;
    
      frame0Sample = inputBuffer[ (interpPosition - 1) * hop];
      frame1Sample = inputBuffer[ (interpPosition)  * hop];
      frame2Sample = inputBuffer[ (interpPosition + 1) * hop];
      frame3Sample = inputBuffer[ (interpPosition + 2) * hop];
  
      // 4-point, 2nd-order Watte tri-linear (x-form)
      float ym1py2 = frame0Sample + frame3Sample;
      float c0 = frame1Sample;
      float c1 = 3/2.0*frame2Sample - 1/2.0*(frame1Sample+ym1py2);
      float c2 = 1/2.0*(ym1py2-frame1Sample-frame2Sample);
      outputBuffer[i * hop] = (c2*t+c1)*t+c0;

    }
  }
  
  //////////////////////////////////////////
  /// Hermite Interpolator
  //////////////////////////////////////////
  
  void HermiteInterpolator::process(SampleType* inputBuffer, SampleType* outputBuffer, SampleType* interpolationBuffer, size_t numFrames, int hop){
  
    SampleType frame0Sample, frame1Sample, frame2Sample, frame3Sample;
    
    for (int i = 0; i < numFrames; i++) {
    
      int interpPosition = (int)interpolationBuffer[i];
      SampleType t = interpolationBuffer[i] - interpPosition;
      
      frame0Sample = inputBuffer[ (interpPosition - 1) * hop];
      frame1Sample = inputBuffer[ (interpPosition)  * hop];
      frame2Sample = inputBuffer[ (interpPosition + 1) * hop];
      frame3Sample = inputBuffer[ (interpPosition + 2) * hop];
    
      float c0 = frame1Sample;
      float c1 = .5F * (frame2Sample - frame0Sample);
      float c2 = frame0Sample - (2.5F * frame1Sample) + (2 * frame2Sample) - (.5F * frame3Sample);
      float c3 = (.5F * (frame3Sample - frame0Sample)) + (1.5F * (frame1Sample - frame2Sample));
      outputBuffer[i * hop] = (((((c3 * t) + c2) * t) + c1) * t) + c0;

    }
  }

 

}