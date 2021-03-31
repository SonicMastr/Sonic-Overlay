/**
 * This file is a part of Sonic Overlay Kernel
 *
 * Fast, GXM Accelerated Overlays on the PSVita
 *
 * Copyright (c) SonicMastr, GrapheneCt 2021
 */

#include <stdlib.h>
#include <stddef.h>
#include <scetypes.h>
#include <kernel.h>
#include <sce_atomic.h>
#include <kernel/sysmem.h>
#include <kernel/threadmgr.h>
#include <kernel/cpu.h>
#include <sblacmgr.h>

#include "so_interface_common.h"

#define SO_MAX_MODULES 25

static SceUID reqSem, freeSem, finSem, queueSem[SO_MAX_MODULES];
static SceBool dispatchPending = SCE_FALSE;
static SceUID mappedBlockId = SCE_UID_INVALID_UID;
static ScePVoid kernelSharedMem = SCE_NULL;
static SoModuleInfo soModuleInfo[SO_MAX_MODULES];
static SceInt8 soRenderOrder[SO_MAX_MODULES];
static SceInt8 soRenderCount;
static SceInt8 soRenderIndex = 0;

SceVoid soSortModuleInfo()
{
	int position, d, temp;
	SoModuleInfo tempModuleInfo, tempModuleInfoArray[SO_MAX_MODULES];
	//sceDebugPrintf("Sorting\n");
	memcpy(tempModuleInfoArray, soModuleInfo, sizeof(tempModuleInfoArray));
	for (int c = 0; c < (SO_MAX_MODULES - 1); c++) {
		position = c;

		for (d = c + 1; d < SO_MAX_MODULES; d++) {
			if (tempModuleInfoArray[position].priority > tempModuleInfoArray[d].priority)
			position = d;
		}
		if (position != c) {
			tempModuleInfo = tempModuleInfoArray[c];
			tempModuleInfoArray[c] = tempModuleInfoArray[position];
			tempModuleInfoArray[position] = tempModuleInfo;
		}
	}
	//sceDebugPrintf("Sorted. Now Organizing\n");
	memset(soRenderOrder, -1, sizeof(soRenderOrder));
	position = 0;
	for (int c = 0; c < SO_MAX_MODULES; c++) {
		if (tempModuleInfoArray[c].id != -1) {
			soRenderOrder[position] = tempModuleInfoArray[c].id;
			//sceDebugPrintf("Render Order ID: %d, Priority: %d, Position: %d\n", (SceInt32)soRenderOrder[position], (SceInt32)tempModuleInfoArray[c].priority, (SceInt32)position);
			position++;
		}
	}
	//sceDebugPrintf("Organaized\n");
	soRenderCount = position;
	//sceDebugPrintf("Render Count: %d\n", (SceInt32)soRenderCount);
}

SceVoid soWaitRenderQueueForKernel(SceInt8 id)
{
	sceKernelWaitSema(queueSem[id], 1, NULL);
	sceKernelSignalSema(queueSem[id], 1);
	//sceDebugPrintf("Render ID %d recieved Signal!\n", (SceInt32)id);
	sceKernelSignalSema(reqSem, 1);
}

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

SceVoid soTestDrawFinishForKernel(SceInt8 id)
{
	if (mappedBlockId < 0)
		return SO_ERROR_NOT_INITIALIZED;
	if (soRenderOrder[soRenderIndex] != id)
		return SO_ERROR_INVALID_ARGUMENT;
	sceKernelWaitSema(reqSem, 1, NULL);
	// sceDebugPrintf("Render Index: %d\n", (SceInt32)soRenderIndex);
	// sceDebugPrintf("Render ID: %d\n", (SceInt32)soRenderOrder[soRenderIndex]);
	sceKernelWaitSema(queueSem[soRenderOrder[soRenderIndex]], 1, NULL);
	soRenderIndex++;
	if (soRenderCount == 1 || soRenderIndex == soRenderCount)
	{
		sceKernelSignalSema(finSem, 1);
		soRenderIndex = 0;
		sceKernelWaitSema(finSem, 1, NULL);
	}
	sceKernelSignalSema(queueSem[soRenderOrder[soRenderIndex]], 1);
}

