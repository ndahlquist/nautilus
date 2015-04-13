// The graphics headers are in different places for different operating systems.

#ifdef ANDROID_NDK
    #include "importgl.h"
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <jni.h>
#elif ARCH_Darwin // OS X
    #include <GL/glew.h>
    #include <stdio.h>
#elif __APPLE__ // iOS
    #include <stdlib.h>
    #include <OpenGLES/ES2/gl.h>
#else // Linux
    #include <GL/glew.h>
    #include <stdio.h>
#endif
