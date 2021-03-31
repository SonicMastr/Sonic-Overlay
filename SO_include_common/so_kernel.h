#ifndef _SO_KERNEL_H_
#define _SO_KERNEL_H_

#include <scetypes.h>

SceVoid soTestDrawForKernel(SceInt8 id);
SceInt8 soRegisterModuleForKernel(SceInt8 priority);
SceInt8 soRemoveModuleForKernel(SceInt8 id);
SceVoid soTestDrawFinishForKernel(SceInt8 id);
SceVoid soWaitRenderQueueForKernel(SceInt8 id);

#endif