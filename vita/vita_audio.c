// Author: skogaby
// Heavily modified version of frangarcj's audio code:
// https://github.com/frangarcj/HandyVITA

#include "vita_audio.h"

/***
 * Audio callback for libretro.
 */
size_t retro_audio_sample_batch_callback(const int16_t *data, size_t frames)
{
    sceKernelLockMutex(audio_mutex, 1, NULL);

	// make sure we don't overbuffer
    if (frames + curr_buffer_frames > AUDIO_SAMPLE_COUNT * 2)
    {
        frames = (AUDIO_SAMPLE_COUNT * 2) - curr_buffer_frames;
    }

    // store the data in our buffer
    memcpy(&retro_audio_callback_buffer[curr_buffer_frames * 2], data, frames * 2 * sizeof(int16_t));
    curr_buffer_frames += frames;

    sceKernelUnlockMutex(audio_mutex, 1);
    return frames;
}

/***
 * Callback the audio thread uses to see if we have enough available data.
 */
int audio_callback(void *buffer, unsigned int *length, void *userdata)
{
    int i;
    int len = *length;

    sceKernelLockMutex(audio_mutex, 1, NULL);

    // see if we've buffered enough frames
    if (len <= curr_buffer_frames)
    {
        memcpy(buffer, &retro_audio_callback_buffer[0], len * 2 * sizeof(int16_t));
        curr_buffer_frames -= len;
        memmove(&retro_audio_callback_buffer[0], &retro_audio_callback_buffer[len * 2], sizeof(int16_t) * 2 * curr_buffer_frames);

        sceKernelUnlockMutex(audio_mutex, 1);
        return len;
    }
    else
    {
        sceKernelUnlockMutex(audio_mutex, 1);
        return 0;
    }
}

/***
 * Initializes the audio buffers and a callback thread for each channel.
 */
int setup_audio()
{
    int i, j, failed;

    stop_audio = 0;

    for (i = 0; i < AUDIO_CHANNELS; i++)
    {
        audio_status[i].Handle = -1;
        audio_status[i].ThreadHandle = -1;
        audio_status[i].LeftVolume = PSP_AUDIO_MAX_VOLUME;
        audio_status[i].RightVolume = PSP_AUDIO_MAX_VOLUME;
        audio_status[i].Callback = NULL;
        audio_status[i].Userdata = NULL;

        for (j = 0; j < 2; j++)
        {
            audio_buffer[i][j] = NULL;
            audio_buffer_samples[i][j] = 0;
        }
    }

    /* Initialize buffers */
    for (i = 0; i < AUDIO_CHANNELS; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (!(audio_buffer[i][j] = (short*)malloc(AUDIO_SAMPLE_COUNT * sizeof(PspStereoSample))))
            {
                printf("Couldn't initialize audio buffer for channel %i, bailing.", i);
                free_buffers();
                sceKernelExitProcess(0);

                return 0;
            }

            audio_buffer_samples[i][j] = AUDIO_SAMPLE_COUNT;
        }
    }

    /* Initialize channels */
    for (i = 0, failed = 0; i < AUDIO_CHANNELS; i++)
    {
        audio_status[i].Handle = sceAudioOutOpenPort(PSP2_AUDIO_OUT_PORT_TYPE_VOICE, AUDIO_SAMPLE_COUNT, AUDIO_OUTPUT_RATE, PSP2_AUDIO_OUT_MODE_STEREO);

        if (audio_status[i].Handle < 0)
        {
            failed = 1;
            break;
        }
    }

    if (failed)
    {
        for (i = 0; i < AUDIO_CHANNELS; i++)
        {
            if (audio_status[i].Handle != -1)
            {
                sceAudioOutReleasePort(audio_status[i].Handle);
                audio_status[i].Handle = -1;
            }
        }

        printf("Couldn't open audio port for the device, bailing.");
        free_buffers();
        sceKernelExitProcess(0);

        return 0;
    }

    char label[16];
    strcpy(label, "audiotX");

    for (i = 0; i < AUDIO_CHANNELS; i++)
    {
        label[6] = '0' + i;
        audio_status[i].ThreadHandle =
            sceKernelCreateThread(label, (void*)&audio_channel_thread, 0x10000100, 0x10000,
                0, 0, NULL);

        if (audio_status[i].ThreadHandle < 0)
        {
            audio_status[i].ThreadHandle = -1;
            failed = 1;
            break;
        }

        if (sceKernelStartThread(audio_status[i].ThreadHandle, sizeof(i), &i) != 0)
        {
            failed = 1;
            break;
        }
    }

    if (failed)
    {
        stop_audio = 1;

        for (i = 0; i < AUDIO_CHANNELS; i++)
        {
            if (audio_status[i].ThreadHandle != -1)
            {
                sceKernelDeleteThread(audio_status[i].ThreadHandle);
            }

            audio_status[i].ThreadHandle = -1;
        }


        printf("Couldn't initialize audio callback thread. Bailing.");
        sceKernelExitProcess(0);

        return 0;
    }

    // initialize the buffer our libretro audio callback will fill with data as it's available
    retro_audio_callback_buffer = (int16_t*)malloc(sizeof(int16_t) * AUDIO_SAMPLE_COUNT * 4);

    if (!retro_audio_callback_buffer)
    {
        printf("Couldn't initialize retro_audio_callback_buffer. Bailing.");
        sceKernelExitProcess(0);
    }

    curr_buffer_frames = 0;

    // setup our callbacks
    set_audio_channel_callback(0, audio_callback, 0);

    // initialize the audio buffer mutex
    audio_mutex = sceKernelCreateMutex("AudioMutex", 0, 1, 0);

    return AUDIO_SAMPLE_COUNT;
}

