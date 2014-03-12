/*
Copyright 2013 The MITRE Corporation, All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this work except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/* //device/servers/AudioFlinger/AudioFakeHardware.cpp
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/* //device/servers/AudioFlinger/AudioFakeHardware.cpp
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


// MOCSI - MITRE 2012
// Dylan Ladwig <dladwig@mitre.org>, <dylan.ladwig@gmail.com>
// Updates Feb. 2014


#define LOG_TAG "LibMOCSIAudio"

#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/String8.h>
#include <cutils/log.h>

#include "AudioFakeHardware.h"
#include <media/AudioRecord.h>

namespace android_audio_legacy {

// ----------------------------------------------------------------------------

#define BUFFER_SIZE 65536

extern "C" {
	int fd=-1;
	unsigned short *position;
	unsigned short *data;

	int infd=-1;
	unsigned short *inposition;
	unsigned short *indata;

	void setupBuffer() {
		if(fd>0) {
			ALOGW("Tried to open buffer twice!");
			return; // already made
		}

		fd = open("/dev/audio/audio_loop", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
		if(fd<0) {
			ALOGE("Could not open audio loop buffer!");
			return;
		}
			
		ALOGE("successfully opened audio loop buffer in /dev/audio/audio_loop");
	

		// expand file to correct size
		lseek(fd, sizeof(unsigned short)*(BUFFER_SIZE+1)-1, SEEK_SET);
		write(fd, " ", 1);

		// mmap
		position = (unsigned short*) mmap(NULL, sizeof(unsigned short)*(BUFFER_SIZE+1), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		if(!position)
			ALOGE("Could not MMAP audio loop buffer!");

		ALOGI("Opened audio loop successfully.");

		data = position+sizeof(unsigned short);
	}

	void setupInBuffer() {
			if(infd>0) {
				ALOGW("Tried to open buffer twice!");
				return; // already made
			}

			infd = open("/dev/audio/audio_loop", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
			if(infd<0) {
				ALOGE("Could not open audio loop buffer!");
				return;
			}

			ALOGE("successfully opened audio loop buffer in /dev/audio/audio_loop");


			// expand file to correct size
			lseek(infd, sizeof(unsigned short)*(BUFFER_SIZE+1)-1, SEEK_SET);
			write(infd, " ", 1);

			// mmap
			inposition = (unsigned short*) mmap(NULL, sizeof(unsigned short)*(BUFFER_SIZE+1), PROT_READ|PROT_WRITE, MAP_SHARED, infd, 0);
			if(!inposition)
				ALOGE("Could not MMAP audio loop buffer!");

			ALOGI("Opened audio loop successfully.");

			indata = inposition+sizeof(unsigned short);
		}


	void readFromBuffer(int length, unsigned short *out) {

		int pos = *inposition, i;
		for (i=0; i< length; i++, pos = (pos+1)%BUFFER_SIZE){
			out[i]=indata[pos];
		}
		*inposition = pos;
	}

	void writeToBuffer(int length, unsigned short *in) {
		if(fd<0||!position)
			return;

		int pos = *position, i;

		for(i=0;i<length;i++,pos=(pos+1)%BUFFER_SIZE) {
			data[pos]=in[i];
		}
		*position=pos;
	}

}

AudioFakeHardware::AudioFakeHardware() : mMicMute(false)
{
}

AudioFakeHardware::~AudioFakeHardware()
{
}

status_t AudioFakeHardware::initCheck()
{
    return NO_ERROR;
}

AudioStreamOut* AudioFakeHardware::openOutputStream(
        uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status)
{
    AudioAACStreamOut* out = new AudioAACStreamOut();
    status_t lStatus = out->set(format, channels, sampleRate);
    if (status) {
        *status = lStatus;
    }
    if (lStatus == NO_ERROR) {
		ALOGI("Makin' an output...");
        return out;
	}
    delete out;
    return 0;
}

void AudioFakeHardware::closeOutputStream(AudioStreamOut* out)
{
	ALOGI("Gettin' rid of an output...");
    delete out;
}

AudioStreamIn* AudioFakeHardware::openInputStream(
        uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate,
        status_t *status, AudioSystem::audio_in_acoustics acoustics)
{
	AudioAACStreamIn* in = new AudioAACStreamIn();
	status_t lStatus = in->set(format, channels, sampleRate);

	if (status) {
		*status = lStatus;
	}
	if (lStatus == NO_ERROR) {
		//ALOGI("Makin' an output...");
		return in;
	}
	delete in;
	return 0;
}

void AudioFakeHardware::closeInputStream(AudioStreamIn* in)
{
	delete in;
}

status_t AudioFakeHardware::setVoiceVolume(float volume)
{
    return NO_ERROR;
}

status_t AudioFakeHardware::setMasterVolume(float volume)
{
    return NO_ERROR;
}

status_t AudioFakeHardware::dumpInternals(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioFakeHardware::dumpInternals\n");
    snprintf(buffer, SIZE, "\tmMicMute: %s\n", mMicMute? "true": "false");
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioFakeHardware::dump(int fd, const Vector<String16>& args)
{
    dumpInternals(fd, args);
    return NO_ERROR;
}

// ----------------------------------------------------------------------------

status_t AudioAACStreamOut::set(int *pFormat, uint32_t *pChannels, uint32_t *pRate)
{
    if (pFormat) *pFormat = format();
    if (pChannels) *pChannels = channels();
    if (pRate) *pRate = sampleRate();

    return NO_ERROR;
}

status_t AudioAACStreamIn::set(int *pFormat, uint32_t *pChannels, uint32_t *pRate)
{
    if (pFormat) *pFormat = format();
    if (pChannels) *pChannels = channels();
    if (pRate) *pRate = sampleRate();

    return NO_ERROR;
}

unsigned int  AudioAACStreamIn::getInputFramesLost() const
{
	return 0;
}

ssize_t AudioAACStreamIn::read(void* buffer, ssize_t bytes)
{
	struct timeval end;
	gettimeofday(&end, NULL);
	int diff = end.tv_usec-last.tv_usec+1000000ll*(end.tv_sec-last.tv_sec);
	if(diff>250000||diff<0)	{ // 1/4sec
		ALOGI("Resetting time; had %d diff\n", diff);
		gettimeofday(&last, NULL);
	}
	readFromBuffer(bytes/sizeof(unsigned short),(unsigned short*)buffer);	// bytes/2 is because I care about shorts, not bytes

	// advance the time time by a frame
	last.tv_usec += (bytes * 1000000ll) / sizeof(int16_t) / AudioSystem::popCount(channels()) / sampleRate();
	if(last.tv_usec>1000000ll) {
		last.tv_sec++;
		last.tv_usec-=1000000;
	}

	diff = last.tv_usec-end.tv_usec+1000000ll*(last.tv_sec-end.tv_sec);

	if(diff>0)	// If it's positive, sleep the difference between the expected end time and the actual end time
		usleep(diff);

	return bytes;

}


ssize_t AudioAACStreamOut::write(const void* buffer, size_t bytes)
{
	struct timeval end;

	// check if the stored start time is more than .25sec ago - if so, toss it out
	gettimeofday(&end, NULL);
	int diff = end.tv_usec-time.tv_usec+1000000ll*(end.tv_sec-time.tv_sec);
	if(diff>250000||diff<0)	{ // 1/4sec
		ALOGI("Resetting time; had %d diff\n", diff);
		gettimeofday(&time, NULL);
	}

	writeToBuffer(bytes/sizeof(unsigned short),(unsigned short*)buffer);	// bytes/2 is because I care about shorts, not bytes
	gettimeofday(&end, NULL);    

	// advance the time time by a frame
	time.tv_usec += (bytes * 1000000ll) / sizeof(int16_t) / AudioSystem::popCount(channels()) / sampleRate();
	if(time.tv_usec>1000000ll) {
		time.tv_sec++;
		time.tv_usec-=1000000;
	}

	diff = time.tv_usec-end.tv_usec+1000000ll*(time.tv_sec-end.tv_sec);

	if(diff>0)	// If it's positive, sleep the difference between the expected end time and the actual end time
	    usleep(diff);

    return bytes;
}


status_t AudioAACStreamOut::standby()
{
    return NO_ERROR;
}

status_t AudioAACStreamOut::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "AudioAACStreamOut::dump\n");
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

String8 AudioAACStreamOut::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    return param.toString();
}

status_t AudioAACStreamOut::getRenderPosition(uint32_t *dspFrames)
{
    return INVALID_OPERATION;
}



//------------------------------------------------------------------------------
//  Factory
//------------------------------------------------------------------------------

extern "C" AudioHardwareInterface* createAudioHardware(void) {
	ALOGI("Creating Audio Hardware");
	setupBuffer();
	setupInBuffer(); // fake microphone
    return new AudioFakeHardware();
}


// ----------------------------------------------------------------------------

}; // namespace android
