#include "Camera.h"
#include "OpenCvUtilities.h"

#define DEFAULT_FG_TRAINING_FRAMES  5

// List of foreground detection algorithms
CvFGDetector* cvCreateFGDetector0      ( ) { return cvCreateFGDetectorBase( CV_BG_MODEL_FGD,        NULL); }
CvFGDetector* cvCreateFGDetector0Simple( ) { return cvCreateFGDetectorBase( CV_BG_MODEL_FGD_SIMPLE, NULL); }
CvFGDetector* cvCreateFGDetector1      ( ) { return cvCreateFGDetectorBase( CV_BG_MODEL_MOG,        NULL); }

// Initialize the list of FG detection modules
DefModule_FGDetector FGDetector_Modules[] =
{
    { cvCreateFGDetector0,          "FG_0",     "Foreground Object Detection from Videos Containing Complex Background. ACM MM2003." },
    { cvCreateFGDetector0Simple,    "FG_0S",    "Simplified version of FG_0" },
    { cvCreateFGDetector1,          "FG_1",     "Adaptive background mixture models for real-time tracking. CVPR1999" },
    { NULL,                         NULL,       NULL}
};

// Initialize the list of blob detection modules
DefModule_BlobDetector BlobDetector_Modules[] =
{
    { cvCreateBlobDetectorCC,       "BD_CC",        "Detect new blob by tracking CC of FG mask" },
    { cvCreateBlobDetectorSimple,   "BD_Simple",    "Detect new blob by uniform moving of connected components of FG mask" },
    { NULL,                         NULL,           NULL }
};

// Initialize the list of blob tracker modules
DefModule_BlobTracker BlobTracker_Modules[] =
{
    { cvCreateBlobTrackerCCMSPF,        "CCMSPF",   "connected component tracking and MSPF resolver for collision" },
    { cvCreateBlobTrackerCC,            "CC",       "Simple connected component tracking" },
    { cvCreateBlobTrackerMS,            "MS",       "Mean shift algorithm " },
    { cvCreateBlobTrackerMSFG,          "MSFG",     "Mean shift algorithm with FG mask using" },
    { cvCreateBlobTrackerMSPF,          "MSPF",     "Particle filtering based on MS weight" },
    { cvCreateBlobTrackerMSFGS,         "MSFGS",    "Mean shift algorithm with scale change and FG mask using" },
    { NULL,NULL,NULL}
};

// Initialize the list of blob post processing modules
DefModule_BlobTrackPostProc BlobTrackPostProc_Modules[] =
{
    {cvCreateModuleBlobTrackPostProcKalman,"Kalman","Kalman filtering of blob position and size"},
    {NULL,"None","No post processing filter"},
    {NULL,NULL,NULL}
};

// Initialize the list of blob track analysis
DefModule_BlobTrackAnalysis BlobTrackAnalysis_Modules[] =
{
    {NULL,"None","No trajectory analyzer"},
    {cvCreateModuleBlobTrackAnalysisHistPVS,"HistPVS","Histogram of 5D feature vector analysis (x,y,vx,vy,state)"},
    {cvCreateModuleBlobTrackAnalysisHistP,"HistP","Histogram of 2D feature vector analysis (x,y)"},
    {cvCreateModuleBlobTrackAnalysisHistPV,"HistPV","Histogram of 4D feature vector analysis (x,y,vx,vy)"},
    {cvCreateModuleBlobTrackAnalysisHistSS,"HistSS","Histogram of 4D feature vector analysis (startpos,endpos)"},
    {cvCreateModuleBlobTrackAnalysisTrackDist,"TrackDist","Compare tracks directly"},
    {cvCreateModuleBlobTrackAnalysisIOR,"IOR","Integrator (by OR operation) of several analyzers "},
    {NULL,NULL,NULL}
};

// Initialize the list of blob generation modules
DefModule_BlobTrackGen BlobTrackGen_Modules[] =
{
    {NULL,"None","No Blob generator"},
    {cvCreateModuleBlobTrackGenYML,"YML","Generate track record in YML format as synthetic video data"},
    {cvCreateModuleBlobTrackGen1,"RawTracks","Generate raw track record (x,y,sx,sy),()... in each line"},
    {NULL,NULL,NULL}
};

