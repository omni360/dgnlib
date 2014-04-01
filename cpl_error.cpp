/**********************************************************************
 * $Id: cpl_error.cpp,v 1.36 2006/03/07 22:05:32 fwarmerdam Exp $
 *
 * Name:     cpl_error.cpp
 * Project:  CPL - Common Portability Library
 * Purpose:  Error handling functions.
 * Author:   Daniel Morissette, danmo@videotron.ca
 *
 **********************************************************************
 * Copyright (c) 1998, Daniel Morissette
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************
 *
 * $Log: cpl_error.cpp,v $
 * Revision 1.36  2006/03/07 22:05:32  fwarmerdam
 * fix up docs a bit
 *
 * Revision 1.35  2006/02/27 23:24:13  fwarmerdam
 * Removed printf about growing error buffer.
 *
 * Revision 1.34  2006/02/27 02:40:11  fwarmerdam
 * Dynamically grow error buffer.
 *
 * Revision 1.33  2006/02/19 21:54:34  mloskot
 * [WINCE] Changes related to Windows CE port of CPL. Most changes are #ifdef wrappers.
 *
 * Revision 1.32  2005/07/25 18:09:10  fwarmerdam
 * Made pfnErrorHandler static.
 *
 * Revision 1.31  2005/07/25 17:56:34  fwarmerdam
 * Altered so that CPLSetErrorHandler sets a handler global to all threads, but
 * that the error handler stack takes precidence over it.
 *
 * Revision 1.30  2005/07/08 14:35:56  fwarmerdam
 * restructured to use error context and TLS
 *
 * Revision 1.29  2005/05/23 03:58:51  fwarmerdam
 * make handler stack threadlocal
 *
 * Revision 1.28  2005/04/04 15:23:31  fwarmerdam
 * some functions now CPL_STDCALL
 *
 * Revision 1.27  2003/05/27 20:44:16  warmerda
 * use VSI time services
 * 
 * Revision 1.26  2003/05/08 21:51:14  warmerda
 * added CPL{G,S}etConfigOption() usage
 *
 * Revision 1.25  2003/04/04 14:57:38  dron
 * _vsnprintf() hack moved to the cpl_config.h.vc.
 *
 * Revision 1.24  2003/04/04 14:16:07  dron
 * Use _vsnprintf() in Windows environment.
 */

#include "cpl_error.h"
#include "cpl_vsi.h"
#include "cpl_conv.h"
#include "cpl_multiproc.h"

#if defined(WIN32CE)
#  include "cpl_wince.h"
#  include <wce_stdlib.h>
#endif
 
#define TIMESTAMP_DEBUG

CPL_CVSID("$Id: cpl_error.cpp,v 1.36 2006/03/07 22:05:32 fwarmerdam Exp $");

static void *hErrorMutex = NULL;
static CPLErrorHandler pfnErrorHandler = CPLDefaultErrorHandler;

#if !defined(HAVE_VSNPRINTF)
#  define DEFAULT_LAST_ERR_MSG_SIZE 20000
#else
#  define DEFAULT_LAST_ERR_MSG_SIZE 500
#endif

typedef struct errHandler
{
    struct errHandler   *psNext;
    CPLErrorHandler     pfnHandler;
} CPLErrorHandlerNode;

typedef struct {
    int     nLastErrNo;
    CPLErr  eLastErrType;
    CPLErrorHandlerNode *psHandlerStack;
    int     nLastErrMsgMax;
    char    szLastErrMsg[DEFAULT_LAST_ERR_MSG_SIZE];
} CPLErrorContext;

/************************************************************************/
/*                         CPLGetErrorContext()                         */
/************************************************************************/

static CPLErrorContext *CPLGetErrorContext()

