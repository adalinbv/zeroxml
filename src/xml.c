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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WIN32
# include <io.h>
#else
# include <sys/mman.h>
#endif

#include <stdio.h>
#if HAVE_STRINGS_H
# include <strings.h>	/* strncasecmp */
# if defined(WIN32)
#  define strncasecmp _strnicmp
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif
#include <ctype.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#include <xml.h>

#include <types.h>
#include "api.h"

#ifndef XML_NONVALIDATING
# define FILENAME_LEN		1024
# define BUF_LEN		2048

#ifndef XML_NONVALIDATING
static struct _zeroxml_error __zeroxml_info = { NULL, 0 };
static const char *__zeroxml_error_str[XML_MAX_ERROR];
static void __xmlErrorSet(const struct _xml_id*, const char *, int);
#endif

# ifndef NDEBUG
static char __zeroxml_strerror[BUF_LEN+1];
static char _xml_filename[FILENAME_LEN+1];
#  define PRINT_INFO(a, b, c) \
    if (0 <= (c) && (c) < XML_MAX_ERROR) { \
        int i, last = 0, nl = 1; \
        for (i=0; i<(b)-(a)->root->start; ++i) { \
            if ((a)->root->start[i] == '\n') { last = i+1; nl++; } \
        } \
        snprintf(__zeroxml_strerror, BUF_LEN, "%s:\n\t%s at line %i offset %i\n", _xml_filename, __zeroxml_error_str[(c)], nl, i-last); \
        fprintf(stderr, "%s\tdetected in %s at line %i\n", __zeroxml_strerror, __func__, __LINE__); \
    } else { \
        fprintf(stderr, "%s: in %s at line %i: Unknown error number!\n", \
                        _xml_filename, __func__, __LINE__); \
    }

# define xmlErrorSet(a, b, c) do { \
  __xmlErrorSet(a, b, c); PRINT_INFO(a, (char*)b, c); \
} while(0)
# else
# define xmlErrorSet(a, b, c) __xmlErrorSet(a, b, c);
#  define PRINT_INFO(a, b, c)
# endif
#else /* !XML_NONVALIDATING */
# define PRINT_INFO(a, b, c)
# define xmlErrorSet(a, b, c)
#endif

#define SET_ERROR_AND_RETURN(a, b) { \
        printf("# line: %i\n", __LINE__); \
       *name = (a); *len = (b); *rlen = 0; return NULL; \
 }

static long __xml_strtol(const char*, char**, int);
static int __xml_strtob(const char*, const char*);
static void __xmlPrepareData( const char**, int*, char);
static char *__xmlGetString(const xmlId*, char);
static int __xmlNodeGetNum(const xmlId*, const char*, char);
static const char *__xmlDeclarationProcess(const char*, int);
static  const char *__xmlNodeGetPath(const cacheId**, const char*, int*,  const char**, int*);
static  const char *__xmlNodeGet(const cacheId*, const char**, int*,  const char**, int*, int*, char);
static xmlId *__xmlNodeGetPos(const xmlId*, xmlId*, const char*, int, char);
static  const char *__xmlAttributeGetDataPtr(const struct _xml_id*, const char *, int*);
#ifdef WIN32
/*
 * map 'filename' and return a pointer to it.
 */
static void *simple_mmap(int, int, SIMPLE_UNMMAP *);
static void simple_unmmap(void*, int, SIMPLE_UNMMAP *);

#else
# define simple_mmap(a, b, c)	mmap(0, (b), PROT_READ, MAP_PRIVATE, (a), 0L)
# define simple_unmmap(a, b, c)	munmap((a), (b))

#endif
static const char *comment = XML_COMMENT;

#define MMAP_FREE	-2
#define	MMAP_ERROR	-1
#define STRIPPED	0
#define RAW		1


XML_API xmlId* XML_APIENTRY
xmlOpen(const char *filename)
{
    struct _root_id *rid = 0;

# ifndef NDEBUG
    snprintf(_xml_filename, FILENAME_LEN, "%s", filename);
#endif

    if (filename)
    {
        int fd = open(filename, O_RDONLY);
        if (fd >= 0)
        {
            rid = calloc(1, sizeof(struct _root_id));
            if (rid)
            {
                struct stat statbuf;
                char *mm;

                /* UTF-8 unicode support */
#ifdef HAVE_LOCALE_H
//              rid->locale = setlocale(LC_CTYPE, "");
#endif

                fstat(fd, &statbuf);
                mm = simple_mmap(fd, (int)statbuf.st_size, &rid->un);
                if (mm != (void *)MMAP_ERROR)
                {
                    int blocklen = statbuf.st_size;
                    const char *start;

                    start = __xmlDeclarationProcess(mm, blocklen);
                    blocklen -= start-mm;

                    __xmlPrepareData(&start, &blocklen, RAW);

#ifdef XML_USE_NODECACHE
                    do
                    {
                        const char *n = "*";
                        int num = -1, nlen = 1;
                        const char *ret, *new = start;
                        int len = blocklen;

                        rid->node = cacheInit();
                        ret = __xmlNodeGet(rid->node, &new, &len, &n, &nlen,
                                           &num, RAW);
                        if (!ret)
                        {
                            PRINT_INFO(rid, n, len);
                            simple_unmmap(mm, len, &rid->un);
                            close(fd);

                            cacheFree(rid->node);
                            free(rid);
                            rid = 0;
                        }
                    } while(0);
#endif
                    if (rid)
                    {
                        rid->fd = fd;
                        rid->mmap = mm;
                        rid->start = start;
                        rid->len = blocklen;
#ifndef XML_NONVALIDATING
                        rid->root = rid;
#endif
                    }
                }
            }
        }
    }

    return (void *)rid;
}

XML_API xmlId* XML_APIENTRY
xmlInitBuffer(const char *buffer, int blocklen)
{
    struct _root_id *rid = 0;

# ifndef NDEBUG
    snprintf(_xml_filename, FILENAME_LEN, "XML buffer");
#endif

    if (buffer && (blocklen > 0))
    {
        rid = calloc(1, sizeof(struct _root_id));
        if (rid)
        {
            const char *start;

            start = __xmlDeclarationProcess(buffer, blocklen);
            blocklen -= start-buffer;

            __xmlPrepareData(&start, &blocklen, RAW);

#ifdef XML_USE_NODECACHE
            do
            {
                const char *n = "*";
                int num = -1, nlen = 1;
                const char *ret, *new = start;
                int len = blocklen;

                rid->node = cacheInit();
                ret = __xmlNodeGet(rid->node, &new, &len, &n, &nlen, &num,
                                   RAW);
                if (!ret)
                {
                    PRINT_INFO(rid, n, len);
                    cacheFree(rid->node);
                    free(rid);
                    rid = 0;
                }
            } while(0);
#endif
            if (rid)
            {
                rid->fd = MMAP_ERROR;
                rid->mmap = (char*)buffer;
                rid->start = start;
                rid->len = blocklen;
#ifndef XML_NONVALIDATING
                rid->root = rid;
#endif

#ifdef HAVE_LOCALE_H
//              rid->locale = setlocale(LC_CTYPE, "");
#endif
            }
        }
    }

    return (void *)rid;
}

XML_API void XML_APIENTRY
xmlClose(xmlId *id)
{
    struct _root_id *rid = (struct _root_id *)id;


    if (rid && !rid->name
#ifndef XML_NONVALIDATING
         && rid->root == rid
#endif
       )
    {
        if (rid->fd == MMAP_FREE) {
           free(rid->mmap);
        }
        else if (rid->fd != MMAP_ERROR)
        {
            simple_unmmap(rid->mmap, (int)rid->len, &rid->un);
            close(rid->fd);
        }

#ifdef XML_USE_NODECACHE
        if (rid->node) cacheFree(rid->node);
#endif
#ifndef XML_NONVALIDATING
        if (rid->info) free(rid->info);
#endif

#ifdef HAVE_LOCALE_H
//      if (rid->locale) {
//         setlocale(LC_CTYPE, rid->locale);
//      }
#endif
        free(rid);
        id = 0;
    }
}

