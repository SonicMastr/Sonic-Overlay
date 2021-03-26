/**
 * This file is a part of Sonic Overlay Kernel
 *
 * Fast, GXM Accelerated Overlays on the PSVita
 *
 * Copyright (c) SonicMastr, GrapheneCt 2021
 */

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

SceBool soPeekDispatchForVsh(SceSize *pArgBlockSize)
{
	if (dispatchPending) {
		sceKernelMemcpyKernelToUser(pArgBlockSize, &currentDispatch.argBlockSize, sizeof(SceSize));
		return SCE_TRUE;
	}
	else
		return SCE_FALSE;
	return SCE_FALSE;
}

SceBool soWaitDispatchForVsh(SceSize *pArgBlockSize)
{
	sceKernelLockFastMutex(&freeMtx);
	sceKernelMemcpyKernelToUser(pArgBlockSize, &currentDispatch.argBlockSize, sizeof(SceSize));
	sceKernelUnlockFastMutex(&freeMtx);
}

SceVoid soGetDispatchForVsh(SoDispatch *pDispatch)
{
	//sceKernelMemcpyKernelToUser(pDispatch, &currentDispatch, (sizeof(SoDispatch) - sizeof(ScePVoid)));
	sceKernelMemcpyKernelToUser(pDispatch->pArgBlock, currentDispatch.pArgBlock, currentDispatch.argBlockSize);
	dispatchPending = SCE_FALSE;
	/*sceKernelUnlockFastMutex(&reqMtx);
	sceKernelLockFastMutex(&freeMtx);*/
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

int module_start(SceSize argc, void *args) 
{
	sceKernelInitializeFastMutex(&reqMtx, "SonicOverlayDispatchMtx", 0, 0);
	sceKernelInitializeFastMutex(&freeMtx, "SonicOverlayFreeMtx", 0, 0);
	sceKernelLockFastMutex(&freeMtx);

	currentDispatch.pArgBlock = tempArgBlock;

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) 
{
	return SCE_KERNEL_STOP_SUCCESS;
}