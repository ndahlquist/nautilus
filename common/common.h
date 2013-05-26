#ifndef __nativeGraphics__common__
#define __nativeGraphics__common__

#include "RenderPipeline.h"

// Callback function to load resources.
extern void*(*resourceCallback)(const char *);
extern int displayWidth;
extern int displayHeight;
extern RenderPipeline * pipeline;

void SetResourceCallback(void*(*callbackfunc)(const char *));
void Setup(int w, int h);
void setFrameBuffer(int handle);
void RenderFrame();

void PointerDown(float x, float y, int pointerIndex = -1);
void PointerMove(float x, float y, int pointerIndex = -1);
void PointerUp(float x, float y, int pointerIndex = -1);

#endif // __nativeGraphics__common__
