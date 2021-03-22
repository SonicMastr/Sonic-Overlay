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
#include <libsysmodule.h>

#include "../include/hooks.h"
#include "../include/patches.h"

static int addingWidgets = 0;

#define HOOK(id, nid, func) taiHookFunctionImport(hook_ref+id, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, nid, func)
#define HOOK_PAF(id, nid, func) taiHookFunctionImport(hook_ref+id, "ScePaf", TAI_ANY_LIBRARY, nid, func)

int scePafWidgetSetColor_patched(void *pWidget, int a2, int a3, int a4, const ScePafWidgetColor *pColor)
{
	if (addingWidgets) {
		ScePafWidgetColor newColor;
		newColor.red = 0.0f;
		newColor.green = 0.0f;
		newColor.blue = 0.0f;
		newColor.alpha = 1.0f;

		return TAI_CONTINUE(int, hook_ref[7], pWidget, a2, a3, a4, &newColor);
	}
	else
		return TAI_CONTINUE(int, hook_ref[7], pWidget, a2, a3, a4, pColor);
}

int scePafWidgetSetPosition_patched(void *pWidget, ScePafWidgetPos *pWidgetPos, float a3, int a4, int a5, int a6, int a7, int a8)
{
	if (addingWidgets) {

		ScePafWidgetPos newpos;
		newpos.x = 0.0f - (960.0f / 2.0f);
		newpos.y = (544.0f / 2.0f) + 30.0f;
		newpos.z = 0.0f;
		newpos.unk_0x0C = 0;

		addingWidgets = 0;
		return TAI_CONTINUE(int, hook_ref[8], pWidget, &newpos, a3, a4, a5, a6, a7, a8);
	}
	else
		return TAI_CONTINUE(int, hook_ref[8], pWidget, pWidgetPos, a3, a4, a5, a6, a7, a8);
}

void hook_paf()
{
	hook[1] = HOOK_PAF(1, 0xC8A0F04E, sceGxmCreateRenderTargetInternal_overlay);
	hook[2] = HOOK_PAF(2, 0x8734FF4E, sceGxmBeginScene_overlay);
	hook[3] = HOOK_PAF(3, 0x05032658, sceGxmShaderPatcherCreate_overlay);
	hook[4] = HOOK_PAF(4, 0x565A9AB6, sceSharedFbUpdateProcessEnd_overlay);
	hook[9] = HOOK_PAF(9, 0xF9754AD9, sceSharedFbUpdateProcessBegin_overlay);
	/*SCE_DBG_LOG_INFO("hookRet0: 0x%08x\n", hookRet0);
	SCE_DBG_LOG_INFO("hookRet1: 0x%08x\n", hookRet1);*/
}

int sceSysmoduleLoadModuleInternalWithArg_patched(SceSysmoduleInternalModuleId id, SceSize args, void *argp, void *unk) {
	int res = TAI_CONTINUE(int, hook_ref[0], id, args, argp, unk);
	if (res >= 0 && id == 0x80000008) {
		hook_paf();
		hook[7] = HOOK(7, 0xCD20EF38, scePafWidgetSetColor_patched);
		hook[8] = HOOK(8, 0xE29AB31F, scePafWidgetSetPosition_patched);
		taiHookRelease(hook[0], hook_ref[0]);
	}
	return res;
}

void add_red_widgets(void *someObj)
{
	addingWidgets = 1;
	TAI_CONTINUE(void, hook_ref[6], someObj);
}

int sceSblPmMgrGetCurrentMode_patched(int *mode)
{
	if (addingWidgets) {
		*mode = 1;
		return 0;
	}
	else
		return TAI_CONTINUE(int, hook_ref[5], mode);
}

SceInt initHooks(void)
{
	int add_red_widget_offset;
	tai_module_info_t info;
	info.size = sizeof(info);
	taiGetModuleInfo("SceShell", &info);

	switch(info.module_nid) {
		case 0x0552F692: // 3.60 retail
		case 0x532155E5: // 3.61 retail
		case 0xBB4B0A3E: // 3.63 retail
			add_red_widget_offset = 0x14F466;
			break;
		case 0x5549BF1F: // 3.65 retail
		case 0x34B4D82E: // 3.67 retail
		case 0x12DAC0F3: // 3.68 retail
		case 0x0703C828: // 3.69 retail
		case 0x2053B5A5: // 3.70 retail
		case 0xF476E785: // 3.71 retail
		case 0x939FFBE9: // 3.72 retail
		case 0x734D476A: // 3.73 retail
			add_red_widget_offset = 0x14F4BE;
			break;
		case 0xEAB89D5C: // 3.60 testkit
			add_red_widget_offset = 0x14789A;
			break;
		case 0x587F9CED: // 3.65 testkit
			add_red_widget_offset = 0x1478F2;
			break;
		default:
			return -1;
	}

	hook[0] = HOOK(0, 0xC3C26339, sceSysmoduleLoadModuleInternalWithArg_patched);
	hook[5] = HOOK(5, 0xDA4EDEBF, sceSblPmMgrGetCurrentMode_patched);

	hook[6] = taiHookFunctionOffset(
		&hook_ref[6], 
		info.modid,
		0,
		add_red_widget_offset,
		1,
		add_red_widgets);

    //SCE_DBG_LOG_INFO("PAF modstart hook: 0x%08x\n", pafColorHook);
    return SCE_OK;
}