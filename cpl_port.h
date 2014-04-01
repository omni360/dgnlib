/******************************************************************************
 * $Id: cpl_port.h,v 1.51 2006/03/21 20:11:54 fwarmerdam Exp $
 *
 * Project:  CPL - Common Portability Library
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 * Purpose:  Include file providing low level portability services for CPL.  
 *           This should be the first include file for any CPL based code.  
 *
 ******************************************************************************
 * Copyright (c) 1998, 2005, Frank Warmerdam <warmerdam@pobox.com>
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
 * $Log: cpl_port.h,v $
 * Revision 1.51  2006/03/21 20:11:54  fwarmerdam
 * fixup headers a bit
 *
 * Revision 1.50  2006/03/13 20:58:32  mloskot
 * Test if macros _CRT_SECURE_NO_DEPRECATE and  _CRT_NONSTDC_NO_DEPRECATE
 * are already defined, to get rid of warnings
 *
 * Revision 1.49  2006/02/20 01:03:01  fwarmerdam
 * Fixed last change.
 *
 * Revision 1.48  2006/02/20 00:59:58  fwarmerdam
 * Avoid deprecation of some common functions with VS8.
 * http://bugzilla.remotesensing.org/show_bug.cgi?id=1083
 *
 * Revision 1.47  2006/02/19 21:54:34  mloskot
 * [WINCE] Changes related to Windows CE port of CPL. Most changes are #ifdef wrappers.
 *
 * Revision 1.46  2005/12/08 20:21:10  fwarmerdam
 * added CPL_ODLL declaration
 *
 * Revision 1.45  2005/06/15 09:47:40  dron
 * Fixed typo.
 *
 * Revision 1.44  2005/06/15 09:11:58  dron
 * Added CPLIsEqual() macro.
 *
 * Revision 1.43  2005/05/23 03:57:08  fwarmerdam
 * added default definition of CPL_THREADLOCAL
 *
 * Revision 1.42  2005/04/04 15:22:36  fwarmerdam
 * added CPL_STDCALL declaration
 *
 * Revision 1.41  2005/03/17 04:20:24  fwarmerdam
 * added FORCE_CDECL
 *
 * Revision 1.40  2005/03/11 14:59:07  fwarmerdam
 * Default to assuming nothing is infinite if isinf() macro not defined.
 * Per http://bugzilla.remotesensing.org/show_bug.cgi?id=795
 *
 * Revision 1.39  2005/03/01 21:22:07  fwarmerdam
 * added CPLIsFinite()
 *
 * Revision 1.38  2005/03/01 20:44:38  fwarmerdam
 * Check for _MSC_VER instead of WIN32.
 *
 * Revision 1.37  2005/03/01 19:57:55  fwarmerdam
 * Added CPLIsNan and CPLIsInf macros.
 */

#ifndef CPL_BASE_H_INCLUDED
#define CPL_BASE_H_INCLUDED

/**
 * \file cpl_port.h
 *
 * Core portability definitions for CPL.
 *
 */

/* ==================================================================== */
/*      We will use macos_pre10 to indicate compilation with MacOS      */
/*      versions before MacOS X.                                        */
/* ==================================================================== */
#ifdef macintosh
#  define macos_pre10
#endif

/* ==================================================================== */
/*      We will use WIN32 as a standard windows define.                 */
/* ==================================================================== */
#if defined(_WIN32) && !defined(WIN32) && !defined(_WIN32_WCE)
#  define WIN32
#endif

#if defined(_WINDOWS) && !defined(WIN32) && !defined(_WIN32_WCE)
#  define WIN32
#endif

/* ==================================================================== */
/*      We will use WIN32CE as a standard Windows CE (Mobile) define.   */
/* ==================================================================== */
#if defined(_WIN32_WCE)
#  define WIN32CE
#endif


#include "cpl_config.h"

/* ==================================================================== */
/*      This will disable most WIN32 stuff in a Cygnus build which      */
/*      defines unix to 1.                                              */
/* ==================================================================== */

#ifdef unix
#  undef WIN32
#  undef WIN32CE
#endif

#if defined(VSI_NEED_LARGEFILE64_SOURCE) && !defined(_LARGEFILE64_SOURCE)
#  define _LARGEFILE64_SOURCE 1
#endif

