/*
 * This software is available under 2 licenses -- choose whichever you prefer.
 *
 * ALTERNATIVE A - Modified BSD license
 *
 * Copyright (C) 2008-2022 by Erik Hofman.
 * Copyright (C) 2009-2022 by Adalin B.V.
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
#include <locale.h>
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
static const char *__zeroxml_error_str[XML_MAX_ERROR];

static void __xmlErrorSet(const struct _xml_id*, const char *, size_t);
# define xmlErrorSet(a, b, c)	__xmlErrorSet(a, b, c)

# ifndef NDEBUG
#define FILENAME_LEN		1024
static char _xml_filename[FILENAME_LEN+1];
#  define PRINT_INFO(a, b, c) \
    if (0 < (c) && (c) < XML_MAX_ERROR) { \
        fprintf(stderr, "%s: detected in %s at line %i:\n\t%s at %li\n", \
            _xml_filename, __func__, __LINE__, __zeroxml_error_str[(c)], (long)(b-a)); \
    } else { \
        fprintf(stderr, "%s: in %s at line %i: Unknown error number!\n", \
                        _xml_filename, __func__, __LINE__); \
    }
# else
#  define PRINT_INFO(a, b, c)
# endif

# define SET_ERROR_AND_RETURN(a, b, c) \
        { *rlen = 0; *name = (b); *len = (c); PRINT_INFO(a, b, c); return 0; }

#else /* !XML_NONVALIDATING */
# define xmlErrorSet(a, b, c)
# define SET_ERROR_AND_RETURN(a, b, c)	return 0;
#endif

static char *__xmlNodeGetPath(void **, const char *, size_t *, char **, size_t *);
static char *__xmlNodeGet(void *, const char *, size_t *, char **, size_t *, size_t *, char);
static char *__xmlAttributeGetDataPtr(const struct _xml_id*, const char *, size_t *);
static char *__xmlProcessCDATA(char **, size_t *, char);
static char *__xmlCommentSkip(const char *, size_t);
static char *__xmlInfoProcess(const char *, size_t);
static int __xmlDecodeBoolean(const char *, const char *);
static void __xmlPrepareData(char **, size_t *, char);

static char *__xml_memmem(const char *, size_t, const char *, size_t);
static char *__xml_memncasestr(const char *, size_t, const char *);
static void *__xml_memncasecmp(const char *, size_t *, char **, size_t *);
static long __xml_strtol(const char *, char **, int);
#ifdef WIN32
/*
 * map 'filename' and return a pointer to it.
 */
static void *simple_mmap(int, size_t, SIMPLE_UNMMAP *);
static void simple_unmmap(void*, size_t, SIMPLE_UNMMAP *);

#else
# define simple_mmap(a, b, c)	mmap(0, (b), PROT_READ, MAP_PRIVATE, (a), 0L)
# define simple_unmmap(a, b, c)	munmap((a), (b))

#endif

#define STRUCT_ALIGN(a) 	((a) & 0xF) ? (((a) | 0xF)+1) : (a)

#ifndef NDEBUG
# define PRINT(a, b, c) { \
    size_t l1 = (b), l2 = (c); \
    char *s = (a); \
    if (s) { \
    size_t q, len = l2; \
    if (l1 < l2) len = l1; \
    if (len < 50000) { \
        printf("(%i) '", len); \
        for (q=0; q<len; q++) printf("%c", s[q]); \
        printf("'\n"); \
    } else printf("Length (%u) seems too large at line %i\n",len, __LINE__); \
    } else printf("NULL pointer at line %i\n", __LINE__); \
}
#endif

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
                void *mm;

                /* UTF-8 unicode support */
#ifdef HAVE_LOCALE_H
//              rid->locale = setlocale(LC_CTYPE, "");
#endif

                fstat(fd, &statbuf);
                mm = simple_mmap(fd, (size_t)statbuf.st_size, &rid->un);
                if (mm != (void *)-1)
                {
                    size_t blen = statbuf.st_size;
#ifdef XML_USE_NODECACHE
                    size_t num = 0, nlen = 1;
                    char *rv, *n = "*";

                    rid->node = cacheInit();
                    rv = __xmlNodeGet(rid->node, mm, &blen, &n, &nlen, &num, 0);
                    if (!rv)
                    {
                        simple_unmmap(rid->start, blen, &rid->un);
                        close(fd);

                        cacheFree(rid->node);
                        free(rid);
                        rid = 0;
                    }
#endif
                    rid->fd = fd;
                    rid->start = mm;
                    rid->len = blen;
                }
            }
        }
    }

    return (void *)rid;
}

XML_API xmlId* XML_APIENTRY
xmlInitBuffer(const char *buffer, size_t size)
{
    struct _root_id *rid = 0;

# ifndef NDEBUG
    snprintf(_xml_filename, FILENAME_LEN, "XML buffer");
#endif

    if (buffer && (size > 0))
    {
        rid = calloc(1, sizeof(struct _root_id));
        if (rid)
        {
#ifdef XML_USE_NODECACHE
            size_t num = 0, nlen = 1;
            size_t blen = size;
            char *rv, *n = "*";

            rid->node = cacheInit();
            rv = __xmlNodeGet(rid->node, buffer, &blen, &n, &nlen, &num, 0);
            if (!rv)
            {
                cacheFree(rid->node);
                free(rid);
                rid = 0;
            }
#endif
            rid->fd = -1;
            rid->start = (char *)buffer;
            rid->len = size;

#ifdef HAVE_LOCALE_H
//          rid->locale = setlocale(LC_CTYPE, "");
#endif
        }
    }

    return (void *)rid;
}

