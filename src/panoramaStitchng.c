#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>


#define MAX_INPUT_NUM 20
#define SURF_TH 0.1
#define SURF_HESSEIAN 750
#define SURF_DESC_DIM 128
#define RANSAC_TH 0.1

int _images;
IplImage* _img[MAX_INPUT_NUM];
double Height;
double Width;
double Scale;

#define SQUARE(x) ((x)*(x))

CvPoint2D32f* clonePoint2D32f( CvPoint2D32f pt){
  CvPoint2D32f* ref = (CvPoint2D32f *)malloc(sizeof(CvPoint2D32f));
  ref->x = pt.x;
  ref->y = pt.y;
  return ref;
}

int panoramaStitching( int inputImages, char* inputImagesFile[], char outputImageFile[] );

int extractKeyPoints( IplImage* src, 
		      CvSeq** keypoint, CvSeq** descriptor, 
		      CvMemStorage* storage);

int matchPointsInImages( CvSeq* keypoint1, CvSeq* descriptor1, 
			 CvSeq* keypoint2, CvSeq* descriptor2, 
			 CvSeq** correspondent , CvMemStorage* storage);
int nearestNeighbor( float *vec, int laplacian, CvSeq* keypoint, CvSeq* descriptor);

double euclidDistance( float x[], float y[], int dim );

void findCorrespondence( IplImage* imgLeft, IplImage* imgRight, CvSeq** correspondent, CvMemStorage* storage);


int findTransformMatrix( CvSeq* pt1, CvSeq* pt2, CvMat* dst );

void printMatrix( CvMat* mat );

IplImage* projectImages( CvMat *transformMat[MAX_INPUT_NUM] );

CvPoint2D32f orthogonal2Cylinder( CvPoint2D32f pt );
CvPoint2D32f cylinder2Orthogonal( CvPoint2D32f pt );

/* for( i = 0; i < _images-1; ++i ){ */
/*   printf("image %d -> %d\n", i, i+1); */
/*   while( correspondents[i]->total > 0 ){ */
/*     CvPoint2D32f pt1, pt2; */
/*     cvSeqPopFront( correspondents[i], &pt1 ); */
/*     cvSeqPopFront( correspondents[i], &pt2 ); */
/*     printf("\t(%03.1f, %03.1f) -> (%03.1f, %03.1f)\n", pt1.x, pt1.y, pt2.x, pt2.y); */
/*   } */
/*   printf("\n\n"); */
/* } */

IplImage* showCorrespondence( IplImage* img1, IplImage* img2, CvSeq *correspondent);


int main( int argc, char* argv[] ){
  panoramaStitching( argc-2, &(argv[1]), argv[argc-1] );
  return 0;
}

int panoramaStitching( int inputImages, 
		       char* inputImagesFile[], 
		       char outputImageFile[] )
 {
   _images = inputImages;
   int i;

   // A : varidation and loading
   if( _images >= MAX_INPUT_NUM ) return 1;
   for(  i = 0; i < _images ; ++i )
     _img[i] = cvLoadImage( inputImagesFile[i], CV_LOAD_IMAGE_GRAYSCALE );
   CvMemStorage *storage = cvCreateMemStorage(0);

   Height = _img[0]->height;
   Width = _img[0]->width;
   Scale = ( Height > Width ) ? Height : Width ;
   printf("size = %lf, %lf\n", Width, Height);


   // B : Extracs keypoints
   CvSeq* keypoints[MAX_INPUT_NUM];
   CvSeq* descriptors[MAX_INPUT_NUM];
   for( i = 0; i < _images ; ++i ){
     extractKeyPoints( _img[i], &keypoints[i], &descriptors[i], storage);
   }
   printf("extract keypoints done\n");

   // C : Matching images
   CvSeq* correspondents[MAX_INPUT_NUM-1];
   for( i = 0; i < _images-1 ; ++i ){
     matchPointsInImages( keypoints[i]  , descriptors[i],
			  keypoints[i+1], descriptors[i+1],
			  &correspondents[i], storage );
     printf("%d -> %d correspondent points = %d \n", i, i+1, correspondents[i]->total);

     IplImage* toshow = showCorrespondence( _img[i], _img[i+1], correspondents[i] );
     char filename[256];
     sprintf( filename, "matching%02d.png", i);
     cvSaveImage( filename, toshow , NULL);

   }
   printf("matching keypoints done\n");
   

   // finding transform matrix
   CvMat* transformMat[MAX_INPUT_NUM];
   transformMat[0] = cvCreateMat( 3, 3, CV_64FC1);
   cvSetIdentity( transformMat[0], cvRealScalar( 1.0 ) );
   CvSeq* ptSeq1 = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint2D32f), storage);
   CvSeq* ptSeq2 = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint2D32f), storage);

   CvMat *inv = cvCreateMat( 3, 3, CV_64FC1 );

   for( i=0; i < _images-1; ++i){
     transformMat[i+1] = cvCreateMat( 3, 3, CV_64FC1);
     cvClearSeq( ptSeq1 );
     cvClearSeq( ptSeq2 );
     while( correspondents[i]->total > 0 ){
       CvPoint2D32f pt;

       cvSeqPopFront( correspondents[i], &pt );
       cvSeqPush( ptSeq1, clonePoint2D32f(pt));

       cvSeqPopFront( correspondents[i], &pt );
       cvSeqPush( ptSeq2, clonePoint2D32f(pt));

     }
     findTransformMatrix( ptSeq1, ptSeq2, transformMat[i+1]);
     //cvInvert( transformMat[i], inv, CV_LU );
     cvMatMul( transformMat[i], transformMat[i+1], transformMat[i+1] );
     printf(": 0 -> %d\n", i+1);
     printMatrix( transformMat[i+1] );
   }

   // projection <--> back projection images
   IplImage* dst = projectImages( transformMat );
   
   // save
   if( dst != NULL )
     cvSaveImage( outputImageFile, dst, NULL );

  return 0;
}

