#include "legato.h"
#include "interfaces.h"

static le_audio_StreamRef_t MdmRxAudioRef = NULL;
static le_audio_StreamRef_t MdmTxAudioRef = NULL;
static le_audio_StreamRef_t FeInRef = NULL;
static le_audio_StreamRef_t FeOutRef = NULL;
static le_audio_StreamRef_t FileAudioRef = NULL;

static int   AudioFileFd = -1;
static bool PlayInLoop = true;

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

static le_audio_ConnectorRef_t AudioInputConnectorRef = NULL;
static le_audio_ConnectorRef_t AudioOutputConnectorRef = NULL;

static le_audio_MediaHandlerRef_t MediaHandlerRef = NULL;

static const char* AudioTestCase;
static const char* MainAudioSoundPath;
static const char* AudioFilePath;

void DisconnectAllAudio(void);
void PlaySamples(void* param1Ptr,void* param2Ptr);

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
 * Handler function for Stream Event Notifications.
 *
 */
//--------------------------------------------------------------------------------------------------
static void MyMediaEventHandler
(
    le_audio_StreamRef_t          streamRef,
    le_audio_MediaEvent_t          event,
    void*                         contextPtr
)
{
    if (!streamRef)
    {
        LE_ERROR("Bad streamRef");
        return;
    }


    switch(event)
    {
        case LE_AUDIO_MEDIA_ENDED:
            LE_INFO("File event is LE_AUDIO_MEDIA_ENDED.");
            if (PlayInLoop)
            {
                le_audio_PlayFile(streamRef, LE_AUDIO_NO_FD);
            }
            break;

        case LE_AUDIO_MEDIA_ERROR:
            LE_INFO("File event is LE_AUDIO_MEDIA_ERROR.");
        break;

        case LE_AUDIO_MEDIA_NO_MORE_SAMPLES:
            LE_INFO("File event is LE_AUDIO_MEDIA_NO_MORE_SAMPLES.");
        break;
        default:
            LE_INFO("File event is %d.", event);
        break;
    }

//    if (GainTimerRef)
//    {
//        le_timer_Stop (GainTimerRef);
//        le_timer_Delete (GainTimerRef);
//        GainTimerRef = NULL;
//    }
//
//    if (MuteTimerRef)
//    {
//        le_timer_Stop (MuteTimerRef);
//        le_timer_Delete (MuteTimerRef);
//        le_audio_Unmute(FileAudioRef);
//        MuteTimerRef = NULL;
//    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Connect player to connector
 *
 */
//--------------------------------------------------------------------------------------------------
static void ConnectAudioToFileLocalPlay
(
    void
)
{
    le_result_t res;
    if ((AudioFileFd=open(AudioFilePath, O_RDONLY)) == -1)
    {
        LE_ERROR("Open file %s failure: errno.%d (%s)",  AudioFilePath, errno, strerror(errno));
        DisconnectAllAudio();
        exit(0);
    }
    else
    {
        LE_INFO("Open file %s with AudioFileFd.%d",  AudioFilePath, AudioFileFd);
    }

    // Play local on output connector.
    FileAudioRef = le_audio_OpenPlayer();
    LE_ERROR_IF((FileAudioRef==NULL), "OpenFilePlayback returns NULL!");

    le_audio_SetEncodingFormat(FileAudioRef, LE_AUDIO_WAVE);

    LE_ERROR_IF((le_audio_SetGain(FileAudioRef, 0x3000) != LE_OK), "Cannot set multimedia gain");

    le_audio_EnableNoiseSuppressor(FileAudioRef);
    le_audio_Unmute(FileAudioRef);


    MediaHandlerRef = le_audio_AddMediaHandler(FileAudioRef, MyMediaEventHandler, NULL);

    if (FileAudioRef && AudioOutputConnectorRef)
    {
        res = le_audio_Connect(AudioOutputConnectorRef, FileAudioRef);
        if(res != LE_OK)
        {
            LE_ERROR("Failed to connect FilePlayback on output connector!");
            return;
        }

//        if (strncmp(AudioTestCase,"PB_SAMPLES",10)==0)
//        {
//            memset(&PbRecSamplesThreadCtx,0,sizeof(PbRecSamplesThreadCtx_t));
//            PbRecSamplesThreadCtx.pipefd[0] = -1;
//            PbRecSamplesThreadCtx.pipefd[1] = -1;
//            PbRecSamplesThreadCtx.mainThreadRef = le_thread_GetCurrent();
//
//            PlaySamples(&PbRecSamplesThreadCtx, NULL);
//        }
//        else
//        {
            LE_INFO("FilePlayback is now connected.");
            res = le_audio_PlayFile(FileAudioRef, AudioFileFd);

            if(res != LE_OK)
            {
                LE_ERROR("Failed to play the file!");
                return;
            }
            else
            {
                LE_INFO("File is now playing");
            }
//        }

//        ExecuteNextOption();
    }
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
 * Connect I2S to the connectors
 *
 */
//--------------------------------------------------------------------------------------------------
void ConnectAudioToI2s
(
    void
)
{
    le_result_t res;

    // Redirect audio to the I2S interface.
    FeOutRef = le_audio_OpenI2sTx(LE_AUDIO_I2S_STEREO);
    LE_ERROR_IF((FeOutRef==NULL), "OpenI2sTx returns NULL!");
    FeInRef = le_audio_OpenI2sRx(LE_AUDIO_I2S_STEREO);
    LE_ERROR_IF((FeInRef==NULL), "OpenI2sRx returns NULL!");

    LE_INFO("Open I2s: FeInRef.%p FeOutRef.%p", FeInRef, FeOutRef);

    AudioInputConnectorRef = le_audio_CreateConnector();
    LE_ERROR_IF((AudioInputConnectorRef==NULL), "AudioInputConnectorRef is NULL!");
    AudioOutputConnectorRef = le_audio_CreateConnector();
    LE_ERROR_IF((AudioOutputConnectorRef==NULL), "AudioOutputConnectorRef is NULL!");
    {
        res = le_audio_Connect(AudioInputConnectorRef, FeInRef);
        LE_ERROR_IF((res!=LE_OK), "Failed to connect I2S RX on Input connector!");
        res = le_audio_Connect(AudioOutputConnectorRef, FeOutRef);
        LE_ERROR_IF((res!=LE_OK), "Failed to connect I2S TX on Output connector!");
    }
    LE_INFO("Open I2s: FeInRef.%p FeOutRef.%p", FeInRef, FeOutRef);
}

//--------------------------------------------------------------------------------------------------
/**
 *  Play Samples thread.
 *
 */
//--------------------------------------------------------------------------------------------------

void PlaySamples
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

void RecSamples
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
//--------------------------------------------------------------------------------------------------
/**
 * Connect PCM to connectors
 *
 */
//--------------------------------------------------------------------------------------------------
void ConnectAudioToPcm
(
    void
)
{
    le_result_t res;


    // Redirect audio to the PCM interface.
    FeOutRef = le_audio_OpenPcmTx(0);
    LE_ERROR_IF((FeOutRef==NULL), "OpenPcmTx returns NULL!");
    FeInRef = le_audio_OpenPcmRx(0);
    LE_ERROR_IF((FeInRef==NULL), "OpenPcmRx returns NULL!");

    AudioInputConnectorRef = le_audio_CreateConnector();
    LE_ERROR_IF((AudioInputConnectorRef==NULL), "AudioInputConnectorRef is NULL!");
    AudioOutputConnectorRef = le_audio_CreateConnector();
    LE_ERROR_IF((AudioOutputConnectorRef==NULL), "AudioOutputConnectorRef is NULL!");
    {
        res = le_audio_Connect(AudioInputConnectorRef, FeInRef);
        LE_ERROR_IF((res!=LE_OK), "Failed to connect PCM RX on Input connector!");
        res = le_audio_Connect(AudioOutputConnectorRef, FeOutRef);
        LE_ERROR_IF((res!=LE_OK), "Failed to connect PCM TX on Output connector!");
    }
}

//#define ENABLE_CODEC 1
//#ifdef ENABLE_CODEC
//--------------------------------------------------------------------------------------------------
/**
 * Connect speaker and MIC to connectors
 *
 */
//--------------------------------------------------------------------------------------------------
void ConnectAudioToCodec
(
    void
)
{
    le_result_t res;

    // Redirect audio to the in-built Microphone and Speaker.
    FeOutRef = le_audio_OpenSpeaker();
    LE_ERROR_IF((FeOutRef==NULL), "OpenSpeaker returns NULL!");
    FeInRef = le_audio_OpenMic();
    LE_ERROR_IF((FeInRef==NULL), "OpenMic returns NULL!");

    AudioInputConnectorRef = le_audio_CreateConnector();
    LE_ERROR_IF((AudioInputConnectorRef==NULL), "AudioInputConnectorRef is NULL!");
    AudioOutputConnectorRef = le_audio_CreateConnector();
    LE_ERROR_IF((AudioOutputConnectorRef==NULL), "AudioOutputConnectorRef is NULL!");

    {
        res = le_audio_Connect(AudioInputConnectorRef, FeInRef);
        LE_ERROR_IF((res!=LE_OK), "Failed to connect Mic on Input connector!");
        res = le_audio_Connect(AudioOutputConnectorRef, FeOutRef);
        LE_ERROR_IF((res!=LE_OK), "Failed to connect Speaker on Output connector!");
    }
}
//#endif

//--------------------------------------------------------------------------------------------------
/**
 * Disconnect all streams and connectors
 *
 */
//--------------------------------------------------------------------------------------------------
void DisconnectAllAudio
(
    void
)
{
    if (AudioInputConnectorRef)
    {
        if(FileAudioRef)
        {
            LE_INFO("Disconnect %p from connector.%p", FileAudioRef, AudioInputConnectorRef);
            le_audio_Disconnect(AudioInputConnectorRef, FileAudioRef);
        }
        if (FeInRef)
        {
            LE_INFO("Disconnect %p from connector.%p", FeInRef, AudioInputConnectorRef);
            le_audio_Disconnect(AudioInputConnectorRef, FeInRef);
        }
        if(MdmTxAudioRef)
        {
            LE_INFO("Disconnect %p from connector.%p", MdmTxAudioRef, AudioInputConnectorRef);
            le_audio_Disconnect(AudioInputConnectorRef, MdmTxAudioRef);
        }
    }
    if(AudioOutputConnectorRef)
    {
        if(FileAudioRef)
        {
            LE_INFO("Disconnect %p from connector.%p", FileAudioRef, AudioOutputConnectorRef);
            le_audio_Disconnect(AudioOutputConnectorRef, FileAudioRef);
        }
        if(FeOutRef)
        {
            LE_INFO("Disconnect %p from connector.%p", FeOutRef, AudioOutputConnectorRef);
            le_audio_Disconnect(AudioOutputConnectorRef, FeOutRef);
        }
        if(MdmRxAudioRef)
        {
            LE_INFO("Disconnect %p from connector.%p", MdmRxAudioRef, AudioOutputConnectorRef);
            le_audio_Disconnect(AudioOutputConnectorRef, MdmRxAudioRef);
        }
    }

    if(AudioInputConnectorRef)
    {
        le_audio_DeleteConnector(AudioInputConnectorRef);
        AudioInputConnectorRef = NULL;
    }
    if(AudioOutputConnectorRef)
    {
        le_audio_DeleteConnector(AudioOutputConnectorRef);
        AudioOutputConnectorRef = NULL;
    }

    if(FileAudioRef)
    {
        le_audio_Close(FileAudioRef);
        FeOutRef = NULL;
    }
    if(FeInRef)
    {
        le_audio_Close(FeInRef);
        FeInRef = NULL;
    }
    if(FeOutRef)
    {
        le_audio_Close(FeOutRef);
        FeOutRef = NULL;
    }
    if(MdmRxAudioRef)
    {
        le_audio_Close(MdmRxAudioRef);
        FeOutRef = NULL;
    }
    if(MdmTxAudioRef)
    {
        le_audio_Close(MdmTxAudioRef);
        FeOutRef = NULL;
    }

    close(AudioFileFd);

    exit(0);
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

//--------------------------------------------------------------------------------------------------
/**
 * The signal event handler function for SIGINT/SIGTERM when process dies.
 */
//--------------------------------------------------------------------------------------------------
static void SigHandler
(
    int sigNum
)
{
    LE_INFO("Disconnect All Audio and end test");
    if (RecPbThreadRef)
    {
        le_thread_Cancel(RecPbThreadRef);
        RecPbThreadRef = NULL;
    }
    DisconnectAllAudio();
    exit(EXIT_SUCCESS);
}

COMPONENT_INIT
{
	signal(SIGINT, SigHandler);

	LE_INFO("Hello, world.");

	AudioTestCase = "tqk";
	MainAudioSoundPath = "audio.wav";
	ConnectAudioToCodec();

//	AudioFilePath = "audio.wav";
//	ConnectAudioToFileLocalPlay();

	AudioFilePath = "say_you_do.wav";
	ConnectAudioToFileLocalPlay();
	audio_volumn(4000);
//	DisconnectAllAudio();

}