XML_API void XML_APIENTRY
xmlClose(xmlId *id)
{
    struct _root_id *rid = (struct _root_id *)id;

    if (rid)
    {
        assert(rid->name == 0);

        if (rid->fd != -1)
        {
            simple_unmmap(rid->start, (size_t)rid->len, &rid->un);
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
    size_t len, slen;
    char *node;
    void *nc, *nnc;

    assert(id != 0);
    assert(path != 0);

    node = (char *)path;
    len = xid->len;
    slen = strlen(path);

    nnc = nc = cacheNodeGet(xid);
    return __xmlNodeGetPath(&nnc, xid->start, &len, &node, &slen) ? 1 : 0;
}

XML_API xmlId* XML_APIENTRY
xmlNodeGet(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    struct _xml_id *xsid = 0;
    size_t len, slen;
    char *ptr, *node;
    void *nc, *nnc;

    assert(id != 0);
    assert(path != 0);

    node = (char *)path;
    len = xid->len;
    slen = strlen(path);

    nnc = nc = cacheNodeGet(xid);
    ptr = __xmlNodeGetPath(&nnc, xid->start, &len, &node, &slen);
    if (ptr)
    {
        xsid = malloc(sizeof(struct _xml_id));
        if (xsid)
        {
            xsid->name = node;
            xsid->name_len = slen;
            xsid->start = ptr;
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
        else
        {
            xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
        }
    }
    else if (slen == 0)
    {
        xmlErrorSet(xid, node, len);
    }

    return (void *)xsid;
}

XML_API xmlId* XML_APIENTRY
xmlNodeCopy(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *ptr, *node;
    size_t slen, len;
    void *ret = 0;
    void *nc, *nnc;


    node = (char *)path;
    len = xid->len;
    slen = strlen(path);

    nnc = nc = cacheNodeGet(xid);
    ptr = __xmlNodeGetPath(&nnc, xid->start, &len, &node, &slen);
    if (ptr)
    {
        const int rsize = sizeof(struct _root_id);
        const int nsize = sizeof(struct _xml_id);
#ifndef XML_NONVALIDATING
        const int esize = sizeof(struct _zeroxml_error);
#else
        const int esize = 0;
#endif
        struct _xml_id *nid;
        size_t new_len;

        new_len = STRUCT_ALIGN(slen + len);
        nid = malloc(nsize + new_len + rsize + esize);
        if (nid)
        {
            struct _root_id *rid;
            char *nptr = (char *)nid;

            memcpy(nptr, xid, nsize);
            nptr += nsize;
            nid->name = nptr;
            nid->name_len = slen;
            memcpy(nptr, node, slen);

            nid->start = nptr + slen;
            nid->len = len;
            memcpy(nptr+slen, ptr, len);
            nptr += new_len;

            rid = (struct _root_id *)nptr;
#ifndef XML_NONVALIDATING
            nid->root = rid;
            memcpy(nptr, xid->root, rsize);
            nptr += rsize;
#endif

            rid->start = nid->start;
            rid->len = nid->len;
            rid->fd = 0;
#ifndef XML_NONVALIDATING
            rid->info = (struct _zeroxml_error *)nptr;
            memset(nptr, 0, esize);
#endif
#ifdef XML_USE_NODECACHE
            nid->node = nc;
#endif
            ret = nid;
        }
        else
        {
            xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
        }
    }
    else if (slen == 0)
    {
        xmlErrorSet(xid, node, len);
    }

    return ret;
}

XML_API char * XML_APIENTRY
xmlNodeGetName(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t len;
    char *ret;

    assert(xid != 0);

    len = xid->name_len;
    ret = malloc(len+1);
    if (ret)
    {
        memcpy(ret, xid->name, len);
        *(ret + len) = 0;
    }
    else
    {
        xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
    }

    return ret;
}

XML_API size_t XML_APIENTRY
xmlNodeCopyName(const xmlId *id, char *buf, size_t buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t slen = 0;

    assert(buf != 0);
    assert(buflen > 0);

    slen = xid->name_len;
    if (slen >= buflen)
    {
        slen = buflen-1;
        xmlErrorSet(xid, 0, XML_TRUNCATE_RESULT);
    }
    memcpy(buf, xid->name, slen);
    *(buf + slen) = 0;

    return slen;
}

XML_API size_t XML_APIENTRY
xmlAttributeCopyName(const xmlId *id, char *buf, size_t buflen, size_t pos)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t slen = 0;

    assert(buf != 0);
    assert(buflen > 0);

    if (xid->name_len)
    {
        char *ps, *pe, *new;
        size_t num = 0;

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
                *(buf + slen) = 0;
                break;
            }

            ps = new+2;
            while ((ps<pe) && (*ps != '"' && *ps != '\'')) ps++;
            ps++;
        }
    }

    return slen;
}

XML_API char * XML_APIENTRY
xmlAttributeGetName(const xmlId *id, size_t pos)
{
   struct _xml_id *xid = (struct _xml_id *)id;
   char buf[4096];
   size_t len;
   char *ret;

    assert(xid != 0);

    len = xmlAttributeCopyName(id, buf, 4096, pos);
    ret = malloc(len+1);
    if (ret)
    {
        memcpy(ret, xid->name, len);
        *(ret + len) = 0;
    }
    else
    {
        xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
    }

    return ret;
}

static size_t
__xmlNodeGetNum(const xmlId *id, const char *path, char raw)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t num = 0;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        char *nodename, *pathname;
        size_t len, slen;
        void *nc;
        char *p;

        nodename = (char *)path;
        if (*path == '/') nodename++;
        slen = strlen(nodename);

        nc = cacheNodeGet(xid);
        pathname = strchr(nodename, '/');
        if (pathname)
        {
            char *node;

            len = xid->len;
            pathname++;
            slen -= pathname-nodename;
            node = pathname;
            p = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
            if (p == 0 && slen == 0)
            {
                xmlErrorSet(xid, node, len);
            }
        }
        else
        {
            p = xid->start;
            len = xid->len;
        }

        if (p)
        {
            char *ret, *node = nodename;
#ifndef XML_USE_NODECACHE
            ret = __xmlNodeGet(nc, p, &len, &node, &slen, &num, raw);
#else
            ret = __xmlNodeGetFromCache(&nc, p, &len, &node, &slen, &num);
#endif
            if (ret == 0 && slen == 0)
            {
                xmlErrorSet(xid, node, len);
                num = 0;
            }
        }
    }

    return num;
}

XML_API size_t XML_APIENTRY
xmlNodeGetNum(const xmlId *id, const char *path)
{
   return __xmlNodeGetNum(id, path, 0);
}

XML_API size_t XML_APIENTRY
xmlNodeGetNumRaw(const xmlId *id, const char *path)
{
   return __xmlNodeGetNum(id, path, 1);
}

