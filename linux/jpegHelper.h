/* jpegHelper.h
   Adapted from 
   stackoverflow.com/questions/694080/how-do-i-read-jpeg-and-png-pixels-in-c-on-linux/694092#694092 */
#include <jpeglib.h>

void * jpegResourceCallback(const char * fileName, int & width, int & height) {

    const char * path = "../res/drawable/";
    char * filePath = (char *) malloc(strlen(path) + strlen(fileName) + 1);
    strcpy(filePath, path);
    strcat(filePath, fileName);
    FILE * infile = fopen(filePath, "rb");
    free(filePath);
    if(!infile) {
        printf("Can't open %s\n", fileName);
        return 0;
    }
  
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    width = cinfo.output_width;
    height = cinfo.output_height;

    unsigned char * pDummy = new unsigned char [width*height*4];
    unsigned char * pTest=pDummy;
    if(!pDummy){
        printf("NO MEM FOR JPEG CONVERT!\n");
        return 0;
    }
    int row_stride = width * cinfo.output_components ;
    
    JSAMPARRAY pJpegBuffer = (*cinfo.mem->alloc_sarray) // Output row buffer
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while(cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
        for(int x=0;x<width;x++) {
            unsigned char a,r,g,b;
            a = 1; // alpha value is not supported on jpg
            r = pJpegBuffer[0][cinfo.output_components*x];
            if(cinfo.output_components>2) {
                g = pJpegBuffer[0][cinfo.output_components*x+1];
                b = pJpegBuffer[0][cinfo.output_components*x+2];
            } else {
                g = r;
                b = r;
            }
            *(pDummy++) = r;
            *(pDummy++) = g;
            *(pDummy++) = b;
            *(pDummy++) = a;
        }
    }
    fclose(infile);
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    
    return pTest;
}
