#ifndef OPENCV_DEFINITIONS_H
#define OPENCV_DEFINITIONS_H

#pragma warning(disable: 4996)

#include "cv.h"
#include "cvaux.h"
#include "highgui.h"
#include "cxcore.h"

//load the opencv static libraries
#ifdef OPENCV2_3
#if !defined(_DEBUG)

#pragma comment(lib,"opencv_core230.lib")
#pragma comment(lib,"opencv_highgui230.lib")    
#pragma comment(lib,"opencv_legacy230.lib")    
#pragma comment(lib,"opencv_imgproc230")    

#else

#pragma comment(lib,"opencv_core230d.lib")
#pragma comment(lib,"opencv_highgui230d.lib")    
#pragma comment(lib,"opencv_legacy230d.lib")    
#pragma comment(lib,"opencv_imgproc230d")    

#endif
#endif

//load the opencv static libraries
#ifdef OPENCV2_2
#if !defined(_DEBUG)

#pragma comment(lib,"opencv_core220.lib")
#pragma comment(lib,"opencv_highgui220.lib")    
#pragma comment(lib,"opencv_legacy220.lib")    
#pragma comment(lib,"opencv_imgproc220")   

#else

#pragma comment(lib,"opencv_core220d.lib")
#pragma comment(lib,"opencv_highgui220d.lib")    
#pragma comment(lib,"opencv_legacy220d.lib")    
#pragma comment(lib,"opencv_imgproc220d")  

#endif
#endif

#ifdef OPENCV2_1
#if !defined(_DEBUG)

#pragma comment(lib,"cv210.lib")
#pragma comment(lib,"cxcore210.lib")
#pragma comment(lib,"highgui210.lib")
#pragma comment(lib,"ml210.lib")
#pragma comment(lib,"cvaux210.lib")
#pragma comment(lib,"cxts210.lib")

#else

#pragma comment(lib,"cv210d.lib")
#pragma comment(lib,"cxcore210d.lib")
#pragma comment(lib,"highgui210d.lib")
#pragma comment(lib,"ml210d.lib")
#pragma comment(lib,"cvaux210d.lib")
#pragma comment(lib,"cxts210d.lib")

#endif
#endif

/* Select appropriate case insensitive string comparison function: */
#if defined WIN32 || defined _MSC_VER || defined WIN64
#define MY_STRNICMP strnicmp
#define MY_STRICMP  stricmp
#else
#define MY_STRNICMP strncasecmp
#define MY_STRICMP strcasecmp
#endif

// Declare foreground detection module
typedef struct DefModule_FGDetector
{
    CvFGDetector*   (*create)( );
    const char*     nickname;
    const char*     description;
} DefModule_FGDetector;

// Declare blob detection module
typedef struct DefModule_BlobDetector
{
    CvBlobDetector* (*create)( );
    const char*     nickname;
    const char*     description;
} DefModule_BlobDetector;

//Declare blob tracking modules
typedef struct DefModule_BlobTracker
{
    CvBlobTracker* (*create)();
    const char*     nickname;
    const char*     description;
} DefModule_BlobTracker;

// Declare Blob Processing module
typedef struct DefModule_BlobTrackPostProc
{
    CvBlobTrackPostProc* (*create)();
    const char* nickname;
    const char* description;
} DefModule_BlobTrackPostProc;

// Initialize the list of BLOB TRAJECTORY ANALYSIS modules
CvBlobTrackAnalysis* cvCreateModuleBlobTrackAnalysisDetector();
typedef struct DefModule_BlobTrackAnalysis
{
    CvBlobTrackAnalysis* (*create)();
    const char*     nickname;
    const char*     description;
} DefModule_BlobTrackAnalysis;

// Declare blob generation module
typedef struct DefModule_BlobTrackGen
{
    CvBlobTrackGen* (*create)();
    const char* nickname;
    const char* description;
} DefModule_BlobTrackGen;
#endif