XML_API int XML_APIENTRY
xmlNodeTest(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    const cacheId *nc, *nnc;
    const char *node;
    int len, slen;
    int rv;

    assert(id != 0);
    assert(path != 0);

    node = ( const char *)path;
    len = xid->len;
    slen = strlen(path);

    nnc = nc = cacheNodeGet(id);

    if (!strcmp(path, XML_COMMENT)) {
        rv = (xid->name == comment) ? XML_TRUE : XML_FALSE;
    } else {
        if (__xmlNodeGetPath(&nnc, xid->start, &len, &node, &slen)) {
            rv  = XML_TRUE;
        } else {
            rv = XML_FALSE;
        }
    }

    return rv;
}

XML_API xmlId* XML_APIENTRY
xmlNodeGet(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    struct _xml_id *xsid = NULL;
    const  cacheId *nc, *nnc;
    const char *ptr, *node;
    int len, slen;

    assert(id != 0);
    assert(path != 0);

    node = ( const char *)path;
    len = xid->len;
    slen = strlen(path);

    nnc = nc = cacheNodeGet(id);
    ptr = __xmlNodeGetPath(&nnc, xid->start, &len, &node, &slen);
    if (ptr)
    {
        xsid = malloc(sizeof(struct _xml_id));
        if (xsid)
        {
            xsid->name = node;
            xsid->name_len = slen;
            xsid->start = (char*)ptr;
            xsid->len = len;
#ifdef XML_USE_NODECACHE
            xsid->node = nnc;
#endif
#ifndef XML_NONVALIDATING
            if (xid->name)
                xsid->root = xid->root;
            else
                xsid->root = (struct _root_id *)xid;
#endif
        }
        else {
            xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
        }
    }
    else if (slen == 0) {
        xmlErrorSet(xid, node, len);
    }

    return (void *)xsid;
}

XML_API xmlId* XML_APIENTRY
xmlNodeCopy(const xmlId *id, const char *path)
{
    struct _root_id *rv = NULL;
    xmlId *xid;

    if ((xid = xmlNodeGet(id, path)) != NULL)
    {
        char *ptr;
        if ((ptr = xmlGetString(xid)) != NULL)
        {
            rv = xmlInitBuffer(ptr, strlen(ptr));
            rv->fd = MMAP_FREE; /* let xmlClose free ptr */
        }
        xmlFree(xid);
    }

    return (xmlId*)rv;
}