{
    CPLErrorContext *psCtx = 
        (CPLErrorContext *) CPLGetTLS( CTLS_ERRORCONTEXT );

    if( psCtx == NULL )
    {
        psCtx = (CPLErrorContext *) CPLCalloc(sizeof(CPLErrorContext),1);
        psCtx->eLastErrType = CE_None;
        psCtx->nLastErrMsgMax = sizeof(psCtx->szLastErrMsg);
        CPLSetTLS( CTLS_ERRORCONTEXT, psCtx, TRUE );
    }

    return psCtx;
}


/**********************************************************************
 *                          CPLError()
 **********************************************************************/

/**
 * Report an error.
 *
 * This function reports an error in a manner that can be hooked
 * and reported appropriate by different applications.
 *
 * The effect of this function can be altered by applications by installing
 * a custom error handling using CPLSetErrorHandler().
 *
 * The eErrClass argument can have the value CE_Warning indicating that the
 * message is an informational warning, CE_Failure indicating that the
 * action failed, but that normal recover mechanisms will be used or
 * CE_Fatal meaning that a fatal error has occured, and that CPLError()
 * should not return.  
 *
 * The default behaviour of CPLError() is to report errors to stderr,
 * and to abort() after reporting a CE_Fatal error.  It is expected that
 * some applications will want to supress error reporting, and will want to
 * install a C++ exception, or longjmp() approach to no local fatal error
 * recovery.
 *
 * Regardless of how application error handlers or the default error
 * handler choose to handle an error, the error number, and message will
 * be stored for recovery with CPLGetLastErrorNo() and CPLGetLastErrorMsg().
 *
 * @param eErrClass one of CE_Warning, CE_Failure or CE_Fatal.
 * @param err_no the error number (CPLE_*) from cpl_error.h.
 * @param fmt a printf() style format string.  Any additional arguments
 * will be treated as arguments to fill in this format in a manner
 * similar to printf().
 */

void    CPLError(CPLErr eErrClass, int err_no, const char *fmt, ...)
{
    va_list args;

    /* Expand the error message 
     */
    va_start(args, fmt);
    CPLErrorV( eErrClass, err_no, fmt, args );
    va_end(args);
}

/************************************************************************/
/*                             CPLErrorV()                              */
/************************************************************************/

void    CPLErrorV(CPLErr eErrClass, int err_no, const char *fmt, va_list args )
{
    CPLErrorContext *psCtx = CPLGetErrorContext();

/* -------------------------------------------------------------------- */
/*      Expand the error message                                        */
/* -------------------------------------------------------------------- */
#if defined(HAVE_VSNPRINTF)
    {
        int nPR;

        while( ((nPR = vsnprintf( psCtx->szLastErrMsg, 
                                 psCtx->nLastErrMsgMax, fmt, args )) == -1
                || nPR >= psCtx->nLastErrMsgMax-1)
               && psCtx->nLastErrMsgMax < 1000000 )
        {
            psCtx->nLastErrMsgMax *= 3;
            psCtx = (CPLErrorContext *) 
                CPLRealloc(psCtx, sizeof(CPLErrorContext) - DEFAULT_LAST_ERR_MSG_SIZE + psCtx->nLastErrMsgMax + 1);
            CPLSetTLS( CTLS_ERRORCONTEXT, psCtx, TRUE );
        }
    }
#else
    vsprintf( psCtx->szLastErrMsg, fmt, args);
#endif

/* -------------------------------------------------------------------- */
/*      If the user provided his own error handling function, then      */
/*      call it, otherwise print the error to stderr and return.        */
/* -------------------------------------------------------------------- */
    psCtx->nLastErrNo = err_no;
    psCtx->eLastErrType = eErrClass;

    if( CPLGetConfigOption("CPL_LOG_ERRORS",NULL) != NULL )
        CPLDebug( "CPLError", "%s", psCtx->szLastErrMsg );

/* -------------------------------------------------------------------- */
/*      Invoke the current error handler.                               */
/* -------------------------------------------------------------------- */
    if( psCtx->psHandlerStack != NULL )
    {
        psCtx->psHandlerStack->pfnHandler(eErrClass, err_no, 
                                          psCtx->szLastErrMsg);
    }
    else
    {
        CPLMutexHolderD( &hErrorMutex );
        if( pfnErrorHandler != NULL )
            pfnErrorHandler(eErrClass, err_no, psCtx->szLastErrMsg);
    }

    if( eErrClass == CE_Fatal )
        abort();
}

