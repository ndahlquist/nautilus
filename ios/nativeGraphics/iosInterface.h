//
//  iosInterface.h
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 5/5/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#ifndef __nativeGraphics__iosInterface__
#define __nativeGraphics__iosInterface__

#include <iostream>
#include <string>
#include <vector>

using namespace::std;

struct resource {
    const char *name;
    const char *contents;
};



void passResource(const char *fileName, const char *fileContents);
char *resourceCB(const char *fileName);

#endif /* defined(__nativeGraphics__iosInterface__) */
