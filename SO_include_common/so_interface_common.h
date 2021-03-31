#ifndef _SO_DISPATCH_COMMON_H_
#define _SO_DISPATCH_COMMON_H_

#include <scetypes.h>

#define SO_DISPATCH_MAGIC 0x534F4450
#define SO_DISPATCH_SHARED_MEM_SIZE 0x1000

typedef enum soError {
	SO_ERROR_ALREADY_INITIALIZED = -1000,
	SO_ERROR_INVALID_ARGUMENT = -1001,
	SO_ERROR_PROHIBITED = -1002,
	SO_ERROR_NOT_INITIALIZED = -1003

};

typedef struct SoModuleInfo {
	SceChar8 id;
	SceChar8 priority;
} SoModuleInfo;

typedef enum SoDispatchId {
	SO_DISPATCH_ID_TEST_DRAW
} SoDispatchId;

typedef struct SoDispatch {
	SceUInt32 magic;
	SoDispatchId dispatchId;
	SceSize argBlockSize;
	SceUChar8 argBlock[SO_DISPATCH_SHARED_MEM_SIZE - 0xC];
} SoDispatch;

/* KERNEL */

SceVoid soWaitDispatchForVsh();
SceVoid soDispatchDoneForVsh();
SceInt32 soInitForVsh(const ScePVoid buf, SceSize size);

#endif