XML_API char* XML_APIENTRY
xmlNodeGetName(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int len;
    char *rv;

    assert(xid != 0);

    len = xid->name_len;
    rv = malloc(len+1);
    if (rv)
    {
        memcpy(rv, xid->name, len);
        rv[len] = 0;
    }
    else {
        xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlNodeCopyName(const xmlId *id, char *buf, int buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int slen = 0;

    assert(buf != 0);
    assert(buflen > 0);

    slen = xid->name_len;
    if (slen >= buflen)
    {
        slen = buflen-1;
        xmlErrorSet(xid, 0, XML_TRUNCATE_RESULT);
    }
    memcpy(buf, xid->name, slen);
    buf[slen] = 0;

    return slen;
}

XML_API int XML_APIENTRY
xmlNodeCompareName(const xmlId *id, const char *str)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int slen = str ? strlen(str) : 0;
    int nlen, rv = XML_TRUE;

    nlen = xid->name_len;
    if (nlen >= slen) {
        rv = strncasecmp(xid->name, str, slen);
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlAttributeCopyName(const xmlId *id, char *buf, int buflen, int pos)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int slen = 0;

    assert(buf != 0);
    assert(buflen > 0);

    if (xid->name_len && xid->name != comment)
    {
        const char *ps, *pe, *new;
        int num = 0;

        assert(xid->start > xid->name);

        ps = xid->name + xid->name_len + 1;
        pe = xid->start - 1;
        while (ps<pe)
        {
            while ((ps<pe) && isspace(*ps)) ps++;

            new = memchr(ps, '=', pe-ps);
            if (!new) break;

            if (num++ == pos)
            {
                slen = new-ps;
                if (slen >= buflen)
                {
                    slen = buflen-1;
                    xmlErrorSet(xid, 0, XML_TRUNCATE_RESULT);
                }
                memcpy(buf, ps, slen);
                buf[slen] = 0;
                break;
            }

            ps = new+2;
            while ((ps<pe) && (*ps != '"' && *ps != '\'')) ps++;
            ps++;
        }
    }

    return slen;
}

XML_API char* XML_APIENTRY
xmlAttributeGetName(const xmlId *id, int pos)
{
   struct _xml_id *xid = (struct _xml_id *)id;
   char buf[4096];
   char *rv;
   int len;

    assert(xid != 0);

    len = xmlAttributeCopyName(id, buf, 4096, pos);
    rv = malloc(len+1);
    if (rv)
    {
        memcpy(rv, buf, len);
        rv[len] = 0;
    }
    else {
        xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlAttributeCompareName(const xmlId *id, int pos, const char *str)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = XML_TRUE;

    assert(str != 0);

    if (xid->name_len && xid->name != comment)
    {
        const char *ps, *pe, *new;
        int num = 0;

        assert(xid->start > xid->name);

        ps = xid->name + xid->name_len + 1;
        pe = xid->start - 1;
        while (ps<pe)
        {
            while ((ps<pe) && isspace(*ps)) ps++;

            new = memchr(ps, '=', pe-ps);
            if (!new) break;

            if (num++ == pos)
            {
                int slen = new-ps;
                rv = strncasecmp(ps, str, slen);
                break;
            }

            ps = new+2;
            while ((ps<pe) && (*ps != '"' && *ps != '\'')) ps++;
            ps++;
        }
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlNodeGetNum(const xmlId *id, const char *path)
{
   return __xmlNodeGetNum(id, path, STRIPPED);
}

XML_API int XML_APIENTRY
xmlNodeGetNumRaw(const xmlId *id, const char *path)
{
   return __xmlNodeGetNum(id, path, RAW);
}

XML_API int XML_APIENTRY
xmlAttributeGetNum(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int num = 0;

    if (xid->name_len && xid->name != comment)
    {
        const char *ps, *pe, *new;

        assert(xid->start > xid->name);

        ps = xid->name + xid->name_len + 1;
        pe = xid->start - 1;
        while (ps<pe)
        {
            new = memchr(ps, '=', pe-ps);
            if (!new) break;

            ps = new+1;
            num++;
        }
    }
    return num;
}

XML_API xmlId* XML_APIENTRY
xmlNodeGetPos(const xmlId *pid, xmlId *id, const char *element, int num)
{
   return __xmlNodeGetPos(pid, id, element, num, STRIPPED);
}

XML_API xmlId* XML_APIENTRY
xmlNodeGetPosRaw(const xmlId *pid, xmlId *id, const char *element, int num)
{
   return __xmlNodeGetPos(pid, id, element, num, RAW);
}


XML_API xmlId* XML_APIENTRY
xmlNodeCopyPos(const xmlId *pid, xmlId *id, const char *element, int num)
{
    struct _root_id *rv = NULL;
    xmlId *xid, *nid;
    char *ptr;

    xid = xmlMarkId(id);
    if ((nid = __xmlNodeGetPos(id, xid, element, num, RAW)) != NULL)
    {
        if ((ptr = xmlGetString(nid)) != NULL)
        {
            rv = xmlInitBuffer(ptr, strlen(ptr));
            rv->fd = MMAP_FREE; /* let xmlClose free ptr */
        }
    }
    xmlFree(xid);

    return (xmlId*)rv;
}

XML_API char* XML_APIENTRY
xmlGetString(const xmlId *id)
{
   return __xmlGetString(id, STRIPPED);
}

XML_API char* XML_APIENTRY
xmlGetStringRaw(const xmlId *id)
{
   return __xmlGetString(id, RAW);
}

XML_API int XML_APIENTRY
xmlCopyString(const xmlId *id, char *buffer, int buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = 0;

    assert(xid != 0);
    assert(buffer != 0);
    assert(buflen > 0);

    *buffer = '\0';
    if (xid->len)
    {
        int len;
        const char *ps;

        ps = xid->start;
        len = xid->len;
        __xmlPrepareData(&ps, &len, STRIPPED);
        if (len)
        {
            if (len >= buflen)
            {
                len = buflen-1;
                xmlErrorSet(xid, 0, XML_TRUNCATE_RESULT);
            }
            memcpy(buffer, ps, len);
            buffer[len] = 0;
        }
        rv = len;
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlCompareString(const xmlId *id, const char *s)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = XML_TRUE;

    assert(xid != 0);
    assert(s != 0);

    if (xid->len && (strlen(s) > 0))
    {
        int len;
        const char *ps;

        ps = xid->start;
        len = xid->len;
        __xmlPrepareData(&ps, &len, STRIPPED);
        rv = strncasecmp(ps, s, len) ? XML_TRUE : XML_FALSE;
    }

    return rv;
}

XML_API char* XML_APIENTRY
xmlNodeGetString(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *rv = NULL;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        const char *node, *str;
        const cacheId *nc;
        int len, slen;

        len = xid->len;
        slen = strlen(path);
        node = (const char *)path;
        nc = cacheNodeGet(id);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str && len)
        {
            const char *ps = str;
            __xmlPrepareData(&ps, &len, STRIPPED);
            rv = calloc(1, len+1);
            if (rv)
            {
                memcpy(rv, ps, len);
                rv[len] = 0;
            }
            else {
                xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
            }
        }
        else if (slen == 0) {
            xmlErrorSet(xid, node, len);
        }
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlNodeCopyString(const xmlId *id, const char *path, char *buffer, int buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = 0;

    assert(xid != 0);
    assert(path != 0);
    assert(buffer != 0);
    assert(buflen > 0);

    *buffer = '\0';
    if (xid->len)
    {
        const char *p, *node = (const char *)path;
        int slen = strlen(node);
        int len = xid->len;
        const cacheId *nc;

        nc = cacheNodeGet(id);
        p = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (p)
        {
            __xmlPrepareData(&p, &len, STRIPPED);
            if (len)
            {
                if (len >= buflen)
                {
                    len = buflen-1;
                    xmlErrorSet(xid, 0, XML_TRUNCATE_RESULT);
                }

                memcpy(buffer, p, len);
                buffer[len] = 0;
            }
            rv = len;
        }
        else if (slen == 0) {
            xmlErrorSet(xid, node, len);
        }
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlNodeCompareString(const xmlId *id, const char *path, const char *s)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = -1;

    assert(xid != 0);
    assert(path != 0);
    assert(s != 0);

    if (xid->len)
    {
        const char *node, *str;
        const cacheId *nc;
        int len, slen;

        len = xid->len;
        slen = strlen(path);
        node = (const char *)path;
        nc = cacheNodeGet(id);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str && len)
        {
            const char *ps = str;
            __xmlPrepareData(&ps, &len, STRIPPED);
            rv = strncasecmp(ps, s, len);
        }
        else if (slen == 0) {
            xmlErrorSet(xid, node, len);
        }
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlGetBool(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = __XML_BOOL_NONE;

    assert(xid != 0);

    if (xid->len)
    {
        const char *end = xid->start + xid->len;
        rv = __xml_strtob(xid->start, end);
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlNodeGetBool(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = __XML_BOOL_NONE;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        const char *str, *node;
        const cacheId *nc;
        int len, slen;

        len = xid->len;
        slen = strlen(path);
        node = (const char *)path;
        nc = cacheNodeGet(id);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str)
        {
            const char *end = str+len;
            rv = __xml_strtob(str, end);
        }
        else if (slen == 0) {
            xmlErrorSet(xid, node, len);
        }
    }

    return rv;
}

XML_API long int XML_APIENTRY
xmlGetInt(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    long int rv = __XML_NONE;

    assert(xid != 0);

    if (xid->len)
    {
        char *end = (char*)xid->start + xid->len;
        rv = __xml_strtol(xid->start, &end, 10);
    }

    return rv;
}

XML_API long int XML_APIENTRY
xmlNodeGetInt(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    long int rv = __XML_NONE;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        const char *str, *node;
        const cacheId *nc;
        int len, slen;

        len = xid->len;
        slen = strlen(path);
        node = (const char *)path;
        nc = cacheNodeGet(id);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str)
        {
            char *end = (char*)str+len;
            rv = __xml_strtol(str, &end, 10);
        }
        else if (slen == 0) {
            xmlErrorSet(xid, node, len);
        }
    }

    return rv;
}

XML_API double XML_APIENTRY
xmlGetDouble(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    double rv = __XML_FPNONE;

    assert(xid != 0);

    if (xid->len)
    {
        char *end = (char*)xid->start + xid->len;
        rv = strtod(xid->start, &end);
    }

    return rv;
}

XML_API double XML_APIENTRY
xmlNodeGetDouble(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    double rv = __XML_FPNONE;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        const char *str, *node;
        const cacheId *nc;
        int len, slen;

        len = xid->len;
        slen = strlen(path);
        node = (const char *)path;
        nc = cacheNodeGet(id);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str)
        {
            char *end = (char*)str+len;
            rv = strtod(str, &end);
        }
        else if (slen == 0) {
            xmlErrorSet(xid, node, len);
        }
    }

    return rv;
}

XML_API xmlId* XML_APIENTRY
xmlMarkId(const xmlId *id)
{
    struct _xml_id *xmid = NULL;

    assert(id != 0);

    xmid = malloc(sizeof(struct _xml_id));
    if (xmid)
    {
        struct _root_id *xrid = (struct _root_id *)id;
#ifndef XML_NONVALIDATING
        if (xrid->root == xrid)
#else
        if (xrid->name == NULL)
#endif
        {
            xmid->name = "";
            xmid->name_len = 0;
            xmid->start = xrid->start;
            xmid->len = xrid->len;
#ifdef XML_USE_NODECACHE
            xmid->node = xrid->node;
#endif
#ifndef XML_NONVALIDATING
            xmid->root = xrid;
#endif
        }
        else {
            memcpy(xmid, id, sizeof(struct _xml_id));
        }
    }
    else {
        xmlErrorSet((struct _xml_id*)id, 0, XML_OUT_OF_MEMORY);
    }

    return (void *)xmid;
}

XML_API void XML_APIENTRY
xmlFree(void *id)
{
    if (id) free(id);
}

XML_API int XML_APIENTRY
xmlAttributeExists(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = XML_FALSE;

    if (xid->name_len && xid->name != comment)
    {
        int len;
        rv = __xmlAttributeGetDataPtr(xid, name, &len) ? XML_TRUE : XML_FALSE;
    }
    return rv;
}

XML_API double XML_APIENTRY
xmlAttributeGetDouble(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    double rv = __XML_FPNONE;

    if (xid->name_len && xid->name != comment)
    {
        const char *ptr;
        int len;

        ptr = __xmlAttributeGetDataPtr(xid, name, &len);
        if (ptr)
        {
            char *eptr = (char*)ptr+len;
            rv = strtod(ptr, &eptr);
        }
    }
    return rv;
}

XML_API int XML_APIENTRY
xmlAttributeGetBool(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = __XML_BOOL_NONE;

    if (xid->name_len && xid->name != comment)
    {
        const char *ptr;
        int len;

        ptr = __xmlAttributeGetDataPtr(xid, name, &len);
        if (ptr)
        {
            const char *eptr = ptr+len;
            rv = __xml_strtob(ptr, eptr);
        }
    }
    return rv;
}

XML_API long int XML_APIENTRY
xmlAttributeGetInt(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    long int rv = __XML_NONE;

    if (xid->name_len && xid->name != comment)
    {
        const char *ptr;
        int len;

        ptr = __xmlAttributeGetDataPtr(xid, name, &len);
        if (ptr)
        {
            char *eptr = (char*)ptr+len;
            rv = __xml_strtol(ptr, &eptr, 10);
        }
    }
    return rv;
}

XML_API char * XML_APIENTRY
xmlAttributeGetString(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *rv = NULL;

    if (xid->name_len && xid->name != comment)
    {
        int len;
        const char *ptr;

        ptr = __xmlAttributeGetDataPtr(xid, name, &len);
        if (ptr)
        {
            rv = malloc(len+1);
            if (rv)
            {
                memcpy(rv, ptr, len);
                rv[len] = 0;
            }
            else {
                xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
            }
        }
    }
    return rv;
}

XML_API int XML_APIENTRY
xmlAttributeCopyString(const xmlId *id, const char *name,
                                        char *buffer, int buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = 0;

    if (xid->name_len && xid->name != comment)
    {
        int len;
        const char *ptr;

        assert(buffer != 0);
        assert(buflen > 0);

        ptr = __xmlAttributeGetDataPtr(xid, name, &len);
        if (ptr)
        {
            int restlen = len;
            if (restlen >= buflen)
            {
                restlen = buflen-1;
                xmlErrorSet(xid, ptr, XML_TRUNCATE_RESULT);
            }

            memcpy(buffer, ptr, restlen);
            buffer[restlen] = 0;
            rv = restlen;
        }
    }
    return rv;
}

XML_API int XML_APIENTRY
xmlAttributeCompareString(const xmlId *id, const char *name, const char *s)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = XML_TRUE;

    if (xid->name_len && xid->name != comment)
    {
        int len;
        const char *ptr;

        assert(s != 0);

        ptr = __xmlAttributeGetDataPtr(xid, name, &len);
        if (ptr && (len == strlen(s))) {
            rv = strncasecmp(ptr, s, len);
        }
    }
    return rv;
}


#ifndef XML_NONVALIDATING
XML_API int XML_APIENTRY
xmlErrorGetNo(const xmlId *id, int clear)
{
    int rv = 0;

    if (id)
    {
        struct _xml_id *xid = (struct _xml_id *)id;
        struct _root_id *rid = xid->root;

        if (rid->info)
        {
            struct _zeroxml_error *err = rid->info;

            rv = __zeroxml_info.err_no = err->err_no;
            if (clear) {
                err->err_no = __zeroxml_info.err_no = 0;
            }
        }
    }
    else {
        rv = __zeroxml_info.err_no;
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlErrorGetLineNo(const xmlId *id, int clear)
{
    int rv = 0;

    if (id)
    {
        struct _xml_id *xid = (struct _xml_id *)id;
        struct _root_id *rid;

        if (xid->name) rid = xid->root;
        else rid = (struct _root_id *)xid;

        assert(rid != 0);

        if (rid->info)
        {
            struct _zeroxml_error *err = rid->info;
            const char *ps = rid->start;
            const char *pe = err->pos;
            const char *new;

            rv++;
            while (ps<pe)
            {
                new = memchr(ps, '\n', pe-ps);
                if (new) rv++;
                else break;
                ps = new+1;
            }

            if (clear) {
                err->err_no = __zeroxml_info.err_no = 0;
            }
        }
    }

    return rv;
}

XML_API int XML_APIENTRY
xmlErrorGetColumnNo(const xmlId *id, int clear)
{
    int rv = 0;

    if (id)
    {
        struct _xml_id *xid = (struct _xml_id *)id;
        struct _root_id *rid;

        if (xid->name) rid = xid->root;
        else rid = (struct _root_id *)xid;

        assert(rid != 0);

        if (rid->info)
        {
            struct _zeroxml_error *err = rid->info;
            const char *ps = rid->start;
            const char *pe = err->pos;
            const char *new;

            while (ps<pe)
            {
                new = memchr(ps, '\n', pe-ps);
                new = memchr(ps, '\n', pe-ps);
                if (new) rv++;
                else break;
                ps = new+1;
            }
            rv = pe-ps;

            if (clear) {
                err->err_no = __zeroxml_info.err_no = 0;
            }
        }
    }

    return rv;
}

XML_API const char* XML_APIENTRY
xmlErrorGetString(const xmlId *id, int clear)
{
    char *rv = NULL;

    if (id)
    {
        struct _xml_id *xid = (struct _xml_id *)id;
        struct _root_id *rid;

        if (xid->name) rid = xid->root;
        else rid = (struct _root_id *)xid;

        assert(rid != 0);

        if (rid->info)
        {
            struct _zeroxml_error *err = rid->info;
            if (XML_NO_ERROR <= err->err_no && err->err_no < XML_MAX_ERROR) {
                rv = (char*)__zeroxml_error_str[err->err_no];
            } else {
                rv = "incorrect error number.";
            }

            if (clear) {
                err->err_no = __zeroxml_info.err_no = 0;
            }
        }
    }
    else {
       rv = (char*)__zeroxml_error_str[__zeroxml_info.err_no];
    }

    return rv;
}

#else

XML_API int XML_APIENTRY
xmlErrorGetNo(const xmlId *id, int clear)
{
    return XML_NO_ERROR;
}

XML_API int XML_APIENTRY
xmlErrorGetLineNo(const xmlId *id, int clear)
{
    return 0;
}

XML_API int XML_APIENTRY
xmlErrorGetColumnNo(const xmlId *id, int clear)
{
    return 0;
}

XML_API const char* XML_APIENTRY
xmlErrorGetString(const xmlId *id, int clear)
{
    return "error detection was not enabled at compile time: no error.";
}
#endif

/* -------------------------------------------------------------------------- */

static const char *__xmlProcessCDATA(const char**, int*, char);

static const char *__xml_memmem(const char*, int, const char*, int);
static const char *__xml_memncasestr(const char*, int, const char*);
static const char *__xml_memncasecmp(const char*, int*, const char**, int*);

#ifndef XML_NONVALIDATING
static const char *__zeroxml_error_str[XML_MAX_ERROR] =
{
    "no error",
    "unable to allocate enough memory",
    "unable to open file for reading",
    "node name is invalid",
    "unexpected end of section",
    "buffer too small to hold all data, truncating",
    "incorrect comment section",
    "bad declaration block",
    "incompatible opening tag for element",
    "missing or invalid closing tag for element",
    "missing or invalid opening quote for attribute",
    "missing or invalid closing quote for attribute"
};
#endif

static const char*
__xmlAttributeGetDataPtr(const struct _xml_id *id, const char *name, int *len)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    const char *rv = NULL;

    assert(xid != 0);
    assert(name != 0);
    assert(len != 0);

    *len = 0;
    if (xid->name_len)
    {
        int slen = strlen(name);
        const char *ps, *pe;

        assert(xid->start > xid->name);

        ps = xid->name + xid->name_len + 1;
        pe = xid->start - 1;
        while (ps<pe)
        {
            while ((ps<pe) && isspace(*ps)) ps++;
            if (((int)(pe-ps) > slen) && (strncasecmp(ps, name, slen) == 0))
            {
                ps += slen;
                if ((ps<pe) && (*ps == '='))
                {
                    const char *start;

                    ps++;
                    if (*ps == '"' || *ps == '\'') ps++;
                    else
                    {
                        xmlErrorSet(xid, ps, XML_ATTRIB_NO_OPENING_QUOTE);
                        return NULL;
                    }

                    start = ps-1;
                    while ((ps<pe) && (*ps != *start)) ps++;
                    if (ps<pe)
                    {
                        start++;
                        rv = start;
                        *len = ps-start;
                    }
                    else
                    {
                        xmlErrorSet(xid, ps, XML_ATTRIB_NO_CLOSING_QUOTE);
                        return NULL;
                    }
                }
                else
                {
                    while ((ps<pe) && !isspace(*ps)) ps++;
                    continue;
                }

                break;
            }
            while ((ps<pe) && !isspace(*ps)) ps++;
        }
    }

    return rv;
}

/*
 * Search for the subsection of the last node in the node path inside the
 * current section.
 *
 * When finished *len will be set to the length of the requested subsection,
 * *name will point to the actual name of the node (useful in case the name was
 * a wildcard character) and *nlen will return the length of the actual name.
 *
 * It the node-cache is enabled *nc will return the requested cashed node.
 *
 * In case of an error *node will point to the location of the error within the
 * buffer and *len will contain the error code.
 *
 * @param nc node from the node cache
 * @param start starting point of the current section
 * @param len length of the current section
 * @param name a pointer to the path to walk
 * @param nlen length of the path string
 * @retrun a pointer to the section containing the last node in the path
 */
const char*
__xmlNodeGetPath(const cacheId **nc, const char *start, int *len, const char **name, int *nlen)
{
    const char *path, *end;
    const char *rv = NULL;
    int pathlen;

    assert(start != 0);
    assert(len != 0);
    assert(name != 0);
    assert(*name != 0);
    assert(nlen != 0);
    assert(*nlen != 0);

    if (*len == 0 || *nlen == 0 || *nlen > *len) {
        return rv;
    }

    path = *name;
    pathlen = *nlen;
    end = path + pathlen;
    if (*path == '/') {
        path++;
    }

    if (path < end)
    {
        int num, blocklen, nodelen;
        const char *new, *node, *p;

        node = path;
        nodelen = end - node;
        path = memchr(node, '/', nodelen);
        if (path) nodelen = path - node;

        num = 0;
        if ((p = memchr(node, '[', nodelen)) != NULL)
        {
            char *e;

            nodelen = p++ - node;
            e = (char*)p + nodelen;
            num = __xml_strtol(p, &e, 10);
            if (*e++ != ']') {
                return rv;
            }

            path = e;
            if (path == end) path = NULL;
        }

        rv = start;
        blocklen = *len;
#ifndef XML_USE_NODECACHE
        new = __xmlNodeGet(*nc, &rv, &blocklen, &node, &nodelen, &num,STRIPPED);
#else
        new = __xmlNodeGetFromCache(nc, &rv, &blocklen, &node, &nodelen, &num);
#endif
        if (new)
        {
            if (path)
            {
                pathlen = end - path;
                rv = __xmlNodeGetPath(nc, rv, &blocklen, &path, &pathlen);
                *name = path;
                *nlen = pathlen;
                *len = blocklen;
            }
            else
            {
                *name = node;
                *nlen = nodelen;
                *len = blocklen;
            }
        }
        else if (nodelen == 0) /* error upstream */
        {
            *name = node;
            *nlen = nodelen;
            *nlen = blocklen;
            rv = NULL;
        }
    }

    return rv;
}

/*
 * Recursively walk the node tree to get te section with the '*name' name.
 *
 * When finished *buf will point to the start of the data section of the node,
 * *len will be set to the length of the requested data section, *name will
 * point to the actual name of the node (useful in case the name was a wildcard
 * character), *nlen will return the length of the actual name and *nodenum will
 * return the current occurence number of the requested section.
 *
 * In case of an error *name will point to the location of the error within the
 * buffer and *len will contain the error code.
 *
 * @param nc node from the node-cache
 * @param *buf starting pointer for this section
 * @param len length to the end of the buffer
 * @param *name name of the node to look for
 * @param rlen length of the name of the node to look for
 * @param nodenum which occurence of the node name to look for
 * @param mode process CDATA or not
 * @return a pointer to the section right after the current node or NULL in case
           of an error
 */
#ifndef XML_NONVALIDATING
# define DECR_LEN(a,b,c) { a -= b-c; \
  if (a<0) { printf("Line %i\n", __LINE__); break; } \
}
#else
# define DECR_LEN(a,b,c) break
#endif

 const char*
__xmlNodeGet(const cacheId *nc, const char **buf, int *len, const char **name, int *rlen, int *nodenum, char mode)
{
    const char *open_element = *name;
    const char *element, *start_tag = 0;
    int restlen, elementlen;
    int open_len = *rlen;
    const cacheId *nnc = NULL;
    const char *rv = NULL;
    const char *rptr, *start;
    const char *new, *cur;
    int found;
    int num;

    assert(buf != 0);
    assert(*buf != 0);
    assert(len != 0);
    assert(name != 0);
    assert(rlen != 0);
    assert(nodenum != 0);

    start = *buf;
    if (open_len == 0 || *name == 0) {
        SET_ERROR_AND_RETURN(start, XML_NO_ERROR);
    }

    found = 0;
    num = *nodenum;
    if (*rlen > *len) goto __xmlNodeGetExit;

    *nodenum = 0;
    restlen = *len;
    *len = XML_UNEXPECTED_EOF;
    cur = start;

#ifdef XML_USE_NODECACHE
    cacheInitLevel(nc);
#endif

    /* search for an opening tag */
    rptr = start;
    element = *name;
    elementlen = *rlen;
    while ((new = memchr(cur, '<', restlen)) != 0)
    {
        new++; /* skip '<' */
        DECR_LEN(restlen, new, cur);
        cur = new;

        if (*new == '/')
        {
            DECR_LEN(restlen, new+1, cur);
            cur = new+1; /* look past '/' */

            /* different name?: end of a subsection */
            if (strncasecmp(cur, element, elementlen))
//          if (!__xml_memncasecmp(cur, &restlen, &element, &elementlen))
            {
                *len = new-start-2; /* strlen("</") */
                return rv;
            }

            /* returned from recursive and the same name: continue */
        }
        else
        {
            /* not returned from recursive */
            element = *name;
            elementlen = *rlen;
        }

        DECR_LEN(restlen, new, cur);
        cur = new;

        if (*cur == '!') /* comment: "<!---->" or CDATA: "<![CDATA[]]>" */
        {
            const char *start = cur;
            int blocklen = restlen;
            new = __xmlProcessCDATA(&start, &blocklen, mode);
            if (!new && start && open_len) { /* CDATA */
                SET_ERROR_AND_RETURN(start, XML_INVALID_COMMENT);
            }

#if defined XML_COMMENTNODE && defined XML_USE_NODECACHE
            /* Create a new leaf node for the current branch */
            nnc = cacheNodeNew(nc);
            cacheDataSet(nnc, comment, strlen(comment), start, blocklen);
#endif

            DECR_LEN(restlen, new, cur);
            cur = new;
            continue;
        }

        if (element == *name) /* did we return from recursive? */
        {                     /* no */
            /* Get the element name and a pointer right after the opening tag */
            rptr = __xml_memncasecmp(cur, &restlen, &element, &elementlen);
            if (rptr) 			/* the requested element was found */
            {
                new = rptr;
                if (found == num || num == -1)
                {
                    rv = new;
                    *buf = new;
                    open_len = elementlen;
                    start_tag = element;
                }
                else start_tag = 0;
            }
            else { /* a different element name was found */
                SET_ERROR_AND_RETURN(cur, XML_ELEMENT_NO_CLOSING_TAG);
            }
            if (!restlen) {
                break;
            }

#ifdef XML_USE_NODECACHE
            /* Create a new sub-branch or leaf node for the current branch */
            nnc = cacheNodeNew(nc);
#endif

            if ((new-start) > 2 && *(new-2) == '/') /* e.g. <test n="1"/> */
            {
                cur = new;
                if (rptr)
                {
#ifdef XML_USE_NODECACHE
                    /* fill the node-cache node with info. */
                    cacheDataSet(nnc, element, elementlen, rptr, STRIPPED);
#endif
                    if (found == num || num == -1)
                    {
                        open_element = start_tag;
                        *len = 0;
                    }
                    found++;
                }
                continue;
            }

            /*
             * Skip the value of the node and get the next XML tag.
             */
            /* restlen -= new-cur; not necessary because of __xml_memncasecmp */
            cur = new;
            if ((new = memchr(cur, '<', restlen)) == 0) {
                SET_ERROR_AND_RETURN(cur, XML_ELEMENT_NO_OPENING_TAG);
            }

            new++; /* skip '<' */
            DECR_LEN(restlen, new, cur);
            cur = new;

            /* comment: "<!---->" or CDATA: "<![CDATA[]]>" */
            while (*cur == '!')
            {
                const char *start = cur;
                int blocklen = restlen;
                new = __xmlProcessCDATA(&start, &blocklen, mode);
                if (!new && start && open_len) { /* CDATA */
                    SET_ERROR_AND_RETURN(start, XML_INVALID_COMMENT);
                }

                DECR_LEN(restlen, new, cur);
                cur = new;

                /*
                 * Skip the value of the node and get the next XML tag.
                 */
                if ((new = memchr(cur, '<', restlen)) == 0) {
                    SET_ERROR_AND_RETURN(cur, XML_ELEMENT_NO_CLOSING_TAG);
                }

                new++; /* skip '<' */
                DECR_LEN(restlen, new, cur);
                cur = new;
            }
        } /* element == *name */

        if (*cur == '/') /* a closing tag of a leaf node is found */
        {
            if (!strncasecmp(new+1, element, elementlen))
            {
                const char *pe = new+restlen;
                const char *ps = new+elementlen+1;
                while ((ps<pe) && isspace(*ps)) ps++;

#ifdef XML_USE_NODECACHE
                cacheDataSet(nnc, element, elementlen, rptr, new-rptr-1);
#endif
                if (*ps != '>') {
                    SET_ERROR_AND_RETURN(new+1, XML_ELEMENT_NO_CLOSING_TAG);
                }

                if (found == num || num == -1)
                {
                    if (start_tag)
                    {
                        *len = new-rv-1;
                        open_element = start_tag;
                        start_tag = 0;
                        if (num != -1) {
                            break;
                        }

                        new += elementlen+2; /* skip "elementname/>" */
                        DECR_LEN(restlen, new, cur);
                        cur = new;
                        continue;
                    }
                    else	/* element not found, no real error */
                    {
                        found = 0;
                        break;
                    }
                }
                found++;
            }
        }

        do
        {
            /* No leaf node, continue */
            const char *node = "*";
            int slen = restlen+1; /* due to cur-1 below*/
            int nlen = 1;
            int pos = -1;

            /*
             * Recursively walk the XML tree from here.
             *
             * When finished:
             *  *slen contains the length of the requested data section
             *  *node will point to the actual name of the node
             *  *nlen will return the length of the actual name and
             *  *pos will return the current occurence of the section
             *
             * In case of an error:
             *  *node will point to the location of the error
             *  *slen will contain the error code
             *
             * returns a pointer to the data section of the node with the
             * requested name or NULL in case of an error.
             */
            new = cur-1;
            rptr = new;
            if (!__xmlNodeGet(nnc, &new, &slen, &node, &nlen, &pos, STRIPPED))
            {
                if (nlen == 0) /* error upstream */
                {
                    *rlen = nlen;
                    SET_ERROR_AND_RETURN(node, slen);
                }

                if (slen == restlen) {
                    SET_ERROR_AND_RETURN(cur, XML_UNEXPECTED_EOF);
                }
                return NULL;
            }
            cur += slen;
            DECR_LEN(restlen, slen, 0);
         }
         while(0);
    } /* while */

__xmlNodeGetExit:
    if (found != num && num != -1)
    {
        rv = NULL;
        *rlen = 0;
        *name = start_tag;
        *len = XML_NO_ERROR;	/* element not found, no real error */
    }
    else
    {
        *rlen = open_len;
        *name = open_element;
        *nodenum = found;
    }
    return rv;
}

/*
 * Return the nth occurance of a node with the 'name' name.
 *
 * Afterwards the contents of *id will be changed to reflect the new node.
 *
 * @param pid a reference to the paren node of id
 * @param id the node which will have updated contents
 * @param name the name of the node to search for
 * @param nodenum which occurence of the node name to look for
 * @param mode indicate whether the result must include the XML indicatiors
 * @return the updated version of *id or NULL in case of an error
 */
static xmlId*
__xmlNodeGetPos(const xmlId *pid, xmlId *id, const char *name, int nodenum, char mode)
{
    struct _xml_id *xpid = (struct _xml_id *)pid;
    struct _xml_id *xid = (struct _xml_id *)id;
    const char *ptr, *new;
    const cacheId *nc;
    int len, slen;
    xmlId *rv = NULL;

    assert(xpid != 0);
    assert(xid != 0);
    assert(name != 0);

    len = xpid->len;
    ptr = xpid->start;
    slen = strlen(name);
    nc = cacheNodeGet(pid);
#ifndef XML_USE_NODECACHE
    new = __xmlNodeGet(nc, &ptr, &len, &name, &slen, &nodenum, mode);
#else
    new = __xmlNodeGetFromCache(&nc, &ptr, &len, &name, &slen, &nodenum);
#endif
    if (new)
    {
        xid->len = len;
        xid->start = (char*)ptr;
        xid->name = name;
        xid->name_len = slen;
#ifdef XML_USE_NODECACHE
        xid->node = nc;
#endif
        rv = (xmlId*)xid;
    }
    else if (slen == 0) {
        xmlErrorSet(xpid, name, len);
    }

    return rv;
}

/*
 * Walks the XML tree and return the number of nodes with the same name
 * of the last section of the path.
 *
 * @param id XML-id of the node
 * @param path path to the xml node
 * @param mode indicate whether the result must include the XML indicatiors
 * @return the number of nodes with the same name
 */
static int
__xmlNodeGetNum(const xmlId *id, const char *path, char mode)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int rv = 0;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        const char *nodename = (*path == '/') ? path+1 : path;
        const char *end = path + strlen(path);
        int len, slen = end-nodename;
        const char *pathname, *ptr;
        const cacheId *nc;

        nc = cacheNodeGet(id);
        if ((pathname = strchr(nodename, '/')) != NULL)
        {
            const char *node = ++pathname;
            len = xid->len;
            slen = end-node;
            ptr = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
            if (ptr == NULL && slen == 0) {
                xmlErrorSet(xid, node, len);
            }
        }
        else
        {
            ptr = xid->start;
            len = xid->len;
        }

        if (ptr)
        {
            const char *node = nodename;
            const char *new;
#ifndef XML_USE_NODECACHE
            new = __xmlNodeGet(nc, &ptr, &len, &node, &slen, &rv, mode);
#else
            new = __xmlNodeGetFromCache(&nc, &ptr, &len, &node, &slen, &rv);
#endif
            if (new == NULL && len != 0)
            {
                xmlErrorSet(xid, node, len);
                rv = 0;
            }
        }
    }

    return rv;
}

/*
 * Return the contents of a node a a string.
 * The returned string has to be free'd by the caller.
 *
 * @param id XML-id of the node
 * @param mode indicate whether the result must include the XML indicatiors
 * @return a pointer right after the XML comment or CDATA section
 */
static char*
__xmlGetString(const xmlId *id, char mode)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *rv = NULL;

    assert(xid != NULL);

    if (xid->len)
    {
        const char *ps = xid->start;
        int len = xid->len;

// printf("0 "); PRINT(ps, len, len);
        if (mode == STRIPPED) {
// printf("1 "); PRINT(ps, len, len);
             __xmlPrepareData(&ps, &len, mode);
// printf("2 "); PRINT(ps, len, len);
        }
        if (len)
        {
            if ((rv = malloc(len+1)) != NULL)
            {
                memcpy(rv, ps, len);
                rv[len] = 0;
            }
            else {
                xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
            }
        }
    }

    return rv;
}

