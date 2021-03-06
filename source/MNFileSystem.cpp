//
//  MNFileSystem.cpp
//  me_app
//
//  Created by hahasasa on 3/2/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#include <stdio.h>
#include "MNFileSystem.h"
#include "MNPlatformDefine.h"
#include "MNPrimaryType.h"

#if defined(PLATFORM_IOS) || defined(PLATFORM_OSX) || defined(PLATFORM_WIN32)
const tstring& MNResourceFolderPath(const char* path)
{
    static tstring s_basePath(tstring(path) + "/");
    return s_basePath;
}

FILE* mnfopen(const char* path, const char* mode)
{
    tstring fullPath = MNResourceFolderPath() + path;;
    FILE* file = fopen(fullPath.c_str(), mode);
    return file;
}

#elif defined(PLATFORM_ANDROID)

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <errno.h>

int android_read(void* cookie, char* buf, int size)
{
    return AAsset_read((AAsset*)cookie, buf, size);
}

int android_write(void* cookie, const char* buf, int size)
{
    return EACCES; // can't provide write access to the apk
}

fpos_t android_seek(void* cookie, fpos_t offset, int whence)
{
    return AAsset_seek((AAsset*)cookie, offset, whence);
}

int android_close(void* cookie)
{
    AAsset_close((AAsset*)cookie);
    return 0;
}

FILE* mnfopen(const char* path, const char* mode)
{
    AAsset* asset = AAssetManager_open( m_assetMgr, filePath.c_str(), 0);
    if( asset == NULL ) return NULL;
    
    FILE* file = funopen(asset, android_read, android_write, android_seek, android_close);
    return new SWFileInputStream( file );
    return file;
}

#endif

