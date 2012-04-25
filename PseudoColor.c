#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <cv.h>
#include <highgui.h>

IplImage *cvtGrayScale2Pseudo( IplImage *img , int saturation, int value);

int main( int argc, char* argv[] ){
  if( argc < 3 ){
    printf("%s\n", argv[0]);
    printf("argv[1] = input image file\n");
    printf("argv[2] = output image file\n");
    printf("argv[3] = saturation ( 0 - 255 )\n");
    printf("argv[4] = birghtness ( 0 - 255 )\n");
    return 0;
  }

  IplImage *img = cvLoadImage( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
  int s, v;
  if( argc > 3 ){
    s = atoi( argv[3] );
    v = atoi( argv[4] );
  }else{
    s = 255;
    v = 255;
  }

  IplImage *dst = cvtGrayScale2Pseudo( img,s,v );
  cvSaveImage( argv[2], dst, NULL );

  cvReleaseImage( &img );
  cvReleaseImage( &dst );
  return 0;

}


IplImage *cvtGrayScale2Pseudo( IplImage *img , int saturation, int value)
{
  if( img->nChannels != 1 ) return NULL ;

  IplImage *hsv = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 3);


  for(int h = 0; h < img->height; ++h){
    for( int w  = 0; w < img->width; ++w){
      CV_IMAGE_ELEM( hsv, uchar, h, w*3 + 0 ) = (255 - CV_IMAGE_ELEM( img, uchar, h, w ) / 1.5 ) + 32.0;
      CV_IMAGE_ELEM( hsv, uchar, h, w*3 + 1 ) = saturation;
      CV_IMAGE_ELEM( hsv, uchar, h, w*3 + 2 ) = value;
    }
  }

  IplImage *bgr = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 3);
  cvCvtColor( hsv, bgr, CV_HSV2BGR );
  cvReleaseImage( &hsv );
  return bgr;
}