/*
 * Skip an XML comment section or a CDATA section.
 *
 * If mode is set to XML_TRUE then the result will include the XML comment
 * indicators or XML CDATA indicators.
 *
 * *start returns the starting location of the data within the section.
 * *len will retrun 0 when a comment section was found or
 * *len will return the length of the CDATA section otherwise.
 *
 * @param start a pointer to the start of the XML block
 * @param len the length of the XML block
 * @param mode indicate whether the result must include the XML indicatiors
 * @return a pointer right after the XML comment or CDATA section
 */
const char*
__xmlProcessCDATA(const char **start, int *len, char mode)
{
    const char *new = *start;
    const char *cur = new;
    int restlen = *len;

    /* comment: "<!---->" */
    if ((restlen >= 7) && (memcmp(cur, "!--", 3) == 0))
    {
        cur += 3;
        restlen -= 3;
        *start = cur;
        *len = 0;

        new = __xml_memmem(cur, restlen, "-->", 3);
        if (new)
        {
           *len = new - *start;
           new += 3;
        }
    }

    /* CDATA: "<![CDATA[]]>" */
    else if (restlen >= 12 && (memcmp(cur, "![CDATA[", 8) == 0))
    {
        cur += 8;
        restlen -= 8;
        if (mode == STRIPPED) *start = cur;
        *len = 0;

        new = __xml_memmem(cur, restlen, "]]>", 3);
        if (new)
        {
           *len = new - *start;
           if (mode == RAW) *len += 3;
           new += 3;
        }
    }
    else {
        new = NULL;
    }

    return new;
}