XML_API size_t XML_APIENTRY
xmlAttributeGetNum(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t num = 0;

    if (xid->name_len)
    {
        char *ps, *pe, *new;

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

static void *
__xmlNodeGetPos(const xmlId *pid, xmlId *id, const char *element, size_t num, char raw)
{
    struct _xml_id *xpid = (struct _xml_id *)pid;
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t len, slen;
    char *ptr, *node;
    void *ret = 0;
    void *nc;

    assert(xpid != 0);
    assert(xid != 0);
    assert(element != 0);

    len = xpid->len;
    slen = strlen(element);
    node = (char *)element;
    nc = cacheNodeGet(xpid);
#ifndef XML_USE_NODECACHE
    ptr = __xmlNodeGet(nc, xpid->start, &len, &node, &slen, &num, raw);
#else
    ptr = __xmlNodeGetFromCache(&nc, xpid->start, &len, &node, &slen, &num);
#endif
    if (ptr)
    {
        xid->len = len;
        xid->start = ptr;
        xid->name = node;
        xid->name_len = slen;
#ifdef XML_USE_NODECACHE
        /* unused for the cache but tested at the start of this function */
        if (len == 0) xid->len = 1;
        xid->node = nc;
#endif
        ret = xid;
    }
    else if (slen == 0)
    {
        xmlErrorSet(xpid, node, len);
    }

    return ret;
}

XML_API xmlId* XML_APIENTRY
xmlNodeGetPos(const xmlId *pid, xmlId *id, const char *element, size_t num)
{
   return __xmlNodeGetPos(pid, id, element, num, 0);
}

XML_API xmlId* XML_APIENTRY
xmlNodeGetPosRaw(const xmlId *pid, xmlId *id, const char *element, size_t num)
{
   return __xmlNodeGetPos(pid, id, element, num, 1);
}


XML_API xmlId* XML_APIENTRY
xmlNodeCopyPos(const xmlId *pid, xmlId *id, const char *element, size_t num)
{
    struct _xml_id *xpid = (struct _xml_id *)pid;
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t len, slen;
    char *ptr, *node;
    void *ret = 0;
    void *nc;

    assert(xpid != 0);
    assert(xid != 0);
    assert(element != 0);

    len = xpid->len;
    slen = strlen(element);
    node = (char *)element;
    nc = cacheNodeGet(xpid);
#ifndef XML_USE_NODECACHE
    ptr = __xmlNodeGet(nc, xpid->start, &len, &node, &slen, &num, 0);
#else
    ptr = __xmlNodeGetFromCache(&nc, xpid->start, &len, &node, &slen, &num);
#endif
    if (ptr)
    {
        const int rsize = sizeof(struct _root_id);
        const int nsize = sizeof(struct _xml_id);
#ifndef XML_NONVALIDATING
        const int esize = sizeof(struct _zeroxml_error);
#else
        const int esize = 0;
#endif
        struct _xml_id *nid;
        size_t new_len;

        new_len = STRUCT_ALIGN(slen + len);
        nid = malloc(nsize + new_len + rsize + esize);
        if (nid)
        {
            struct _root_id *rid;
            char *nptr = (char *)nid;

            /* node name and attributes */
            memcpy(nptr, xid, nsize);
            nptr += nsize;

            nid->name = nptr;
            nid->name_len = slen;

            memcpy(nptr, node, slen);
            nid->start = nptr+slen;	/* contents of this node */
            nid->len = len;

            memcpy(nptr+slen, ptr, len);
            nptr += new_len;		/* 4 bytes aligned */

            /* set up a new root node */
            rid = (struct _root_id *)nptr;
#ifndef XML_NONVALIDATING
            nid->root = rid;
            memcpy(nptr, xid->root, rsize);
            nptr += rsize;
#endif

            rid->start = nid->start;
            rid->len = nid->len;
            rid->fd = 0;

#ifndef XML_NONVALIDATING
            rid->info = (struct _zeroxml_error *)nptr;
            memset(nptr, 0, esize);
#endif
#ifdef XML_USE_NODECACHE
            /* unused for the cache but tested at the start of this function */
            if (len == 0) nid->len = 1;
            nid->node = nc;
#endif
            ret = nid;
        }
    }
    else if (slen == 0)
    {
        xmlErrorSet(xpid, node, len);
    }

    return ret;
}

static char *
__xmlGetString(const xmlId *id, char raw)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *str = 0;

    assert(xid != 0);

    if (xid->len)
    {
        size_t len;
        char *ps;

        ps = xid->start;
        len = xid->len;
        __xmlPrepareData(&ps, &len, raw);
        if (len)
        {
            str = malloc(len+1);
            if (str)
            {
                memcpy(str, ps, len);
                *(str+len) = 0;
            }

            else
            {
                xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
            }
        }
    }

    return str;
}

XML_API char * XML_APIENTRY
xmlGetString(const xmlId *id)
{
   return __xmlGetString(id, 0);
}

XML_API char* XML_APIENTRY
xmlGetStringRaw(const xmlId *id)
{
   return __xmlGetString(id, 1);
}

XML_API size_t XML_APIENTRY
xmlCopyString(const xmlId *id, char *buffer, size_t buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t ret = 0;

    assert(xid != 0);
    assert(buffer != 0);
    assert(buflen > 0);

    *buffer = '\0';
    if (xid->len)
    {
        size_t len;
        char *ps;

        ps = xid->start;
        len = xid->len;
        __xmlPrepareData(&ps, &len, 0);
        if (len)
        {
            if (len >= buflen)
            {
                len = buflen-1;
                xmlErrorSet(xid, 0, XML_TRUNCATE_RESULT);
            }
            memcpy(buffer, ps, len);
            *(buffer+len) = 0;
        }
        ret = len;
    }

    return ret;
}

XML_API int XML_APIENTRY
xmlCompareString(const xmlId *id, const char *s)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int ret = -1;

    assert(xid != 0);
    assert(s != 0);

    if (xid->len && (strlen(s) > 0))
    {
        size_t len;
        char *ps;

        ps = xid->start;
        len = xid->len;
        __xmlPrepareData(&ps, &len, 0);
        ret = strncasecmp(ps, s, len);
    }

    return ret;
}

