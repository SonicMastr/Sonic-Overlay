#ifndef _SO_KERNEL_H_
#define _SO_KERNEL_H_

#include <scetypes.h>

SceVoid soTestDrawForKernel(SceChar8 id);
SceChar8 soRegisterModuleForKernel(SceChar8 priority);
SceChar8 soRemoveModuleForKernel(SceChar8 id);
SceVoid soTestDrawFinishForKernel(SceChar8 id);
SceVoid soWaitRenderQueueForKernel(SceChar8 id);

#endif