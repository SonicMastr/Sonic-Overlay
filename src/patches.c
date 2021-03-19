/**
 * This file is a part of Sonic Overlay
 * 
 * Fast, GXM Accelerated Overlays on the PSVita
 * 
 * Copyright (c) SonicMastr, GrapheneCt 2021 
 */

#include <stdlib.h>
#include <stdio.h>
#include <kernel.h>
#include <libdbg.h>
//#include <vita2d_sys.h>

#include "../include/hooks.h"
#include "../include/patches.h"

extern const SceGxmProgram _binary_texture_v_gxp_start;
extern const SceGxmProgram _binary_texture_f_gxp_start;
extern const SceGxmProgram _binary_color_v_gxp_start;
extern const SceGxmProgram _binary_color_f_gxp_start;
static const SceGxmProgram *const colorVertexProgramGxp	    = &_binary_color_v_gxp_start;
static const SceGxmProgram *const colorFragmentProgramGxp   = &_binary_color_f_gxp_start;
static const SceGxmProgram *const textureVertexProgramGxp	= &_binary_texture_v_gxp_start;
static const SceGxmProgram *const textureFragmentProgramGxp	= &_binary_texture_f_gxp_start;
SceGxmShaderPatcherId	vertexProgramId;
SceGxmShaderPatcherId	colorVertexProgramId;
SceGxmShaderPatcherId	fragmentProgramId;
SceGxmShaderPatcherId	colorFragmentProgramId;
SceGxmVertexProgram		*vertexProgram = NULL;
SceGxmVertexProgram		*colorVertexProgram = NULL;
SceGxmFragmentProgram	*fragmentProgram = NULL;
SceGxmFragmentProgram	*colorFragmentProgram = NULL;
static const SceGxmProgramParameter *paramPositionAttribute, *paramColorPositionAttribute = NULL;
static const SceGxmProgramParameter *paramTextureAttribute, *paramColorColorAttribute = NULL;

typedef struct VertexTexture { float x, y, z, u, v;} VertexTexture;
typedef struct VertexColor {float x, y, z; unsigned int color;} VertexColor;
static SceUID					verticesUid, colorVerticesUid;
static SceUID					indicesUid, colorIndicesUid;
static VertexTexture	*vertices = NULL;
VertexColor      *colorVertices = NULL;
static SceUInt16	*indices, *colorIndices = NULL;

SceBool drawing, active, isWithinScene = 0;

/*static SceGxmContext *lastContext = NULL;
static unsigned int lastFlags;
static SceGxmRenderTarget *lastRenderTarget = NULL;
static SceGxmValidRegion *lastValidRegion = NULL;
static SceGxmSyncObject *lastVertexSyncObject, *lastFragmentSyncObject = NULL;
static SceGxmDepthStencilSurface *lastDepthStencil = NULL;

static SceGxmColorSurface *displaySurface[2];*/
static int cSurfaceIndex = 0;
static int cSurfaceIndexWork = 0;

static SceGxmContext *cContext = SCE_NULL;
static SceGxmRenderTarget *cRenderTarget = SCE_NULL;
static SceGxmShaderPatcher *cShaderPatcher = SCE_NULL;
static SceGxmDepthStencilSurface cDepthStencil;
static SceGxmColorSurface cDisplaySurface[2];
static SceGxmSyncObject *cDisplayBufferSync[2];

int releaseRet = -1;

//int(*_vita2d_init_external)(vita2d_init_param_external *param);

SceGxmErrorCode sceGxmCreateRenderTargetInternal_overlay(const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget)
{
	int ret = TAI_CONTINUE(SceGxmErrorCode, hook_ref[1], params, renderTarget);

	if (params->width == 960 && params->height == 544) {
		cRenderTarget = *renderTarget;
		taiHookRelease(hook[1], hook_ref[1]);
	}

	return ret;
}



static int beginSceneHookIndex = 0;
SceGxmErrorCode sceGxmBeginScene_overlay(SceGxmContext *context, unsigned int flags, 
	                            const SceGxmRenderTarget *renderTarget, const SceGxmValidRegion *validRegion,
	                            SceGxmSyncObject *vertexSyncObject, SceGxmSyncObject *fragmentSyncObject, 
                                const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil)
{
	int ret = TAI_CONTINUE(SceGxmErrorCode, hook_ref[2], context, flags, renderTarget, validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);

	if (renderTarget == cRenderTarget) {
		cContext = context;
		cDisplayBufferSync[beginSceneHookIndex] = fragmentSyncObject;
		cDisplaySurface[beginSceneHookIndex] = *colorSurface;
		cDepthStencil = *depthStencil;
		beginSceneHookIndex++;
		if (beginSceneHookIndex == 2) {
			releaseRet = taiHookRelease(hook[2], hook_ref[2]);
		}
	}

	return ret;
}