XML_API char * XML_APIENTRY
xmlNodeGetString(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *str = 0;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        char *ptr, *node = (char *)path;
        size_t slen = strlen(node);
        size_t len = xid->len;
        void *nc;

        nc = cacheNodeGet(xid);
        ptr = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (ptr && len)
        {
            __xmlPrepareData(&ptr, &len, 0);
            str = malloc(len+1);
            if (str)
            {
                memcpy(str, ptr, len);
                *(str+len) = '\0';
            }
            else
            {
                xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
            }
        }
        else
        {
            xmlErrorSet(xid, node, len);
        }
    }

    return str;
}

XML_API size_t XML_APIENTRY
xmlNodeCopyString(const xmlId *id, const char *path, char *buffer, size_t buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t ret = 0;

    assert(xid != 0);
    assert(path != 0);
    assert(buffer != 0);
    assert(buflen > 0);

    *buffer = '\0';
    if (xid->len)
    {
        char *p, *node = (char *)path;
        size_t slen = strlen(node);
        size_t len = xid->len;
        void *nc;

        nc = cacheNodeGet(xid);
        p = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (p)
        {
            __xmlPrepareData(&p, &len, 0);
            if (len)
            {
                if (len >= buflen)
                {
                    len = buflen-1;
                    xmlErrorSet(xid, 0, XML_TRUNCATE_RESULT);
                }

                memcpy(buffer, p, len);
                *(buffer+len) = '\0';
            }
            ret = len;
        }
        else if (slen == 0)
        {
            xmlErrorSet(xid, node, len);
        }
    }

    return ret;
}

XML_API int XML_APIENTRY
xmlNodeCompareString(const xmlId *id, const char *path, const char *s)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int ret = -1;

    assert(xid != 0);
    assert(path != 0);
    assert(s != 0);

    if (xid->len && (strlen(s) > 0))
    {
        char *node, *str, *ps;
        size_t len, slen;
        void *nc;

        len = xid->len;
        slen = strlen(path);
        node = (char *)path;
        nc = cacheNodeGet(xid);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str)
        {
            ps = str;
            __xmlPrepareData(&ps, &len, 0);
            ret = strncasecmp(ps, s, len);
        }
        else if (slen == 0)
        {
            xmlErrorSet(xid, node, len);
        }
    }

    return ret;
}

XML_API int XML_APIENTRY
xmlGetBool(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int li = 0;

    assert(xid != 0);

    if (xid->len)
    {
        char *end = xid->start + xid->len;
        li = __xmlDecodeBoolean(xid->start, end);
    }

    return li;
}

XML_API int XML_APIENTRY
xmlNodeGetBool(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int li = 0;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        size_t len, slen;
        char *str, *node;
        void *nc;

        len = xid->len;
        slen = strlen(path);
        node = (char *)path;
        nc = cacheNodeGet(xid);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str)
        {
            char *end = str+len;
            li = __xmlDecodeBoolean(str, end);
        }
        else if (slen == 0)
        {
            xmlErrorSet(xid, node, len);
        }
    }

    return li;
}

XML_API long int XML_APIENTRY
xmlGetInt(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    long int li = __XML_NONE;

    assert(xid != 0);

    if (xid->len)
    {
        char *end = xid->start + xid->len;
        li = __xml_strtol(xid->start, &end, 10);
    }

    return li;
}

XML_API long int XML_APIENTRY
xmlNodeGetInt(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    long int li = __XML_NONE;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        size_t len, slen;
        char *str, *node;
        void *nc;

        len = xid->len;
        slen = strlen(path);
        node = (char *)path;
        nc = cacheNodeGet(xid);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str)
        {
            char *end = str+len;
            li = __xml_strtol(str, &end, 10);
        }
        else if (slen == 0)
        {
            xmlErrorSet(xid, node, len);
        }
    }

    return li;
}

XML_API double XML_APIENTRY
xmlGetDouble(const xmlId *id)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    double d = __XML_FPNONE;

    assert(xid != 0);

    if (xid->len)
    {
        char *end = xid->start + xid->len;
        d = strtod(xid->start, &end);
    }

    return d;
}

XML_API double XML_APIENTRY
xmlNodeGetDouble(const xmlId *id, const char *path)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    double d = __XML_FPNONE;

    assert(xid != 0);
    assert(path != 0);

    if (xid->len)
    {
        size_t len, slen;
        char *str, *node;
        void *nc;

        len = xid->len;
        slen = strlen(path);
        node = (char *)path;
        nc = cacheNodeGet(xid);
        str = __xmlNodeGetPath(&nc, xid->start, &len, &node, &slen);
        if (str)
        {
            char *end = str+len;
            d = strtod(str, &end);
        }
        else if (slen == 0)
        {
            xmlErrorSet(xid, node, len);
        }
    }

    return d;
}

XML_API xmlId* XML_APIENTRY
xmlMarkId(const xmlId *id)
{
    struct _xml_id *xmid = 0;

    assert(id != 0);

    xmid = malloc(sizeof(struct _xml_id));
    if (xmid)
    {
        struct _root_id *xrid = (struct _root_id *)id;
        if (xrid->name == 0)
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
        else
        {
            memcpy(xmid, id, sizeof(struct _xml_id));
        }
    }
    else
    {
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
    size_t len;
    return __xmlAttributeGetDataPtr(xid, name, &len) ? -1 : 0;
}

XML_API double XML_APIENTRY
xmlAttributeGetDouble(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    double ret = __XML_FPNONE;
    size_t len;
    char *ptr;

    ptr = __xmlAttributeGetDataPtr(xid, name, &len);
    if (ptr)
    {
        char *eptr = ptr+len;
        ret = strtod(ptr, &eptr);
    }
    return ret;
}

XML_API int XML_APIENTRY
xmlAttributeGetBool(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int ret = 0;
    size_t len;
    char *ptr;

    ptr = __xmlAttributeGetDataPtr(xid, name, &len);
    if (ptr)
    {
        char *eptr = ptr+len;
        ret = __xmlDecodeBoolean(ptr, eptr);
    }
    return ret;
}

XML_API long int XML_APIENTRY
xmlAttributeGetInt(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    long int ret = __XML_NONE;
    size_t len;
    char *ptr;

    ptr = __xmlAttributeGetDataPtr(xid, name, &len);
    if (ptr)
    {
        char *eptr = ptr+len;
        ret = __xml_strtol(ptr, &eptr, 10);
    }

    return ret;
}

XML_API char * XML_APIENTRY
xmlAttributeGetString(const xmlId *id, const char *name)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *ret = 0;
    size_t len;
    char *ptr;

    ptr = __xmlAttributeGetDataPtr(xid, name, &len);
    if (ptr)
    {
        ret = malloc(len+1);
        if (ret)
        {
            memcpy(ret, ptr, len);
            *(ret+len) = '\0';
        }
        else {
            xmlErrorSet(xid, 0, XML_OUT_OF_MEMORY);
        }
    }

    return ret;
}