/* ==================================================================== */
/*      Standard include files.                                         */
/* ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#if !defined(WIN32CE)
#  include <time.h>
#else
#  include <wce_time.h>
#  include <wce_errno.h>
#endif


#if defined(HAVE_ERRNO_H)
#  include <errno.h>
#endif 

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

#ifdef _AIX
#  include <strings.h>
#endif

#if defined(HAVE_LIBDBMALLOC) && defined(HAVE_DBMALLOC_H) && defined(DEBUG)
#  define DBMALLOC
#  include <dbmalloc.h>
#endif

#if !defined(DBMALLOC) && defined(HAVE_DMALLOC_H)
#  define USE_DMALLOC
#  include <dmalloc.h>
#endif

/* ==================================================================== */
/*      Base portability stuff ... this stuff may need to be            */
/*      modified for new platforms.                                     */
/* ==================================================================== */

/*---------------------------------------------------------------------
 *        types for 16 and 32 bits integers, etc...
 *--------------------------------------------------------------------*/
#if UINT_MAX == 65535
typedef long            GInt32;
typedef unsigned long   GUInt32;
#else
typedef int             GInt32;
typedef unsigned int    GUInt32;
#endif

typedef short           GInt16;
typedef unsigned short  GUInt16;
typedef unsigned char   GByte;
typedef int             GBool;

/* -------------------------------------------------------------------- */
/*      64bit support                                                   */
/* -------------------------------------------------------------------- */

#if defined(WIN32) && defined(_MSC_VER)

#define VSI_LARGE_API_SUPPORTED
typedef __int64          GIntBig;
typedef unsigned __int64 GUIntBig;

#elif HAVE_LONG_LONG

typedef long long        GIntBig;
typedef unsigned long long GUIntBig;

#else

typedef long             GIntBig;
typedef unsigned long    GUIntBig;

#endif

/* ==================================================================== */
/*      Other standard services.                                        */
/* ==================================================================== */
#ifdef __cplusplus
#  define CPL_C_START           extern "C" {
#  define CPL_C_END             }
#else
#  define CPL_C_START
#  define CPL_C_END
#endif

#ifndef CPL_DLL
#if defined(_MSC_VER) && !defined(CPL_DISABLE_DLL)
#  define CPL_DLL     __declspec(dllexport)
#else
#  define CPL_DLL
#endif
#endif

// Should optional (normally private) interfaces be exported?
#ifdef CPL_OPTIONAL_APIS
#  define CPL_ODLL CPL_DLL
#else
#  define CPL_ODLL
#endif

#ifndef CPL_STDCALL
#if defined(_MSC_VER) && !defined(CPL_DISABLE_STDCALL)
#  define CPL_STDCALL     __stdcall
#else
#  define CPL_STDCALL
#endif
#endif

#ifdef _MSC_VER
#  define FORCE_CDECL  __cdecl
#else
#  define FORCE_CDECL 
#endif

#ifndef NULL
#  define NULL  0
#endif

#ifndef FALSE
#  define FALSE 0
#endif

#ifndef TRUE
#  define TRUE  1
#endif

#ifndef MAX
#  define MIN(a,b)      ((a<b) ? a : b)
#  define MAX(a,b)      ((a>b) ? a : b)
#endif

#ifndef ABS
#  define ABS(x)        ((x<0) ? (-1*(x)) : x)
#endif

/* -------------------------------------------------------------------- */
/*      Macro to test equality of two floating point values.            */
/*      We use fabs() function instead of ABS() macro to avoid side     */
/*      effects.                                                        */
/* -------------------------------------------------------------------- */
#ifndef CPLIsEqual
#  define CPLIsEqual(x,y) (fabs(fabs(x) - fabs(y)) < 0.0000000000001 ? 1 : 0)
#endif

#ifndef EQUAL
#if defined(WIN32) || defined(WIN32CE)
#  define EQUALN(a,b,n)           (strnicmp(a,b,n)==0)
#  define EQUAL(a,b)              (stricmp(a,b)==0)
#else
#  define EQUALN(a,b,n)           (strncasecmp(a,b,n)==0)
#  define EQUAL(a,b)              (strcasecmp(a,b)==0)
#endif
#endif