namespace Tracker
{
    /********************************************************************
    Camera
        Encapsulated Video Clip object
    Exceptions:
        None
    *********************************************************************/
    Camera::Camera( const std::string&    sourceDirectory,
                    const std::string&    videoFileName )
        : m_cameraId( 0 ),
        m_videoSourceDirectory( sourceDirectory ),
        m_videoFileName( videoFileName ),
        m_videoCap( ),
        m_videoOutputDirectory( ),
        m_saveIntermediateResult( "" ),
        m_pOutputRecordFileStream( NULL ),
        m_pFGAvi( ),
        m_pBTAvi( ),
        m_displayIntermediateResult( false ),
        m_fgDetectorModName ( "" ),
        m_blobDetectorModName( "" ),
        m_blobTrackerModName( "" ),
        m_blobAnalyzerModName( "" ),
        m_blobPostProcModName( "" ),
        m_downScaleImage ( false ),
        m_fgTrainFrames( DEFAULT_FG_TRAINING_FRAMES ),
        m_initializied( false ),
        m_width( 0 ),
        m_height( 0 ),
        m_originalFrameMat( ),
        m_frame( ),
        m_frameIpl( ),
        m_pFGMaskIpl( ),
        m_pTracker( NULL ),
        m_pFGDetector( NULL ),
        m_pBlobDetector( NULL ),
        m_pBlobTracker( NULL ),
        m_pBlobTrackAnalysis( NULL ),
        m_pBlobProcessing( NULL ),
        m_pTrackerParams( new CvBlobTrackerAutoParam1( ) )
    {
    }

    /********************************************************************
    Default Destructor for class Camera
    *********************************************************************/
    Camera::~Camera()
    {
        try
        {
            LOG( "Destroying Camera " );
            m_videoCap.release();

            LOG( "Closing output file" );
            if ( m_pOutputRecordFileStream != NULL )
            {
                m_pOutputRecordFileStream->close();
                delete m_pOutputRecordFileStream;
            }

            LOG( "Destroying output windows" );
            if ( m_displayIntermediateResult )
            {
                cv::destroyWindow( m_videoFileName + "_FGMask" );
                cv::destroyWindow( m_videoFileName + "_Tracking" );
            }

            LOG( "Releasing foreground mask..." );
            if ( m_pFGMaskIpl != NULL )
            {
                cvReleaseImage( &m_pFGMaskIpl );
            }

            LOG( "Releasing Auto tracker..." );
            if ( m_pTracker != NULL )
            {
               cvReleaseBlobTrackerAuto( &m_pTracker );
            }

            LOG( "Releasing foreground detector..." );
            if ( m_pFGDetector != NULL )
            {
                cvReleaseFGDetector( &m_pFGDetector );
            }

            LOG( "Releasing blob tracker..." );
            if ( m_pBlobTracker != NULL )
            {
                cvReleaseBlobTracker( &m_pBlobTracker );
            }

            LOG( "Releasing blob track analyzer..." );
            if ( m_pBlobTrackAnalysis != NULL )
            {
                cvReleaseBlobTrackAnalysis( &m_pBlobTrackAnalysis );
            }

            LOG( "Releasing blob processor..." );
            if ( m_pBlobProcessing != NULL )
            {
                m_pBlobProcessing->Release( );
            }
        
            LOG( "Releasing blob detector..." );
            if ( m_pBlobDetector != NULL )
            {
                cvReleaseBlobDetector( &m_pBlobDetector );
            }

            LOG( "Deleting tracking params" );
            if ( m_pTrackerParams != NULL )
            {
                delete m_pTrackerParams;
            }

            LOG( "Successfully Destroyed Camera " );
        }
        EXCEPTION_CATCH_AND_LOG( "Failed to destroy the camera object" );
    }

