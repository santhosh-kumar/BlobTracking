#include "OpenCvUtilities.h"
#include "CommonMacros.h"

#include <iostream>
#include <math.h>

namespace OpenCvWrapper
{
    /********************************************************************
    Utils::CreateHistogram
        CreateHistogram
    Exceptions:
        None
    *********************************************************************/
    CvHistogram* Utils::CreateHistogram(  )
    {
        CvHistogram* pHistogram = NULL;
        try
        {
            int     h_bins      = 30;
            int     s_bins      = 32;
            int     hist_size[] = { h_bins, s_bins };
            float   h_ranges[]  = { 0, 180 };
            float   s_ranges[]  = { 0, 255 };
            float*  ranges[]    = { h_ranges, s_ranges };

            CvHistogram* pHistogram =  cvCreateHist( 2, hist_size, CV_HIST_ARRAY, ranges, 1 );
        }
        EXCEPTION_CATCH_AND_ABORT("Failed to create histogram");

        return pHistogram;
    }

   /********************************************************************
    Utils::CropImage
        CropImage
    Exceptions:
        None
    *********************************************************************/
    IplImage* Utils::CropImage( IplImage*  pIplImage,
                                CvRect*    pRectangle  )
    {
        IplImage* pCroppedImage = NULL;

        try
        {
            //set region of interest in the input pIplImage
            cvSetImageROI( pIplImage, *pRectangle );

            pCroppedImage = cvCreateImage( cvSize( pRectangle->width, pRectangle->height),
                                            pIplImage->depth,
                                            pIplImage->nChannels );

            // Copy the pIplImage
            cvCopy( pIplImage, pCroppedImage );

            // Reset the ROI
            cvResetImageROI(pIplImage);
        }
        EXCEPTION_CATCH_AND_ABORT( "Failed to crop image" );

        return pCroppedImage;
    }


    /********************************************************************
    Utils::CalculateColorHistogram
        CalculateColorHistogram
    Exceptions:
        None
    *********************************************************************/
    void Utils::CalculateColorHistogram( IplImage*     pIplImage,
                                         CvHistogram*  pHistogram,
                                         CvRect*       pRectangle )
    {
        try
        {
            IplImage* pObjectImage;

            if ( pRectangle != NULL )
            {
                pObjectImage = OpenCvWrapper::Utils::CropImage( pIplImage, 
                                                            pRectangle );
            }
            else
            {
                pObjectImage = pIplImage;
            }


            IplImage* h_plane = cvCreateImage( cvGetSize( pObjectImage ), 8, 1 );
            IplImage* s_plane = cvCreateImage( cvGetSize( pObjectImage ), 8, 1 );
            IplImage* v_plane = cvCreateImage( cvGetSize( pObjectImage ), 8, 1 );
            IplImage* planes[] = { h_plane, s_plane };

            //convert pixel to plane
            cvCvtPixToPlane( pObjectImage, h_plane, s_plane, v_plane, 0 );

            //calculate the histogram
            cvCalcHist( planes, pHistogram, 0, 0 );

            //normalize the histogram
            cvNormalizeHist( pHistogram, 1.0 );

            //release the locally created images
            cvReleaseImage( &h_plane );
            cvReleaseImage( &s_plane );
            cvReleaseImage( &v_plane );

            if ( pRectangle != NULL )
            {
                cvReleaseImage( &pObjectImage );
            }
        }
        EXCEPTION_CATCH_AND_ABORT( "Failed to Calculate Color Histogram" );
    }

   /********************************************************************
    Utils::GetRectangleCenter
        GetRectangleCenter
    Exceptions:
        None
    *********************************************************************/
    CvPoint* Utils::GetRectangleCenter( CvRect*     pRectangle )
    {
        CvPoint* pCenterPoint = new CvPoint();

        pCenterPoint->x = static_cast<int>( pRectangle->x + (pRectangle->width/2.0) );
        pCenterPoint->y = static_cast<int>( pRectangle->y + (pRectangle->height/2.0) );

        return pCenterPoint;
    }


    /********************************************************************
    Utils::CalculateEuclideanDistance
        CalculateEuclideanDistance
    Exceptions:
        None
    *********************************************************************/
    double  Utils::CalculateEuclideanDistance( CvPoint*     pPoint1,
                                               CvPoint*     pPoint2 )
    {
        return sqrt( pow( ( pPoint1->x - pPoint2->x ), 2.0) + pow( ( pPoint1->y - pPoint2->y ), 2.0 ) );
    }

    /********************************************************************
    Utils::SetParameters
        Sets parameters for different opencv modules
    Exceptions:
        None
    *********************************************************************/
    void Utils::SetParameters( char*        pParamStr,
                               CvVSModule*  pModule,
                               const char*  pPrefixStr, 
                               const char*  pModuleStr )
    {
        int prefixLength = std::strlen( pPrefixStr );

        while ( pParamStr[0] != '\0' )
        {
            int j;
            char* ptr_eq = NULL;
            int   cmd_param_len=0;
            char* cmd = pParamStr;

            if ( MY_STRNICMP( pPrefixStr, cmd, prefixLength )!= 0 ) 
            { 
                pParamStr++;
                continue;
            }

            cmd += prefixLength;
            if ( cmd[0] != ':' ) 
            { 
                pParamStr++;
                continue;
            }
            cmd++;

            ptr_eq = std::strchr(cmd,'=');
            if ( ptr_eq ) 
            {
                cmd_param_len = ptr_eq-cmd;
            }

            ptr_eq = strchr(cmd,';'); //look for next command
            if ( ptr_eq ) 
            {
                *ptr_eq = '\0';
                pParamStr = ptr_eq+1;
            }
            else
            {
                break; //command must ends with ";"
            }

            for ( j=0; ; ++j )
            {
                int     param_len;
                const char*   param = pModule->GetParamName(j);
                if ( param == NULL )
                {
                    break;
                }

                param_len = std::strlen(param);
                if ( cmd_param_len != param_len )
                {
                   continue;
                }
                if ( MY_STRNICMP( param,cmd,param_len ) !=0 )
                {
                    continue;
                }

                cmd+=param_len;
                if ( cmd[0]!='=' )
                {
                    continue;
                }

                cmd++;
                pModule->SetParamStr(param,cmd);
                printf("%s:%s param set to %g\n",pModuleStr,param,pModule->GetParam(param));
            }
        }

        pModule->ParamUpdate();
    }

    /********************************************************************
    Utils::HistogramEqualize
        Performs Histogram Equalization on the given image
    Exceptions:
        None
    *********************************************************************/
    cv::Mat Utils::HistogramEqualize(const cv::Mat& inputImage)
    {
        if ( inputImage.channels() >= 3 )
        {
            cv::Mat ycrcb;

            cvtColor(inputImage,ycrcb,CV_BGR2YCrCb);

            std::vector<cv::Mat> channels;
            split(ycrcb,channels);

            equalizeHist(channels[0], channels[0]);

            cv::Mat result;
            merge(channels,ycrcb);

            cvtColor(ycrcb,result,CV_YCrCb2BGR);

            return result;
        }
        return cv::Mat();
    }
}