/************************************************************************/
/*                              CPLDebug()                              */
/************************************************************************/

/**
 * Display a debugging message.
 *
 * The category argument is used in conjunction with the CPL_DEBUG
 * environment variable to establish if the message should be displayed.
 * If the CPL_DEBUG environment variable is not set, no debug messages
 * are emitted (use CPLError(CE_Warning,...) to ensure messages are displayed).
 * If CPL_DEBUG is set, but is an empty string or the word "ON" then all
 * debug messages are shown.  Otherwise only messages whose category appears
 * somewhere within the CPL_DEBUG value are displayed (as determinted by
 * strstr()).
 *
 * Categories are usually an identifier for the subsystem producing the
 * error.  For instance "GDAL" might be used for the GDAL core, and "TIFF"
 * for messages from the TIFF translator.  
 *
 * @param pszCategory name of the debugging message category.
 * @param pszFormat printf() style format string for message to display.
 *        Remaining arguments are assumed to be for format.
 */ 

void CPLDebug( const char * pszCategory, const char * pszFormat, ... )

{
    CPLErrorContext *psCtx = CPLGetErrorContext();
    char        *pszMessage;
    va_list     args;
    const char  *pszDebug = CPLGetConfigOption("CPL_DEBUG",NULL);

#define ERROR_MAX 25000

/* -------------------------------------------------------------------- */
/*      Does this message pass our current criteria?                    */
/* -------------------------------------------------------------------- */
    if( pszDebug == NULL )
        return;

    if( !EQUAL(pszDebug,"ON") && !EQUAL(pszDebug,"") )
    {
        int            i, nLen = strlen(pszCategory);

        for( i = 0; pszDebug[i] != '\0'; i++ )
        {
            if( EQUALN(pszCategory,pszDebug+i,nLen) )
                break;
        }

        if( pszDebug[i] == '\0' )
            return;
    }

/* -------------------------------------------------------------------- */
/*    Allocate a block for the error.                                   */
/* -------------------------------------------------------------------- */
    pszMessage = (char *) VSIMalloc( ERROR_MAX );
    if( pszMessage == NULL )
        return;
        
/* -------------------------------------------------------------------- */
/*      Dal -- always log a timestamp as the first part of the line     */
/*      to ensure one is looking at what one should be looking at!      */
/* -------------------------------------------------------------------- */

    pszMessage[0] = '\0';
#ifdef TIMESTAMP_DEBUG
    if( CPLGetConfigOption( "CPL_TIMESTAMP", NULL ) != NULL )
    {
        strcpy( pszMessage, VSICTime( VSITime(NULL) ) );
        
        // On windows anyway, ctime puts a \n at the end, but I'm not 
        // convinced this is standard behaviour, so we'll get rid of it
        // carefully

        if (pszMessage[strlen(pszMessage) -1 ] == '\n')
        {
            pszMessage[strlen(pszMessage) - 1] = 0; // blow it out
        }
        strcat( pszMessage, ": " );
    }
#endif

/* -------------------------------------------------------------------- */
/*      Add the category.                                               */
/* -------------------------------------------------------------------- */
    strcat( pszMessage, pszCategory );
    strcat( pszMessage, ": " );
    
/* -------------------------------------------------------------------- */
/*      Format the application provided portion of the debug message.   */
/* -------------------------------------------------------------------- */
    va_start(args, pszFormat);
#if defined(HAVE_VSNPRINTF)
    vsnprintf(pszMessage+strlen(pszMessage), ERROR_MAX - strlen(pszMessage), 
              pszFormat, args);
#else
    vsprintf(pszMessage+strlen(pszMessage), pszFormat, args);
#endif
    va_end(args);

/* -------------------------------------------------------------------- */
/*      Invoke the current error handler.                               */
/* -------------------------------------------------------------------- */
    if( psCtx->psHandlerStack != NULL )
    {
        psCtx->psHandlerStack->pfnHandler( CE_Debug, CPLE_None, pszMessage );
    }
    else
    {
        CPLMutexHolderD( &hErrorMutex );
        if( pfnErrorHandler != NULL )
            pfnErrorHandler( CE_Debug, CPLE_None, pszMessage );
    }

    VSIFree( pszMessage );
}