SceGxmErrorCode sceGxmShaderPatcherCreate_overlay(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher)
{
    int ret = TAI_CONTINUE(SceGxmErrorCode, hook_ref[3], params, shaderPatcher);

	int err = SCE_OK;
	err = sceGxmProgramCheck(textureVertexProgramGxp);
	err = sceGxmProgramCheck(textureFragmentProgramGxp);
	err = sceGxmProgramCheck(colorVertexProgramGxp);
	err = sceGxmProgramCheck(colorFragmentProgramGxp);
	err = sceGxmShaderPatcherRegisterProgram(*shaderPatcher, textureVertexProgramGxp, &vertexProgramId);
	err = sceGxmShaderPatcherRegisterProgram(*shaderPatcher, textureFragmentProgramGxp, &fragmentProgramId);
	err = sceGxmShaderPatcherRegisterProgram(*shaderPatcher, colorVertexProgramGxp, &colorVertexProgramId);
	err = sceGxmShaderPatcherRegisterProgram(*shaderPatcher, colorFragmentProgramGxp, &colorFragmentProgramId);
	paramPositionAttribute = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "aPosition");
	paramTextureAttribute = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "aTexcoord");
	paramColorPositionAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "aPosition");
	paramColorColorAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "aColor");
	if (paramTextureAttribute == NULL || paramPositionAttribute == NULL || paramColorPositionAttribute == NULL || paramColorColorAttribute == NULL)
		return -1;

	SceGxmVertexAttribute vertexAttributes[2];
	SceGxmVertexStream vertexStreams[1];
	vertexAttributes[0].streamIndex = 0;
	vertexAttributes[0].offset = 0;
	vertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	vertexAttributes[0].componentCount = 3;
	vertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramPositionAttribute);
	vertexAttributes[1].streamIndex = 0;
	vertexAttributes[1].offset = 12;
	vertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	vertexAttributes[1].componentCount = 2;
	vertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramTextureAttribute);
	vertexStreams[0].stride = sizeof(VertexTexture);
	vertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	err = sceGxmShaderPatcherCreateVertexProgram(
		*shaderPatcher,
		vertexProgramId,
		vertexAttributes, 2,
		vertexStreams, 1,
		&vertexProgram);

	SceGxmBlendInfo blendInfo;

	blendInfo.colorMask = SCE_GXM_COLOR_MASK_ALL;
	blendInfo.colorFunc = SCE_GXM_BLEND_FUNC_ADD;
	blendInfo.alphaFunc = SCE_GXM_BLEND_FUNC_ADD;
	blendInfo.colorSrc = SCE_GXM_BLEND_FACTOR_SRC_ALPHA;
	blendInfo.colorDst = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendInfo.alphaSrc = SCE_GXM_BLEND_FACTOR_ONE;
	blendInfo.alphaDst = SCE_GXM_BLEND_FACTOR_ZERO;

	err = sceGxmShaderPatcherCreateFragmentProgram(
		*shaderPatcher,
		fragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		SCE_GXM_MULTISAMPLE_NONE,
		&blendInfo,
		textureVertexProgramGxp,
		&fragmentProgram);

	SceGxmVertexAttribute colorVertexAttributes[2];
	SceGxmVertexStream colorVertexStreams[1];
	colorVertexAttributes[0].streamIndex = 0;
	colorVertexAttributes[0].offset = 0;
	colorVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	colorVertexAttributes[0].componentCount = 3;
	colorVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorPositionAttribute);
	colorVertexAttributes[1].streamIndex = 0;
	colorVertexAttributes[1].offset = 12;
	colorVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N;
	colorVertexAttributes[1].componentCount = 4;
	colorVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorColorAttribute);
	colorVertexStreams[0].stride = sizeof(VertexColor);
	colorVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	err = sceGxmShaderPatcherCreateVertexProgram(
		*shaderPatcher,
		colorVertexProgramId,
		colorVertexAttributes, 2,
		colorVertexStreams, 1,
		&colorVertexProgram);

	err = sceGxmShaderPatcherCreateFragmentProgram(
		*shaderPatcher,
		colorFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		SCE_GXM_MULTISAMPLE_NONE,
		&blendInfo,
		colorVertexProgramGxp,
		&colorFragmentProgram);

	/* Sometimes this may be ran more than once. Need to account for that */
	if (vertices)
		graphicsFree(verticesUid);
	if (colorVertices)
		graphicsFree(colorVerticesUid);
	if (indices)
		graphicsFree(indicesUid);
	// if (texture)
	// 	freeJpegTexture(texture);
	vertices = gpu_alloc_map(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_MEMORY_ATTRIB_READ, 4 * sizeof(VertexTexture), &verticesUid);
	colorVertices = gpu_alloc_map(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_MEMORY_ATTRIB_READ, 4 * sizeof(VertexColor), &colorVerticesUid);
	indices = gpu_alloc_map(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_MEMORY_ATTRIB_READ, 4 * sizeof(unsigned short), &indicesUid);
	int i;
	for (i = 0; i < 4; i++) {
		indices[i] = i;
	}

	cShaderPatcher = *shaderPatcher;
	taiHookRelease(hook[3], hook_ref[3]);

	return ret;
	
}