    /*************************************************************************
    InitializeOutputVideos
        Initialize output videos
    Exceptions
        None
    *************************************************************************/
    void Camera::InitializeOutputVideos( )
    {
        try 
        {
            if ( !m_saveIntermediateResult )
            {
                return;
            }

            //initialize the foreground avi
            m_pFGAvi = VideoWritePtr( new cv::VideoWriter( m_videoOutputDirectory +  "/" +  m_videoFileName + "_FG.avi", ///filename
                                            CV_FOURCC('x','v','i','d'), //video file format
                                            15,                         //frame rate
                                            m_frame.size(),             //number of frame
                                            1 ) );

            //initialize the blob tracker avi
            m_pBTAvi = VideoWritePtr( new cv::VideoWriter( m_videoOutputDirectory + "/" + m_videoFileName + "_BT.avi",
                                            CV_FOURCC('x','v','i','d'),
                                            15, 
                                            m_frame.size(), 
                                            1 ) );

            ASSERT_TRUE( m_pBTAvi->isOpened( ) );
            ASSERT_TRUE( m_pFGAvi->isOpened( ) );
        }
        EXCEPTION_CATCH_AND_ABORT( "Failed to initialize output videos" );
    }


    /*************************************************************************
    InitializeDisplayWindows
        Initialize display windows
    Exceptions
        None
    *************************************************************************/
    void Camera::InitializeDisplayWindows( )
    {
        try
        {
            //display intermediate result
            if ( m_displayIntermediateResult )
            {
                //create windows for detector outputs
                cv::namedWindow( m_videoFileName + "_FGMask", 1 );
                cv::namedWindow( m_videoFileName + "_Tracking", 1 );

                cvMoveWindow( (m_videoFileName+"_FGMask").c_str(), 700, 10 );
                cvMoveWindow( (m_videoFileName+"_Tracking").c_str(), 700, 400 );
            }
        }
        EXCEPTION_CATCH_AND_ABORT( "Failed to initialize display windows" );
    }

