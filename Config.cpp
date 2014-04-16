#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "Config.h"

InputParameters  g_configInput;
std::ofstream    g_logFile;

// Mapping_Map Syntax:
// {NAMEinConfigFile,  &ConfigInput.VariableName,  m_type,  InitialValue,LimitType,  MinLimit,  MaxLimit}
// Types : {0:int, 1:text, 2: double}
// LimitType: {0:none, 1:both, 2:minimum, 3: for special case}
// We could separate this based on types to make it more flexible and allow also defaults for text types.
// m_defaultValue only for int/double type.
InputMapping Map[] = {
    {"Input_Video_Directory",           &g_configInput.m_inputVideoDirectory,               1,        0.0,        0,            0.0,    0.0},
    {"Input_Video_Name_List",           &g_configInput.m_inputVideoListCStr,                1,        0.0,        0,            0.0,    0.0},
    {"Input_Camera_ID_List",            &g_configInput.m_inputVideoCameraIDListCStr,        1,        0.0,        0,            0.0,    0.0},
    {"Start_Frame_Index",               &g_configInput.m_startFrameIndex,                   0,        0.0,        2,            0.0,    0.0},
    {"Number_Of_Frames",                &g_configInput.m_numberOfFramesToProcess,           0,        0.0,        2,            0.0,    0.0},
    {"OutPut_Video_Directory",          &g_configInput.m_outputVideoDirectory,              1,        0.0,        0,            0.0,    0.0},
    {"Log_File_Name",                   &g_configInput.m_logFileName,                       1,        0.0,        0,            0.0,    0.0},
    {"Display_Intermediate_Result",     &g_configInput.m_displayIntermediateResult,         0,        0.0,        1,            0.0,    1.0},
    {"Save_Intermediate_Result",        &g_configInput.m_saveIntermediateResult,            0,        0.0,        1,            0.0,    1.0},
    {"Down_Scale_Image",                &g_configInput.m_downScaleImage,                    0,        0.0,        1,            0.0,    1.0},
    {"Foreground_Detector_Module",      &g_configInput.m_fgDetectorMod,                     1,        0.0,        0,            0.0,    0.0},
    {"Blob_Detector_Module",            &g_configInput.m_blobDetectorMod,                   1,        0.0,        0,            0.0,    0.0},
    {"Blob_Tracker_Module",             &g_configInput.m_blobTrackerMod,                    1,        0.0,        0,            0.0,    0.0},
    {"Blob_Processing_Module",          &g_configInput.m_blobProcessingMod,                 1,        0.0,        0,            0.0,    0.0},
    {"Blob_Analysis_Module",            &g_configInput.m_blobAnalysisMod,                   1,        0.0,        0,            0.0,    0.0},
    {"FG_Train_Frames",                 &g_configInput.m_numberFGTrainFrames,               0,        2.0,        2,            1.0,    0.0},
    {"FG_Detection_Params",             &g_configInput.m_fgDetectionParams,                 1,        0.0,        0,            0.0,    0.0},
    {"Blob_Detection_Params",           &g_configInput.m_blobDetectionParams,               1,        0.0,        0,            0.0,    0.0},
    {"Blob_Tracker_Params",             &g_configInput.m_blobTrackerParams,                 1,        0.0,        0,            0.0,    0.0},
    {"Blob_Analysis_Params",            &g_configInput.m_blobAnalysisParams,                1,        0.0,        0,            0.0,    0.0},
    {"Blob_PostProc_Params",            &g_configInput.m_blobPostProcessingParams,          1,        0.0,        0,            0.0,    0.0},
    {NULL,                              NULL,                                              -1,        0.0,        0,            0.0,    0.0}
};

static void Usage(void)
{
     fprintf( stderr, "\n   BlobTracker [-h] or BlobTracker -d config.cfg [-p xxx=xxx xxxx=xxxx]");
}

 /***********************************************************************
 * \brief
 *    Returns the index number from Map[] for a given parameter name.
 * \param s
 *    parameter name string
 * \return
 *    the index number if the string is a valid parameter name,         \n
 *    -1 for error
 ***********************************************************************/
