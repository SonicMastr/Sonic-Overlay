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

extern int releaseRet;
static char tempArgBlock[100];
static SoDispatch currentDispatch;

int soDispatchGetter(SceUInt32 args, void* argp)
{
	SceSize argBlockSize;

	sceKernelDelayThread(15000000);

	while (1) {
		if (soPeekDispatchForVsh(&argBlockSize)) {

			currentDispatch.pArgBlock = tempArgBlock;

			soGetDispatchForVsh(&currentDispatch);

			sceClibPrintf("argBlockSize: %d\n", argBlockSize);
			sceClibPrintf("a1: %d\n", *(SceInt32 *)&tempArgBlock[0]);
			sceClibPrintf("a2: %d\n", *(SceInt32 *)&tempArgBlock[4]);
		}
		else
			sceKernelDelayThread(15000);
	}
}

int module_start(SceSize argc, void *args) {
	SCE_DBG_LOG_INFO("Sonic Overlay Started\n");

	sceClibMemset(tempArgBlock, 0, 100);

	SceUID dgThread = sceKernelCreateThread("SODispatchGetter", soDispatchGetter, 160, 0x1000, 0, 0, NULL);
	sceKernelStartThread(dgThread, 0, NULL);

	initHooks();
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	SCE_DBG_LOG_INFO("Sonic Overlay Stopped\n");
	return SCE_KERNEL_STOP_SUCCESS;
}