/*
 * Handle the XML Declaration.
 *
 * The XML declaration is a processing instruction that identifies the
 * document as being XML. See: xmlwriter.net/xml_guide/xml_declaration.shtml
 *
 * The library only supports single character encoding.
 *
 * @param start a pointer to the start of the XML document
 * @param len the lenght of the XML document
 * @return a pointer to the memory location right after the declaration
 */
const char*
__xmlDeclarationProcess(const char *start, int len)
{
    const char *cur, *end;
    const char *rv = start;

    cur = start;
    if (len < 7 || *cur++ != '<') { /*"<?xml?>" */
        return start;
    }
    len--;
    end = cur;

    // Note: http://www.w3.org/TR/REC-xml/#sec-guessing
    if (*cur++ == '?')
    {
        const char *element = "?>";
        len--;
        end = __xml_memncasestr(cur, len, element);
        if (end)
        {
            const char *new;

            rv = end+2; /* skip: "?>" */
            len = end-cur;

            element = "encoding=\"";
            if ((new = __xml_memncasestr(cur, len, element)) != NULL)
            {
                int elementlen = strlen(element);
                len -= new-cur+elementlen;
                cur = new+elementlen;
                if ((end = memchr(cur, '"', len)) != NULL)
                {
                    const char *str = "UTF-8";
                    int slen = strlen(str);
                    len--;
                    if (len >= slen && !strncasecmp(cur, str, len)
#ifdef HAVE_LANGINFO_H
                         && !strcmp(nl_langinfo(CODESET), str)
#endif
                      )
                    {
#ifdef HAVE_LOCALE_H
//                      setlocale(LC_CTYPE, "C.UTF-8");
#endif
                    }
                }
            }
        }
    }

    return rv;
}

