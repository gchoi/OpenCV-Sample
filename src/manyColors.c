#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cv.h>
#include <highgui.h>

int main( int argc, char* argv[] ){

  int height = atoi(argv[1]);
  int width  = atoi(argv[2]);

  IplImage *img = cvCreateImage( cvSize( height, width ), IPL_DEPTH_8U, 3);
  srand(time(NULL));
  int h, w, c;

  for( h = 0; h < height; ++h)
    for( w=0 ; w < width; ++w)
      for( c = 0; c < 3; ++c)
	CV_IMAGE_ELEM( img, uchar, h, w*3+c ) = rand() % (UCHAR_MAX);

  cvSaveImage( argv[3],img,  NULL );

  return 0;
}
