#ifndef CAMERA_H
#define CAMERA_H

#include "OpenCvDefinitions.h"
#include "CommonMacros.h"

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

namespace Tracker
{
    // typedef for video writer
    typedef cv::VideoWriter* VideoWritePtr;

    /*
    *   Performs camera level processing.
    */
    class Camera
    {
    public:
    // Constructor
    Camera( const std::string&     sourceDirectoryStr,
            const std::string&     videoFileNameStr );

    // Destructor
    ~Camera( );

    // Process the videos frame by frame and store the results
    void Process( const int startFrameIndex,
                  const int endFrameIndex );

    // Initialize various modules - detection, tracking
    void Initialize( std::string fgDetectionParams,
                     std::string blobDetectionParams,
                     std::string blobTrackerParams,
                     std::string blobAnalysisParams,
                     std::string blobPostProcessingParams );

    // Setters related to camera, output, input and region
    void SetCameraId( const int id )                            { m_cameraId                = id; }
    void SetVideoOutputDirectory( const char * directoryName )  { m_videoOutputDirectory    = directoryName;}
    void SetFGTrainFrame(const int num)                         { m_fgTrainFrames           = num; }
    void SetFgDetectionModule( const char* fgname )             { m_fgDetectorModName       = fgname; }
    void SetBlobDetectionModule( const char* bdname )           { m_blobDetectorModName     = bdname; }
    void SetBlobTrackerModule( const char* btname )             { m_blobTrackerModName      = btname; }
    void SetBlobAnalysisModule ( const char* baname  )          { m_blobAnalyzerModName     = baname; }
    void SetBlobProcessingModule ( const char* bpname  )        { m_blobPostProcModName     = bpname; }
   
    // Enable Methods
    void EnableDisplayIntermediateResult( ) { m_displayIntermediateResult   = true; }
    void EnableSaveIntermediateResult( )    { m_saveIntermediateResult      = true; }
    void EnableOriginalImageDownScale( )    { m_downScaleImage              = true; }
    
    private:

    DISALLOW_IMPLICIT_CONSTRUCTORS( Camera );

    // Initialization methods
    void    InitializeOutputVideos( );
    void    InitializeDisplayWindows( );

    // Save methods
    void    SaveBlobRecord( CvBlob* pBlob, int frameNumber );
    void    SaveBlobAsPngImage( CvBlob* pB, std::string& strName );

    // input related - camera
    int                             m_cameraId; 
    std::string                     m_videoSourceDirectory;
    std::string                     m_videoFileName;
    cv::VideoCapture                m_videoCap;

    // output related variable
    bool                            m_displayIntermediateResult;
    bool                            m_saveIntermediateResult; 
    std::string                     m_videoOutputDirectory;
    std::ofstream*                  m_pOutputRecordFileStream;
    VideoWritePtr                   m_pFGAvi;
    VideoWritePtr                   m_pBTAvi;

    // processing settings
    std::string                     m_fgDetectorModName;
    std::string                     m_blobDetectorModName;
    std::string                     m_blobTrackerModName;
    std::string                     m_blobAnalyzerModName;
    std::string                     m_blobPostProcModName;

    bool                            m_downScaleImage;
    bool                            m_initializied;
    int                             m_fgTrainFrames;
    int                             m_width;
    int                             m_height;
    cv::Mat                         m_originalFrameMat;
    cv::Mat                         m_frame;
    cv::Mat                         m_fgMask;
    IplImage                        m_frameIpl;
    IplImage*                       m_pFGMaskIpl;

    CvBlobTrackerAuto*              m_pTracker;
    CvFGDetector*                   m_pFGDetector; 
    CvBlobDetector*                 m_pBlobDetector;
    CvBlobTracker *                 m_pBlobTracker;
    CvBlobTrackAnalysis*            m_pBlobTrackAnalysis;
    CvBlobTrackPostProc*            m_pBlobProcessing;
    CvBlobTrackerAutoParam1*        m_pTrackerParams;
    };

    // declaration for shared pointer
    typedef boost::shared_ptr<Camera>   CameraPtr;
}
#endif