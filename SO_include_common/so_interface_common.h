#ifndef _SO_DISPATCH_COMMON_H_
#define _SO_DISPATCH_COMMON_H_

#include <scetypes.h>

#define SO_DISPATCH_MAGIC 0x497E0F1E

typedef enum SoDispatchId {
	SO_DISPATCH_ID_TEST_DRAW
} SoDispatchId;

typedef struct SoDispatch {
	SceUInt32 magic;
	SoDispatchId dispatchId;
	SceSize argBlockSize;
	ScePVoid pArgBlock;
} SoDispatch;

/* KERNEL */

SceBool soPeekDispatchForVsh(SceSize *pArgBlockSize);
SceBool soWaitDispatchForVsh(SceSize *pArgBlockSize);
SceVoid soGetDispatchForVsh(SoDispatch *pDispatch);

#endif