    /*************************************************************************
    Initialize
        Initialize Camera
    Exceptions
        None
    *************************************************************************/
    void Camera::Initialize( std::string fgDetectionParams,
                             std::string blobDetectionParams,
                             std::string blobTrackerParams,
                             std::string blobAnalysisParams,
                             std::string blobPostProcessingParams )
    {
        LOG_FILE( "Initialize video " + m_videoSourceDirectory + '/' + m_videoFileName );
        LOG_FILE( "\t from camera: " + m_cameraId );

        // open input video
        m_videoCap.open( m_videoSourceDirectory + '/' + m_videoFileName );
        if ( !m_videoCap.isOpened()) 
        {
            LOG_FILE( "Unable to open input video file " + m_videoSourceDirectory + '/' + m_videoFileName );
            AbortError( __LINE__, __FILE__, "Unable to open input video file" );
        }

        // open record file 
        m_pOutputRecordFileStream = new std::ofstream( (m_videoOutputDirectory+"/"+m_videoFileName+"_record.txt").c_str(), std::ios_base::out );
        if ( !m_pOutputRecordFileStream->is_open( ) )
        {
            LOG_FILE( "Unable to open record file " + m_videoOutputDirectory + '/' + m_videoFileName+"_record.txt" );
            AbortError( __LINE__, __FILE__, "Unable to open record file" );
        }

        // a temporary blob params variable
        char tempCharString[STRING_SIZE];

        // initialize foreground detection module
        DefModule_FGDetector*            pFGModule;
        for( pFGModule = FGDetector_Modules; pFGModule->nickname; ++pFGModule )
        {
            if ( m_fgDetectorModName.compare( pFGModule->nickname ) == 0 ) 
            {
                break;
            }
        }
        ASSERT_TRUE( pFGModule != NULL );
        m_pFGDetector = (*(pFGModule->create))( );
        strcpy( tempCharString, fgDetectionParams.c_str() ); 
        OpenCvWrapper::Utils::SetParameters( tempCharString, m_pFGDetector, "fg", m_fgDetectorModName.c_str( ) );

        // Initialize blob detection module
        DefModule_BlobDetector*         pBDModule;
        for( pBDModule = BlobDetector_Modules; pBDModule->nickname; ++pBDModule )
        {
            if ( m_blobDetectorModName.compare(pBDModule->nickname)==0 )
            {
                break;
            }
        }
        ASSERT_TRUE( pBDModule != NULL );
        m_pBlobDetector = (*(pBDModule->create))( );
        strcpy( tempCharString, blobDetectionParams.c_str() ); 
        OpenCvWrapper::Utils::SetParameters(tempCharString, m_pBlobDetector, "bd", m_blobDetectorModName.c_str());

        // Initialize blob tracker module
        DefModule_BlobTracker*            pBTModule;
        for ( pBTModule=BlobTracker_Modules; pBTModule->nickname; ++pBTModule )
        {
            if( m_blobTrackerModName.compare(pBTModule->nickname) == 0 ) 
            {
                break;
            }
        }
        ASSERT_TRUE(pBTModule!=NULL);
        m_pBlobTracker = (*(pBTModule->create))(); 
        strcpy( tempCharString, blobTrackerParams.c_str() ); 
        OpenCvWrapper::Utils::SetParameters(tempCharString, m_pBlobTracker, "bt", m_blobTrackerModName.c_str());

        // Initialize Blob Analysis Module
        DefModule_BlobTrackAnalysis*            pBAnalysisModule;
        for ( pBAnalysisModule = BlobTrackAnalysis_Modules; pBAnalysisModule->nickname; ++pBAnalysisModule )
        {
            if ( m_blobAnalyzerModName.compare(pBAnalysisModule->nickname) == 0 ) 
            {
                break;
            }
        }
        ASSERT_TRUE(pBAnalysisModule!=NULL);
        if ( m_blobAnalyzerModName.compare("None") != 0 )
        {
            m_pBlobTrackAnalysis = (*(pBAnalysisModule->create))();

            strcpy( tempCharString, blobAnalysisParams.c_str() ); 
            OpenCvWrapper::Utils::SetParameters( tempCharString, 
                                                 m_pBlobTrackAnalysis,
                                                 "bta",
                                                 m_blobAnalyzerModName.c_str() );

        }

        // Initialize Blob Post Processing Module
        DefModule_BlobTrackPostProc*            pBlobPostProcModule;
        for ( pBlobPostProcModule = BlobTrackPostProc_Modules; pBlobPostProcModule->nickname; ++pBlobPostProcModule )
        {
            if ( m_blobPostProcModName.compare(pBlobPostProcModule->nickname) == 0 )
            {
                break;
            }
        }
        ASSERT_TRUE( pBlobPostProcModule!=NULL );
        if ( m_blobPostProcModName.compare("None") != 0 )
        {
            m_pBlobProcessing = (*(pBlobPostProcModule->create))();

            strcpy( tempCharString, blobAnalysisParams.c_str() ); 
            OpenCvWrapper::Utils::SetParameters( tempCharString, 
                                                m_pBlobProcessing,
                                                "btpp",
                                                m_blobPostProcModName.c_str() );
        }


        // read first frame and discard it 
        m_videoCap >> m_originalFrameMat;

        // resize the image if downscale option is chosen
        if ( m_downScaleImage )
        {
            cv::resize( m_originalFrameMat, m_frame, cv::Size(0,0),0.5,0.5);
        }
        else
        {
            m_frame = m_originalFrameMat;
        }

        // set width and height
        m_width         = m_frame.size().width;
        m_height        = m_frame.size().height;

        //Initialize Output videos
        InitializeOutputVideos( );

        // display results
        if ( m_displayIntermediateResult )
        {
            cv::imshow( m_videoFileName + "_Tracking", m_frame );
        }

        if ( m_saveIntermediateResult )
        {
            *m_pFGAvi << cv::Mat::zeros(m_width,m_height,CV_8UC3);
            *m_pBTAvi << m_frame;
        }

        // Set the OpenCv Auto Tracker Params
        ASSERT_TRUE( m_pTrackerParams != NULL );
        m_pTrackerParams->FGTrainFrames = m_fgTrainFrames;
        m_pTrackerParams->pBD           = m_pBlobDetector;
        m_pTrackerParams->pBT           = m_pBlobTracker;
        m_pTrackerParams->pBTA          = m_pBlobTrackAnalysis;
        m_pTrackerParams->pBTPP         = m_pBlobProcessing;
        m_pTrackerParams->pFG           = m_pFGDetector;
        m_pTrackerParams->UsePPData     = false;

        m_pTracker = cvCreateBlobTrackerAuto1( m_pTrackerParams );

        ASSERT_TRUE( m_pTracker != NULL );

        // Set the flag
        m_initializied = true;
    }

