#include <iostream>
#include <string>

#include <stdio.h>

#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;


class Image {
public:

  // properties
  Mat img;
  Mat desc;
  vector<KeyPoint> kpt;
  int height;
  int width;

  // methods
  Image( char filename[] );

private:
  // private variable
  
  // private methods

};




int main( int argc, char *argv[] ){
  if( argc < 1 ) return 1;

  Vector<Image*> imgs;
  
  // read image
  // extract keypoints
  for( int i = 1; i < argc; ++i ){ 
    imgs.push_back( new Image( argv[i] ) );
  }


  // match keypoints
  FlannBasedMatcher matcher;
  vector<DMatch> match;
  for( int i = 0; i < imgs.size()-1; ++ i ){
    matcher.radiusMatch( imgs[i]->desc, imgs[i+1]->desc, match, 0.5);
    printf("%2d -> %2d : matched %d points\n", i, i+1, (int)match.size());

    char filename[256];
    sprintf( filename, "matching%02d.png", i );
    Mat dst;
    drawMatches( imgs[i]->img, imgs[i]->kpt, 
		 imgs[i+1]->img, imgs[i+1]->kpt,
		 match, dst);
    printf("%s\n", filename);
    imwrite( filename, dst );

  }


  return 0;
}


Image::Image( char filename[] ){
  printf("filename is %s \n", filename);
  img = imread( filename, 0 );
  height = img.rows;
  width = img.cols;

  SurfFeatureDetector detector(2000);
  detector.detect( img, kpt );

  SurfDescriptorExtractor extractor;
  extractor.compute( img, kpt, desc );


  printf("surf operating done, ");
  printf("%d points\n", (int)kpt.size());


}
