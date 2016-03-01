//
//  Header.h
//  me_app
//
//  Created by hahasasa on 3/2/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//
#ifndef MNPlatformDefine_h__
#define MNPlatformDefine_h__

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef WIN32

# define PLATFORM_WIN32 (1)

#elif TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE

# define PLATFORM_IOS (1)

#elif TARGET_OS_MAC

# define PLATFORM_OSX (1)

#elif ANDROID

# define PLATFORM_ANDROID (1)

#endif


#endif //! MNPlatformDefine_h__