#ifdef macos_pre10
int strcasecmp(char * str1, char * str2);
int strncasecmp(char * str1, char * str2, int len);
char * strdup (char *instr);
#endif

#ifndef CPL_THREADLOCAL 
#  define CPL_THREADLOCAL 
#endif

/* -------------------------------------------------------------------- */
/*      The following apparently allow you to use strcpy() and other    */
/*      functions judged "unsafe" by microsoft in VS 8 (2005).          */
/* -------------------------------------------------------------------- */
#ifdef _MSC_VER
#  ifndef _CRT_SECURE_NO_DEPRECATE
#    define _CRT_SECURE_NO_DEPRECATE
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#endif


/* -------------------------------------------------------------------- */
/*      Handle isnan() and isinf().  Note that isinf() and isnan()      */
/*      are supposed to be macros according to C99.  Some systems       */
/*      (ie. Tru64) don't have isinf() at all, so if the macro is       */
/*      not defined we just assume nothing is infinite.  This may       */
/*      mean we have no real CPLIsInf() on systems with an isinf()      */
/*      function but no corresponding macro, but I can live with        */
/*      that since it isn't that important a test.                      */
/* -------------------------------------------------------------------- */
#ifdef _MSC_VER
#  define CPLIsNan(x) _isnan(x)
#  define CPLIsInf(x) (!_isnan(x) && !_finite(x))
#  define CPLIsFinite(x) _finite(x)
#else
#  define CPLIsNan(x) isnan(x)
#  ifdef isinf 
#    define CPLIsInf(x) isinf(x)
#    define CPLIsFinite(x) (!isnan(x) && !isinf(x))
#  else
#    define CPLIsInf(x)    FALSE
#    define CPLIsFinite(x) (!isnan(x))
#  endif
#endif

/*---------------------------------------------------------------------
 *                         CPL_LSB and CPL_MSB
 * Only one of these 2 macros should be defined and specifies the byte 
 * ordering for the current platform.  
 * This should be defined in the Makefile, but if it is not then
 * the default is CPL_LSB (Intel ordering, LSB first).
 *--------------------------------------------------------------------*/
#if defined(WORDS_BIGENDIAN) && !defined(CPL_MSB) && !defined(CPL_LSB)
#  define CPL_MSB
#endif

#if ! ( defined(CPL_LSB) || defined(CPL_MSB) )
#define CPL_LSB
#endif

#if defined(CPL_LSB)
#  define CPL_IS_LSB 1
#else
#  define CPL_IS_LSB 0
#endif

/*---------------------------------------------------------------------
 *        Little endian <==> big endian byte swap macros.
 *--------------------------------------------------------------------*/

#define CPL_SWAP16(x) \
        ((GUInt16)( \
            (((GUInt16)(x) & 0x00ffU) << 8) | \
            (((GUInt16)(x) & 0xff00U) >> 8) ))

#define CPL_SWAP16PTR(x) \
{                                                                 \
    GByte       byTemp, *_pabyDataT = (GByte *) (x);              \
                                                                  \
    byTemp = _pabyDataT[0];                                       \
    _pabyDataT[0] = _pabyDataT[1];                                \
    _pabyDataT[1] = byTemp;                                       \
}                                                                    
                                                            
#define CPL_SWAP32(x) \
        ((GUInt32)( \
            (((GUInt32)(x) & (GUInt32)0x000000ffUL) << 24) | \
            (((GUInt32)(x) & (GUInt32)0x0000ff00UL) <<  8) | \
            (((GUInt32)(x) & (GUInt32)0x00ff0000UL) >>  8) | \
            (((GUInt32)(x) & (GUInt32)0xff000000UL) >> 24) ))

#define CPL_SWAP32PTR(x) \
{                                                                 \
    GByte       byTemp, *_pabyDataT = (GByte *) (x);              \
                                                                  \
    byTemp = _pabyDataT[0];                                       \
    _pabyDataT[0] = _pabyDataT[3];                                \
    _pabyDataT[3] = byTemp;                                       \
    byTemp = _pabyDataT[1];                                       \
    _pabyDataT[1] = _pabyDataT[2];                                \
    _pabyDataT[2] = byTemp;                                       \
}                                                                    
                                                            
