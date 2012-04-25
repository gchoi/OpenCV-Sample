#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <highgui.h>


// Image Resize with OpenCV
int main( int argc, char *argv[] ){

  if( argc < 10 ){
    printf("usage of %s\n", argv[0]);
    printf("input_file output_file left top right bottom RED GREEN BLUE width \n");
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
  double r = atof( argv[7] );
  double g = atof( argv[8] );
  double b = atof( argv[9] );
  double th = atof( argv[10] );

  if(left >= right || top >= bottom ||
      left < 0 || top < 0 || 
      right >= img->width || bottom >= img->height ){
    printf("input size is incorrect. size should be inside of Input image\n");
    return 1;
  }

  CvPoint pt1 = cvPoint( left, top );
  CvPoint pt2 = cvPoint( right, bottom );
  cvRectangle( img, pt1, pt2, CV_RGB(r,g,b), th, 8, 0 );
  cvSaveImage( argv[2], img, NULL);

  cvReleaseImage( &img );

  return 0;
}