XML_API size_t XML_APIENTRY
xmlAttributeCopyString(const xmlId *id, const char *name,
                                        char *buffer, size_t buflen)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    size_t len, ret = 0;
    char *ptr;

    assert(buffer != 0);
    assert(buflen > 0);

    ptr = __xmlAttributeGetDataPtr(xid, name, &len);
    if (ptr)
    {
        size_t restlen = len;
        if (restlen >= buflen)
        {
            restlen = buflen-1;
            xmlErrorSet(xid, ptr, XML_TRUNCATE_RESULT);
        }

        memcpy(buffer, ptr, restlen);
        *(buffer+restlen) = 0;
        ret = restlen;
    }
    return ret;
}

XML_API int XML_APIENTRY
xmlAttributeCompareString(const xmlId *id, const char *name, const char *s)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    int ret = -1;
    size_t len;
    char *ptr;

    assert(s != 0);

    ptr = __xmlAttributeGetDataPtr(xid, name, &len);
    if (ptr && (len == strlen(s))) {
        ret = strncasecmp(ptr, s, len);
    }

    return ret;
}


#ifndef XML_NONVALIDATING
XML_API size_t XML_APIENTRY
xmlErrorGetNo(const xmlId *id, int clear)
{
    size_t ret = 0;

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

            ret = err->err_no;
            if (clear) err->err_no = 0;
        }
    }

    return ret;
}

XML_API size_t XML_APIENTRY
xmlErrorGetLineNo(const xmlId *id, int clear)
{
    size_t ret = 0;

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
            char *ps = rid->start;
            char *pe = err->pos;
            char *new;

            ret++;
            while (ps<pe)
            {
                new = memchr(ps, '\n', pe-ps);
                if (new) ret++;
                else break;
                ps = new+1;
            }

            if (clear) err->err_no = 0;
        }
    }

    return ret;
}

XML_API size_t XML_APIENTRY
xmlErrorGetColumnNo(const xmlId *id, int clear)
{
    size_t ret = 0;

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
            char *ps = rid->start;
            char *pe = err->pos;
            char *new;

            while (ps<pe)
            {
                new = memchr(ps, '\n', pe-ps);
                new = memchr(ps, '\n', pe-ps);
                if (new) ret++;
                else break;
                ps = new+1;
            }
            ret = pe-ps;

            if (clear) err->err_no = 0;
        }
    }

    return ret;
}

XML_API const char * XML_APIENTRY
xmlErrorGetString(const xmlId *id, int clear)
{
    char *ret = 0;

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
            if (XML_NO_ERROR <= err->err_no && err->err_no < XML_MAX_ERROR)
            {
                ret = (char *)__zeroxml_error_str[err->err_no];
            }
            else
            {
                ret = "incorrect error number.";
            }

            if (clear) err->err_no = 0;
        }
    }

    return ret;
}

#else

XML_API int XML_APIENTRY
xmlErrorGetNo(const xmlId *id, int clear)
{
    return XML_NO_ERROR;
}

XML_API size_t XML_APIENTRY
xmlErrorGetLineNo(const xmlId *id, int clear)
{
    return 0;
}

XML_API size_t XML_APIENTRY
xmlErrorGetColumnNo(const xmlId *id, int clear)
{
    return 0;
}

XML_API const char * XML_APIENTRY
xmlErrorGetString(const xmlId *id, int clear)
{
    return "error detection was not enabled at compile time: no error.";
}
#endif

/* -------------------------------------------------------------------------- */

#ifndef XML_NONVALIDATING
static const char *__zeroxml_error_str[XML_MAX_ERROR] =
{
    "no error.",
    "unable to allocate enough memory.",
    "unable to open file for reading.",
    "requested node name is invalid.",
    "unexpected end of section.",
    "buffer too small to hold all data, truncating.",
    "incorrect comment section.",
    "bad information block.",
    "incompatible opening tag for element.",
    "missing or invalid closing tag for element.",
    "missing or invalid opening quote for attribute.",
    "missing or invalid closing quote for attribute."
};
#endif