/*
 * Reformat the value string of an XML node.
 *
 * Leading and trainling white-space characters will be removed.
 *    These are: space, form-feed, newline, carriage return,
 *               horizontal tab and vertical tab.
 * Comment sections and CDATA indicators will be removed when mode is set to RAW
 *
 * When returning *start points to the new start of the value string and
 * *blocklen returns the new length of the value string.
 *
 * @param start Start of the value string
 * @param blocklen length of the value string
 * @param mode indicate whether comments and CDATA sections should be kept (RAW)
   or not (STRIPPED)
 */
static void
__xmlPrepareData(const char **start, int *blocklen, char mode)
{
    int len = *blocklen;
    const char *ps = *start;
    const char *pe = ps+len-1;

    if (mode == RAW)
    {
        while ((ps<pe) && isspace(*ps)) ps++;
        while ((pe>ps) && isspace(*pe)) pe--;
        len = (++pe-ps);
    }
    else
    {
        do
        {
            while ((ps<pe) && isspace(*ps)) ps++;
            len = (pe-ps);

            if (len >= 6)
            {
                if (!strncmp(ps, "<!", 2)) /* <----> */
                {
                    const char *start = ps+1; /* skip '<' */
                    const char *new = __xmlProcessCDATA(&start, &len, mode);
                    if (new)
                    {
                        ps = new;

                        while ((ps<pe) && isspace(*ps)) ps++;
                        len = (pe-ps);
                    }
                }
                else break;
            }
        }
        while(len >= 6);

        do
        {
            while ((pe>ps) && isspace(*pe)) pe--;
            len = (++pe-ps);

            if (len >= 6) /* <----> */
            {
                if (!strncmp(pe-2, "->", 2))
                {
                    const char *pc = pe;

                    while ((pc>ps) && *pc != '<') pc--;
                    if (pc == ps)
                    {
                        len = 0;
                        break;
                    }

                    if (pc[1] == '!')
                    {
                        pe = --pc;

                        while ((pe>ps) && isspace(*pe)) pe--;
                        len = (++pe-ps);
                    }
                }
                else break;
            }
        }
        while(len >= 6);
    }

    *start = ps;
    *blocklen = len;
}

