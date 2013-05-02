#ifdef __APPLE__
@interface Common : NSObject

- (void)SetupWidth:(int)w Height:(int)h;
- (void)RenderFrame;

@end

#else
void Setup(int w, int h);
void RenderFrame();
#endif



