#include <stddef.h>
#include <scetypes.h>
#include <kernel.h>
#include <kernel/sysmem.h>
#include <kernel/threadmgr.h>

#include "so_interface_common.h"

static char tempArgBlock[128];
static SoDispatch currentDispatch;
static SceKernelFastMutexWork reqMtx;
static SceKernelFastMutexWork freeMtx;
static SceBool dispatchPending = SCE_FALSE;

SceBool soPeekDispatchForUser(SceSize *pArgBlockSize)
{
	if (dispatchPending) {
		sceKernelMemcpyKernelToUser(pArgBlockSize, &currentDispatch.argBlockSize, sizeof(SceSize));
		return SCE_TRUE;
	}
	else
		return SCE_FALSE;
}

SceBool soWaitDispatchForUser(SceSize *pArgBlockSize)
{
	sceKernelLockFastMutex(&freeMtx);
	sceKernelMemcpyKernelToUser(pArgBlockSize, &currentDispatch.argBlockSize, sizeof(SceSize));
	sceKernelUnlockFastMutex(&freeMtx);
}

SceVoid soGetDispatchForUser(SoDispatch *pDispatch)
{
	sceKernelMemcpyKernelToUser(pDispatch, &currentDispatch, (sizeof(SoDispatch) - sizeof(ScePVoid)));
	sceKernelMemcpyKernelToUser(pDispatch->pArgBlock, &currentDispatch.pArgBlock, currentDispatch.argBlockSize);
	dispatchPending = SCE_FALSE;
	sceKernelUnlockFastMutex(&reqMtx);
	sceKernelLockFastMutex(&freeMtx);
}

SceVoid soTestDraw()
{
	sceKernelLockFastMutex(&reqMtx);
	currentDispatch.magic = SO_DISPATCH_MAGIC;
	currentDispatch.dispatchId = SO_DISPATCH_ID_TEST_DRAW;
	currentDispatch.argBlockSize = 8;

	*(SceInt32 *)&tempArgBlock[0] = 123;
	*(SceInt32 *)&tempArgBlock[4] = 321;

	dispatchPending = SCE_TRUE;
	sceKernelUnlockFastMutex(&freeMtx);
}

SceVoid soInitialize()
{
	sceKernelInitializeFastMutex(&reqMtx, "SonicOverlayDispatchMtx", 0, 0);
	sceKernelInitializeFastMutex(&freeMtx, "SonicOverlayFreeMtx", 0, 0);
	sceKernelLockFastMutex(&freeMtx);

	currentDispatch.pArgBlock = tempArgBlock;
}