/**********************************************************************
 *                          CPLErrorReset()
 **********************************************************************/

/**
 * Erase any traces of previous errors.
 *
 * This is normally used to ensure that an error which has been recovered
 * from does not appear to be still in play with high level functions.
 */

void CPL_STDCALL CPLErrorReset()
{
    CPLErrorContext *psCtx = CPLGetErrorContext();

    psCtx->nLastErrNo = CPLE_None;
    psCtx->szLastErrMsg[0] = '\0';
    psCtx->eLastErrType = CE_None;
}


/**********************************************************************
 *                          CPLGetLastErrorNo()
 **********************************************************************/

/**
 * Fetch the last error number.
 *
 * This is the error number, not the error class.
 *
 * @return the error number of the last error to occur, or CPLE_None (0)
 * if there are no posted errors.
 */

int CPL_STDCALL CPLGetLastErrorNo()
{
    CPLErrorContext *psCtx = CPLGetErrorContext();

    return psCtx->nLastErrNo;
}

/**********************************************************************
 *                          CPLGetLastErrorType()
 **********************************************************************/

/**
 * Fetch the last error type.
 *
 * This is the error class, not the error number.
 *
 * @return the error number of the last error to occur, or CE_None (0)
 * if there are no posted errors.
 */

CPLErr CPL_STDCALL CPLGetLastErrorType()
{
    CPLErrorContext *psCtx = CPLGetErrorContext();

    return psCtx->eLastErrType;
}

/**********************************************************************
 *                          CPLGetLastErrorMsg()
 **********************************************************************/

/**
 * Get the last error message.
 *
 * Fetches the last error message posted with CPLError(), that hasn't
 * been cleared by CPLErrorReset().  The returned pointer is to an internal
 * string that should not be altered or freed.
 *
 * @return the last error message, or NULL if there is no posted error
 * message.
 */

const char* CPL_STDCALL CPLGetLastErrorMsg()
{
    CPLErrorContext *psCtx = CPLGetErrorContext();

    return psCtx->szLastErrMsg;
}

/************************************************************************/
/*                       CPLDefaultErrorHandler()                       */
/************************************************************************/

void CPL_STDCALL CPLDefaultErrorHandler( CPLErr eErrClass, int nError, 
                             const char * pszErrorMsg )

{
    static int       bLogInit = FALSE;
    static FILE *    fpLog = stderr;

    if( !bLogInit )
    {
        bLogInit = TRUE;

        fpLog = stderr;
        if( CPLGetConfigOption( "CPL_LOG", NULL ) != NULL )
        {
            fpLog = fopen( CPLGetConfigOption("CPL_LOG",""), "wt" );
            if( fpLog == NULL )
                fpLog = stderr;
        }
    }

    if( eErrClass == CE_Debug )
        fprintf( fpLog, "%s\n", pszErrorMsg );
    else if( eErrClass == CE_Warning )
        fprintf( fpLog, "Warning %d: %s\n", nError, pszErrorMsg );
    else
        fprintf( fpLog, "ERROR %d: %s\n", nError, pszErrorMsg );

    fflush( fpLog );
}

