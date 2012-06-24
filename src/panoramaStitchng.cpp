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
  Mat homography;

  // methods
  Image( char filename[] );

private:
  // private variable
  
  // private methods

};

void printMat( Mat m );
Mat findRotationMatrix( const vector<Point2f> srcPoints, const vector<Point2f>  dstPoints );


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

    matcher.radiusMatch( imgs[i+1]->desc, imgs[i]->desc, matches, 0.3);

    vector<DMatch> topMatch;
    for( int j = 0; j < matches.size() ; ++ j ){
      if( matches[j].size() > 0 )
	topMatch.push_back( matches[j][0] );
    }
    printf("matches from %2d to %2d is %ld\n", i, i+1, topMatch.size());

    char filename[256];
    sprintf( filename, "matching%02d.png", i );
    Mat dst;
    drawMatches( imgs[i+1]->img, imgs[i+1]->kpt, 
		 imgs[i]->img, imgs[i]->kpt,
		 topMatch, dst);
    imwrite( filename, dst );

    vector<Point2f> srcPoints, dstPoints;
    for( int j = 0; j < topMatch.size() ; ++j ){
      int srcIdx = topMatch[j].queryIdx;
      int dstIdx = topMatch[j].trainIdx;
      KeyPoint srcKpt = imgs[i+1]->kpt[srcIdx];
      KeyPoint dstKpt = imgs[i]->kpt[dstIdx];
      srcPoints.push_back( srcKpt.pt );
      dstPoints.push_back( dstKpt.pt );
    }

    imgs[i]->homography = findRotationMatrix( srcPoints, dstPoints );
    printMat( imgs[i]->homography );
  }

  // project Image
  int Width = imgs[0]->width * imgs.size() * 1.0;
  int Height = imgs[0]->height * 2.0;
  Mat panorama = Mat( Height, Width, CV_8UC1 );
  Mat trans = Mat::eye( 3, 3, CV_64F );
  Mat src = Mat( 3, 1, CV_64F );
  Mat dst = Mat( 3, 1, CV_64F );

  for( int i = 0; i < imgs.size() ; ++ i){

    for( int h = 0; h < imgs[i]->height; ++h ){
      for( int w = 0; w < imgs[i]->width; ++w){
	src.at<double>(0, 0) = w;
	src.at<double>(1, 0) = h;
	src.at<double>(2, 0) = 1.0;

	dst = trans * src; 
	
	if( h == imgs[i]->height/2 && w == imgs[i]->width/2) {
	  printf("%lf, %lf, %lf -> %lf, %lf, %lf\n", 
		 src.at<double>( 0, 0), src.at<double>( 1, 0), src.at<double>( 2, 0),
		 dst.at<double>( 0, 0), dst.at<double>( 1, 0), dst.at<double>( 2, 0));
	}

	int x = dst.at<double>(0,0) ;
	int y = dst.at<double>(1,0) ;
	x += ( x < 0 )? panorama.cols : 0;
	x -= (x >= panorama.cols ) ? panorama.cols : 0 ;
	y += ( y < 0 )? panorama.rows : 0;
	y -= (y >= panorama.rows ) ? panorama.rows : 0 ;

	panorama.at<uchar>( y, x ) = imgs[i]->img.at<uchar>(h, w);
      }
    }

    if( i != imgs.size() -1 )
      trans = trans * imgs[i]->homography;
    printMat( trans );
  }

  imwrite( "panorama.png", panorama );


  return 0;
}


Image::Image( char filename[] ){
  printf("filename is %s \n", filename);
  img = imread( filename, 0 );
  height = img.rows;
  width = img.cols;

  SurfFeatureDetector detector(1000);
  detector.detect( img, kpt );

  SurfDescriptorExtractor extractor;
  extractor.compute( img, kpt, desc );

}


void printMat( Mat m ){
    for( int h = 0; h < m.rows; ++h ){
      for( int w = 0; w < m.cols; ++w ){
	printf("\t%lf", m.at<double>(h, w) );
      }
      printf("\n");
    }
}


Mat findRotationMatrix( const vector<Point2f> srcPoints, const vector<Point2f>  dstPoints )
{
  double f = 1.0;
  Mat A = Mat( srcPoints.size() * 2, 3,CV_64F);
  Mat y = Mat( srcPoints.size() * 2, 1, CV_64F);

  for( int i = 0; i < A.rows; i += 2){
    
    Point2f srcPt = srcPoints[i/2];
    Point2f dstPt = dstPoints[i/2];
    A.at<double>(0, i )	  = -srcPt.x * srcPt.y / f;
    A.at<double>(1, i )	  = f + srcPt.x * srcPt.x  / f;
    A.at<double>(2, i )	  = -srcPt.y;
    A.at<double>(0, i+1 ) = -( f + srcPt.y * srcPt.y  / f );
    A.at<double>(1, i+1 ) = srcPt.x * srcPt.y / f;
    A.at<double>(2, i+1 ) = srcPt.x;
    y.at<double>(0, i )   = dstPt.x - srcPt.x ;
    y.at<double>(0, i+1 ) = dstPt.y - srcPt.y ;
    printf("f = %lf\tsrc = %f, %f\t dst = %f, %f\n",f, srcPt.x, srcPt.y, dstPt.x, dstPt.y);
  }

  printf("putting done\n");

  Mat AtA = A.t() * A;
  printf("AtA\n");
  printMat(AtA);
  Mat AtAInv = AtA.inv(DECOMP_SVD);
  printf("AtAInv\n");
  printMat(AtAInv);
  Mat omega = AtAInv * A.t() * y;
  printf("omega\n");
  printMat(omega);
  double wx = omega.at<double>( 0, 0 );
  double wy = omega.at<double>( 1, 0 );
  double wz = omega.at<double>( 2, 0 );
  Mat nCross = ( Mat_<double>(3, 3) <<
		 0  , -wz,  wy,
		 wz ,   0, -wx,
		 -wy,  wx,   0);
  return Mat::eye(3, 3, CV_64F) + nCross;


}