#ifndef XML_NONVALIDATING
void
__xmlErrorSet(const struct _xml_id *id, const char *pos, int err_no)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    struct _root_id *rid;

    assert(xid != 0);

    rid = xid->root;

    if (rid->info == 0) {
        rid->info = malloc(sizeof(struct _zeroxml_error));
    }

    if (rid->info)
    {
        struct _zeroxml_error *err = rid->info;
        err->pos = __zeroxml_info.pos = (const char *)pos;
        err->err_no = __zeroxml_info.err_no = err_no;
    }
    else
    {
        __zeroxml_info.pos = (const char *)pos;
        __zeroxml_info.err_no = err_no;
    }
}
#endif

/*
 * Convert a non NULL-terminated string to a long integer.
 *
 * When the string starts with "0b" or "\b" base 2 is being used (binary),
 * when the string starts with "0x" or "\x" base 16 is being used (hexadecimal),
 * else when the string starts with "0" and the length is greater than 2,
 * or when the string starts with "0o" or "\0" base 8 is being used (octal).
 * otherwise the provided base is being used.
 *
 * Upon succes *end will point to first unconvertable character in the string.
 *
 * If an underflow occurs the function returns LONG_MIN.
 * If an overflow occurs the function returns LONG_MAX.
 * In both cases, errno is set to ERANGE.
 *
 * @param str a pointer to the non NULL-terminated string
 * @param end a pointer to the last character in the string to convert
 * @param base the base to use if the string is not hexadecimal
 * @return the string convered into a long integer
 */
static long
__xml_strtol(const char *str, char **end, int base)
{
    int len = *end - str;

    if (len >= 2)
    {
        if (str[0] == '0' || str[0] == '\\')
        {
            switch(str[1])
            {
            case 'b': /* binary */
                str += 2;
                base = 2;
                break;
            case 'd': /* decimal */
                str += 2;
                base = 10;
                break;
            case 'x': /* hexadecimal */
                str += 2;
                base = 16;
                break;
            case 'o': /* octal */
               str++;
               // intentional fallthrough
            default:
                str++;
                base = 8;
                break;
            }
        }
    }
    return strtol(str, end, base);
}

/*
 * Interpret a string as a boolean.
 *
 * Values for true are "on", "yes", "true" and any number not being zero.
 *
 * @param start a pointer to the start of the string to interpret
 * @param end a pointer to the end of the string to interpret
 * @return XML_FALSE when false or XML_TRUE when true
 */