static char *
__xmlAttributeGetDataPtr(const struct _xml_id *id, const char *name, size_t *len)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    char *ret = 0;

    assert(xid != 0);
    assert(name != 0);
    assert(len != 0);

    *len = 0;
    if (xid->name_len)
    {
        size_t slen = strlen(name);
        char *ps, *pe;

        assert(xid->start > xid->name);

        ps = xid->name + xid->name_len + 1;
        pe = xid->start - 1;
        while (ps<pe)
        {
            while ((ps<pe) && isspace(*ps)) ps++;
            if (((size_t)(pe-ps) > slen) && (strncasecmp(ps, name, slen) == 0))
            {
                ps += slen;
                if ((ps<pe) && (*ps == '='))
                {
                    char *start;

                    ps++;
                    if (*ps == '"' || *ps == '\'') ps++;
                    else
                    {
                        xmlErrorSet(xid, ps, XML_ATTRIB_NO_OPENING_QUOTE);
                        return 0;
                    }

                    start = ps-1;
                    while ((ps<pe) && (*ps != *start)) ps++;
                    if (ps<pe)
                    {
                        start++;
                        ret = start;
                        *len = ps-start;
                    }
                    else
                    {
                        xmlErrorSet(xid, ps, XML_ATTRIB_NO_CLOSING_QUOTE);
                        return 0;
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

    return ret;
}

static int
__xmlDecodeBoolean(const char *start, const char *end)
{
    int rv = 0;
    char *ptr;

    ptr = (char *)end;
    if (__xml_strtol(start, &ptr, 10) == 0)
    {
        size_t len = end-start;
        if (!strncasecmp(start, "on", len) || !strncasecmp(start, "yes", len)
            || !strncasecmp(start, "true", len))
        {
            rv = -1;
        }
    }
    else {
        rv = -1;
    }

    return rv;
}


char *
__xmlNodeGetPath(void **nc, const char *start, size_t *len, char **name, size_t *nlen)
{
    char *path;
    char *ret = 0;

    assert(start != 0);
    assert(len != 0);
    assert(name != 0);
    assert(*name != 0);
    assert(nlen != 0);
    assert(*nlen != 0);

    if (*len == 0 || *nlen > *len) {
        return ret;
    }

    path = *name;
    if (*path == '/') path++;
    if (*path != '\0')
    {
        size_t num, blocklen, pathlen, nodelen;
        char *node;

        node = path;
        pathlen = strlen(path);
        path = strchr(node, '/');

        if (!path) nodelen = pathlen;
        else nodelen = path++ - node;

        num = 0;
        blocklen = *len;

#ifndef XML_USE_NODECACHE
        ret = __xmlNodeGet(nc, start, &blocklen, &node, &nodelen, &num, 0);
#else
        ret = __xmlNodeGetFromCache(nc, start, &blocklen, &node, &nodelen, &num);
#endif
        if (ret)
        {
            if (path)
            {
                ret = __xmlNodeGetPath(nc, ret, &blocklen, &path, &pathlen);
                *name = path;
                *len = blocklen;
                *nlen = pathlen;
            }
            else
            {
                *name = node;
                *nlen = nodelen;
                *len = blocklen;
            }
        }
    }

    return ret;
}

char *
__xmlNodeGet(void *nc, const char *start, size_t *len, char **name, size_t *rlen, size_t *nodenum, char raw)
{
    char *open_element = *name; // *cdata
    char *element, *start_tag=0;
    char *new, *cur, *ne, *ret = 0;
    size_t restlen, elementlen;
    size_t open_len = *rlen;
    int found;
    size_t num;
    void *nnc = 0;

    assert(start != 0);
    assert(len != 0);
    assert(name != 0);
    assert(rlen != 0);
    assert(nodenum != 0);

    if (open_len == 0 || *name == 0) {
        SET_ERROR_AND_RETURN((char *)start,(char *)start,XML_INVALID_NODE_NAME);
    }

    if (*rlen > *len)
    {
        *rlen = 0;
        *name = start_tag;
        *len = XML_NO_ERROR;    /* element not found, no real error */
        return 0;
    }

    found = 0;
    num = *nodenum;
    restlen = *len;
    cur = (char *)start;
    ne = cur + restlen;

#ifdef XML_USE_NODECACHE
    cacheInitLevel(nc);
#endif

    /* search for an opening tag */
    while ((new = memchr(cur, '<', restlen)) != 0)
    {
        size_t len_remaining;
        char *rptr;

        if (*(new+1) == '/')		/* end of section */
        {
            *len -= restlen;
            break;
        }

        new++;
        restlen -= new-cur;
        cur = new;

        if (*cur == '!') /* comment */
        {
            char *start = cur;
            size_t blocklen = restlen;
            new = __xmlProcessCDATA(&start, &blocklen, raw);
            if (!new && start && open_len) {		/* CDATA */
                SET_ERROR_AND_RETURN((char *)start,cur,XML_INVALID_COMMENT);
            }

            restlen -= new-cur;
            cur = new;
            continue;
        }
        else if (*cur == '?') /* info block */
        {
            new = __xmlInfoProcess(cur, restlen);
            if (!new) {
                SET_ERROR_AND_RETURN((char *)start,cur, XML_INVALID_INFO_BLOCK);
            }

            restlen -= new-cur;
            cur = new;
            continue;
        }

        /*
        * get element name and a pointer to after the opening tag
        */
        element = *name;
        elementlen = *rlen;
        len_remaining = restlen;
        rptr = __xml_memncasecmp(cur, &restlen, &element, &elementlen);
        if (rptr) 			/* requested element was found */
        {
            new = rptr;
            if (found == num)
            {
                ret = new;
                open_len = elementlen;
                start_tag = element;
            }
            else start_tag = 0;
        }
        else				/* different element name was foud */
        {
            new = cur + (len_remaining - restlen);
            if (new >= ne) {
                SET_ERROR_AND_RETURN((char *)start,cur, XML_UNEXPECTED_EOF);
            }

            element = *name;
        }

#ifdef XML_USE_NODECACHE
        nnc = cacheNodeNew(nc);
#endif

        if (*(new-2) == '/')				/* e.g. <test/> */
        {
            cur = new;
            if (rptr)
            {
#ifdef XML_USE_NODECACHE
                cacheDataSet(nnc, element, elementlen, rptr, 0);
#endif
                if (found == num)
                {
                    open_element = start_tag;
                    *len = 0;
                }
                found++;
            }
            continue;
        }

        /*
        * get the next xml tag
        */
        /* restlen -= new-cur; not necessary because of __xml_memncasecmp */
        cur = new;
        new = memchr(cur, '<', restlen);
        if (!new) {
            SET_ERROR_AND_RETURN((char *)start,cur, XML_ELEMENT_NO_CLOSING_TAG);
            return 0;
        }

        new++;
        restlen -= new-cur;
        cur = new;
        if (*cur == '!')				/* comment, CDATA */
        {
            char *start = cur;
            size_t blocklen = restlen;
            new = __xmlProcessCDATA(&start, &blocklen, raw);
            if (!new) {
                SET_ERROR_AND_RETURN((char *)start,cur, XML_INVALID_COMMENT);
            }

            restlen -= new-cur;
            cur = new;

            /*
            * look for the closing tag of the cascading block
            */
            new = memchr(cur, '<', restlen);
            if (!new) {
                SET_ERROR_AND_RETURN((char *)start,cur, XML_ELEMENT_NO_CLOSING_TAG);
            }

            new++;
            restlen -= new-cur;
            cur = new;
        }

        if (*cur == '/')		/* closing tag of leaf node found */
        {
            if (!strncasecmp(new+1, element, elementlen))
            {
                char *pe = new+restlen;
                char *ps = new+elementlen+1;
                while ((ps<pe) && isspace(*ps)) ps++;

#ifdef XML_USE_NODECACHE
                cacheDataSet(nnc, element, elementlen, rptr, new-rptr-1);
#endif
                if (*ps != '>') {
                    SET_ERROR_AND_RETURN((char *)start,new+1, XML_ELEMENT_NO_CLOSING_TAG);
                }

                if (found == num)
                {
                    if (start_tag)
                    {
                        *len = new-ret-1;
                        open_element = start_tag;
                        start_tag = 0;
                    }
                    else	/* element not found, no real error */
                    {
                        *rlen = 0;
                        *name = start_tag;
                        *len = XML_NO_ERROR;
                        return 0;
                    }
                }
                found++;
            }

            new = memchr(cur, '>', restlen);
            if (!new) {
                SET_ERROR_AND_RETURN((char *)start,cur, XML_ELEMENT_NO_CLOSING_TAG);
            }

            restlen -= new-cur;
            cur = new;
            continue;
        }

        /* no leaf node, continue */
        if (*cur != '/')			/* cascading tag found */
        {
            char *node = "*";
            size_t slen = restlen+1; /* due to cur-1 below*/
            size_t nlen = 1;
            size_t pos = -1;

            /*
            * recursively walk the xml tree from here
            */
            new = __xmlNodeGet(nnc, cur-1, &slen, &node, &nlen, &pos, 0);
            if (!new)
            {
                if (nlen == 0)		/* error upstream */
                {
                    *rlen = nlen;
                    *name = node;
                    *len = slen;
                    return 0;
                }

                if (slen == restlen) {
                    SET_ERROR_AND_RETURN((char *)start,cur, XML_UNEXPECTED_EOF);
                }

                slen--;
                new = cur + slen;
                restlen -= slen;
            }
            else restlen -= slen;

            /*
             * look for the closing tag of the cascading block
             */
            cur = new;
            new = memchr(cur, '<', restlen);
            if (!new) {
                SET_ERROR_AND_RETURN((char *)start,cur, XML_ELEMENT_NO_CLOSING_TAG);
            }

            new++;
            restlen -= new-cur;
            cur = new;
        }

        if (*cur == '/')			/* closing tag found */
        {
            if (!strncasecmp(new+1, element, elementlen))
            {
                char *pe = new+restlen;
                char *ps = new+elementlen+1;
                while ((ps<pe) && isspace(*ps)) ps++;

                if (*ps != '>') {
                    SET_ERROR_AND_RETURN((char *)start,new+1, XML_ELEMENT_NO_CLOSING_TAG);
                }

#ifdef XML_USE_NODECACHE
                cacheDataSet(nnc, element, elementlen, rptr, new-rptr-1);
#endif
                if (found == num)
                {
                    if (start_tag)
                    {
                        *len = new-ret-1;
                        open_element = start_tag;
                        start_tag = 0;
                    }
                    else { /* report error */
                        SET_ERROR_AND_RETURN((char *)start,new, XML_ELEMENT_NO_OPENING_TAG);
                    }
                }
                found++;
            }

            new = memchr(cur, '>', restlen);
            if (!new) {
                SET_ERROR_AND_RETURN((char *)start,cur, XML_ELEMENT_NO_CLOSING_TAG);
            }

            restlen -= new-cur;
            cur = new;
        }
        else {
            SET_ERROR_AND_RETURN((char *)start,cur, XML_ELEMENT_NO_CLOSING_TAG);
        }

    } /* while */

    if (found == 0)
    {
        ret = 0;
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

    return ret;
}

char *
__xmlProcessCDATA(char **start, size_t *len, char raw)
{
    char *cur, *new = NULL;
    size_t restlen = *len;

    cur = *start;
    if (!raw && (restlen > 6) && (*(cur+1) == '-'))            /* comment */
    {
        new = __xmlCommentSkip(cur, restlen);
        if (new)
        {
            *start = new;
            *len = 0;
        }
        return new;
    }

    if (restlen < 12) return 0;                        /* ![CDATA[ ]]> */

    cur = *start;
    if (!raw && memcmp(cur, "![CDATA[", 8) == 0)
    {
        *len = 0;
        *start = cur+8;
        cur += 8;
        restlen -= 8;

        new = __xml_memmem(cur, restlen, "]]>", 3);
        if (new)
        {
           *len = new - *start;
           new += 3;
        }
    }

    return new;
}

char *
__xmlCommentSkip(const char *start, size_t len)
{
    char *cur, *new;

    if (len < 7) return 0;				 /* !-- --> */

    cur = (char *)start;
    new = 0;

    if (memcmp(cur, "!--", 3) == 0)
    {
        cur += 3;
        len -= 3;
        new = __xml_memmem(cur, len, "-->", 3);
        if (new) {
           new += 3;
        }
    }
    return new;
}

char *
__xmlInfoProcess(const char *start, size_t len)
{
    char *cur, *new, *ret = 0;

    cur = (char *)start;
    new = 0;

    // Note: http://www.w3.org/TR/REC-xml/#sec-guessing
    if (*cur == '?')
    {
        char *element;

        if (len < 3) return 0;				/* <? ?> */

        cur++;
        len--;

        element = "?>";
        new = __xml_memncasestr(cur, len, element);
        if (new)
        {
            len = new - cur;
            new += strlen("?>");
            ret = new;

            element = "encoding=\"";
            new = __xml_memncasestr(cur, len, element);
            if (new)
            {
               size_t elementlen = strlen(element);
               len -= elementlen;
               cur = new + elementlen;
               new = memchr(cur, '"', len);
               if (new)
               {
                   size_t slen = strlen("UTF-8");
                   size_t restlen = new-cur;
                   char *str = "UTF-8";
                   if (!__xml_memncasecmp(cur, &restlen, &str, &slen)
#ifdef HAVE_LANGINFO_H
                       && !strcmp(nl_langinfo(CODESET), "UTF-8")
#endif
                      )
                   {
#ifdef HAVE_LOCALE_H
//                     setlocale(LC_CTYPE, "C.UTF-8");
#endif
                   }
               }
            }
        }
    }

    return ret;
}

static void
__xmlPrepareData(char **start, size_t *blocklen, char raw)
{
    size_t len = *blocklen;
    char *pe, *ps = *start;

    if (len > 1)
    {
        pe = ps + len-1;
        while ((ps<pe) && isspace(*ps)) ps++;
        while ((pe>ps) && isspace(*pe)) pe--;
        len = (pe-ps)+1;
    }
    else if (isspace(*(ps+1))) len--;

    /* CDATA or comment */
    if ((len >= 2) && !strncmp(ps, "<!", 2))
    {
        char *start = ps+1;
        size_t blocklen = len-1;
        if (blocklen >= 6)                    /* !-- --> */
        {
            char *new = __xmlProcessCDATA(&start, &len, raw);
            if (new)
            {
                ps = start;
                pe = ps + len;

                while ((ps<pe) && isspace(*ps)) ps++;
                while ((pe>ps) && isspace(*pe)) pe--;
                len = (pe-ps);
            }
        }
    }

    *start = ps;
    *blocklen = len;
}

#ifndef XML_NONVALIDATING
void
__xmlErrorSet(const struct _xml_id *id, const char *pos, size_t err_no)
{
    struct _xml_id *xid = (struct _xml_id *)id;
    struct _root_id *rid;

    assert(xid != 0);

    if (xid->name) rid = xid->root;
    else rid = (struct _root_id *)xid;

    assert(rid != 0);
    assert(rid->name == 0);
    if (rid->info == 0) {
        rid->info = malloc(sizeof(struct _zeroxml_error));
    }

    if (rid->info)
    {
        struct _zeroxml_error *err = rid->info;
        err->pos = (char *)pos;
        err->err_no = err_no;
    }
}
#endif

static long
__xml_strtol(const char *str, char **end, int base)
{
    size_t len = *end - str;

    if (len > 2 && str[1] == 'x' && (str[0] == '0' || str[0] == '\\')) {
        return strtol(str+2, end, 16);
    }
    return strtol(str, end, base);
}

static char *
__xml_memmem(const char *cur, size_t len, const char *str, size_t slen)
{
    char *rv = NULL;

    if (str && str[0] != '\0')
    {
        char *new;
        do
        {
            new = memchr(cur, str[0], len);
            if (new)
            {
                len -= (new-cur);
                if ((len >= 3) && (memcmp(new, str, slen) == 0))
                {
                    rv = new;
                    break;
                }
                cur = new+1;
                len -= cur-new;
            }
            else break;
        }
        while (new && (len > 2));
    }
    return rv;
}

static char *
__xml_memncasestr(const char *s, size_t slen, const char *find)
{
   char c, sc;
   size_t len;

   if ((c = *find++) != '\0')
   {
      len = strlen(find);
      do
      {
         do
         {
            if (slen-- < 1 || (sc = *s++) == '\0') {
               return (NULL);
            }
         }
         while (toupper(sc) != toupper(c));

         if (len > slen) {
            return (NULL);
         }
      }
      while (strncasecmp(s, find, len) != 0);
      s--;
   }
   return ((char *)s);
}

#define NOCASECMP(a,b)  ( ((a)^(b)) & 0xdf )
static void *
__xml_memncasecmp(const char *haystack, size_t *haystacklen,
                    char **needle, size_t *needlelen)
{
    char *rptr = 0;

    if (haystack && needle && needlelen && (*needlelen > 0)
        && (*haystacklen >= *needlelen))
    {
        char *hs = (char *)haystack;
        char *ns;
        size_t i;

        ns = *needle;

        /* search for everything */
        if ((*ns == '*') && (*needlelen == 1))
        {
            char *he = hs + *haystacklen;

            while ((hs < he) && !isspace(*hs) && (*hs != '>')) hs++;
            if (*(hs-1) == '/') hs--;

            *needle = (char *)haystack;
            *needlelen = hs - haystack;

            ns = memchr(hs, '>', he-hs);
            if (ns) hs = ns+1;
            else hs = he;

            rptr = hs;
        }
        else
        {
            size_t nlen = *needlelen;
            char *he = hs + *haystacklen;

            for (i=0; i<nlen; i++)
            {
                if (NOCASECMP(*hs,*ns) && (*ns != '?')) break;
                if (isspace(*hs) || (*hs == '/') || (*hs == '>')) break;
                hs++;
                ns++;
            }
            if (!isspace(*hs) && (*hs != '/') && (*hs != '>')) i=0;

            if (i == nlen)
            {
                *needle = (char *)haystack;
                *needlelen = hs - haystack;

                ns = memchr(hs, '>', he-hs);
                if (ns) hs = ns+1;
                else hs = he;

                rptr = hs;
            }
            else /* not found */
            {
                while((hs < he) && !isspace(*hs) && (*hs != '>')) hs++;
                if (*(hs-1) == '/') hs--;

                *needle = (char *)haystack;
                *needlelen = hs - haystack;

                ns = memchr(hs, '>', he-hs);
                if (ns) hs = ns+1;
                else hs = he;
            }
        }

        *haystacklen -= hs - haystack;
    }

    return rptr;
}

#ifdef WIN32
/* Source:
 * https://mollyrocket.com/forums/viewtopic.php?p=2529
 */

void *
simple_mmap(int fd, size_t length, SIMPLE_UNMMAP *un)
{
    HANDLE f;
    HANDLE m;
    void *p;

    f = (HANDLE)_get_osfhandle(fd);
    if (!f) return (void *)-1;

    m = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!m) return (void *)-1;

    p = MapViewOfFile(m, FILE_MAP_READ, 0, 0, 0);
    if (!p)
    {
        CloseHandle(m);
        return (void *)-1;
    }

    if (un)
    {
        un->m = m;
        un->p = p;
    }

    return p;
}

void
simple_unmmap(void *addr, size_t len, SIMPLE_UNMMAP *un)
{
    UnmapViewOfFile(un->p);
    CloseHandle(un->m);
}
#endif
