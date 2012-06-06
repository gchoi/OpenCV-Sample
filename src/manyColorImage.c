#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

#include <cv.h>
#include <highgui.h>

int main( int argc, char *argv[] ){
  srand(time(NULL));
  
  int height = atoi( argv[1] );
  int width  = atoi( argv[2] );
  
  IplImage *img = cvCreateImage( cvSize( height, width ), IPL_DEPTH_8U, 3);
  
  for( int h = 0; h < height; ++h ){
    for( int w = 0; w < width; ++w ){
      for(int c = 0; c < 3; ++c ){
	CV_IMAGE_ELEM( img, uchar, h, w*3+c ) = rand() % UCHAR_MAX;
      }
    }
  }
      
  cvSaveImage( argv[3], img, NULL );
  return 0;
}