/*SceGxmErrorCode sceGxmColorSurfaceInit_overlay(
	SceGxmColorSurface *surface,
	SceGxmColorFormat colorFormat,
	SceGxmColorSurfaceType surfaceType,
	SceGxmColorSurfaceScaleMode scaleMode,
	SceGxmOutputRegisterSize outputRegisterSize,
	uint32_t width,
	uint32_t height,
	uint32_t strideInPixels,
	void *data)
{
	SceGxmErrorCode ret = 0;

	ret = TAI_CONTINUE(SceGxmErrorCode, hook_ref[3], surface, colorFormat, surfaceType, scaleMode, outputRegisterSize, width, height, strideInPixels, data);

	if (width == 960 && height == 544 && cSurfaceIndex < 2) {
		displaySurface[cSurfaceIndex] = surface;
	}

	return ret;
}*/

int sceSharedFbUpdateProcessEnd_overlay(SceUID masterShfbId)
{
	int ret = TAI_CONTINUE(SceGxmErrorCode, hook_ref[4], masterShfbId);
    if (cContext != NULL) {
        //drawing = SCE_TRUE;
		if (sceGxmIsWithinScene())
			isWithinScene = SCE_TRUE;
		else
			isWithinScene = SCE_FALSE;
		if (!isWithinScene)
			sceGxmBeginScene(cContext, 0, cRenderTarget, NULL, NULL, cDisplayBufferSync[cSurfaceIndexWork], &cDisplaySurface[cSurfaceIndexWork], &cDepthStencil);

        colorVertices[0].x = -30.0f/960.0f;
		colorVertices[0].y = -60.0f/544.0f;
		colorVertices[0].z = 0.0f;
		colorVertices[0].color = 0xFFFF00FF;

		colorVertices[1].x = 30.0f/960.0f;
		colorVertices[1].y = -60.0f/544.0f;
		colorVertices[1].z = 0.0f;
		colorVertices[1].color = 0xFF00FFFF;

		colorVertices[2].x = -30.0f/960.0f;
		colorVertices[2].y = 60.0f/544.0f;
		colorVertices[2].z = 0.0f;
		colorVertices[2].color = 0xFFFFFF00;

		colorVertices[3].x = 30.0f/960.0f;
		colorVertices[3].y = 60.0f/544.0f;
		colorVertices[3].z = 0.0f;
		colorVertices[3].color = 0xFFFF0000;

        //Set texture shaders
		sceGxmSetVertexProgram(cContext, colorVertexProgram);
		sceGxmSetFragmentProgram(cContext, colorFragmentProgram);

        //Depth/Stencil Tests
		sceGxmSetFrontDepthFunc(
			cContext,
			SCE_GXM_DEPTH_FUNC_ALWAYS
		);
		sceGxmSetFrontStencilFunc(
			cContext,
			SCE_GXM_STENCIL_FUNC_ALWAYS,
			SCE_GXM_STENCIL_OP_KEEP,
			SCE_GXM_STENCIL_OP_KEEP,
			SCE_GXM_STENCIL_OP_KEEP,
			0xFF,
			0xFF);
		//Draw the texture	
		sceGxmSetVertexStream(cContext, 0, colorVertices);
		sceGxmDraw(cContext, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, indices, 4);
		if (!isWithinScene)
        	sceGxmEndScene(cContext, NULL, NULL);
    }

	cSurfaceIndexWork = (cSurfaceIndexWork + 1) % 2;

	return ret;
}