/************************************************************************/
/*                        CPLQuietErrorHandler()                        */
/************************************************************************/

void CPL_STDCALL CPLQuietErrorHandler( CPLErr eErrClass , int nError, 
                           const char * pszErrorMsg )

{
    if( eErrClass == CE_Debug )
        CPLDefaultErrorHandler( eErrClass, nError, pszErrorMsg );
}

/************************************************************************/
/*                       CPLLoggingErrorHandler()                       */
/************************************************************************/

void CPL_STDCALL CPLLoggingErrorHandler( CPLErr eErrClass, int nError, 
                             const char * pszErrorMsg )

{
    static int       bLogInit = FALSE;
    static FILE *    fpLog = stderr;

    if( !bLogInit )
    {
        const char *cpl_log = NULL;

        CPLSetConfigOption( "CPL_TIMESTAMP", "ON" );

        bLogInit = TRUE;

        cpl_log = CPLGetConfigOption("CPL_LOG", NULL );

        fpLog = stderr;
        if( cpl_log != NULL && EQUAL(cpl_log,"OFF") )
        {
            fpLog = NULL;
        }
        else if( cpl_log != NULL )
        {
            char      path[5000];
            int       i = 0;

            strcpy( path, cpl_log );

            while( (fpLog = fopen( path, "rt" )) != NULL ) 
            {
                fclose( fpLog );

                /* generate sequenced log file names, inserting # before ext.*/
                if (strrchr(cpl_log, '.') == NULL)
                {
                    sprintf( path, "%s_%d%s", cpl_log, i++,
                             ".log" );
                }
                else
                {
                    int pos = 0;
                    char *cpl_log_base = strdup(cpl_log);
                    pos = strcspn(cpl_log_base, ".");
                    if (pos > 0)
                    {
                        cpl_log_base[pos] = '\0';
                    }
                    sprintf( path, "%s_%d%s", cpl_log_base,
                             i++, ".log" );
                }
            }

            fpLog = fopen( path, "wt" );
        }
    }

    if( fpLog == NULL )
        return;

    if( eErrClass == CE_Debug )
        fprintf( fpLog, "%s\n", pszErrorMsg );
    else if( eErrClass == CE_Warning )
        fprintf( fpLog, "Warning %d: %s\n", nError, pszErrorMsg );
    else
        fprintf( fpLog, "ERROR %d: %s\n", nError, pszErrorMsg );

    fflush( fpLog );
}

/**********************************************************************
 *                          CPLSetErrorHandler()
 **********************************************************************/

/**
 * Install custom error handler.
 *
 * Allow the library's user to specify his own error handler function.
 * A valid error handler is a C function with the following prototype:
 *
 * <pre>
 *     void MyErrorHandler(CPLErr eErrClass, int err_no, const char *msg)
 * </pre>
 *
 * Pass NULL to come back to the default behavior.  The default behaviour
 * (CPLDefaultErrorHandler()) is to write the message to stderr. 
 *
 * The msg will be a partially formatted error message not containing the
 * "ERROR %d:" portion emitted by the default handler.  Message formatting
 * is handled by CPLError() before calling the handler.  If the error
 * handler function is passed a CE_Fatal class error and returns, then
 * CPLError() will call abort(). Applications wanting to interrupt this
 * fatal behaviour will have to use longjmp(), or a C++ exception to
 * indirectly exit the function.
 *
 * Another standard error handler is CPLQuietErrorHandler() which doesn't
 * make any attempt to report the passed error or warning messages but
 * will process debug messages via CPLDefaultErrorHandler.
 *
 * Note that error handlers set with CPLSetErrorHandler() apply to all
 * threads in an application, while error handlers set with CPLPushErrorHandler
 * are thread-local.  However, any error handlers pushed with 
 * CPLPushErrorHandler (and not removed with CPLPopErrorHandler) take 
 * precidence over the global error handlers set with CPLSetErrorHandler(). 
 * Generally speaking CPLSetErrorHandler() would be used to set a desired
 * global error handler, while CPLPushErrorHandler() would be used to install
 * a temporary local error handler, such as CPLQuietErrorHandler() to suppress
 * error reporting in a limited segment of code. 
 *
 * @param pfnErrorHandlerNew new error handler function.
 * @return returns the previously installed error handler.
 */ 