SceVoid soTestDrawForKernel(SceInt8 id)
{
	if (mappedBlockId < 0)
		return SO_ERROR_NOT_INITIALIZED;
	if (soRenderOrder[soRenderIndex] != id)
		return SO_ERROR_INVALID_ARGUMENT;
	sceKernelWaitSema(reqSem, 1, NULL);

	SoDispatch *currentDispatch = (SoDispatch *)kernelSharedMem;

	currentDispatch->magic = SO_DISPATCH_MAGIC;
	currentDispatch->dispatchId = SO_DISPATCH_ID_TEST_DRAW;
	currentDispatch->argBlockSize = 8;

	*(SceInt32 *)&currentDispatch->argBlock[0] = (SceInt32)id; // Changed to be ID for testing
	*(SceInt32 *)&currentDispatch->argBlock[4] = 321;

	sceKernelSignalSema(freeSem, 1);
}

SceInt8 soRegisterModuleForKernel(SceInt8 priority)
{
	if (priority < 1 || priority > 100)
		return SO_ERROR_INVALID_ARGUMENT;

	//sceDebugPrintf("Registering Module\n");
	 // Make sure no draw calls can be made while registering
	sceKernelWaitSema(finSem, 1, NULL);

	int i = 0;
	while (i < SO_MAX_MODULES && soModuleInfo[i].id != -1)
		i++;
	if (i == SO_MAX_MODULES) {
		sceKernelSignalSema(finSem, 1);
		return -1004; // Too many modules registered
	}
	soModuleInfo[i].id = i;
	soModuleInfo[i].priority = priority;

	soSortModuleInfo();

	queueSem[i] = sceKernelCreateSema("SonicOverlayQueueSem", SCE_KERNEL_SEMA_ATTR_TH_FIFO, 1, 1, NULL); // Create Queue Semaphore for ID
	sceKernelWaitSema(queueSem[i], 1, NULL);
	sceKernelSignalSema(queueSem[soRenderOrder[soRenderIndex]], 1);

	sceKernelSignalSema(finSem, 1); // Done registering module. Continue with drawing
	return soModuleInfo[i].id;
}

SceInt8 soRemoveModuleForKernel(SceInt8 id)
{
	if (id < 0 || id >= SO_MAX_MODULES || soModuleInfo[id].id == -1)
		return SO_ERROR_INVALID_ARGUMENT;

	if (soRenderCount < 1) // Can't Remove a module by itself if waiting for ending
		sceKernelWaitSema(finSem, 1, NULL); // Make sure no draw calls can be made while removing

	//sceDebugPrintf("Removing Module ID %d\n", (SceInt32)id);
	soModuleInfo[id].id = -1;
	soModuleInfo[id].priority = -1;

	sceKernelDeleteSema(queueSem[id]); // Delete Semaphore for ID

	soSortModuleInfo();

	sceKernelSignalSema(finSem, 1); // Done removing module. Continue with drawing
	return SCE_OK;
}

int module_start(SceSize argc, void *args) 
{
	memset(soModuleInfo, -1, sizeof(soModuleInfo));
	reqSem = sceKernelCreateSema("SonicOverlayDispatchSem", SCE_KERNEL_SEMA_ATTR_TH_FIFO, 1, 1, NULL);
	freeSem = sceKernelCreateSema("SonicOverlayFreeSem", SCE_KERNEL_SEMA_ATTR_TH_FIFO, 1, 1, NULL);
	finSem = sceKernelCreateSema("SonicOverlayFinishedSem", SCE_KERNEL_SEMA_ATTR_TH_FIFO, 1, 1, NULL);
	sceKernelWaitSema(freeSem, 1, NULL);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) 
{
	return SCE_KERNEL_STOP_SUCCESS;
}