/**
 * This file is a part of Sonic Overlay
 * 
 * Fast, GXM Accelerated Overlays on the PSVita
 * 
 * Copyright (c) SonicMastr, GrapheneCt 2021 
 */

#ifndef PATCHES_H_
#define PATCHES_H_

#include <gxm.h>

SceGxmErrorCode sceGxmBeginScene_overlay(SceGxmContext *context, unsigned int flags, 
	                            const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion,
	                            SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, 
                                const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil);

SceGxmErrorCode sceGxmShaderPatcherCreate_overlay(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher);
SceGxmErrorCode sceGxmPadHeartbeat_overlay(const SceGxmColorSurface *displaySurface, SceGxmSyncObject *displaySyncObject);

SceGxmErrorCode sceGxmColorSurfaceInit_overlay(
	SceGxmColorSurface *surface,
	SceGxmColorFormat colorFormat,
	SceGxmColorSurfaceType surfaceType,
	SceGxmColorSurfaceScaleMode scaleMode,
	SceGxmOutputRegisterSize outputRegisterSize,
	uint32_t width,
	uint32_t height,
	uint32_t strideInPixels,
	void *data);

int sceSharedFbUpdateProcessEnd_overlay(SceUID masterShfbId);

SceGxmErrorCode sceGxmCreateRenderTargetInternal_overlay(const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget);

int sceSharedFbUpdateProcessBegin_overlay(SceUID shared_fb_id, void *a2, void *a3);

#endif /* PATCHES_H_ */
