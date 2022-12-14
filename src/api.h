/*
 * Copyright (C) 2008-2016 by Erik Hofman.
 * Copyright (C) 2009-2016 by Adalin B.V.
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
# endif
#endif

#ifdef XML_NONEVALUE
# define __XML_FPNONE		XML_FPNONE
# define __XML_NONE		XML_NONE
#else
# define __XML_FPNONE		0.0
# define __XML_NONE		0
#endif

#ifdef XML_USE_NODECACHE
#include <xml_cache.h>
#else
void *cacheGet(void *);
void *cacheNodeGet(void *);
#endif

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

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
    const char *name;
    char *start;
    off_t len;
    int fd;
    const char *locale;
#ifdef XML_USE_NODECACHE
    void *node;
#endif
#ifndef XML_NONVALIDATING
    struct _zeroxml_error *info;
#endif
#ifdef WIN32
    SIMPLE_UNMMAP un;
#else
    int un;				/* referenced but not used */
#endif
};

struct _xml_id
{
    const char *name;
    char *start;
    off_t len;
    off_t name_len;
#ifndef XML_NONVALIDATING
    struct _root_id *root;
#endif
#ifdef XML_USE_NODECACHE
    void *node;
#endif
};

#endif /* __XML_API_H */

