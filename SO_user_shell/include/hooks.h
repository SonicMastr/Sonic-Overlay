/**
 * This file is a part of Sonic Overlay
 * 
 * Fast, GXM Accelerated Overlays on the PSVita
 * 
 * Copyright (c) SonicMastr, GrapheneCt 2021 
 */

#ifndef HOOKS_H_
#define HOOKS_H_

#include <taihen.h>

#define SCE_PAF_WIDGET_COLOR_UNKNOWN_1   (1)
#define SCE_PAF_WIDGET_COLOR_TEXT        (2)
#define SCE_PAF_WIDGET_COLOR_TEXT_SHADOW (3)
#define SCE_PAF_WIDGET_COLOR_BACK_BAR    (4)
#define SCE_PAF_WIDGET_COLOR_UNKNOWN_5   (5)

typedef struct ScePafWidgetColor {
	float red;
	float green;
	float blue;
	float alpha;
} ScePafWidgetColor;

typedef struct ScePafWidgetPos {
	float x;      // Distance from display center
	float y;      // Distance from display center
	float z;
	int unk_0x0C; // set 0
} ScePafWidgetPos;

#define MAX_HOOKS 10
SceUID hook[MAX_HOOKS];
tai_hook_ref_t hook_ref[MAX_HOOKS];

SceInt initHooks(void);

#endif /* HOOKS_H_ */
