#include <stdio.h>

#include <cv.h>
#include <highgui.h>

int main( int argc, char* argv[] ){
  printf("hello world\n");
  
  IplImage *img = cvLoadImage( argv[1], CV_LOAD_IMAGE_COLOR );

  cvNamedWindow(argv[0], CV_WINDOW_AUTOSIZE);
  cvShowImage(argv[0], img );
  cvWaitKey(0);
  cvDestroyWindow( argv[0] );

  


  return 0;

}
