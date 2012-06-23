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

    vector<DMatch> topMatch;
    for( int j = 0; j < matches.size() ; ++ j ){
      if( matches[j].size() > 0 )
	topMatch.push_back( matches[j][0] );
    }

    char filename[256];
    sprintf( filename, "matching%02d.png", i );
    Mat dst;
    drawMatches( imgs[i]->img, imgs[i]->kpt, 
		 imgs[i+1]->img, imgs[i+1]->kpt,
		 topMatch, dst);
    printf("%s\n", filename);
    imwrite( filename, dst );

    
    vector<Point2f> srcPoints, dstPoints;
    for( int j = 0; j < topMatch.size() ; ++j ){
      int srcIdx = topMatch[j].queryIdx;
      int dstIdx = topMatch[j].trainIdx;
      KeyPoint srcKpt = imgs[i]->kpt[srcIdx];
      KeyPoint dstKpt = imgs[i+1]->kpt[dstIdx];
      srcPoints.push_back( srcKpt.pt );
      dstPoints.push_back( dstKpt.pt );
    }

    Mat homography = findHomography( srcPoints, dstPoints, CV_RANSAC, 3);
    for( int h = 0; h < homography.rows; ++h ){
      for( int w = 0; w < homography.cols; ++w ){
	printf("\t%lf", homography.at<double>(h, w) );
      }
      printf("\n");
    }



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