static int ParameterNameToMapIndex (char *s)
{
  int i = 0;

  while (Map[i].m_tokenNameCStr != NULL)
    if (0==strcasecmp (Map[i].m_tokenNameCStr, s))
      return i;
    else
      i++;
  return -1;
}


/************************************************************************
 * \brief
 *    Sets initial values for parameters.
 * \return
 *    -1 for error
 ***********************************************************************/
static int InitParams(void)
{
  int i = 0;

  while (Map[i].m_tokenNameCStr != NULL)
  {
    if (Map[i].m_type == 0)  // int
      * (int *) (Map[i].m_pPosition) = (int) Map[i].m_defaultValue;
    else if (Map[i].m_type == 1) // text string
      * (char*) (Map[i].m_pPosition) = '\0'; //default empty string
    else if (Map[i].m_type == 2) // double
      * (double *) (Map[i].m_pPosition) = Map[i].m_defaultValue;
      i++;
  }
  return -1;
}

/************************************************************************
 * \brief
 *    allocates memory buf, opens file Filename in f, reads contents into
 *    buf and returns buf
 * \param Filename
 *    name of config file
 * \return
 *    if successfull, content of config file
 *    NULL in case of error. Error message will be set in errortext
 ***********************************************************************/
static char *GetConfigFileContent (char *Filename)
{
  long FileSize;
  FILE *f;
  char *buf;

  if (NULL == (f = fopen (Filename, "r")))
  {
    printf ("Cannot open configuration file %s.\n", Filename);
    return NULL;
  }

  if (0 != fseek (f, 0, SEEK_END))
  {
    printf ("Cannot fseek in configuration file %s.\n", Filename);
    return NULL;
  }

  FileSize = ftell (f);
  if (FileSize < 0 || FileSize > 60000)
  {
    printf ("Unreasonable Filesize %ld reported by ftell for configuration file %s.\n", FileSize, Filename);
    return NULL;
  }
  if (0 != fseek (f, 0, SEEK_SET))
  {
    printf ("Cannot fseek in configuration file %s.\n", Filename);
    return NULL;
  }

  if ((buf = (char*)malloc (FileSize + 1))==NULL) 
  {
    printf("Cannot alloc mem: buf\n");
    return NULL;
  }

  // Note that ftell() gives us the file size as the file system sees it.  The actual file size,
  // as reported by fread() below will be often smaller due to CR/LF to CR conversion and/or
  // control characters after the dos EOF marker in the file.

  FileSize = (long) fread (buf, 1, FileSize, f);
  buf[FileSize] = '\0';

  fclose (f);
  return buf;
}


/************************************************************************
 * \brief
 *    Parses the character array buf and writes global variable input, which is defined in
 *    configfile.h.  This hack will continue to be necessary to facilitate the addition of
 *    new parameters through the Map[] mechanism (Need compiler-generated addresses in map[]).
 * \param buf
 *    buffer to be parsed
 * \param bufsize
 *    buffer size of buffer
 * \return
 *    0: success
 *    non-zero: fail
 ***********************************************************************/
