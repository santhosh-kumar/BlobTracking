#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <iostream>
#include <fstream>

#define STRING_SIZE                 1000
#define MAX_ITEMS_TO_PARSE          10000
#define MAX_NUMBER_OF_VIEWS         100

#define DEFAULTCONFIGFILENAME       "config.cfg"


#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#if defined(WIN32) || defined(WIN64)
#define strcasecmp  strcmpi
#endif

//InputParameters
typedef struct 
{
    //input related
    char    m_inputVideoDirectory[STRING_SIZE];         //input video directory
    char    m_inputVideoCameraIDListCStr[STRING_SIZE];  //input video camera ID list
    char    m_inputVideoListCStr[STRING_SIZE];          //input video list corresponding to the camera ID
    int     m_startFrameIndex;                          //starting frame index
    int     m_numberOfFramesToProcess;                  //number of frames

    //output/display related
    char       m_outputVideoDirectory[STRING_SIZE];        //output video directory file location
    int        m_displayIntermediateResult;                //should display intermediate results
    int        m_saveIntermediateResult;                   //save intermediate results

    //general setting
    int        m_downScaleImage;                           //should downscale image?
    int        m_numberFGTrainFrames;                      //number of frames for foreground training
    char       m_fgDetectorMod[STRING_SIZE];               //FG detection Module
    char       m_blobDetectorMod[STRING_SIZE];             //blob detection module
    char       m_blobTrackerMod[STRING_SIZE];              //blob tracker module
    char       m_blobProcessingMod[STRING_SIZE];           //blob processing module
    char       m_blobAnalysisMod[STRING_SIZE];             //blob analysis module
    char       m_fgDetectionParams[STRING_SIZE];           //FG detection params
    char       m_blobDetectionParams[STRING_SIZE];         //blob detection params
    char       m_blobTrackerParams[STRING_SIZE];           //blob tracker params
    char       m_blobAnalysisParams[STRING_SIZE];          //blob analysis params
    char       m_blobPostProcessingParams[STRING_SIZE];    //blob post processing params
    char       m_logFileName[STRING_SIZE];                 //name of the log file
} InputParameters;

//InputMapping
typedef struct 
{
    char*   m_tokenNameCStr;
    void*   m_pPosition;
    int     m_type;
    double  m_defaultValue;
    int     m_paramLimit; //! 0: no limits, 1: both min and max, 2: only min (i.e. no negatives), 3: for special cases (not defined yet)
    double  m_minLimit;
    double  m_maxLimit;
} InputMapping;

// define the global constants
extern std::ofstream        g_logFile;
extern InputParameters      g_configInput;
extern InputMapping         Map[];

// configures the input parameters by reading from the .cfg file
int  Configure ( int ac, char *av[] );

#endif