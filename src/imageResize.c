#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <highgui.h>


// Image Resize with OpenCV
int main( int argc, char *argv[] ){

  if( argc < 4 ){
    printf("usage of %s\n", argv[0]);
    printf("input_file output_file new_height new_width\n");
    return 0;
  }

  int height = atoi(argv[3]);
  int width = atoi(argv[4]);
  IplImage *src = cvLoadImage( argv[1], CV_LOAD_IMAGE_ANYCOLOR );
  IplImage *dst = cvCreateImage( cvSize( atoi(argv[4]), atoi(argv[3]) ), IPL_DEPTH_8U, src->nChannels );
  cvResize(src, dst, CV_INTER_NN);

  cvSaveImage( argv[2], dst, NULL );
  
  
  return 0;
}
