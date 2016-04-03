#pragma once
#include "MNPlatformDefine.h"

#ifdef __cplusplus
extern "C" {
#endif

    
#if defined(PLATFORM_OSX) || defined(PLATFORM_IOS) || defined(PLATFORM_WIN32)
    void MNStart(const char* resourceFolder);
	void MNUpdate();
    void MNRender();
	void MNTouch(int type, int posX, int posY);
#endif
    
#ifdef __cplusplus
}
#endif