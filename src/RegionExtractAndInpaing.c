#include <stdio.h>

#include <cv.h>
#include <highgui.h>

/*
  水着をインペイントで隠すプログラム
  step
  1. mean-shiftであらく領域分割
  2. ユーザーがマウスでインペイントする領域を選択
  3. その領域のマスクをつくる
  4. 元画像とマスクでインペイント
  5. 終了 or 2. へ
  6. 出力
*/


void extractRegion( IplImage *src, IplImage *extractedRegion);

void onMouse( int event, int x, int y, int flags, void *param);
IplImage* dspImg = NULL;
char* windowName;
CvPoint mousePoint;

void createMask( IplImage *src, IplImage *mask, int th);
int isSameColor( CvScalar val1, CvScalar val2, int th );

void inpaint( IplImage *src, IplImage *mask, IplImage *dst, int inpaintRadius);

void merge( IplImage* originalImage, IplImage *mask, IplImage *inpaintImage);

int main( int argc, char* argv[] ){
  
  IplImage *img = cvLoadImage( argv[1], CV_LOAD_IMAGE_COLOR );

  IplImage *extractedRegion = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 3);
  IplImage *mask = cvCreateImage( cvGetSize( img ), IPL_DEPTH_8U, 1);
  IplImage *inpaintedImage = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 3);
  IplImage *dst = cvCloneImage( img );

  // region extract
  printf("region extracting...\n");
  extractRegion( img, extractedRegion );
  // region extract done

  // show image and make user choose region
  dspImg = cvCloneImage( extractedRegion );
  windowName = argv[0];
  cvNamedWindow(argv[0], CV_WINDOW_AUTOSIZE);
  cvSetMouseCallback( windowName, onMouse, 0);

  int c=0;
  while( c != 'q' ){
    
    cvShowImage(argv[0], dspImg );
    c = cvWaitKey(0);
    printf("choosing region done point(%d, %d) is selected\n", mousePoint.x, mousePoint.y);
    // choosing region done
  
    // create mask
    cvSetZero( mask );
    int sameColorTh = 20;
    createMask( extractedRegion, mask, sameColorTh );
    // create mask done

    // inpaintng
    int inpaintRadius = 4;
    inpaint( img, mask, inpaintedImage, inpaintRadius );
    printf("inpaint done\n");
    // inpaint done

    // merge
    merge( dst, mask, inpaintedImage );

    
  }

  cvDestroyWindow( argv[0] );
  
  cvSaveImage( argv[2], dst, NULL );

  return 0;

}


void extractRegion( IplImage *src, IplImage *extractedRegion)
{

  CvRect roi;
  int level = 2;
  roi.x = roi.y = 0;
  roi.width = src->width & -(1 << level ); 
  roi.height = src->height & -(1 << level );

  IplImage* resized = cvCreateImage( cvSize( roi.height, roi.width), IPL_DEPTH_8U, 3 );
  cvResize( src, resized, CV_INTER_AREA );
  IplImage* dstResize = cvCreateImage( cvSize( roi.height, roi.width), IPL_DEPTH_8U, 3 );
  
  cvPyrMeanShiftFiltering( resized, dstResize, 30.0, 30.0, level,
			   cvTermCriteria( CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 5, 1 ));
  
  cvResize( dstResize, extractedRegion, CV_INTER_NN );

  cvReleaseImage( &resized );
  cvReleaseImage( &dstResize );

  return;
}



void onMouse( int event, int x, int y, int flags, void *param){
  
  if( event == CV_EVENT_LBUTTONDOWN ){
    mousePoint = cvPoint(x, y);
    cvCircle( dspImg,  mousePoint, 10, cvScalarAll( 255 ), CV_FILLED,  8, 0);
    cvShowImage( windowName, dspImg );
  }

}

void createMask( IplImage *src, IplImage *mask, int th)
{
  // create mask
  CvScalar inpaintColor = cvGet2D( src, mousePoint.y, mousePoint.x );

  for( int h = 0; h < mask->height; ++h){
    for( int w = 0; w < mask->width; ++w){
      CvScalar color = cvGet2D( src, h, w);
      CV_IMAGE_ELEM( mask, uchar, h, w) 
	= ( isSameColor(color, inpaintColor, th) )? 200 : 0 ;
    }
  }

  cvSaveImage( "extractedRegion.png", src, NULL );
  cvSaveImage( "mask.png", mask, NULL );
}

int isSameColor( CvScalar val1, CvScalar val2, int th )
{
  double val = 0;
  int ch = 3;
  for( int c = 0; c < ch; ++c){
    val += fabs( val1.val[c] - val2.val[c] );
  }

  return ( val < th ) ? 1 : 0;

}

void inpaint( IplImage *src, IplImage *mask, IplImage *dst, int inpaintRadius)
{
  cvInpaint( src, mask, dst, inpaintRadius, CV_INPAINT_TELEA);
}


void merge( IplImage* originalImage, IplImage *mask, IplImage *inpaintImage)
{
  for( int h = 0;h < originalImage->height ; ++h ){
    for( int w = 0; w < originalImage->width; ++w ){
      if( CV_IMAGE_ELEM( mask, uchar, h, w) != 0 ){
	for(int c = 0; c < 3; ++c ){
	  CV_IMAGE_ELEM( originalImage, uchar, h, w*3+c ) 
	    = CV_IMAGE_ELEM( inpaintImage, uchar, h, w*3+c );
	} 
      }
    }
  }
  return ;
}
