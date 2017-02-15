#include "legato.h"
#include "interfaces.h"

static le_audio_StreamRef_t FileAudioRef = NULL;
static int   AudioFileFd = -1;
static bool PlayInLoop = false;

static uint32_t ChannelsCount;
static uint32_t SampleRate;
static uint32_t BitsPerSample;

typedef struct {
    uint32_t riffId;         ///< "RIFF" constant. Marks the file as a riff file.
    uint32_t riffSize;       ///< Size of the overall file - 8 bytes
    uint32_t riffFmt;        ///< File Type Header. For our purposes, it always equals "WAVE".
    uint32_t fmtId;          ///< Equals "fmt ". Format chunk marker. Includes trailing null
    uint32_t fmtSize;        ///< Length of format data as listed above
    uint16_t audioFormat;    ///< Audio format (PCM)
    uint16_t channelsCount;  ///< Number of channels
    uint32_t sampleRate;     ///< Sample frequency in Hertz
    uint32_t byteRate;       ///< sampleRate * channelsCount * bps / 8
    uint16_t blockAlign;     ///< channelsCount * bps / 8
    uint16_t bitsPerSample;  ///< Bits per sample
    uint32_t dataId;         ///< "data" chunk header. Marks the beginning of the data section.
    uint32_t dataSize;       ///< Data size
} WavHeader_t;

typedef struct
{
    WavHeader_t hd;
    uint32_t wroteLen;
    int pipefd[2];
    le_thread_Ref_t mainThreadRef;
    bool playDone;
}
PbRecSamplesThreadCtx_t;

static PbRecSamplesThreadCtx_t PbRecSamplesThreadCtx;
static le_thread_Ref_t RecPbThreadRef = NULL;


