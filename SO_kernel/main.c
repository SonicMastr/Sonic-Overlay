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
#include <sblacmgr.h>

#include "so_interface_common.h"

static SceUID reqSem, freeSem;
static SceBool dispatchPending = SCE_FALSE;
static SceUID mappedBlockId = SCE_UID_INVALID_UID;
static ScePVoid kernelSharedMem = SCE_NULL;

SceVoid soWaitDispatchForVsh()
{
	sceKernelWaitSema(freeSem, 1, NULL);
	sceKernelSignalSema(freeSem, 1);
}

SceVoid soDispatchDoneForVsh()
{
	sceKernelWaitSema(freeSem, 1, NULL);
	sceKernelSignalSema(reqSem, 1);
}

SceInt32 soInitForVsh(const ScePVoid buf, SceSize size)
{
	if (!sceSblACMgrIsSceShell(0))
		return SO_ERROR_PROHIBITED;

	if (buf == SCE_NULL || size == 0)
		return SO_ERROR_INVALID_ARGUMENT;

	if (mappedBlockId > 0)
		return SO_ERROR_ALREADY_INITIALIZED;

	ScePVoid kernelPage = SCE_NULL;
	SceSize kernelSize = 0;
	SceSize kernelOffset = 0;

	mappedBlockId = sceKernelMapUserBlockDefaultType("SonicOverlaySharedMemKernel", 3, buf, size, &kernelPage, &kernelSize, &kernelOffset);
	if (mappedBlockId < 0)
		return mappedBlockId;

	kernelSharedMem = (const ScePVoid)(((uintptr_t)kernelPage) + kernelOffset);

	return SCE_OK;
}

SceVoid soTestDrawForKernel()
{
	if (mappedBlockId < 0)
		return SO_ERROR_NOT_INITIALIZED;

	sceKernelWaitSema(reqSem, 1, NULL);

	SoDispatch *currentDispatch = (SoDispatch *)kernelSharedMem;

	currentDispatch->magic = SO_DISPATCH_MAGIC;
	currentDispatch->dispatchId = SO_DISPATCH_ID_TEST_DRAW;
	currentDispatch->argBlockSize = 8;

	*(SceInt32 *)&currentDispatch->argBlock[0] = 123;
	*(SceInt32 *)&currentDispatch->argBlock[4] = 321;

	sceKernelSignalSema(freeSem, 1);
}

int module_start(SceSize argc, void *args) 
{
	reqSem = sceKernelCreateSema("SonicOverlayDispatchSem", SCE_KERNEL_SEMA_ATTR_TH_FIFO, 1, 1, NULL);
	freeSem = sceKernelCreateSema("SonicOverlayfreeSem", SCE_KERNEL_SEMA_ATTR_TH_FIFO, 1, 1, NULL);
	sceKernelWaitSema(freeSem, 1, NULL);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) 
{
	return SCE_KERNEL_STOP_SUCCESS;
}