/***
 * Shuts down the audio systems and frees the buffers.
 */
void audio_shutdown()
{
    int i;
    stop_audio = 1;

    for (i = 0; i < AUDIO_CHANNELS; i++)
    {
        if (audio_status[i].ThreadHandle != -1)
        {
            sceKernelDeleteThread(audio_status[i].ThreadHandle);
        }

        audio_status[i].ThreadHandle = -1;
    }

    for (i = 0; i < AUDIO_CHANNELS; i++)
    {
        if (audio_status[i].Handle != -1)
        {
            sceAudioOutReleasePort(audio_status[i].Handle);
            audio_status[i].Handle = -1;
        }
    }

    free_buffers();
    sceKernelDeleteMutex(audio_mutex);
}

/***
 * Callback thread that handles audio output, when it's available.
 */
static int audio_channel_thread(int args, void *argp)
{
    volatile int bufidx = 0;
    int channel = *(int*)argp;
    int i;
    int readedLength;
    unsigned short *ptr_m;
    unsigned int *ptr_s;
    void *bufptr;
    unsigned int samples;
    pspAudioCallback callback;

    for (i = 0; i < 2; i++)
    {
        memset(audio_buffer[channel][bufidx], 0, AUDIO_SAMPLE_COUNT * sizeof(PspStereoSample));
    }

    while (!stop_audio)
    {
        callback = audio_status[channel].Callback;
        bufptr = audio_buffer[channel][bufidx];
        samples = audio_buffer_samples[channel][bufidx];

        if (callback)
        {
            readedLength = callback(bufptr, &samples, audio_status[channel].Userdata);
        }
        else
        {
            for (i = 0, ptr_s = bufptr; i < samples; i++)
            {
                *(ptr_s++) = 0;
            }
        }

        if (readedLength > 0) 
        {
            output_audio_blocking(channel, audio_status[channel].LeftVolume,
                audio_status[channel].RightVolume, bufptr, samples);

            bufidx = (bufidx ? 0 : 1);
        }
    }

    sceKernelExitThread(0);
    return 0;
}

/***
 * Outputs the given audio data, in a blocking fashion.
 */
static int output_audio_blocking(unsigned int channel, unsigned int vol1, unsigned int vol2, void *buf, int length)
{
    if (channel >= AUDIO_CHANNELS) 
        return -1;

    if (vol1 > PSP_AUDIO_MAX_VOLUME) 
        vol1 = PSP_AUDIO_MAX_VOLUME;

    if (vol2 > PSP_AUDIO_MAX_VOLUME) 
        vol2 = PSP_AUDIO_MAX_VOLUME;

    int vols[2] = { vol1, vol2 };
    sceAudioOutSetConfig(audio_status[channel].Handle, length, -1, -1);
    sceAudioOutSetVolume(audio_status[channel].Handle, PSP2_AUDIO_VOLUME_FLAG_L_CH | PSP2_AUDIO_VOLUME_FLAG_R_CH, vols);

    return sceAudioOutOutput(audio_status[channel].Handle, buf);
}

/***
 * Frees up the memory our audio buffers were using.
 */
static void free_buffers()
{
    int i, j;

    for (i = 0; i < AUDIO_CHANNELS; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (audio_buffer[i][j])
            {
                free(audio_buffer[i][j]);
                audio_buffer[i][j] = NULL;
            }
        }
    }
}

/***
 * Sets the callback function for the given channel.
 */
void set_audio_channel_callback(int channel, pspAudioCallback callback, void *userdata)
{
    volatile ChannelInfo *pci = &audio_status[channel];
    pci->Callback = NULL;
    pci->Userdata = userdata;
    pci->Callback = callback;
}
