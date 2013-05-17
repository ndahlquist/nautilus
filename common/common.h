#ifndef __nativeGraphics__common__
#define __nativeGraphics__common__

// Callback function to load resources.
extern void*(*resourceCallback)(const char *);

void setFrameBuffer(int handle);

void SetResourceCallback(void*(*callbackfunc)(const char *));
void Setup(int w, int h);
void RenderFrame();

void PointerDown(float x, float y, int pointerIndex = -1);
void PointerMove(float x, float y, int pointerIndex = -1);
void PointerUp(float x, float y, int pointerIndex = -1);

#endif /* defined(__nativeGraphics__common__) */