static int
__xml_strtob(const char *start, const char *end)
{
    int res, rv = __XML_BOOL_NONE;
    char *ptr;

    ptr = (char*)end;
    res = __xml_strtol(start, &ptr, 10) ? XML_TRUE : XML_FALSE;
    if (ptr == start)
    {
        int len = end-start;
        if (!strncasecmp(start, "off", len)
            || !strncasecmp(start, "no", len)
            || !strncasecmp(start, "false", len))
        {   
            rv = XML_FALSE;
        }
        else if (!strncasecmp(start, "on", len)
                 || !strncasecmp(start, "yes", len)
                 || !strncasecmp(start, "true", len))
        {
            rv = XML_TRUE;
        }
    }
    else {
        rv = res;
    }
    return rv;
}

/*
 * Locate a sub-string in a memory block.
 *
 * The comparisson is case sensitive.
 *
 * @param haystack a pointer to the beginning of the memory block
 * @param haystacklen the length of the memory block
 * @param needle the string to search for
 * @param needlelen the length of the needle to search for
 * @return a pointer to the located sub‐string, or NULL if not found
 */
static const char*
__xml_memmem(const char *haystack, int haystacklen, const char *needle, int needlelen)
{
    const char *rv = NULL;
    char first;

    assert (haystack);
    assert (needle);

    first = *needle;
    if (haystacklen && needlelen && first != '\0')
    {
        do
        {
            const char *new = memchr(haystack, first, haystacklen);
            if (!new) break;

            haystacklen -= (new-haystack);
            if (haystacklen < needlelen) break;

            if (memcmp(new, needle, needlelen) == 0)
            {
                rv = new;
                break;
            }
            haystack = new+1;
            haystacklen -= haystack-new;
        }
        while (haystacklen > 2);
    }
    return rv;
}

/*
 * Case insensitive search for a string in a memory block.
 *
 * @param haystack a pointer to the beginning of the memory block
 * @param haystacklen the length of the memory block
 * @param needle the string to search for
 * @return a pointer to the located sub‐string, or NULL if not found
 */
static const char*
__xml_memncasestr(const char *haystack, int haystacklen, const char *needle)
{
    const char *rv = NULL;
    int needlelen;

    assert(needle);

    needlelen = strlen(needle);
    if (needlelen-- != 0)
    {
        char first = tolower(*needle++);
        do
        {
            while (haystacklen && tolower(*haystack++) != first) haystacklen--;
            if (haystacklen < needlelen) return NULL;
        }
        while (strncasecmp(haystack, needle, needlelen) != 0);
        rv = --haystack;
    }
   return rv;
}

/*
 * Search for a needle in the haystack.
 *
 * The needle is an XML node name.
 * The haystack is an XML (sub)section.
 *
 * The needle may be the '*' wildcard character to find any node, or it may
 * contain the '?' wildcard character to indicate that any character is
 * acceptable at that position.
 *
 * When succesful *haystacklen is set to the length of the remaining of the
 * haystack XML section without the node tag, *needle points to the real
 * node name as found in the section and *needlelen is the length of the
 * found node name.
 *
 * @param haystack pointer to the satrt of the XML section
 * @param haystacklen length of the XML section
 * @param needle node name to search for
 * @param needlelen length of the node name to search for
 * @retrun a pointer inside the XML section right after the node name tag
 */
#define CASECMP(a,b)	(tolower(a)==tolower(b))
#define VALIDNAME(a)	(!strchr(" :~/\\;$&%@^=*+()|\"{}[]<>", (a)))
#define ISSEPARATOR(a) (strchr(">/ ", (a)))
#define ISNUM(a)       (isdigit(a))
static const char*
__xml_memncasecmp(const char *haystack, int *haystacklen,
                  const char **needle, int *needlelen)
{
    const char *rptr = 0;

    assert(haystack);
    assert(haystacklen);
    assert(needle);
    assert(needlelen);

    if (*needlelen > 0 && *haystacklen >= *needlelen)
    {
        const char *hs = (const char *)haystack;
        const char *he = hs + *haystacklen;
        const char *ns = *needle;

        /* check for a valid starting character of the node name */
        if (VALIDNAME(*hs) && !ISNUM(*hs))
        {
            /* search for any name */
            if ((*ns == '*') && (*needlelen == 1))
            {
                while ((hs < he) && VALIDNAME(*hs)) ++hs;
                if (ISSEPARATOR(*hs))
                {
                    *needle = (const char *)haystack;
                    *needlelen = hs - haystack;

                    /* find the closing character */
                    if ((ns = memchr(hs, '>', he-hs)) != NULL) hs = ns+1;
                    else hs = he;
                    rptr = hs;
                }
                else /* invalid character in the node name */
                {
                    *needle = hs;
                    *needlelen = 0;
                    hs = he;
                }
            }
            else
            {
                // the above test assures *haystacklen >= *needlelen
                int i = *needlelen;
                do
                {
                    /* does it match or is it a wildcard character? */
                    if (!CASECMP(*hs, *ns) && (*ns != '?')) break;

                    /* is it a character which is not part of the name? */
                    if (!VALIDNAME(*hs)) break;

                    hs++;
                    ns++;
                }
                while(--i);

                /* the next character may not be still part of a name */
                if (i == 0 && ISSEPARATOR(*hs))
                {
                    *needle = (const char *)haystack;
                    *needlelen = hs - haystack;

                    /* find the closing character */
                    if ((ns = memchr(hs, '>', he-hs)) != NULL) hs = ns+1;
                    else hs = he;
                    rptr = hs;
                }
                else /* not found */
                {
                    while((hs < he) && VALIDNAME(*hs)) ++hs;
                    if (ISSEPARATOR(*hs))
                    {
                        *needle = (const char *)haystack;
                        *needlelen = hs - haystack;

                        /* find the closing character */
                        if ((ns = memchr(hs, '>', he-hs)) != NULL) hs = ns+1;
                        else hs = he;
                    }
                    else /* invalid character in the node name */
                    {
                        *needle = hs;
                        *needlelen = 0;
                        hs = he;
                    }
                }
            }
        }
        else /* invalid starting character in the node name */
        {
            *needle = hs;
            *needlelen = 0;
            hs = he;
        }
        *haystacklen -= hs - haystack;
    }

    return rptr;
}

#ifdef WIN32
/*
 * Simple mmap and munmap functions for Windows which behave the same as the
 * following UNIX equivalents.
 *
 * simple_mmap(fd, length, un):  mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0L)
 * simple_unmmap(fd, length, un): munmap(fd, length)
 *
 * Original from:
 * https://mollyrocket.com/forums/viewtopic.php?p=2529
 */

/*
 * Create a new mapping of a file in the virtual address space of the
 * calling process.
 *
 * @param fd file descriptor of the file to map
 * @param length the size of the mapped area, must be greater than 0
 * @param un a structure to hold some Windows specific parameters
 * @return a pointer to the mapped area. On error the MAP_FAILED (that is,
 *         (void *) MMAP_ERROR) is returned.
 */
void*
simple_mmap(int fd, int length, SIMPLE_UNMMAP *un)
{
    HANDLE f;
    HANDLE m;
    void *p;

    f = (HANDLE)_get_osfhandle(fd);
    if (!f) return (void *)MMAP_ERROR;

    m = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!m) return (void *)MMAP_ERROR;

    p = MapViewOfFile(m, FILE_MAP_READ, 0, 0, 0);
    if (!p)
    {
        CloseHandle(m);
        return (void *)MMAP_ERROR;
    }

    if (un)
    {
        un->m = m;
        un->p = p;
    }

    return p;
}

/*
 * Remove a mapping of a file in the virtual address space of the
 * calling process.
 *
 * @param addr a pointer to the mapped area to unmap
 * @param length the size of the mapped area, must be greater than 0
 * @param un a structure to hold some Windows specific parameters
 */
void
simple_unmmap(void *addr, int length, SIMPLE_UNMMAP *un)
{
    UnmapViewOfFile(un->p);
    CloseHandle(un->m);
}
#endif
