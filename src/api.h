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

#include <xml_cache.h>

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
#ifndef XML_NONVALIDATING
    struct _root_id *root;
#endif
    const char *name;
    const char *start;
    off_t len;
#ifdef XML_USE_NODECACHE
    const cacheId *node;
#endif

    /* _root_id specifics */
    int fd;
    char *mmap;
#ifdef HAVE_LOCALE_H
    const char *locale;
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
#ifndef XML_NONVALIDATING
    struct _root_id *root;
#endif
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