le_result_t audio_init(void)
{

	return LE_OK;
}
le_result_t audio_deinit(void)
{

	return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Play Samples thread destructor.
 *
 */
//--------------------------------------------------------------------------------------------------
static void DestroyPlayThread
(
    void *contextPtr
)
{
    PbRecSamplesThreadCtx_t *threadCtxPtr = (PbRecSamplesThreadCtx_t*) contextPtr;

    LE_INFO("DestroyPlayThread playDone %d PlayInLoop %d", threadCtxPtr->playDone, PlayInLoop);

    if( threadCtxPtr->playDone && PlayInLoop )
    {
        le_event_QueueFunctionToThread(threadCtxPtr->mainThreadRef,
                                        PlaySamples,
                                        contextPtr,
                                        NULL);
    }
    else
    {
        LE_DEBUG("Close pipe Tx");
        close(threadCtxPtr->pipefd[1]);
    }

    RecPbThreadRef = NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Play Samples thread.
 *
 */
//--------------------------------------------------------------------------------------------------
static void* PlaySamplesThread
(
    void* contextPtr
)
{
    char data[1024];
    int32_t len = 1;
    uint32_t channelsCount;
    uint32_t sampleRate;
    uint32_t bitsPerSample;
    PbRecSamplesThreadCtx_t *threadCtxPtr = (PbRecSamplesThreadCtx_t*) contextPtr;

    le_audio_ConnectService();

    lseek(AudioFileFd, 0, SEEK_SET);

    if ( ( threadCtxPtr->pipefd[0] == -1 ) && ( threadCtxPtr->pipefd[1] == -1 ) )
    {
        if (pipe(threadCtxPtr->pipefd) == -1)
        {
            LE_ERROR("Failed to create the pipe");
            return NULL;
        }

        LE_ASSERT(le_audio_SetSamplePcmChannelNumber(FileAudioRef, ChannelsCount) == LE_OK);
        LE_ASSERT(le_audio_SetSamplePcmSamplingRate(FileAudioRef, SampleRate) == LE_OK);
        LE_ASSERT(le_audio_SetSamplePcmSamplingResolution(FileAudioRef, BitsPerSample) == LE_OK);

        LE_ASSERT(le_audio_GetSamplePcmChannelNumber(FileAudioRef, &channelsCount) == LE_OK);
        LE_ASSERT(channelsCount == ChannelsCount);
        LE_ASSERT(le_audio_GetSamplePcmSamplingRate(FileAudioRef, &sampleRate) == LE_OK);
        LE_ASSERT(sampleRate == SampleRate);
        LE_ASSERT(le_audio_GetSamplePcmSamplingResolution(FileAudioRef, &bitsPerSample) == LE_OK);
        LE_ASSERT(bitsPerSample == BitsPerSample);

        LE_ASSERT(le_audio_PlaySamples(FileAudioRef, threadCtxPtr->pipefd[0]) == LE_OK);
        LE_INFO("Start playing samples...");
    }

    while ((len = read(AudioFileFd, data, 1024)) > 0)
    {
        int32_t wroteLen = write( threadCtxPtr->pipefd[1], data, len );

        if (wroteLen <= 0)
        {
            LE_ERROR("write error %d", wroteLen);
            return NULL;
        }
    }

    threadCtxPtr->playDone = true;

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Play Samples thread.
 *
 */
//--------------------------------------------------------------------------------------------------

static void PlaySamples
(
    void* param1Ptr,
    void* param2Ptr
)
{
    if (RecPbThreadRef == NULL)
    {
        RecPbThreadRef = le_thread_Create("PlaySamples", PlaySamplesThread, param1Ptr);

        le_thread_AddChildDestructor(RecPbThreadRef,
                                    DestroyPlayThread,
                                    param1Ptr);

        le_thread_Start(RecPbThreadRef);
    }
}

le_result_t audio_playback(const char * file_name)
{

	return LE_OK;
}
//--------------------------------------------------------------------------------------------------
/**
 * Rec Samples thread destructor.
 *
 */
//--------------------------------------------------------------------------------------------------

static void DestroyRecThread
(
    void *contextPtr
)
{
    PbRecSamplesThreadCtx_t *threadCtxPtr = (PbRecSamplesThreadCtx_t*) contextPtr;

    LE_INFO("wroteLen %d", threadCtxPtr->wroteLen);
    close(AudioFileFd);
}

//--------------------------------------------------------------------------------------------------
/**
 * Rec Samples thread.
 *
 */
//--------------------------------------------------------------------------------------------------

static void* RecSamplesThread
(
    void* contextPtr
)
{
    int pipefd[2];
    int32_t len = 1024;
    char data[len];
    uint32_t channelsCount;
    uint32_t sampleRate;
    uint32_t bitsPerSample;
    PbRecSamplesThreadCtx_t *threadCtxPtr = (PbRecSamplesThreadCtx_t*) contextPtr;

    le_audio_ConnectService();

    lseek(AudioFileFd, 0, SEEK_SET);

    if (pipe(pipefd) == -1)
    {
        LE_ERROR("Failed to create the pipe");
        return NULL;
    }

    LE_ASSERT(le_audio_SetSamplePcmChannelNumber(FileAudioRef, ChannelsCount) == LE_OK);
    LE_ASSERT(le_audio_SetSamplePcmSamplingRate(FileAudioRef, SampleRate) == LE_OK);
    LE_ASSERT(le_audio_SetSamplePcmSamplingResolution(FileAudioRef, BitsPerSample) == LE_OK);

    LE_ASSERT(le_audio_GetSamplePcmChannelNumber(FileAudioRef, &channelsCount) == LE_OK);
    LE_ASSERT(channelsCount == ChannelsCount);
    LE_ASSERT(le_audio_GetSamplePcmSamplingRate(FileAudioRef, &sampleRate) == LE_OK);
    LE_ASSERT(sampleRate == SampleRate);
    LE_ASSERT(le_audio_GetSamplePcmSamplingResolution(FileAudioRef, &bitsPerSample) == LE_OK);
    LE_ASSERT(bitsPerSample == BitsPerSample);

    LE_ASSERT(le_audio_GetSamples(FileAudioRef, pipefd[1]) == LE_OK);
    LE_INFO("Start getting samples...");

    int32_t readLen;

    while ((readLen = read( pipefd[0], data, len )))
    {
        int32_t len = write( AudioFileFd, data, readLen );

        if (len < 0)
        {
            LE_ERROR("write error %d %d", readLen, len);
            break;
        }

        threadCtxPtr->wroteLen += len;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Rec Samples.
 *
 */
//--------------------------------------------------------------------------------------------------

static void RecSamples
(
    void
)
{
    memset(&PbRecSamplesThreadCtx,0,sizeof(PbRecSamplesThreadCtx_t));

    RecPbThreadRef = le_thread_Create("RecSamples", RecSamplesThread, &PbRecSamplesThreadCtx);

    le_thread_AddChildDestructor(RecPbThreadRef,
                                DestroyRecThread,
                                &PbRecSamplesThreadCtx);

    le_thread_Start(RecPbThreadRef);
}
le_result_t audio_record(const char * file_name)
{

	return LE_OK;
}
le_result_t audio_mute(void)
{
	le_result_t result;
	result =  le_audio_Mute(FileAudioRef);
    if (result != LE_OK)
    {
        LE_FATAL("le_audio_Mute/le_audio_Unmute Failed %d", result);
    }
	return result;
}
le_result_t audio_unmute(void)
{
	le_result_t result;
	result =  le_audio_Unmute(FileAudioRef);
    if (result != LE_OK)
    {
        LE_FATAL("le_audio_Mute/le_audio_Unmute Failed %d", result);
    }
	return result;
}

le_result_t audio_volumn(int level) // range from 0 to 100
{
	int sysVol = 0;
	le_result_t result;

    LE_INFO("Playback volume: vol %d", level);
    result = le_audio_SetGain(FileAudioRef, level);

    if (result != LE_OK)
    {
        LE_FATAL("le_audio_SetGain error : %d", result);
    }

    result = le_audio_GetGain(FileAudioRef, &sysVol);

    if ((result != LE_OK) || (level != sysVol))
    {
        LE_FATAL("le_audio_GetGain error : %d read volume: %d", result, sysVol);
    }
	return LE_OK;
}

COMPONENT_INIT
{
    LE_INFO("Hello, world.");


}
