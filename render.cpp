/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
	Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
	Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/

#include <Bela.h>
// #include <Gamma/Oscillator.h>
// #include <Gamma/Envelope.h>
#include <Gamma/Delay.h>
#include <Gamma/Spatial.h>

float gPositivePeak = 0;
float gNegativePeak = 0;
const float gDecayConstant = 0.999;
bool gIsPositive = false;

// gam::SamplePlayer<float, gam::ipl::Cubic, gam::phsInc::Loop> player;
// gam::Accum<> tmr{2};
// gam::AD<> env{0, 0.1};
// gam::Sine<> sine{440};
gam::Delay<> delay{2, 0.4};
gam::ReverbMS<> reverb;
gam::OnePole<> lpf;
// gam::Domain myDomain(10);
gam::EchoCSine<> cecho(1);

bool setup(BelaContext *context, void *userData)
{
	gam::sampleRate(context->audioSampleRate);
	cecho.decay(64);
	//cecho.fbkFreq(4);
	cecho.fbkFreq(0, 1./6);
	// myDomain << sine;
	lpf.freq(1000);
	reverb.resize(gam::FREEVERB);
	reverb.decay(5);
	reverb.damping(0.2);
	//player.load("/root/Bela/examples/terminal-only/samples/sample.wav");
	return true;
}

void render(BelaContext *context, void *userData)
{
	for(unsigned int n = 0; n < context->audioFrames; ++n)
	{
		float inSample = audioRead(context, n, 0);
		if(inSample > gPositivePeak) {
			gPositivePeak = inSample;
		}
		else {
			gPositivePeak *= gDecayConstant;
		}

		if(inSample < gNegativePeak) {
			gNegativePeak = inSample;
		}
		else {
			gNegativePeak *= gDecayConstant;
		}		
		
		// if(++gPrintCounter >= 4410) {
		// 	gPrintCounter = 0;
		// 	rt_printf("%f %f %f %f\n", gPositivePeak, gNegativePeak, audioRead(context, n, 0), audioRead(context, n, 1));
		// }
		
		if(gIsPositive && fabsf(gNegativePeak) > fabsf(gPositivePeak) * 1.1)
			gIsPositive = false;
		else if(fabsf(gPositivePeak) > fabsf(gNegativePeak) * 1.1)
			gIsPositive = true;
		
		// if(!gIsPositive) {
		// 	audioWrite(context, n, 0, inSample);
		// 	audioWrite(context, n, 1, inSample);
		// }
		// else {
		// 	audioWrite(context, n, 0, 0);
		// 	audioWrite(context, n, 1, 0);			
		// }		
		
		
		// if(tmr()){
		// 	env.reset();
		// 	// player.reset();
		// }
		float pot = analogRead(context, n, 0) / 0.82f + 0.05f;
		pot = lpf(pot);
		// player.rate(-pot);
		// auto value = player();
		// float value = 0;
		// value = sine() * env() * 0.2f;
		float value = audioRead(context, n, 0);
		//delay.delay(pot);
		//auto dval = delay();
		//value += delay(value + dval * 0.4f) * 0.5f;
		//value += 0.2f * reverb(value);
		cecho.fbkFreq(pot*pot*10.);
		cecho.delay(0.3);
		
		if(gIsPositive)
			value += cecho(value).r*0.5;
		else
			value += cecho(0).r*0.5;
			
		for(unsigned int ch = 0; ch < context->audioOutChannels; ++ch)
		{
			audioWrite(context, n, ch, value);
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{
	printf("clean me\n");
}


/**
\example sinetone/render.cpp

Producing your first bleep!
---------------------------

This sketch is the hello world of embedded interactive audio. Better known as bleep, it 
produces a sine tone.

The frequency of the sine tone is determined by a global variable, `gFrequency`. 
The sine tone is produced by incrementing the phase of a sin function 
on every audio frame.

In render() you'll see a nested for loop structure. You'll see this in all Bela projects. 
The first for loop cycles through 'audioFrames', the second through 'audioChannels' (in this case left 0 and right 1). 
It is good to familiarise yourself with this structure as it's fundamental to producing sound with the system.
*/
