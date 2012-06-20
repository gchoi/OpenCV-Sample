#include <iostream>
#include <string>

#include <stdio.h>

#include <cv.h>
#include <highgui.h>
#include <opencv2/nonfree/nonfree.hpp>

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
  vector< vector<DMatch> > matches;
  for( int i = 0; i < imgs.size()-1; ++ i ){
    matcher.radiusMatch( imgs[i]->desc, imgs[i+1]->desc, matches, 0.3);

    char filename[256];
    sprintf( filename, "matching%02d.png", i );
    Mat dst;


    vector<DMatch> topMatch;
    for( int j = 0; j < matches.size() ; ++ j ){
      for( int h =0 ; h < matches[j].size(); ++h) {
	  DMatch pt = matches[j][h];
	  topMatch.push_back( pt );
	}
    }



    drawMatches( imgs[i]->img, imgs[i]->kpt, 
		 imgs[i+1]->img, imgs[i+1]->kpt,
		 topMatch, dst);
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
