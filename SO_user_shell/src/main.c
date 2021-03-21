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

#include "../include/common.h"
#include "../include/hooks.h"
extern int releaseRet;

int initThreadEntry(SceUInt32 args, void* argp)
{
	while (1) {
		sceClibPrintf("Surface: %d\n", releaseRet);
		sceKernelDelayThread(100000);
	}
}

int module_start(SceSize argc, void *args) {
	SCE_DBG_LOG_INFO("Sonic Overlay Started\n");

	//SceUID initThread = sceKernelCreateThread("SOInitThread", initThreadEntry, 191, 0x1000, 0, 0, NULL);
	//sceKernelStartThread(initThread, 0, NULL);

	initHooks();
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	SCE_DBG_LOG_INFO("Sonic Overlay Stopped\n");
	return SCE_KERNEL_STOP_SUCCESS;
}