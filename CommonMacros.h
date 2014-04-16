#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

#include "Config.h"

#include <iostream>
// ABORT ERROR
static void AbortError( const int   line,
                        const char* file,
                        const char* msg = NULL )
{
    if ( msg == NULL )
    {    
        fprintf(stderr, "%s %d: ERROR\n", file, line );
        g_logFile << file << line << " ERROR\n";
    }
    else
    {
        fprintf(stderr, "%s %d: ERROR: %s\n", file, line, msg );
        g_logFile << file << line << " ERROR: " << msg << '\n';
    }

    g_logFile.flush();

    exit(0);
}

//Template function to Check if the parameter is valid
template <class T>
static T CheckIfValidParameter( bool predicate, T arg, int line, char* file )
{
    if ( predicate )
    {
        return arg;
    }
    else
    {
        AbortError( line, file, "Precondition Failed on Parameter" );
    }
}

//EXCEPTION CATCH AND ABORT
#define EXCEPTION_CATCH_AND_ABORT(context)         \
catch( std::exception& ex )                         \
{                                                   \
    std::cerr << "Context: " << context <<    " Error Message:"     << ex.what() << std::endl;    \
    AbortError( __LINE__, __FILE__, context );      \
}

//EXCEPTION CATCH AND LOG_FILE
#define EXCEPTION_CATCH_AND_LOG(context)            \
catch( std::exception& ex )                         \
{                                                   \
    g_logFile << "Context: " << context <<    " Error Message:"     << ex.what() << std::endl;    \
}

//ASSERT TRUE
#if defined(WIN32) || defined(WIN64)
#define ASSERT_TRUE( predicate )                            \
if ( (predicate) == false )                                    \
{                                                            \
    DebugBreak();                                            \
    AbortError( __LINE__, __FILE__, "Assertion Failed." );    \
}
#else
#define ASSERT_TRUE( predicate )                            \
if ( (predicate) == false )                                    \
{                                                            \
    AbortError( __LINE__, __FILE__, "Assertion Failed." );    \
}
#endif

//ASSERT PRECONDITION PARAMETER
#define ASSERT_PRECONDITION_PARAMETER( predicate, parameter )   \
CheckIfValidParameter( predicate, parameter, __LINE__, __FILE__ )


//DISALLOW EVIL CONSTRUCTOR
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
TypeName(const TypeName&);                        \
void operator=(const TypeName&)

//DISALLOW IMPLICIT CONSTRUCTOR
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
TypeName();                                    \
DISALLOW_EVIL_CONSTRUCTORS(TypeName)

//LOG_FILE
#define LOG_FILE( context )        \
g_logFile << context << std::endl;        

//LOG CONSOLE
#define LOG_CONSOLE( context )    \
std::cout << context << std::endl

// LOG Both file and Console
#define LOG( context )         \
LOG_FILE(context)              \
LOG_CONSOLE(context)
#endif