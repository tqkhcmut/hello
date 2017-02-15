/*
 * AUTO-GENERATED _componentMain.c for the myAudio component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../src/eventLoop.h"
#include "../src/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _myAudio_le_audio_ServiceInstanceName;
const char** le_audio_ServiceInstanceNamePtr = &_myAudio_le_audio_ServiceInstanceName;
void le_audio_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t myAudio_LogSession;
le_log_Level_t* myAudio_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _myAudio_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _myAudio_Init(void)
{
    LE_DEBUG("Initializing myAudio component library.");

    // Connect client-side IPC interfaces.
    le_audio_ConnectService();

    // Register the component with the Log Daemon.
    myAudio_LogSession = log_RegComponent("myAudio", &myAudio_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_myAudio_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