CPLErrorHandler CPL_STDCALL 
CPLSetErrorHandler( CPLErrorHandler pfnErrorHandlerNew )
{
    CPLErrorHandler     pfnOldHandler = pfnErrorHandler;
    CPLErrorContext *psCtx = CPLGetErrorContext();

    if( psCtx->psHandlerStack != NULL )
    {
        CPLDebug( "CPL", 
                  "CPLSetErrorHandler() called with an error handler on\n"
                  "the local stack.  New error handler will not be used immediately.\n" );
    }


    {
        CPLMutexHolderD( &hErrorMutex );

        pfnOldHandler = pfnErrorHandler;
        
        if( pfnErrorHandler == NULL )
            pfnErrorHandler = CPLDefaultErrorHandler;
        else
            pfnErrorHandler = pfnErrorHandlerNew;
    }

    return pfnOldHandler;
}

/************************************************************************/
/*                        CPLPushErrorHandler()                         */
/************************************************************************/

/**
 * Push a new CPLError handler.
 *
 * This pushes a new error handler on the thread-local error handler
 * stack.  This handler will be used untill removed with CPLPopErrorHandler().
 *
 * The CPLSetErrorHandler() docs have further information on how 
 * CPLError handlers work.
 *
 * @param pfnErrorHandlerNew new error handler function.
 */

void CPL_STDCALL CPLPushErrorHandler( CPLErrorHandler pfnErrorHandlerNew )

{
    CPLErrorContext *psCtx = CPLGetErrorContext();
    CPLErrorHandlerNode         *psNode;

    psNode = (CPLErrorHandlerNode *) VSIMalloc(sizeof(CPLErrorHandlerNode));
    psNode->psNext = psCtx->psHandlerStack;
    psNode->pfnHandler = pfnErrorHandlerNew;

    psCtx->psHandlerStack = psNode;
}

/************************************************************************/
/*                         CPLPopErrorHandler()                         */
/************************************************************************/

/**
 * Pop error handler off stack.
 *
 * Discards the current error handler on the error handler stack, and restores 
 * the one in use before the last CPLPushErrorHandler() call.  This method
 * has no effect if there are no error handlers on the current threads error
 * handler stack. 
 */ 

void CPL_STDCALL CPLPopErrorHandler()

{
    CPLErrorContext *psCtx = CPLGetErrorContext();

    if( psCtx->psHandlerStack != NULL )
    {
        CPLErrorHandlerNode     *psNode = psCtx->psHandlerStack;

        psCtx->psHandlerStack = psNode->psNext;
        VSIFree( psNode );
    }
}

/************************************************************************/
/*                             _CPLAssert()                             */
/*                                                                      */
/*      This function is called only when an assertion fails.           */
/************************************************************************/

/**
 * Report failure of a logical assertion.
 *
 * Applications would normally use the CPLAssert() macro which expands
 * into code calling _CPLAssert() only if the condition fails.  _CPLAssert()
 * will generate a CE_Fatal error call to CPLError(), indicating the file
 * name, and line number of the failed assertion, as well as containing
 * the assertion itself.
 *
 * There is no reason for application code to call _CPLAssert() directly.
 */

void CPL_STDCALL _CPLAssert( const char * pszExpression, const char * pszFile,
                 int iLine )

{
    CPLError( CE_Fatal, CPLE_AssertionFailed,
              "Assertion `%s' failed\n"
              "in file `%s', line %d\n",
              pszExpression, pszFile, iLine );
}

