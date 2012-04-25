#include <stdio.h>
#include <cv.h>
#include <highgui.h>

int main( int argc, char *argv[] ){

  if( argc == 1){
    printf("usage of %s\n", argv[0]);
    printf("input_file filter_file output_file\n");
    return 0;
  }else if( argc < 4 ){
    printf("input arguments are wrong...\n");
    return 1;
  }

  IplImage *input_file = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
  IplImage *filter_file = cvLoadImage(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

  int height = input_file->height;
  int width = input_file->width;

  IplImage *output_file = cvCreateImage( cvSize( width, height), IPL_DEPTH_8U, 1 );

  double norm = cvNorm( filter_file, NULL, CV_L1, NULL );
  printf("norm = %lf\n", norm);

  for( int h = 0; h < height; ++h ){
    for( int w = 0; w < width; ++w ){
      
      double sum = 0.0;

      for( int y = 0; y < filter_file->height; ++y ){
	for( int x = 0; x < filter_file->width; ++x){
	  
	  int py = h + y - filter_file->height/2;
	  int px = w + x - filter_file->width/2;
	  
	  if( py < 0 || py >= height || px < 0 || px >= width ){
	    
	  }else{
	    sum += CV_IMAGE_ELEM( input_file, uchar, py, px )
	      * CV_IMAGE_ELEM( filter_file, uchar, y, x);
	  }

	}
      }
      
      CV_IMAGE_ELEM( output_file, uchar, h, w ) = (uchar)(sum / norm );

    }
  }

  cvSaveImage( argv[3], output_file, NULL );


  return 0;

}
