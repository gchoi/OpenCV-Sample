#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cv.h>
#include <highgui.h>

void  myFilter2D( IplImage *img, IplImage *dst, IplImage *psf, CvPoint pt);


int main(int argc, char* argv[]){

  IplImage *img = cvLoadImage( argv[1], CV_LOAD_IMAGE_GRAYSCALE );

  for( int h = 0; h < img->height; ++h){
    for( int w = 0; w < img->width; ++w){
      int val = (int)CV_IMAGE_ELEM( img, uchar, h, w );
      printf("val = %d at %d, %d\n", val, h, w);

    }
  }

  return 0;
}