#define CPL_SWAP64PTR(x) \
{                                                                 \
    GByte       byTemp, *_pabyDataT = (GByte *) (x);              \
                                                                  \
    byTemp = _pabyDataT[0];                                       \
    _pabyDataT[0] = _pabyDataT[7];                                \
    _pabyDataT[7] = byTemp;                                       \
    byTemp = _pabyDataT[1];                                       \
    _pabyDataT[1] = _pabyDataT[6];                                \
    _pabyDataT[6] = byTemp;                                       \
    byTemp = _pabyDataT[2];                                       \
    _pabyDataT[2] = _pabyDataT[5];                                \
    _pabyDataT[5] = byTemp;                                       \
    byTemp = _pabyDataT[3];                                       \
    _pabyDataT[3] = _pabyDataT[4];                                \
    _pabyDataT[4] = byTemp;                                       \
}                                                                    
                                                            

/* Until we have a safe 64 bits integer data type defined, we'll replace
m * this version of the CPL_SWAP64() macro with a less efficient one.
 */
/*
#define CPL_SWAP64(x) \
        ((uint64)( \
            (uint64)(((uint64)(x) & (uint64)0x00000000000000ffULL) << 56) | \
            (uint64)(((uint64)(x) & (uint64)0x000000000000ff00ULL) << 40) | \
            (uint64)(((uint64)(x) & (uint64)0x0000000000ff0000ULL) << 24) | \
            (uint64)(((uint64)(x) & (uint64)0x00000000ff000000ULL) << 8) | \
            (uint64)(((uint64)(x) & (uint64)0x000000ff00000000ULL) >> 8) | \
            (uint64)(((uint64)(x) & (uint64)0x0000ff0000000000ULL) >> 24) | \
            (uint64)(((uint64)(x) & (uint64)0x00ff000000000000ULL) >> 40) | \
            (uint64)(((uint64)(x) & (uint64)0xff00000000000000ULL) >> 56) ))
*/

#define CPL_SWAPDOUBLE(p) CPL_SWAP64PTR(p)

#ifdef CPL_MSB
#  define CPL_MSBWORD16(x)      (x)
#  define CPL_LSBWORD16(x)      CPL_SWAP16(x)
#  define CPL_MSBWORD32(x)      (x)
#  define CPL_LSBWORD32(x)      CPL_SWAP32(x)
#  define CPL_MSBPTR16(x)       
#  define CPL_LSBPTR16(x)       CPL_SWAP16PTR(x)
#  define CPL_MSBPTR32(x)       
#  define CPL_LSBPTR32(x)       CPL_SWAP32PTR(x)
#  define CPL_MSBPTR64(x)       
#  define CPL_LSBPTR64(x)       CPL_SWAP64PTR(x)
#else
#  define CPL_LSBWORD16(x)      (x)
#  define CPL_MSBWORD16(x)      CPL_SWAP16(x)
#  define CPL_LSBWORD32(x)      (x)
#  define CPL_MSBWORD32(x)      CPL_SWAP32(x)
#  define CPL_LSBPTR16(x)       
#  define CPL_MSBPTR16(x)       CPL_SWAP16PTR(x)
#  define CPL_LSBPTR32(x)       
#  define CPL_MSBPTR32(x)       CPL_SWAP32PTR(x)
#  define CPL_LSBPTR64(x)       
#  define CPL_MSBPTR64(x)       CPL_SWAP64PTR(x)
#endif

/***********************************************************************
 * Define CPL_CVSID() macro.  It can be disabled during a build by
 * defining DISABLE_CPLID in the compiler options.
 *
 * The cvsid_aw() function is just there to prevent reports of cpl_cvsid()
 * being unused.
 */

#ifndef DISABLE_CVSID
#  define CPL_CVSID(string)     static char cpl_cvsid[] = string; \
static char *cvsid_aw() { return( cvsid_aw() ? ((char *) NULL) : cpl_cvsid ); }
#else
#  define CPL_CVSID(string)
#endif

#endif /* ndef CPL_BASE_H_INCLUDED */
