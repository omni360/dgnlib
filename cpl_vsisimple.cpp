/******************************************************************************
 * $Id: cpl_vsisimple.cpp,v 1.21 2006/03/27 15:24:41 fwarmerdam Exp $
 *
 * Project:  Common Portability Library
 * Purpose:  Simple implementation of POSIX VSI functions.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1998, Frank Warmerdam
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log: cpl_vsisimple.cpp,v $
 * Revision 1.21  2006/03/27 15:24:41  fwarmerdam
 * buffer in FWrite is const
 *
 * Revision 1.20  2006/03/21 20:11:54  fwarmerdam
 * fixup headers a bit
 *
 * Revision 1.19  2006/02/19 21:54:34  mloskot
 * [WINCE] Changes related to Windows CE port of CPL. Most changes are #ifdef wrappers.
 *
 * Revision 1.18  2005/09/11 18:01:28  fwarmerdam
 * preliminary implementatin of fully virtualized large file api
 *
 * Revision 1.17  2003/09/10 19:44:36  warmerda
 * added VSIStrerrno()
 *
 * Revision 1.16  2003/09/08 08:11:40  dron
 * Added VSIGMTime() and VSILocalTime().
 *
 * Revision 1.15  2003/05/27 20:46:18  warmerda
 * added VSI IO debugging stuff
 *
 * Revision 1.14  2003/05/21 04:20:30  warmerda
 * avoid warnings
 *
 * Revision 1.13  2002/06/17 14:00:16  warmerda
 * segregate VSIStatL() and VSIStatBufL.
 *
 * Revision 1.12  2002/06/15 03:10:22  aubin
 * remove debug test for 64bit compile
 *
 * Revision 1.11  2002/06/15 00:07:23  aubin
 * mods to enable 64bit file i/o
 *
 * Revision 1.10  2001/07/18 04:00:49  warmerda
 * added CPL_CVSID
 *
 * Revision 1.9  2001/04/30 18:19:06  warmerda
 * avoid stat on macos_pre10
 *
 * Revision 1.8  2001/01/19 21:16:41  warmerda
 * expanded tabs
 *
 * Revision 1.7  2001/01/03 05:33:17  warmerda
 * added VSIFlush
 */

#include "cpl_config.h"
#include "cpl_port.h"
#include "cpl_vsi.h"
#include "cpl_error.h"

CPL_CVSID("$Id: cpl_vsisimple.cpp,v 1.21 2006/03/27 15:24:41 fwarmerdam Exp $");

/* for stat() */

/* Unix or Windows NT/2000/XP */
#if !defined(WIN32) && !defined(WIN32CE)
#  include <unistd.h>
#elif !defined(WIN32CE) /* not Win32 platform */
#  include <io.h>
#  include <fcntl.h>
#  include <direct.h>
#endif

/* Windows CE or other platforms */
#if defined(WIN32CE)
#  include <wce_io.h>
#  include <wce_stat.h>
#  include <wce_stdio.h>
#  include <wce_string.h>
#  include <wce_time.h>
# define time wce_time
#else
#  include <sys/stat.h>
#  include <time.h>
#endif

/************************************************************************/
/*                              VSIFOpen()                              */
/************************************************************************/

FILE *VSIFOpen( const char * pszFilename, const char * pszAccess )

{
    FILE * fp;

    fp = fopen( (char *) pszFilename, (char *) pszAccess );

    VSIDebug3( "VSIFOpen(%s,%s) = %p", pszFilename, pszAccess, fp );

    return( fp );
}

/************************************************************************/
/*                             VSIFClose()                              */
/************************************************************************/

int VSIFClose( FILE * fp )

{
    VSIDebug1( "VSIClose(%p)", fp );

    return( fclose(fp) );
}

/************************************************************************/
/*                              VSIFSeek()                              */
/************************************************************************/

int VSIFSeek( FILE * fp, long nOffset, int nWhence )

{
#ifdef VSI_DEBUG
    if( nWhence == SEEK_SET )
    {
        VSIDebug2( "VSIFSeek(%p,%d,SEEK_SET)", fp, nOffset );
    }
    else if( nWhence == SEEK_END )
    {
        VSIDebug2( "VSIFSeek(%p,%d,SEEK_END)", fp, nOffset );
    }
    else if( nWhence == SEEK_CUR )
    {
        VSIDebug2( "VSIFSeek(%p,%d,SEEK_CUR)", fp, nOffset );
    }
    else
    {
        VSIDebug3( "VSIFSeek(%p,%d,%d-Unknown)", fp, nOffset, nWhence );
    }
#endif 

    return( fseek( fp, nOffset, nWhence ) );
}

/************************************************************************/
/*                              VSIFTell()                              */
/************************************************************************/

long VSIFTell( FILE * fp )

{
    VSIDebug2( "VSIFTell(%p) = %ld", fp, ftell(fp) );

    return( ftell( fp ) );
}

/************************************************************************/
/*                             VSIRewind()                              */
/************************************************************************/

void VSIRewind( FILE * fp )

{
    VSIDebug1("VSIRewind(%p)", fp );
    rewind( fp );
}

/************************************************************************/
/*                              VSIFRead()                              */
/************************************************************************/

size_t VSIFRead( void * pBuffer, size_t nSize, size_t nCount, FILE * fp )