static int ParseContent (char *buf, int bufsize)
{

  char *items[MAX_ITEMS_TO_PARSE];
  int MapIdx;
  int item = 0;
  int InString = 0, InItem = 0;
  char *p = buf;
  char *bufend = &buf[bufsize];
  int IntContent;
  double DoubleContent;
  int i;

// Stage one: Generate an argc/argv-type list in items[], without comments and whitespace.
// This is context insensitive and could be done most easily with lex(1).

  while (p < bufend)
  {
    switch (*p)
    {
      case 13:
        p++;
        break;
      case '#':                 // Found comment
        *p = '\0';              // Replace '#' with '\0' in case of comment immediately following integer or string
        while (*p != '\n' && p < bufend)  // Skip till EOL or EOF, whichever comes first
          p++;
        InString = 0;
        InItem = 0;
        break;
      case '\n':
        InItem = 0;
        InString = 0;
        *p++='\0';
        break;
      case ' ':
      case '\t':              // Skip whitespace, leave state unchanged
        if (InString)
          p++;
        else
        {                     // Terminate non-strings once whitespace is found
          *p++ = '\0';
          InItem = 0;
        }
        break;

      case '"':               // Begin/End of String
        *p++ = '\0';
        if (!InString)
        {
          items[item++] = p;
          InItem = ~InItem;
        }
        else
          InItem = 0;
        InString = ~InString; // Toggle
        break;

      default:
        if (!InItem)
        {
          items[item++] = p;
          InItem = ~InItem;
        }
        p++;
    }
  }

  item--;

  for (i=0; i<item; i+= 3)
  {
    if (0 > (MapIdx = ParameterNameToMapIndex (items[i])))
    {
      //snprintf (errortext, ET_SIZE, " Parsing error in config file: Parameter Name '%s' not recognized.", items[i]);
      //error (errortext, 300);
      printf ("\n\tParsing error in config file: Parameter Name '%s' not recognized.", items[i]);
      continue;
    }
    if (strcasecmp ("=", items[i+1]))
    {
      printf (" Parsing error in config file: '=' expected as the second token in each line.");
      return -1;
    }

    // Now interpret the Value, context sensitive...

    switch (Map[MapIdx].m_type)
    {
      case 0:           // Numerical
        if (1 != sscanf (items[i+2], "%d", &IntContent))
        {
          printf (" Parsing error: Expected numerical value for Parameter of %s, found '%s'.", items[i], items[i+2]);
          return -1;
        }
        * (int *) (Map[MapIdx].m_pPosition) = IntContent;
        printf (".");
        break;
      case 1:
        strncpy ((char *) Map[MapIdx].m_pPosition, items [i+2], STRING_SIZE);
        printf (".");
        break;
      case 2:           // Numerical double
        if (1 != sscanf (items[i+2], "%lf", &DoubleContent))
        {
          printf (" Parsing error: Expected numerical value for Parameter of %s, found '%s'.", items[i], items[i+2]);
          return -1;
        }
        * (double *) (Map[MapIdx].m_pPosition) = DoubleContent;
        printf (".");
        break;
      default:
        printf("Unknown value type in the map definition of configfile.h");
        return -1;
    }
  }
  //memcpy (input, &ConfigInput, sizeof (InputParameters));
  return 0;
}

/*!
 ***********************************************************************
 * \brief
 *    Outputs encoding parameters.
 * \return
 *    -1 for error
 ***********************************************************************
 */
static int DisplayParams(void)
{
  int i = 0;

  printf("******************************************************\n");
  printf("*              Configuration Parameters              *\n");
  printf("******************************************************\n");
  while (Map[i].m_tokenNameCStr != NULL)
  {
    if (Map[i].m_type == 0)
      printf("Parameter %s = %d\n",Map[i].m_tokenNameCStr,* (int *) (Map[i].m_pPosition));
    else if (Map[i].m_type == 1)
      printf("Parameter %s = ""%s""\n",Map[i].m_tokenNameCStr,(char *)  (Map[i].m_pPosition));
    else if (Map[i].m_type == 2)
      printf("Parameter %s = %.2f\n",Map[i].m_tokenNameCStr,* (double *) (Map[i].m_pPosition));
      i++;
  }
  printf("******************************************************\n");
  return 0;
}


/*
 * \brief
 *    Chomp space chars at the beginning and at the end of the str
 * \param str
 *    The string to be chomped.
 */
static void ChompStr(char* str)
{
  char* temp;
  size_t i, j, len;
  int found;
  
  len = strlen(str);
  
  if (len <= 0) return;
  
  temp = (char*) malloc(len+1);
  if (temp == NULL)
    return;
  
  memset(temp, 0, len+1);
  found = 0;
  for (i = 0, j = 0; i < len; i++)
    if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != 10 && str[i] != 13)
      break;
      
  for (; i < len; i++)
    temp[j++] = str[i];
    
  for (i = strlen(temp) - 1; i >= 0; i--)
    if (temp[i] == ' ' || temp[i] == '\t' || temp[i] == '\n' || temp[i] == 10 || temp[i] == 13)
      temp[i] = 0;
    else
      break;
  strcpy(str, temp);
  str[strlen(temp)] = 0;
  
  free(temp);
}

