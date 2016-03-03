//
//  MNFileSystem.h
//  me_app
//
//  Created by hahasasa on 3/2/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#ifndef MNFileSystem_h
#define MNFileSystem_h
#include "MNPrimaryType.h"

const tstring& MNResourceFolderPath(const char* path = NULL);

FILE* mnfopen(const char* path, const char* mode);

#endif /* MNFileSystem_h */