{
    size_t nResult = fread( pBuffer, nSize, nCount, fp );

    VSIDebug3( "VSIFRead(%p,%ld) = %ld", 
               fp, (long) nSize * nCount, (long) nResult * nSize );

    return nResult;
}

/************************************************************************/
/*                             VSIFWrite()                              */
/************************************************************************/

size_t VSIFWrite( const void *pBuffer, size_t nSize, size_t nCount, FILE * fp )

{
    size_t nResult = fwrite( pBuffer, nSize, nCount, fp );

    VSIDebug3( "VSIFWrite(%p,%ld) = %ld", 
               fp, (long) nSize * nCount, (long) nResult );

    return nResult;
}

/************************************************************************/
/*                             VSIFFlush()                              */
/************************************************************************/

void VSIFFlush( FILE * fp )

{
    VSIDebug1( "VSIFFlush(%p)", fp );
    fflush( fp );
}

/************************************************************************/
/*                              VSIFGets()                              */
/************************************************************************/

char *VSIFGets( char *pszBuffer, int nBufferSize, FILE * fp )

{
    return( fgets( pszBuffer, nBufferSize, fp ) );
}

/************************************************************************/
/*                              VSIFGetc()                              */
/************************************************************************/

int VSIFGetc( FILE * fp )

{
    return( fgetc( fp ) );
}

/************************************************************************/
/*                             VSIUngetc()                              */
/************************************************************************/

int VSIUngetc( int c, FILE * fp )

{
    return( ungetc( c, fp ) );
}

/************************************************************************/
/*                             VSIFPrintf()                             */
/*                                                                      */
/*      This is a little more complicated than just calling             */
/*      fprintf() because of the variable arguments.  Instead we        */
/*      have to use vfprintf().                                         */
/************************************************************************/

int     VSIFPrintf( FILE * fp, const char * pszFormat, ... )

{
    va_list     args;
    int         nReturn;

    va_start( args, pszFormat );
    nReturn = vfprintf( fp, pszFormat, args );
    va_end( args );

    return( nReturn );
}

/************************************************************************/
/*                              VSIFEof()                               */
/************************************************************************/

int VSIFEof( FILE * fp )

{
    return( feof( fp ) );
}

/************************************************************************/
/*                              VSIFPuts()                              */
/************************************************************************/

int VSIFPuts( const char * pszString, FILE * fp )

{
    return fputs( pszString, fp );
}

/************************************************************************/
/*                              VSIFPutc()                              */
/************************************************************************/

int VSIFPutc( int nChar, FILE * fp )

{
    return( fputc( nChar, fp ) );
}

/************************************************************************/
/*                             VSICalloc()                              */
/************************************************************************/

void *VSICalloc( size_t nCount, size_t nSize )

{
    return( calloc( nCount, nSize ) );
}

/************************************************************************/
/*                             VSIMalloc()                              */
/************************************************************************/

void *VSIMalloc( size_t nSize )

{
    return( malloc( nSize ) );
}

/************************************************************************/
/*                             VSIRealloc()                             */
/************************************************************************/

void * VSIRealloc( void * pData, size_t nNewSize )

{
    return( realloc( pData, nNewSize ) );
}

/************************************************************************/
/*                              VSIFree()                               */
/************************************************************************/

void VSIFree( void * pData )

{
    if( pData != NULL )
        free( pData );
}

/************************************************************************/
/*                             VSIStrdup()                              */
/************************************************************************/

char *VSIStrdup( const char * pszString )

{
    return( strdup( pszString ) );
}

/************************************************************************/
/*                              VSIStat()                               */
/************************************************************************/

int VSIStat( const char * pszFilename, VSIStatBuf * pStatBuf )

{
#if defined(macos_pre10)
    return -1;
#else
    return( stat( pszFilename, pStatBuf ) );
#endif
}

/************************************************************************/
/*                              VSITime()                               */
/************************************************************************/

unsigned long VSITime( unsigned long * pnTimeToSet )

{
    time_t tTime;
        
    tTime = time( NULL );

    if( pnTimeToSet != NULL )
        *pnTimeToSet = (unsigned long) tTime;

    return (unsigned long) tTime;
}

/************************************************************************/
/*                              VSICTime()                              */
/************************************************************************/

const char *VSICTime( unsigned long nTime )

{
    time_t tTime = (time_t) nTime;

    return (const char *) ctime( &tTime );
}

/************************************************************************/
/*                             VSIGMTime()                              */
/************************************************************************/

struct tm *VSIGMTime( const time_t *pnTime, struct tm *poBrokenTime )
{

#if HAVE_GMTIME_R
    gmtime_r( pnTime, poBrokenTime );
#else
    struct tm   *poTime;
    poTime = gmtime( pnTime );
    memcpy( poBrokenTime, poTime, sizeof(tm) );
#endif

    return poBrokenTime;
}

/************************************************************************/
/*                             VSILocalTime()                           */
/************************************************************************/

struct tm *VSILocalTime( const time_t *pnTime, struct tm *poBrokenTime )
{

#if HAVE_LOCALTIME_R
    localtime_r( pnTime, poBrokenTime );
#else
    struct tm   *poTime;
    poTime = localtime( pnTime );
    memcpy( poBrokenTime, poTime, sizeof(tm) );
#endif

    return poBrokenTime;
}

/************************************************************************/
/*                            VSIStrerror()                             */
/************************************************************************/

char *VSIStrerror( int nErrno )

{
    return strerror( nErrno );
}
