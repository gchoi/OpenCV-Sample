#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cv.h>
#include <highgui.h>

int main( int argc, char* argv[] ){
  
  srand(time(NULL));

  IplImage *img = cvCreateImage( cvSize(512, 512), IPL_DEPTH_8U, 3);
  for( int h = 0; h < img->height; ++h){
    for( int w = 0; w < img->width; ++w){
      
      int val = (int)rand() % 255;
      CV_IMAGE_ELEM( img, uchar, h, w*3 + 0 ) = val;
      CV_IMAGE_ELEM( img, uchar, h, w*3 + 1 ) = val;
      CV_IMAGE_ELEM( img, uchar, h, w*3 + 2 ) = val;
    }
  }

  cvSaveImage( argv[1], img, 0);
}
