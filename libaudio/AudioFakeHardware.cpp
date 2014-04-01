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
    
static char const * const kAudioDeviceName = "/dev/audio/audio_loop";

// ----------------------------------------------------------------------------

#define BUFFER_SIZE 65536

extern "C" {
	int fd=-1;
	unsigned short *position;
	unsigned short *data;
	// file locking mmapped files

	int infd=-1;
	unsigned short *inposition;
	unsigned short *indata;
	// fcntl file locking mechanism
	// We use a read lock
	int SetLock(int FD){
		struct flock fl;
		fl.l_type = F_RDLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 0;
		if (fcntl(FD, F_SETLKW,&fl) == -1)
			return -1;
		return 1;
	}
	int GetLock(int FD) {
		struct flock fl;
		fl.l_type = F_RDLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 0;
		if (fcntl(FD, F_GETLK,&fl) == -1)
			return -1;
		// no lock found
		if (fl.l_type == F_UNLCK)
			return 0;
                // lock found
		return 1;
	}
	int UnLock(int FD) {
		struct flock fl;
		fl.l_type = F_UNLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 0;
		if (fcntl(FD, F_SETLKW,&fl) == -1)
			return -1;
		return 1;
	}
	

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

			infd = open("/dev/audio/audio_loop", O_RDWR, S_IRUSR|S_IWUSR);
			if(infd<0) {
				ALOGE("Could not open microphone audio loop buffer!");
				return;
			}

			ALOGE("successfully opened microphone audio loop buffer in /dev/audio/audio_loop");


			// expand file to correct size
			//lseek(infd, sizeof(unsigned short)*(BUFFER_SIZE+1)-1, SEEK_SET);
			//write(infd, " ", 1);

			// mmap
			inposition = (unsigned short*) mmap(NULL, sizeof(unsigned short)*(BUFFER_SIZE+1), PROT_READ|PROT_WRITE, MAP_SHARED, infd, 0);
			if(!inposition)
				ALOGE("Could not MMAP audio loop microphone buffer!");

			ALOGI("Opened audio loop microphone successfully.");

			indata = inposition+sizeof(unsigned short);
			// set initial read lock on buffer
			if (SetLock(infd) == -1) 
                            ALOGI("Error setting fnctl lock for input buffer!\n");
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

AudioFakeHardware::AudioFakeHardware() 
    : mOutput(0), mInput(0), mFd(-1), mMicMute(false)
{
    mFd = ::open(kAudioDeviceName, O_RDWR);
}

AudioFakeHardware::~AudioFakeHardware()
{
    if (mFd >=0) :: close(mFd);
    closeOutputStream((AudioStreamOut *)mOutput);
    closeInputStream((AudioStreamIn *)mInput);
    
}

status_t AudioFakeHardware::initCheck()
{
    //return NO_ERROR;
     if (mFd >= 0) {
        if (::access(kAudioDeviceName, O_RDWR) == NO_ERROR)
            return NO_ERROR;
    }
    return NO_INIT;
}

AudioStreamOut* AudioFakeHardware::openOutputStream(
        uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status)
{
    
    AutoMutex lock(mLock);
    
     // only one output stream allowed
    if (mOutput) {
        if (status) {
            *status = INVALID_OPERATION;
        }
        return 0;
    }    
    
    AudioAACStreamOut* out = new AudioAACStreamOut();
    status_t lStatus = out->set(this,mFd, devices, format, channels, sampleRate);
    if (status) {
        *status = lStatus;
    }
    if (lStatus == NO_ERROR) {
		ALOGI("Makin' an output...");
                mOutput = out;                
    }else { 
            delete out;
    }
    
    return mOutput;
}

void AudioFakeHardware::closeOutputStream(AudioStreamOut* out)
{
    ALOGI("Gettin' rid of an output...");
    if (mOutput && out == mOutput) {
        delete mOutput;
       mOutput = 0;
    }
    //delete out;
}

AudioStreamIn* AudioFakeHardware::openInputStream(
        uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate,
        status_t *status, AudioSystem::audio_in_acoustics acoustics)
{
    // check for valid input source
    if (!AudioSystem::isInputDevice((AudioSystem::audio_devices)devices)) {
        return 0;
    }
    
    AutoMutex lock(mLock);
    
    // only one input stream allowed
    if (mInput) {
        if (status) {
            *status = INVALID_OPERATION;
        }
        return 0;
    }
    
    AudioAACStreamIn* in = new AudioAACStreamIn();
    //status_t lStatus = in->set(format, channels, sampleRate);
    status_t lStatus = in->set(this, mFd, devices, format, channels, sampleRate, acoustics);

    if (status) {
        *status = lStatus;
    }
    if (lStatus == NO_ERROR) {
        ALOGI("Makin' an input...");
        mInput = in;
    }else{
        ALOGI("openInputStream() failure...\n");
        delete in;
    }

    return mInput;
}

void AudioFakeHardware::closeInputStream(AudioStreamIn* in)
{
    if (mInput && in == mInput) {
        delete mInput;
        mInput = 0;
    }
}

status_t AudioFakeHardware::setVoiceVolume(float volume)
{
    return NO_ERROR;
}

status_t AudioFakeHardware::setMasterVolume(float volume)
{    
    // Implement: set master volume
    // return error - software mixer will handle it
    return INVALID_OPERATION;
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
    if (mInput) {
        mInput->dump(fd, args);
    }
    if (mOutput) {
        mOutput->dump(fd, args);
    }
    return NO_ERROR;
}
//android::Mutex AudioFakeHardware::Writelock;
//android::Condition AudioFakeHardware::shouldReadCondition;

// ----------------------------------------------------------------------------

status_t AudioAACStreamOut::set(
        AudioFakeHardware *hw,
        int fd,
        uint32_t devices,
        int *pFormat, 
        uint32_t *pChannels, 
        uint32_t *pRate)
{
//    if (pFormat) *pFormat = format();
//    if (pChannels) *pChannels = channels();
//    if (pRate) *pRate = sampleRate();
    int lFormat = pFormat ? *pFormat : 0;
    uint32_t lChannels = pChannels ? *pChannels : 0;
    uint32_t lRate = pRate ? *pRate : 0;

    // fix up defaults
    if (lFormat == 0) lFormat = format();
    if (lChannels == 0) lChannels = channels();
    if (lRate == 0) lRate = sampleRate();

    // check values
    if ((lFormat != format()) ||
            (lChannels != channels()) ||
            (lRate != sampleRate())) {
        if (pFormat) *pFormat = format();
        if (pChannels) *pChannels = channels();
        if (pRate) *pRate = sampleRate();
        return BAD_VALUE;
    }

    if (pFormat) *pFormat = lFormat;
    if (pChannels) *pChannels = lChannels;
    if (pRate) *pRate = lRate;

    mAudioHardware = hw;
    mFd = fd;
    mDevice = devices;
    return NO_ERROR;

}

AudioAACStreamOut::~AudioAACStreamOut(){}

ssize_t AudioAACStreamOut::write(const void* buffer, size_t bytes)
{
        Mutex::Autolock _l(mLock);
	struct timeval end;

	// check if the stored start time is more than .25sec ago - if so, toss it out
	gettimeofday(&end, NULL);
	int diff = end.tv_usec-time.tv_usec+1000000ll*(end.tv_sec-time.tv_sec);
	if(diff>250000||diff<0)	{ // 1/4sec
		ALOGI("Write:: Resetting time; had %d diff\n", diff);
		gettimeofday(&time, NULL);
	}

	//writeToBuffer(bytes/sizeof(unsigned short),(unsigned short*)buffer);	// bytes/2 is because I care about shorts, not bytes
	writeToBuffer(bytes,(unsigned short*)buffer);
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

status_t AudioAACStreamOut::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioAACStreamOut::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);   
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)mDevice);
    }
    
    ALOGV("getParameters() %s", param.toString().string());            
    return param.toString();
}

