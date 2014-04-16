#ifndef UTILITIES
#define UTILITIES

#include "OpenCvDefinitions.h"

namespace OpenCvWrapper
{
    /* 
    *   Utility Functions for OpenCv
    */
    class Utils 
    {
    public:
        // crop image and return the image pointer
        static IplImage*    CropImage(  IplImage*    pIplImage,
                                        CvRect*      pRectangle );

        // create a histogram object with default parameters
        static CvHistogram* CreateHistogram( );

        // calculate color histogram
        static void         CalculateColorHistogram( IplImage*     pIplImage,
                                                     CvHistogram*  pHistogram,
                                                     CvRect*       pRectangle );

        // get rectangle center position
        static CvPoint*     GetRectangleCenter( CvRect* pRectangle );

        // calculate distance between two points
        static double       CalculateEuclideanDistance( CvPoint* pPoint1,
                                                        CvPoint* pPoint2 );

        // set parameters for different opencv modules
        static void SetParameters(  char*           pParamsStr,
                                    CvVSModule*     pModule,
                                    const char*     pPrefix, 
                                    const char*     pModuleStr );

        // histogram equalize the given image
        static cv::Mat HistogramEqualize( const cv::Mat& inputImage );
    };
}
#endif 