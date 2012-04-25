#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <highgui.h>


// Image Resize with OpenCV
int main( int argc, char *argv[] ){

  if( argc < 7 ){
    printf("usage of %s\n", argv[0]);
    printf("input_file output_file left top right bottom\n");
    return 0;
  }

  IplImage *img = cvLoadImage( argv[1], CV_LOAD_IMAGE_ANYCOLOR );
  if( img == NULL ){
    printf("cannot open argv[1]\n");
  }

  int left = atoi( argv[3] );
  int top = atoi( argv[4] );
  int right = atoi( argv[5] );
  int bottom = atoi( argv[6] );

  if(left >= right || top >= bottom ||
      left < 0 || top < 0 || 
      right >= img->width || bottom >= img->height ){
    printf("input size is incorrect. size should be inside of Input image\n");
    return 1;
  }

  int ch = img->nChannels;
  CvSize size = cvSize( right - left + 1, bottom - top + 1 );

  IplImage *dst = cvCreateImage( size, IPL_DEPTH_8U, ch);
  cvSetZero(dst);
  for( int h = top ; h <= bottom; ++h ){
    for( int w = left ; w <= right; ++w){
      for(int c = 0; c < ch; ++c){
	CV_IMAGE_ELEM( dst, uchar, h-top, (w-left)*ch+c)
	  = CV_IMAGE_ELEM( img, uchar, h, w*ch+c);
      }
    }
  }

  cvSaveImage( argv[2], dst, NULL);

  cvReleaseImage( &img );
  cvReleaseImage( &dst );

  return 0;
}
