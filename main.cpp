#include "Config.h"
#include "Camera.h"

//declare functions
void InitializeInputOutputSettings( std::vector<Tracker::CameraPtr>& theVideoClipList );

//main function
int main( int argc, char* argv[] )
{
    try
    {
        // read configuration file
        int result = Configure( argc, argv );
        ASSERT_TRUE( result == 0 );

        // create the log file
        std::string logFile( g_configInput.m_outputVideoDirectory );
        g_logFile.open( ( logFile + "/" + g_configInput.m_logFileName ).c_str(),
                       std::ios_base::out );

        // create and initialize the video list corresponding to each camera view
        std::vector<Tracker::CameraPtr> videoClipPtrList;

        // initialize the video list with input configuration
        InitializeInputOutputSettings( videoClipPtrList );

        // main processing 
        for ( unsigned int i = 0; i < videoClipPtrList.size(); i++ )
        {
            ASSERT_TRUE( videoClipPtrList[i] != NULL );

            int endFrameIndex;
            if ( g_configInput.m_numberOfFramesToProcess > 0 )
            {
                endFrameIndex = g_configInput.m_startFrameIndex + g_configInput.m_numberOfFramesToProcess;
            }
            else
            {
                endFrameIndex = -1;
            }

            // Process Frames in individual camera views
            videoClipPtrList[i]->Process( g_configInput.m_startFrameIndex,
                                          endFrameIndex );
        }
    }
    catch ( std::exception e)
    {
        LOG( e.what() );
    }

    // Close the log file
    g_logFile.close();

    return 0;
}

// Initializes the module using the input config file
void InitializeInputOutputSettings( std::vector<Tracker::CameraPtr>& cameraPtrList )
{
    // get the input video directory and initialize camera pointers
    std::string inputVideoDirectory = g_configInput.m_inputVideoDirectory;
    int numberOfVideos = 0;

     // strtok - string to token conversion
    char* pch = strtok ( g_configInput.m_inputVideoListCStr, "," );
    while ( pch != NULL )
    {
        numberOfVideos++;
        cameraPtrList.push_back( Tracker::CameraPtr( new Tracker::Camera( inputVideoDirectory, pch ) ) );
        pch = strtok( NULL, "," );
    }

    // set camera id for each video 
    pch = strtok ( g_configInput.m_inputVideoCameraIDListCStr, "," ); //strtok - string to token conversion
    for ( int i=0; i < numberOfVideos; i++ )
    {        
        ASSERT_TRUE(pch!=NULL);
        cameraPtrList[i]->SetCameraId( atoi(pch) );
        pch = strtok( NULL, "," );        
    }

    // set the values in each camera
    for( int i=0; i < numberOfVideos; i++ )
    {
        cameraPtrList[i]->SetVideoOutputDirectory( g_configInput.m_outputVideoDirectory );

        if( g_configInput.m_displayIntermediateResult == 1 )
        {
            cameraPtrList[i]->EnableDisplayIntermediateResult();
        }

        if ( g_configInput.m_saveIntermediateResult == 1)
        {
            cameraPtrList[i]->EnableSaveIntermediateResult( );
        }

        if ( g_configInput.m_downScaleImage == 1)
        {
            cameraPtrList[i]->EnableOriginalImageDownScale( );
        }

        cameraPtrList[i]->SetFGTrainFrame( g_configInput.m_numberFGTrainFrames );
        cameraPtrList[i]->SetFgDetectionModule( g_configInput.m_fgDetectorMod );
        cameraPtrList[i]->SetBlobDetectionModule( g_configInput.m_blobDetectorMod );
        cameraPtrList[i]->SetBlobTrackerModule( g_configInput.m_blobTrackerMod );
        cameraPtrList[i]->SetBlobAnalysisModule( g_configInput.m_blobAnalysisMod );
        cameraPtrList[i]->SetBlobProcessingModule( g_configInput.m_blobProcessingMod );

        cameraPtrList[i]->Initialize(  g_configInput.m_fgDetectionParams,
                                       g_configInput.m_blobDetectionParams,
                                       g_configInput.m_blobTrackerParams,
                                       g_configInput.m_blobAnalysisParams,
                                       g_configInput.m_blobPostProcessingParams );
    }
}