//
//  iosInterface.cpp
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 5/5/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#include "iosInterface.h"

vector<struct resource> resources;

void passResource(const char *fileName, const char *fileContents)
{
    struct resource mResource;
    mResource.name = fileName;
    mResource.contents = fileContents;
    resources.push_back(mResource);
}

char *resourceCB(const char *fileName)
{
    for(int i = 0; i < resources.size(); i++) {
        if(strcmp(resources[i].name, fileName) == 0)
            return strdup(resources[i].contents);
    }
    printf("Resource %s not found", fileName);
    return NULL;
}