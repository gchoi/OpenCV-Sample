#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>


void outputAsText( char input[], char output[], char textFileName[] ){
  
  IplImage *inputImage = cvLoadImage( input, CV_LOAD_IMAGE_COLOR );
  IplImage *outputImage = cvLoadImage( output, CV_LOAD_IMAGE_COLOR );

  FILE *fp = fopen( textFileName, "w");
  if(fp == NULL ){
    printf("cannot open file %s \n", textFileName);
    return;
  }

  int step = 5;
  char colors[3][6] = {
    {"Blue"},
    {"Green"},
    {"Red"}
  };

  for( int c = 0; c < 3; ++c){
    fprintf(fp, "#%s input, output\n", colors[c]);
    for( int h = 0; h < inputImage->height; h += step ){
      for( int w = 0; w < inputImage->width; w += step ){
	fprintf( fp, "%d  ", (int)CV_IMAGE_ELEM( inputImage,  uchar, h, w*3+c ) );
	fprintf( fp, "%d  ", (int)CV_IMAGE_ELEM( outputImage, uchar, h, w*3+c ) );
	fprintf( fp, "\n");
      }
    }
    fprintf( fp, "\n\n");
  }

  fclose(fp);

}


void writeMatrix( CvMat *mat , char matrixName[], char filename[]){
  CvFileStorage *fs = cvOpenFileStorage( filename, NULL, CV_STORAGE_WRITE, NULL );
  cvWrite( fs, matrixName, mat, cvAttrList(0, 0) );
  cvReleaseFileStorage( &fs );
  return;
}

CvMat* readMatrix( char filename[] ,char matrixName[]){
  CvFileStorage *fs = cvOpenFileStorage( filename, 0, CV_STORAGE_READ, NULL);
  CvFileNode *rootNode = cvGetFileNodeByName( fs, 0, matrixName );
  CvMat *ret = (CvMat*)cvRead( fs, rootNode, 0);
  cvReleaseFileStorage(&fs);
  return ret;
}



// (A'A)^-1A' まで計算
void colorCollection_calcInputMatrix( IplImage *img , CvMat *inputMat){

  int N = inputMat->cols;
  CvMat *_inputMat = cvCreateMat( 4, N , CV_64FC1);

  // set input
  for( int h = 0; h < img->height; ++h){
    for( int w = 0; w < img->width ; ++w){
      int idx = h * img->width + w;
      for( int c = 0; c < 3; ++c )
	CV_MAT_ELEM( *_inputMat, double, c, idx ) = CV_IMAGE_ELEM( img, uchar, h, w*3+c );
      
      CV_MAT_ELEM( *_inputMat, double, 3, idx ) = 1.0;
    }
  }

  printf("read input done\n");

  // calc input
  CvMat* ATA = cvCreateMat( N, N , CV_64FC1);
  cvMulTransposed( _inputMat, ATA, 1, NULL, 0.0 ); // ATA = A'A
  
  printf("ATA\n");

  CvMat* ATAinv = cvCreateMat( N, N , CV_64FC1);
  cvInvert( ATA, ATAinv, CV_SVD_SYM ); // ATAinv = (A'A)^-1
  
  printf("ATAinv\n");

  cvGEMM( ATAinv, _inputMat, 1.0, NULL, 0.0, inputMat, CV_GEMM_B_T );

  printf("input\n");

  return;
}



void colorCollection_SetOutputMatrix( IplImage *img, int channel ,CvMat *outputMat){

  for( int h = 0; h < img->height ; ++h){
    for( int w = 0; w < img->width ; ++w){
      int idx = h * img->width + w;
      CV_MAT_ELEM( *outputMat, double, idx, 0 ) 
	= CV_IMAGE_ELEM( img, uchar, h ,w*img->nChannels+channel);
    }
  }
  return;
}


void colorCollectionMatrix( char input[], char output[], char collectionMatrixFileName[] ){
  
  IplImage *inputImage  = cvLoadImage( input , CV_LOAD_IMAGE_COLOR );
  IplImage *outputImage = cvLoadImage( output, CV_LOAD_IMAGE_COLOR );
  CvMat *dst = cvCreateMat( 4, 4, CV_64FC1 );
  int N = inputImage->height * inputImage->width;

  CvMat *inputMat = cvCreateMat( 4, N, CV_64FC1 );
  colorCollection_calcInputMatrix( inputImage, inputMat );
  
  CvMat *dst_OneColor = cvCreateMat( 4, 1, CV_64FC1);
  CvMat *outputMat = cvCreateMat( N, 1, CV_64FC1);

  for( int c = 0; c < 3; ++c ){
    colorCollection_SetOutputMatrix( outputImage, c, outputMat );
    cvMatMul( inputMat, outputMat, dst_OneColor );
    for( int r = 0; r < 4; ++r )
      CV_MAT_ELEM( *dst, double, r, c ) = CV_MAT_ELEM( *dst_OneColor, double, r, 1);
  }

  CV_MAT_ELEM( *dst, double, 0, 3 ) = 0.0;
  CV_MAT_ELEM( *dst, double, 1, 3 ) = 0.0;
  CV_MAT_ELEM( *dst, double, 2, 3 ) = 0.0;
  CV_MAT_ELEM( *dst, double, 3, 3 ) = 1.0;

  writeMatrix( dst, "colorCollection", collectionMatrixFileName );
  
  cvReleaseMat( &dst );
  cvReleaseMat( &inputMat );
  cvReleaseMat( &dst_OneColor );
  cvReleaseMat( &outputMat );
  cvReleaseImage( &inputImage );
  cvReleaseImage( &outputImage );

  return;
}


int main( int argc, char* argv[] ){
  colorCollectionMatrix( argv[1], argv[2], argv[3] );
}
