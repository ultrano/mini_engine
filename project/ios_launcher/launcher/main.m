//
//  main.m
//  launcher
//
//  Created by hahasasa on 2/29/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#include "MiniEngine.h"

int main(int argc, char * argv[]) {
    MNStart("");
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