int extractKeyPoints( IplImage* src, 
		      CvSeq** keypoint, CvSeq** descriptor, 
		      CvMemStorage* storage)
{
  
  CvSURFParams param = cvSURFParams(SURF_HESSEIAN, 1);
  cvExtractSURF( src, NULL, keypoint, descriptor, storage, param, 0);
}

int matchPointsInImages( CvSeq* keypoint1, CvSeq* descriptor1, 
			 CvSeq* keypoint2, CvSeq* descriptor2, 
			 CvSeq** correspondent , CvMemStorage* storage)
{
  *correspondent = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint2D32f), storage);

  for( int i = 0 ; i < descriptor1->total; ++i ){
    CvSURFPoint* pt1 = (CvSURFPoint *)cvGetSeqElem( keypoint1, i );
    float *desc1 = (float*)cvGetSeqElem( descriptor1, i );
    int nn = nearestNeighbor( desc1, pt1->laplacian, keypoint2, descriptor2 );
    if( nn != -1 ){
      CvSURFPoint* pt2 = (CvSURFPoint *)cvGetSeqElem( keypoint2, nn );
      cvSeqPush( *correspondent, &(pt1->pt) );
      cvSeqPush( *correspondent, &(pt2->pt) );
    }
  }
}


int nearestNeighbor( float *vec, int laplacian, CvSeq* keypoint, CvSeq* descriptor) {
  int neighbor = -1;
  double minDistance = DBL_MAX;

  for( int i = 0; i < descriptor->total; ++i ){
    CvSURFPoint* pt = (CvSURFPoint* )cvGetSeqElem( keypoint, i );
    if( pt->laplacian != laplacian ) continue;
    float* v = (float* )cvGetSeqElem( descriptor, i);
    double d = euclidDistance( vec, v, SURF_DESC_DIM );
    if( d < minDistance && d < SURF_TH){
      minDistance = d;
      neighbor = i;
    }
  }

  return neighbor;
}


double euclidDistance( float x[], float y[], int dim ) {
  double sum = 0.0;
  for( int i = 0; i < dim; ++ i ){
    sum += SQUARE( x[i] - y[i] );
  }
  return sum;
}


int findTransformMatrix( CvSeq* pt1, CvSeq* pt2, 
			 CvMat* dst )
{
  CvMat* srcPoints = cvCreateMat( pt1->total, 2, CV_64FC1 );
  CvMat* dstPoints = cvCreateMat( pt1->total, 2, CV_64FC1 );
  for( int i = 0; i < pt1->total; ++i){
    CvPoint2D32f* srcPt = (CvPoint2D32f*)cvGetSeqElem( pt1, i );
    CvPoint2D32f* dstPt = (CvPoint2D32f*)cvGetSeqElem( pt2, i );
    CV_MAT_ELEM( *srcPoints, double, i, 0 ) = (2.0 * srcPt->x - Width ) / Scale;
    CV_MAT_ELEM( *srcPoints, double, i, 1 ) = (2.0 * srcPt->y - Height ) / Scale;
    CV_MAT_ELEM( *dstPoints, double, i, 0 ) = (2.0 * dstPt->x - Width ) / Scale;
    CV_MAT_ELEM( *dstPoints, double, i, 1 ) = (2.0 * dstPt->y - Height ) / Scale;
  }
  
  cvFindHomography( srcPoints, dstPoints, dst, CV_RANSAC, RANSAC_TH, NULL );

  cvReleaseMat( &srcPoints );
  cvReleaseMat( &dstPoints );
  return 0;
}



