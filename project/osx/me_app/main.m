//
//  main.m
//  me_app
//
//  Created by hahasasa on 3/1/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "MiniEngine.h"

int main(int argc, const char * argv[]) {
    
    MNStart("resource/script/test.mn");
    return NSApplicationMain(argc, argv);
}
