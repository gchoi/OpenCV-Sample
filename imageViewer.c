#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <highgui.h>


// Easy image Viewer
char windowName[256];
void mouse(int event, int x, int y, int flags, void* param);

int main( int argc, char *argv[] ){
  
  IplImage *img = cvLoadImage(argv[1], CV_LOAD_IMAGE_ANYCOLOR );
  if( img == NULL ){
    printf("%s, cannot read image %s\n", argv[0], argv[1]);
    return 1;
  }

  cvNamedWindow( windowName, CV_WINDOW_AUTOSIZE );
  cvSetMouseCallback( windowName, mouse, (void*)img );

  printf("easy image viewer\n");
  printf("push ESC or q to quit\n");

  cvShowImage( windowName, img );

  while(1){
    int c = cvWaitKey(0);
    if( c == '\x1b' || c == 'q' ) break;
  }

  cvDestroyWindow( windowName );
  cvReleaseImage( &img );

  return 0;
}

void mouse(int event, int x, int y, int flags, void* param)
{
  if( event == CV_EVENT_LBUTTONDOWN ){
    IplImage *img = (IplImage*)param;
    printf("intensity at %d, %d is ", y, x);
    for(int c = 0; c < img->nChannels; ++c)
      printf("%d, ", (int)CV_IMAGE_ELEM( img, uchar, y, x*img->nChannels+c));
    printf("\n");
  }
}