/*
 * \brief
 *    Get one double number from the buffer
 * \param src
 *    The source buffer
 * \param bufEnd
 *    The end of the source buffer
 * \param data
 *    The data extracted from the buffer
 * \return
 *    The start position after the number is extracted
 */
static char* getNumber(char* src, char* bufEnd, double* data)
{
  char* dst;
  char  temp[255];
  while ( (*src < '0' || *src > '9' ) && *src != '-' && *src != '+' && src < bufEnd )
    src++;
  dst = src;
  while ( ((*src >= '0' && *src <= '9') || *src == '.' || *src == '-' || *src == '+') && src < bufEnd )
    src++;
  strncpy(temp, dst, src-dst);
  temp[src-dst] = 0;
  *data = atof(temp);
  
  return src;
}

/*
 * \brief
 *    Invalidate the parameters
 * \return
 *    0: success
 *    non-zero: fail
 */
int CheckParams()
{

  return 0;
}

/*!
 ***********************************************************************
 * \brief
 *    Parse the command line parameters and read the config files.
 * \param ac
 *    number of command line parameters
 * \param av
 *    command line parameters
 * \return
 *    0: sucess
 *    non-zero: fail
 ***********************************************************************
 */
int Configure(int ac, char*av[])
{
    char* content; 
    int  CLcount, ContentLen, NumberParams;
    char *filename = DEFAULTCONFIGFILENAME;

    memset (&g_configInput, 0, sizeof (InputParameters)); 
  // Set default parameters.
    printf ("Setting m_defaultValue Parameters...\n");
    InitParams();
 
    CLcount = 1;
    if (ac==2)
    {
        if (0 == strncmp (av[1], "-h", 2))
        {
            Usage();
            return -1;
        }
    }
    
    if (ac>=3)
    {
        if (0 == strncmp (av[1], "-d", 2))
        {
            filename=av[2];
            CLcount = 3;
        }
        if (0 == strncmp (av[1], "-h", 2))
        {
            Usage();
            return -1;
        }
    }
    printf ("Parsing Config file %s\n", filename);
    content = GetConfigFileContent (filename);

    if (NULL == content)
       return -1;

    if ( 0 != ParseContent (content, (int)strlen(content)))
    {
        printf ("\n");
        free (content);
        return -1;
    }

    printf ("\n");
    free (content);
  // Parse the command line
  while (CLcount < ac)
  {
    if (0 == strncmp (av[CLcount], "-h", 2))
    {
      Usage();
      return -1;
    }

    if (0 == strncmp (av[CLcount], "-p", 2))  // A config change?
    {
      // Collect all data until next parameter (starting with -<x> (x is any character)),
      // put it into content, and parse content.

      CLcount++;
      ContentLen = 0;
      NumberParams = CLcount;

      // determine the necessary size for content
      while (NumberParams < ac && av[NumberParams][0] != '-')
        ContentLen += (int)strlen (av[NumberParams++]);        // Space for all the strings
      ContentLen += 1000;                     // Additional 1000 bytes for spaces and \0s


      if ((content = (char*) malloc (ContentLen))==NULL) 
      {
        printf("Mem error, Configure: content");
        return -1;
      }
      content[0] = '\0';

      // concatenate all parameters identified before

      while (CLcount < NumberParams)
      {
        char *source = &av[CLcount][0];
        char *destin = &content[strlen (content)];

        while (*source != '\0')
        {
          if (*source == '=')  // The Parser expects whitespace before and after '='
          {
            *destin++=' '; *destin++='='; *destin++=' ';  // Hence make sure we add it
          } else
            *destin++=*source;
          source++;
        }
        *destin = '\0';
        CLcount++;
      }
      printf ("Parsing command line string '%s'", content);
      if ( 0 != ParseContent (content, (int) strlen(content)))
      {
        free (content);
        printf ("\n");
        return -1;
      }
      free (content);
      printf ("\n");
    }
    else
    {
      printf ("Error in command line, ac %d, around string '%s', missing -f or -p parameters?", CLcount, av[CLcount]);
      return -1;
    }
  }
  printf ("\n");
  
  DisplayParams();
  
  return CheckParams();
}
