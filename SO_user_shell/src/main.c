/**
 * This file is a part of Sonic Overlay
 * 
 * Fast, GXM Accelerated Overlays on the PSVita
 * 
 * Copyright (c) SonicMastr, GrapheneCt 2021 
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <kernel.h>
#include <libdbg.h>

#include "common.h"
#include "hooks.h"
#include "so_interface_common.h"

#define SCE_KERNEL_MEMORY_ACCESS_R_SHARED		0x10
#define SCE_KERNEL_MEMORY_ACCESS_W_SHARED		0x30

extern int releaseRet;
static SceUID sharedMemoryId = SCE_UID_INVALID_UID;
static ScePVoid sharedMemory = SCE_NULL;


int soDispatchGetter(SceUInt32 args, void* argp)
{
	SceSize argBlockSize;
	SoDispatch *currentDispatch = (SoDispatch *)sharedMemory;

	while (1) {
		soWaitDispatchForVsh();

		sceClibPrintf("argBlockSize: %d\n", currentDispatch->argBlockSize);
		sceClibPrintf("a1: %d\n", *(SceInt32 *)&currentDispatch->argBlock[0]);
		sceClibPrintf("a2: %d\n", *(SceInt32 *)&currentDispatch->argBlock[4]);

		memset(currentDispatch, 0, 0xC);
		soDispatchDoneForVsh();

		sceKernelDelayThread(15000);
	}
}

int module_start(SceSize argc, void *args) {
	SCE_DBG_LOG_INFO("Sonic Overlay Started\n");

	SceKernelAllocMemBlockOptInternal opt;
	sceClibMemset(&opt, 0, sizeof(SceKernelAllocMemBlockOptInternal));
	opt.size = sizeof(SceKernelAllocMemBlockOptInternal);
	opt.attr = 0x4020;
	opt.flags = SCE_KERNEL_MEMORY_ACCESS_R_SHARED | SCE_KERNEL_MEMORY_ACCESS_W_SHARED;

	sharedMemoryId = sceKernelAllocMemBlock("SonicOverlayDispatchBuffer", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, SO_DISPATCH_SHARED_MEM_SIZE, &opt);
	sceKernelGetMemBlockBase(sharedMemoryId, &sharedMemory);
	sceClibMemset(sharedMemory, 0, SO_DISPATCH_SHARED_MEM_SIZE);

	soInitForVsh(sharedMemory, SO_DISPATCH_SHARED_MEM_SIZE);

	SceUID dgThread = sceKernelCreateThread("SonicOverlayDispatchGetter", soDispatchGetter, 160, 0x1000, 0, 0, NULL);
	sceKernelStartThread(dgThread, 0, NULL);

	initHooks();
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	SCE_DBG_LOG_INFO("Sonic Overlay Stopped\n");
	return SCE_KERNEL_STOP_SUCCESS;
}