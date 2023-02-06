/*
 * This software is available under 2 licenses -- choose whichever you prefer.
 *
 * ALTERNATIVE A - Modified BSD license
 *
 * Copyright (C) 2008-2023 by Erik Hofman.
 * Copyright (C) 2009-2023 by Adalin B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ADALIN B.V. ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL ADALIN B.V. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUTOF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Adalin B.V.
 *
 * -----------------------------------------------------------------------------
 * ALTERNATIVE B - Public Domain (www.unlicense.org)
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or distribute
 * this software, either in source code form or as a compiled binary, for any
 * purpose, commercial or non-commercial, and by any means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors of
 * this software dedicate any and all copyright interest in the software to
 * the public domain. We make this dedication for the benefit of the public at
 * large and to the detriment of our heirs and successors. We intend this
 * dedication to be an overt act of relinquishment in perpetuity of all
 * present and future rights to this software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __XML_API_H
#define __XML_API_H 1

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <xml.h>

#ifdef NDEBUG
# ifdef DEBUG
#  undef DEBUG
# endif
#endif

#if HAVE_ICONV_H
# include <iconv.h>
#endif
#ifdef USE_RMALLOC
# define USE_LOGGING    1
# include <stdio.h>
# include <rmalloc.h>
#else
# define USE_LOGGING    0
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# if HAVE_STRINGS_H
#  include <strings.h>
#  if defined(WIN32)
#   define strncasecmp _strnicmp
#  endif
# endif
#endif

#ifdef XML_NONEVALUE
# define __XML_BOOL_NONE	XML_BOOL_NONE
# define __XML_FPNONE		XML_FPNONE
# define __XML_NONE		XML_NONE
#else
# define __XML_BOOL_NONE	0
# define __XML_FPNONE		0.0
# define __XML_NONE		0
#endif

#ifndef XML_NONVALIDATING
# define FILENAME_LEN		1024
# define BUF_LEN		2048

# ifndef NDEBUG
#  define PRINT_INFO(a, b, c) \
    if (0 <= (c) && (c) < XML_MAX_ERROR) { \
        int i, last = 0, nl = 1; \
        for (i=0; i<(b)-(a)->root->start; ++i) { \
            if ((a)->root->start[i] == '\n') { last = i+1; nl++; } \
        } \
        snprintf(__zeroxml_strerror, BUF_LEN, "%s:\n\t%s at line %i offset %i\n", __zeroxml_filename, __zeroxml_error_str[(c)], nl, i-last); \
        fprintf(stderr, "%s\tdetected in %s at line %i\n", __zeroxml_strerror, __func__, __LINE__); \
    } else { \
        fprintf(stderr, "%s: in %s at line %i: Unknown error number %i\n", \
                        __zeroxml_filename, __func__, __LINE__, c); \
    }

#  define xmlErrorSet(a, b, c) do { \
     __zeroxml_set_error(a, b, c); PRINT_INFO(a, (char*)b, c); \
   } while(0)
# else /* NDEBUG */
#  define xmlErrorSet(a, b, c) __zeroxml_set_error(a, b, c);
#  define PRINT_INFO(a, b, c)
# endif
#else /* XML_NONVALIDATING */
# define PRINT_INFO(a, b, c)
# define xmlErrorSet(a, b, c)
#endif

#define PRINT(s, b, c) { \
  int l1 = (b), l2 = (c); \
  if (s) { \
    int q, len = l2; \
    if (l1 < l2) len = l1; \
    if (len < 50000) { \
        printf("(%i) '", len); \
        for (q=0; q<len; q++) printf("%c", (s)[q]); \
        printf("'\n"); \
    } else printf("Length (%i) seems too large at line %i\n",len, __LINE__); \
  } else printf("NULL pointer at line %i\n", __LINE__); \
}


#define MEMCMP(a,b,c)		memcmp((a),(b),(c))
#define MEMCHR(a,b,c)		memchr((a),(b),(c))
#define CASECMP(a,b)		(CASE(a)==CASE(b))
#ifdef XML_LOCALIZATION
/* Localization support */
int localized_tolower(int c);
int localized_char_cmp(const char s1, const char s2);
void *localized_memchr(const void *s, int c, size_t n);
int localized_memcmp(const void *s1, const void *s2, size_t n);
int localized_strncmp(const char *s1, const char *s2, size_t n);
int localized_strncasecmp(const char *s1, const char *s2, size_t n);
# ifndef XML_CASE_INSENSITIVE
#  define CASE(a)		(a)
#  define STRNCMP(a,b,c)	strncmp((a),(b),(c))
# else
#  define CASE(a)		localized_tolower(a)
#  define STRNCMP(a,b,c)	strncasecmp((a),(b),(c))
# endif
#else /* XML_LOCALIZATION */
# ifndef XML_CASE_INSENSITIVE
#  define CASE(a)		(a)
#  define STRNCMP(a,b,c)	strncmp((a),(b),(c))
# else
#  define CASE(a)		tolower(a)
#  define STRNCMP(a,b,c)	strncasecmp((a),(b),(c))
# endif
#endif

#define MAX_ENCODING	32
#define MMAP_FREE	-2
#define MMAP_ERROR	-1
#define STRIPPED	 0
#define RAW		 1

#include <xml_cache.h>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

# if !defined(__MINGW32__) && !defined(__MINGW64__)
typedef const char* iconv_t;
# endif

typedef struct
{
    HANDLE m;
    void *p;
} SIMPLE_UNMMAP;
#endif

#ifndef XML_NONVALIDATING
struct _zeroxml_error
{
    const char *pos;
    size_t err_no;
};
#endif

/*
 * It is required for both the rood node and the normal xml nodes to both
 * have 'char *name' defined as the first entry. The code tests whether
 * name == 0 to detect the root node.
 */
struct _root_id
{
    struct _root_id *root;
    const char *name;
    const char *start;
    off_t len;
#ifdef XML_USE_NODECACHE
    const cacheId *node;
#endif

    /* _root_id specifics */
    int fd;
    char *mmap;
    char encoding[MAX_ENCODING+1];
#if defined(HAVE_ICONV_H) || defined(WIN32)
    iconv_t cd;
#endif
#ifndef XML_NONVALIDATING
    struct _zeroxml_error *info;
#endif
#ifdef WIN32
    SIMPLE_UNMMAP un;
#endif
};

struct _xml_id
{
    struct _root_id *root;
    const char *name;
    const char *start;
    off_t len;
#ifdef XML_USE_NODECACHE
    const cacheId *node;
#endif

    /* _xml_id specifics */
    off_t name_len;
};

#endif /* __XML_API_H */
