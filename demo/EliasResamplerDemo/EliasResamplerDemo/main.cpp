//
//  main.cpp
//  EliasResamplerDemo
//
//  Created by Morgan Packard on 3/17/15.
//

#include <iostream>
#include "Tonic.h"
#include "RtAudio.h"

using namespace Tonic;

const unsigned int nChannels = 2;

// Static smart pointer for our Synth
static Synth synth;

int renderCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
        double streamTime, RtAudioStreamStatus status, void *userData )
{
    synth.fillBufferOfFloats((float*)outputBuffer, nBufferFrames, nChannels);
    return 0;
}

int main(int argc, const char * argv[])
{
    // Configure RtAudio
    RtAudio dac;
    RtAudio::StreamParameters rtParams;
    rtParams.deviceId = dac.getDefaultOutputDevice();
    rtParams.nChannels = nChannels;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 512; // 512 sample frames
    
    // You don't necessarily have to do this - it will default to 44100 if not set.
    Tonic::setSampleRate(sampleRate);
    
    // --------- MAKE A SYNTH HERE -----------
        

    SampleTable sample = loadAudioFile("/junk from desktop/sounds/think_lc_tambourine.aif");
    //SampleTable sample = loadAudioFile("/junk from desktop/sounds/bass/MINIMOOG/BMM02B.WAV");

    BufferPlayer player;
    player
      .setBuffer(sample)
      .trigger(ControlTrigger().trigger())
      .playbackRate(1 + SineWave().freq(0.1) * 0.5)
      .loop(true);


    synth.setOutputGen(player);
    
    // ---------------------------------------
    
    
    // open rtaudio stream
    try {
        dac.openStream( &rtParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &renderCallback, NULL, NULL );
        
        dac.startStream();
        
        // hacky, yes, but let's just hang out for awhile until someone presses a key
        printf("\n\nPress Enter to stop\n\n");
        cin.get();
        
        dac.stopStream();
    }
    catch ( RtError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        exit( 0 );
    }
    
    return 0;
}
