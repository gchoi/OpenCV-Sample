#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <highgui.h>


// Image Resize with OpenCV
int main( int argc, char *argv[] ){

  if( argc < 4 ){
    printf("usage of %s\n", argv[0]);
    printf("input_file output_file param1 param2 \n");
    printf("for all pixels, out = in * param1 + param2   will be computed\n");
    return 0;
  }

  IplImage *img = cvLoadImage( argv[1], CV_LOAD_IMAGE_ANYCOLOR );
  if( img == NULL ){
    printf("cannot open argv[1]\n");
  }

  double a = atof( argv[3] );
  double b = atof( argv[4] );
  cvConvertScale( img, img, a, b );

  cvSaveImage( argv[2], img, NULL);

  cvReleaseImage( &img );

  return 0;
}