    /*************************************************************************
    Process
        Process the frames in a video one by one.
            1) FG detection
            2) Blob Detection
            3) Blob Tracking and Association
            4) Blob Post Processing
            5) Blob Analysis
            6) Store the results
    Exceptions
        None
    *************************************************************************/
    void Camera::Process(const int startFrameIndex, const int endFrameIndex)
    {
        ASSERT_TRUE ( m_initializied );
        ASSERT_TRUE ( m_pTracker != NULL );

        InitializeDisplayWindows( );

        LOG_CONSOLE( "Start processing " + m_videoFileName );

        int key, oneFrameProcess=0, frameNum; 
        for ( frameNum = 1; 
             m_videoCap.grab() &&
            ( key = cvWaitKey( oneFrameProcess ? 0 : 1 ) ) != 27 &&
            ( frameNum <=  endFrameIndex || endFrameIndex < 0 );
            frameNum++ )
        {
            if ( frameNum >= startFrameIndex )
            {
                std::cout << "frameNum:  " << frameNum << '\r';

                // get the video frame
                m_videoCap.retrieve( m_originalFrameMat );

                // downscale the image if required
                if ( m_downScaleImage )
                {
                    cv::resize( m_originalFrameMat, m_frame,  m_frame.size() );
                }
                else
                {
                    m_frame = m_originalFrameMat;
                }

                m_frameIpl = m_frame; 

                if ( key != -1 )
                {
                    oneFrameProcess = ( key == 'r' ) ? 0 : 1;
                }

                // Process the current frame
                m_pTracker->Process( &m_frameIpl, m_pFGMaskIpl);
                m_fgMask        = m_pTracker->GetFGMask();


                // Process the current video frame using the blob tracker
                IplImage fgMaskIpl = m_fgMask;


                // Save Blob Information in a file
                for( int i = m_pTracker->GetBlobNum(); i> 0; i-- )
                {
                    CvBlob* pBlob = m_pTracker->GetBlob(i-1);

                    ASSERT_TRUE( pBlob != NULL );

                    // Save blob record
                    SaveBlobRecord( pBlob, frameNum );
                }

                if ( m_displayIntermediateResult || m_saveIntermediateResult )
                {
                    char tempString[128];
                    std::string textMessage;
                    //display intermediate result if necessary
                    CvFont    font; 
                    CvSize  TextSize;
                    cvInitFont( &font, CV_FONT_HERSHEY_PLAIN, 0.7, 0.7, 0, 1, CV_AA );

                    sprintf(tempString,"frame # %d", frameNum);
                    textMessage = tempString;
                    cv::putText( m_originalFrameMat, textMessage, cv::Point(10,20), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar((0,255,255)));
                    cv::putText( m_fgMask,textMessage, cv::Point(10,20), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar((0,255,255)));
                    cv::putText( m_frame, textMessage, cv::Point(10,20), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar((0,255,255)));

                    //drawing blobs if any with green ellipse with m_cvBlob id displayed next to it.
                    int c = 0; // 0: g; 255: red
                    for ( int i = m_pTracker->GetBlobNum(); i > 0; i-- )
                    {
                        CvBlob* pBlob = m_pTracker->GetBlob(i-1);

                        ASSERT_TRUE( pBlob != NULL );

                        cv::Point blobCorner( cvRound( pBlob->x * 256 ), cvRound( pBlob->y * 256 ) );

                        CvSize  blobSize = cvSize( MAX( 1, cvRound( CV_BLOB_RX(pBlob) * 256 ) ), 
                                                   MAX( 1, cvRound( CV_BLOB_RY(pBlob) * 256 ) ) );

                        cv::Scalar boundingBoxColor( c, 255-c, 0 );

                        if ( m_pTracker->GetState( CV_BLOB_ID( pBlob ) ) != 0 )
                        {
                            boundingBoxColor = cv::Scalar( 255-c, c, 0 );
                        }

                        cv::ellipse( m_frame, 
                                    cv::RotatedRect( cv::Point2f( pBlob->x, pBlob->y ), cv::Size2f( pBlob->w, pBlob->h ), 0 ),
                                    cv::Scalar( c, 255-c, 0 ) );
                        blobCorner.x >>= 8;      
                        blobCorner.y >>= 8;
                        
                        blobSize.width >>= 8;
                        blobSize.height >>= 8;
                        blobCorner.y -= blobSize.height;

                        sprintf( tempString, "BlobId=%03d", CV_BLOB_ID(pBlob) );
                        cvGetTextSize( tempString, &font, &TextSize, NULL );
                        
                        cv::putText( m_frame,
                                     std::string( tempString ),
                                     blobCorner,
                                     CV_FONT_HERSHEY_PLAIN,
                                     1,
                                     cv::Scalar( 255, 255, 0, 0 ) );
                    }
                }

                if ( m_displayIntermediateResult )
                {
                    cv::imshow(m_videoFileName+"_FGMask", m_fgMask);
                    cv::imshow(m_videoFileName+"_Tracking", m_frame);
                }

                if ( m_saveIntermediateResult )
                {
                    cv::Mat tmpFrame;
                    cv::cvtColor( m_fgMask, tmpFrame, CV_GRAY2BGR );
                    *m_pFGAvi << tmpFrame;             
                    *m_pBTAvi << m_frame;
                }
            }
        }

        g_logFile << "End of  processing " << m_videoFileName << std::endl;
        std::cout << "End of processing " << m_videoFileName << std::endl;
    }