status_t AudioAACStreamOut::getRenderPosition(uint32_t *dspFrames)
{
    return INVALID_OPERATION;
}


// AudioAACStreamIn operations
//
//status_t AudioAACStreamIn::set(int *pFormat, uint32_t *pChannels, uint32_t *pRate)
//{
//    if (pFormat) *pFormat = format();
//    if (pChannels) *pChannels = channels();
//    if (pRate) *pRate = sampleRate();
//
//    return NO_ERROR;
//}
status_t AudioAACStreamIn::set(        
        AudioFakeHardware *hw,
        int fd,
        uint32_t devices,
        int *pFormat,
        uint32_t *pChannels,
        uint32_t *pRate,
        AudioSystem::audio_in_acoustics acoustics)
{
    if (pFormat == 0 || pChannels == 0 || pRate == 0) return BAD_VALUE;
    ALOGV("AudioAACStreamIn::set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
    // check values
    if ((*pFormat != format()) ||
        (*pChannels != channels()) ||
        (*pRate != sampleRate())) {
        ALOGE("Error opening input channel");
        *pFormat = format();
        *pChannels = channels();
        *pRate = sampleRate();
        return BAD_VALUE;
    }

    mAudioHardware = hw;
    mFd = fd;
    mDevice = devices;
    return NO_ERROR;
}

AudioAACStreamIn::~AudioAACStreamIn()
{
}
//
//unsigned int  AudioAACStreamIn::getInputFramesLost() const
//{
//	return 0;
//}

ssize_t AudioAACStreamIn::read(void* buffer, ssize_t bytes)
{
    
//	while ( GetLock(infd) == 1){ // lock is found
//                ALOGI("lock found sleeping...\n");
//		usleep(500000); // half a second for now
//	}
	
	//ALOGI("No lock found reading...\n");
        AutoMutex lock(mLock);
	struct timeval end;
	gettimeofday(&end, NULL);
	int diff = end.tv_usec-last.tv_usec+1000000ll*(end.tv_sec-last.tv_sec);
	if(diff>250000||diff<0)	{ // 1/4sec
		ALOGI("read:: Resetting time; had %d diff\n", diff);
		gettimeofday(&last, NULL);
	}
	//readFromBuffer(bytes/sizeof(unsigned short),(unsigned short*)buffer);	// bytes/2 is because I care about shorts, not bytes
        readFromBuffer(bytes,(unsigned short*)buffer);

	// advance the time time by a frame
	last.tv_usec += (bytes * 1000000ll) / sizeof(int16_t) / AudioSystem::popCount(channels()) / sampleRate();
	if(last.tv_usec>1000000ll) {
		last.tv_sec++;
		last.tv_usec-=1000000;
	}

	diff = last.tv_usec-end.tv_usec+1000000ll*(last.tv_sec-end.tv_sec);

	if(diff>0)	// If it's positive, sleep the difference between the expected end time and the actual end time
		usleep(diff);

	// set read lock. Must be unlocked by next write.
	//SetLock(infd);
	return bytes/sizeof(unsigned short);
}

status_t AudioAACStreamIn::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "AudioStreamInGeneric::dump\n");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tdevice: %d\n", mDevice);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmAudioHardware: %p\n", mAudioHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioAACStreamIn::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioAACStreamIn::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)mDevice);
    }

    ALOGV("getParameters() %s", param.toString().string());
    return param.toString();
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