void printMatrix( CvMat *mat ){
  if( mat != NULL );
  for(int r = 0; r < mat->rows; ++r ){
    for( int c = 0; c < mat->cols; ++c){
      printf("%lf\t\t", CV_MAT_ELEM( *mat, double, r, c));
    }
    printf("\n");
  }
}


IplImage* projectImages( CvMat *transformMat[MAX_INPUT_NUM] )
{
  IplImage *dst = cvCreateImage( cvSize( Width * _images * 0.4, Height * 1.5), IPL_DEPTH_8U, 1);

  CvMat *imgPoint = cvCreateMat( 3, 1, CV_64FC1 );
  CvMat *corPoint = cvCreateMat( 3, 1, CV_64FC1 );

  for( int i = 0; i < _images; ++i ){
    for( int h = 0; h < _img[i]->height; ++h ){
      for( int w = 0; w < _img[i]->width; ++w ){
	
	CvPoint2D32f cylPt = cvPoint2D32f( ( 2.0 * w - Width  ) / Scale, ( 2.0 * h - Height  ) / Scale);

	CV_MAT_ELEM( *imgPoint, double, 0, 0 ) = cylPt.x;
	CV_MAT_ELEM( *imgPoint, double, 1, 0 ) = cylPt.y;
	CV_MAT_ELEM( *imgPoint, double, 2, 0 ) = 1.0;

	cvMatMul( transformMat[i], imgPoint, corPoint);


	CvPoint2D32f ortPt = cvPoint2D32f( CV_MAT_ELEM( *corPoint, double, 0, 0), CV_MAT_ELEM( *corPoint, double, 1, 0) );

	int H = ( ortPt.y * Scale + Height )/ 2.0;
	int W = ( ortPt.x * Scale + Width )/ 2.0;

	if( h == 574/2 && w == 768/2 ){
	  printf("%d : %lf, %lf %lf ", i,
		 CV_MAT_ELEM( *corPoint, double, 0, 0),
		 CV_MAT_ELEM( *corPoint, double, 1, 0),
		 CV_MAT_ELEM( *corPoint, double, 2, 0)
		 );
	  printf("...in image pixel : %d, %d\n", H, W);
	}

	H += ( H < 0 )? dst->height : 0 ;
	H -= ( H >= dst->height )? dst->height : 0;
	W += ( W < 0 )? dst->width : 0 ;
	W -= ( W >= dst->width )? dst->width : 0;
	CV_IMAGE_ELEM( dst, uchar, H, W) = CV_IMAGE_ELEM( _img[i], uchar, h, w );

      }
    }

  }
  
  return dst;
}



CvPoint2D32f orthogonal2Cylinder( CvPoint2D32f pt )
{
  double f = 1.0;
  double x = atan2( pt.x , f );
  double y = pt.y / sqrt( pt.x*pt.x + pt.y*pt.y );
  return cvPoint2D32f( x, y );
}

CvPoint2D32f cylinder2Orthogonal( CvPoint2D32f pt )
{
  double f = 1.0;
  double x = f * tan(pt.x);
  double y = pt.y * sqrt( x*x + f*f );
  return cvPoint2D32f( x, y );
}


IplImage* showCorrespondence( IplImage* img1, IplImage* img2, CvSeq *correspondent)
{
  IplImage* img = cvCreateImage( cvSize( img1->width + img2->width, img1->height), IPL_DEPTH_8U, 1);

  cvSetImageROI( img, cvRect( 0, 0, img1->width, img1->height ));
  cvConvert( img1, img);
  cvResetImageROI(img);
  cvSetImageROI( img, cvRect( img1->width, 0, img2->width, img2->height ));
  cvConvert( img2, img);
  cvResetImageROI(img);


  for( int i = 0; i < correspondent->total; i+=2 ){
    CvPoint2D32f *pt1 = (CvPoint2D32f*)cvGetSeqElem( correspondent, i );
    CvPoint2D32f *pt2 = (CvPoint2D32f*)cvGetSeqElem( correspondent, i+1 );
    pt2->x += img1->width;
    cvLine( img, cvPointFrom32f(*pt1), cvPointFrom32f(*pt2), cvScalarAll( 128 ), 1, 4, 0);
  }

  return img;

}


void findCorrespondence( IplImage* imgLeft, IplImage* imgRight, CvSeq** correspondent, CvMemStorage* storage)
{
  // だいたいどれくらいの移動量があるかを計算
  // 画像の縦方向をベクトルとして，相関を図る
  int shift = 0;

  // その被ってる領域で特徴量を抽出

  // 抽出した特徴量をマッチング

  // 対応点を得る



}
