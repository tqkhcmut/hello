/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef LE_AUDIO_MESSAGES_H_INCLUDE_GUARD
#define LE_AUDIO_MESSAGES_H_INCLUDE_GUARD


#include "legato.h"

#define PROTOCOL_ID_STR "e370edc833f30f7a5ea29bf604784811"

#ifdef MK_TOOLS_BUILD
    extern const char** le_audio_ServiceInstanceNamePtr;
    #define SERVICE_INSTANCE_NAME (*le_audio_ServiceInstanceNamePtr)
#else
    #define SERVICE_INSTANCE_NAME "le_audio"
#endif


// todo: This will need to depend on the particular protocol, but the exact size is not easy to
//       calculate right now, so in the meantime, pick a reasonably large size.  Once interface
//       type support has been added, this will be replaced by a more appropriate size.
#define _MAX_MSG_SIZE 1100

// Define the message type for communicating between client and server
typedef struct
{
    uint32_t id;
    uint8_t buffer[_MAX_MSG_SIZE];
}
_Message_t;

#define _MSGID_le_audio_OpenMic 0
#define _MSGID_le_audio_OpenSpeaker 1
#define _MSGID_le_audio_OpenUsbRx 2
#define _MSGID_le_audio_OpenUsbTx 3
#define _MSGID_le_audio_OpenPcmRx 4
#define _MSGID_le_audio_OpenPcmTx 5
#define _MSGID_le_audio_OpenI2sRx 6
#define _MSGID_le_audio_OpenI2sTx 7
#define _MSGID_le_audio_OpenPlayer 8
#define _MSGID_le_audio_OpenRecorder 9
#define _MSGID_le_audio_OpenModemVoiceRx 10
#define _MSGID_le_audio_OpenModemVoiceTx 11
#define _MSGID_le_audio_AddMediaHandler 12
#define _MSGID_le_audio_RemoveMediaHandler 13
#define _MSGID_le_audio_Close 14
#define _MSGID_le_audio_SetGain 15
#define _MSGID_le_audio_GetGain 16
#define _MSGID_le_audio_Mute 17
#define _MSGID_le_audio_Unmute 18
#define _MSGID_le_audio_CreateConnector 19
#define _MSGID_le_audio_DeleteConnector 20
#define _MSGID_le_audio_Connect 21
#define _MSGID_le_audio_Disconnect 22
#define _MSGID_le_audio_AddDtmfDetectorHandler 23
#define _MSGID_le_audio_RemoveDtmfDetectorHandler 24
#define _MSGID_le_audio_EnableNoiseSuppressor 25
#define _MSGID_le_audio_DisableNoiseSuppressor 26
#define _MSGID_le_audio_EnableEchoCanceller 27
#define _MSGID_le_audio_DisableEchoCanceller 28
#define _MSGID_le_audio_EnableFirFilter 29
#define _MSGID_le_audio_DisableFirFilter 30
#define _MSGID_le_audio_EnableIirFilter 31
#define _MSGID_le_audio_DisableIirFilter 32
#define _MSGID_le_audio_EnableAutomaticGainControl 33
#define _MSGID_le_audio_DisableAutomaticGainControl 34
#define _MSGID_le_audio_SetProfile 35
#define _MSGID_le_audio_GetProfile 36
#define _MSGID_le_audio_SetPcmSamplingRate 37
#define _MSGID_le_audio_SetPcmSamplingResolution 38
#define _MSGID_le_audio_SetPcmCompanding 39
#define _MSGID_le_audio_GetPcmSamplingRate 40
#define _MSGID_le_audio_GetPcmSamplingResolution 41
#define _MSGID_le_audio_GetPcmCompanding 42
#define _MSGID_le_audio_GetDefaultPcmTimeSlot 43
#define _MSGID_le_audio_GetDefaultI2sMode 44
#define _MSGID_le_audio_PlayFile 45
#define _MSGID_le_audio_PlaySamples 46
#define _MSGID_le_audio_RecordFile 47
#define _MSGID_le_audio_GetSamples 48
#define _MSGID_le_audio_Stop 49
#define _MSGID_le_audio_Pause 50
#define _MSGID_le_audio_Flush 51
#define _MSGID_le_audio_Resume 52
#define _MSGID_le_audio_SetSamplePcmChannelNumber 53
#define _MSGID_le_audio_GetSamplePcmChannelNumber 54
#define _MSGID_le_audio_SetSamplePcmSamplingRate 55
#define _MSGID_le_audio_GetSamplePcmSamplingRate 56
#define _MSGID_le_audio_SetSamplePcmSamplingResolution 57
#define _MSGID_le_audio_GetSamplePcmSamplingResolution 58
#define _MSGID_le_audio_PlayDtmf 59
#define _MSGID_le_audio_PlaySignallingDtmf 60
#define _MSGID_le_audio_SetEncodingFormat 61
#define _MSGID_le_audio_GetEncodingFormat 62
#define _MSGID_le_audio_SetSampleAmrMode 63
#define _MSGID_le_audio_GetSampleAmrMode 64
#define _MSGID_le_audio_SetSampleAmrDtx 65
#define _MSGID_le_audio_GetSampleAmrDtx 66
#define _MSGID_le_audio_SetPlatformSpecificGain 67
#define _MSGID_le_audio_GetPlatformSpecificGain 68
#define _MSGID_le_audio_MuteCallWaitingTone 69
#define _MSGID_le_audio_UnmuteCallWaitingTone 70


#endif // LE_AUDIO_MESSAGES_H_INCLUDE_GUARD

