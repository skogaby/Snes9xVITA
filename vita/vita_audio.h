// Author: skogaby
// Heavily modified version of frangarcj's audio code:
// https://github.com/frangarcj/HandyVITA

#ifndef __VITA_AUDIO_H__
#define __VITA_AUDIO_H__

#define AUDIO_CHANNELS 1
#define AUDIO_SAMPLE_COUNT 512
#define AUDIO_OUTPUT_RATE 32000
#define PSP_AUDIO_SAMPLE_ALIGN(s) (((s) + 63) & ~63)
#define PSP_AUDIO_SAMPLE_TRUNCATE(s) ((s) & ~63)
#define PSP_AUDIO_MAX_VOLUME 0x8000

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/audioout.h>

typedef int(*pspAudioCallback)(void *buffer, unsigned int *sample_count, void *userdata);

// represents a single audio sample, with a left and right value
typedef struct
{
    short Left;
    short Right;
} PspStereoSample;

// info about a particular channel, including its current thread handle and callback
typedef struct {
    int ThreadHandle;
    int Handle;
    int LeftVolume;
    int RightVolume;
    pspAudioCallback Callback;
    void *Userdata;
} ChannelInfo;

// maintain concurrency between the thread that's buffering audio
// data and the thread that's playing it
static SceUID audio_mutex;

// keep track of our current audio frame buffer and its length
// (the data received in the libretro audio callback)
static unsigned int curr_buffer_frames;
static int16_t *retro_audio_callback_buffer;

// buffers and variables for each of our audio channels
static volatile int stop_audio;
static ChannelInfo audio_status[AUDIO_CHANNELS];
static short *audio_buffer[AUDIO_CHANNELS][2];
static unsigned int audio_buffer_samples[AUDIO_CHANNELS][2];

// helper functions
int setup_audio();
size_t retro_audio_sample_batch_callback(const int16_t *data, size_t frames);
static int audio_channel_thread(int args, void *argp);
int audio_callback(void *buffer, unsigned int *length, void *userdata);
static void free_buffers();
static int output_audio_blocking(unsigned int channel, unsigned int vol1, unsigned int vol2, void *buf, int length);
void set_audio_channel_callback(int channel, pspAudioCallback callback, void *userdata);
void audio_shutdown();

#endif