    /********************************************************************
    SaveBlobAsPngImage
        Save blob as a PNG image
    Exceptions:
        None
    *********************************************************************/
    void    Camera::SaveBlobAsPngImage( CvBlob* pB,
                                        std::string& strName )
    {
        //save the particular blob to a PNG file
        cv::Rect roi;
        if ( m_downScaleImage )
        {
            roi = cv::Rect( std::max( 0, (int)floor(pB->x * 2 - pB->w) ),
                            std::max( 0,(int)floor(pB->y * 2 - pB->h)),
                            (int)floor(pB->w * 2),
                            (int)floor(pB->h * 2) ); 

            if ( roi.x+roi.width  > m_width * 2 ) 
            {
                roi.width = m_width * 2 - roi.x - 1;
            }

            if ( roi.y+roi.height > m_height * 2 )
            {
                roi.height = m_height * 2 - roi.y - 1;
            }
        }
        else
        {
            roi = cv::Rect( std::max(0,(int)floor(pB->x - pB->w/2)),
                            std::max(0, (int)floor(pB->y - pB->h / 2)),
                            (int)floor(pB->w),
                            (int)floor(pB->h) );

            if ( roi.x+roi.width  > m_width )
            {
                roi.width = m_width - roi.x - 1;
            }

            if ( roi.y+roi.height > m_height )
            {
                roi.height = m_height - roi.y - 1;
            }
        }
        
        cv::imwrite( strName,
                     m_originalFrameMat(roi) );  
    }

    /********************************************************************
    SaveBlobRecord
        Saves the blob record on to a file
    Exceptions:
        None
    *********************************************************************/
    void Camera::SaveBlobRecord( CvBlob* pBlob, int frameNumber )
    {
        ASSERT_TRUE( pBlob != NULL );
        try
        {
            // Stream the frame number
            *m_pOutputRecordFileStream<< frameNumber << ' ';

            int scale = m_downScaleImage ? 2 : 1;

            // Stream the bounding box
            *m_pOutputRecordFileStream << std::max( cvRound( pBlob->x ),0 ) * scale << ' ' << std::max( cvRound( pBlob->y ), 0 ) * scale << ' ' <<
                std::max( cvRound( pBlob->w ), 0 )*scale << ' ' << std::max( cvRound( pBlob->h ), 0 ) * scale << ' ';

            // Stream the blob ID
            *m_pOutputRecordFileStream<< CV_BLOB_ID( pBlob ) << '\t';

            // Stream the timestamp
            *m_pOutputRecordFileStream << 0;

            // Add a new line
            *m_pOutputRecordFileStream << std::endl;
        }
        EXCEPTION_CATCH_AND_ABORT( "Failed to save blob information" );
    }
}