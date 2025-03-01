//
//  Created by Matt Hartley on 27/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudioTypes/CoreAudioTypes.h>

#include "core/core.h"
#include <core/math.h>
#include "system.h"

sys_sound_t _sounds[64];
// int soundCount

SYS_FUNC void Sys_QueueSound(sys_wave_t* wave, float volume) {
	FOR (i, 64) {
		if (!_sounds[i].wave) {
			_sounds[i] = (sys_sound_t){
				.wave = wave,
				.cursor = 0,
				.volume = volume,
			};
			return;
		}
	}
	print_error("Out of sound slots");
}

OSStatus Sys_AURenderCallback(
	void* refCon,
	AudioUnitRenderActionFlags* flags,
	const AudioTimeStamp* timeStamp,
	UInt32 busNumber,
	UInt32 numFrames,
	AudioBufferList* data
) {
	zero_memory(data->mBuffers[0].mData, sizeof(float)*2*numFrames);

	// static int cursor = 0;
	// FOR (i, numFrames) {
	// 	float wave = sinf(440.0f * PI2 * cursor / 44100.0f) * 0.1f;
	// 	((float*)data->mBuffers[0].mData)[i*2] += wave;
	// 	((float*)data->mBuffers[0].mData)[i*2+1] += wave;
	// 	++cursor;
	// }

	FOR (isound, array_size(_sounds)) {
		sys_sound_t* sound = _sounds + isound;
		if (sound->wave) {
			int waveSamples = sound->wave->sampleCount - (int)sound->cursor;
			int samplesToMix = min(numFrames, waveSamples);
			FOR (i, samplesToMix) {
				float left = (float)sound->wave->data[(int)sound->cursor].left / (float)0x7FFF;
				float right = (float)sound->wave->data[(int)sound->cursor].right / (float)0x7FFF;
				((float*)data->mBuffers[0].mData)[i*2] += left * sound->volume;
				((float*)data->mBuffers[0].mData)[i*2+1] += right * sound->volume;
				sound->cursor += 1.0f;
			}

			if ((int)sound->cursor >= sound->wave->sampleCount) {
				sound->wave = NULL;
			}
		}
	}

	return noErr;
}

SYS_FUNC void Sys_InitAudio(audio_mixer_proc mixerProc) {
	AudioComponentDescription desc = {
		.componentType = kAudioUnitType_Output,
		.componentSubType = kAudioUnitSubType_DefaultOutput,
		.componentManufacturer = kAudioUnitManufacturer_Apple,
	};

	AudioComponent outputComponent = AudioComponentFindNext(NULL, &desc);
	if (!outputComponent) {
		print_error("Audio init failed: AudioComponentFindNext");
		return;
	}

	AudioUnit outputUnit;
	OSStatus status = AudioComponentInstanceNew(outputComponent, &outputUnit);
	if (status != noErr) {
		print_error("Audio init failed: AudioComponentInstanceNew");
		return;
	}

	AudioStreamBasicDescription streamDesc = {
		.mSampleRate = 44100,
		.mFormatID = kAudioFormatLinearPCM,
		.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked,
		.mFramesPerPacket = 1,
		.mChannelsPerFrame = 2,
		.mBytesPerFrame = sizeof(float)*2,
		.mBytesPerPacket = sizeof(float)*2,
		.mBitsPerChannel = 32,
	};
	AudioUnitSetProperty(
		outputUnit,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&streamDesc,
		sizeof(streamDesc)
	);

	AURenderCallbackStruct callback;
	callback.inputProc = Sys_AURenderCallback;
	callback.inputProcRefCon = NULL;
	AudioUnitSetProperty(
		outputUnit,
		kAudioUnitProperty_SetRenderCallback,
		kAudioUnitScope_Input,
		0,
		&callback,
		sizeof(callback)
	);

	AudioUnitInitialize(outputUnit);
	AudioOutputUnitStart(outputUnit);
}

#pragma pack(push, 1)
typedef struct {
	char ChunkId[4];
	u32 ChunkSize;
	char WaveId[4];
} WavHeader;
typedef struct {
	u8 id[4];
	u32 size;
	u16 formatTag;
	u16 channels;
	u32 samplesPerSec;
	u32 bytesPerSec;
	u16 blockAlign;
	u16 bitsPerSample;
	u16 cbSize;
	i16 validBitsPerSample;
	i32 channelMask;
	u8 subFormat[16];
} WavFormatChunk;
typedef struct {
	char id[4];
	u32 size;
	void *data;
	char padByte;
} WavDataChunk;
#pragma pack(pop)

SYS_FUNC sys_wave_t* Sys_LoadWave(allocator_t* allocator, file_data_t* fileData) {
	u8* data = fileData->data;
	WavHeader *header = (WavHeader*)data;
	WavFormatChunk *format = NULL;
	WavDataChunk *dataChunk = NULL;
	char *f = (char*)(header + 1);

	if (data) {
		// Parse file and collect structures
		while (f < (char*)data + fileData->stat.size) {
			int id = *(int*)f;
			u32 size = *(u32*)(f+4);
			if (id == (('f'<<0)|('m'<<8)|('t'<<16)|(' '<<24))) {
				format = (WavFormatChunk*)f;
			}
			if (id == (('d'<<0)|('a'<<8)|('t'<<16)|('a'<<24))) {
				dataChunk = (WavDataChunk*)f;
				dataChunk->data = f + 8;
			}
			f += size + 8;
		}

		if (format && dataChunk) {
			assert(format->channels <= 2);
			assert(format->bitsPerSample == 16);
			// assert(dataChunk->size ==);
			// Possibly check whether to alloc or push
			sys_wave_t* wave;
			if(format->channels == 1) {
				// TODO this is temporary solution
				wave = alloc_memory(allocator, sizeof(sys_wave_t) + dataChunk->size*2);
				wave->channels = 2;
				wave->samplesPerSecond = format->samplesPerSec;
				wave->bytesPerSample = format->bitsPerSample / 8;
				wave->sampleCount = dataChunk->size / (wave->channels * wave->bytesPerSample);
				i16* raw_data = dataChunk->data;
				sys_audio_sample_t* output = (sys_audio_sample_t*)(wave + 1);
				FOR(i, wave->sampleCount) {
					output[i].left = raw_data[i];
					output[i].right = raw_data[i];
				}
			} else {
				wave = alloc_memory(allocator, sizeof(sys_wave_t) + dataChunk->size);
				memcpy(wave+1, dataChunk->data, dataChunk->size);
				wave->channels = format->channels;
				wave->samplesPerSecond = format->samplesPerSec;
				wave->bytesPerSample = format->bitsPerSample / 8;
				wave->sampleCount = dataChunk->size / (wave->channels * wave->bytesPerSample);
			}
			return wave;
		}
	}

	return NULL;
}
