/**
 * This file is a part of Sonic Overlay
 * 
 * Fast, GXM Accelerated Overlays on the PSVita
 * 
 * Copyright (c) SonicMastr, GrapheneCt 2021 
 */

#include <kernel.h>
#include <gxm.h>

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

void* alloc(size_t size) {
	SceUID memuid;
	void *addr;
	size = ALIGN(size, 4 * 1024);
	memuid = sceKernelAllocMemBlock("mem", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, size, NULL);
	if (memuid < 0)
		return NULL;
	if (sceKernelGetMemBlockBase(memuid, &addr) < 0)
		return NULL;
	return addr;
}

void graphicsFree(SceUID uid) {
	void *mem;
	if (sceKernelGetMemBlockBase(uid, &mem) < 0)
		return NULL;
	if (sceGxmUnmapMemory(mem) < 0)
		return NULL;
	if (sceKernelFreeMemBlock(uid) < 0)
		return NULL;
}

void* gpu_alloc_map(SceKernelMemBlockType type, SceGxmMemoryAttribFlags gpu_attrib, size_t size, SceUID *uid) {
	SceUID memuid;
	void *addr;
	if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW)
		size = ALIGN(size, 0x40000);
	else
		size = ALIGN(size, 0x1000);
	memuid = sceKernelAllocMemBlock("gpumem", type, size, NULL);
	if (memuid < 0)
		return NULL;
	if (sceKernelGetMemBlockBase(memuid, &addr) < 0)
		return NULL;
	if (sceGxmMapMemory(addr, size, gpu_attrib) < 0) {
		sceKernelFreeMemBlock(memuid);
		return NULL;
	}
	if (uid)
		*uid = memuid;
	return addr;
}

// void freeJpegTexture(Jpeg_texture *texture) {
// 	if (texture) {
// 		if (texture->gxm_rtgt) {
// 			sceGxmDestroyRenderTarget(texture->gxm_rtgt);
// 		}
// 		if(texture->depth_UID) {
// 			graphicsFree(texture->depth_UID);
// 		}
// 		if(texture->palette_UID) {
// 			graphicsFree(texture->palette_UID);
// 		}
// 		graphicsFree(texture->data_UID);
// 		graphicsFree(texture->textureUID);
// 		texture = NULL;
// 	}
// }