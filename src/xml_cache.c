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
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
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

#include <sys/types.h>
#include <assert.h>

#include "xml.h"
#include "api.h"

#ifndef XML_USE_NODECACHE

const xmlCacheId*
cacheNodeGet(const xmlId *id) {
    return 0;
}

#else

# ifndef NDEBUG
#  define PRINT(a, b, c) { \
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
# endif

/* number of pointers to allocate for every block increase */
# define NODE_BLOCKSIZE		16

struct _xml_node
{
    const struct _xml_node *parent;
    const char *name;
    size_t name_len;
    const char *data;
    size_t data_len;
    const xmlCacheId **node;
    size_t no_nodes;
    size_t first_free;
};

const char*
__xmlNodeGetFromCache(const xmlCacheId **nc, const char *start, size_t *len,
                     const char **element, size_t *elementlen , size_t *nodenum)
{
    struct _xml_node *cache;
    size_t num = *nodenum;
    const char *name = *element;
    const char *rv = 0;

    assert(nc != 0);
 
    cache = (struct _xml_node *)*nc;
    assert(cache != 0);

    assert((cache->first_free > num) || (cache->first_free == 0));

    if (cache->first_free == 0) /* leaf node */
    {
        rv = cache->data;
        *len = cache->data_len;
        *element = cache->name;
        *elementlen = cache->name_len;
        *nodenum = 0;
    }
    else if (*name == '*')
    {
        const struct _xml_node *node = cache->node[num];
        *nc = (xmlCacheId*)node;
        rv = node->data;
        *len = node->data_len;
        *element = node->name;
        *elementlen = node->name_len;
        *nodenum = cache->first_free;
    }
    else
    {
        size_t namelen = *elementlen;
        size_t i, pos = 0;

        for (i=0; i<cache->first_free; i++)
        {
             const struct _xml_node *node = cache->node[i];

             assert(node);

             if ((node->name_len == namelen) &&
                 (!strncasecmp(node->name, name, namelen)))
             {
                  if (pos == num)
                  {
                       *nc = (xmlCacheId*)node;
                       rv = node->data;
                       *element = node->name;
                       *elementlen = node->name_len;
                       *len = node->data_len;
                       *nodenum = cache->first_free;
                       break;
                  }
                  pos++;
             }
        }
    }

    return rv;
}


const xmlCacheId*
cacheInit()
{
    return calloc(1, sizeof(struct _xml_node));
}

void
cacheInitLevel(const xmlCacheId *nc)
{
    struct _xml_node *cache = (struct _xml_node *)nc;

    assert(cache != 0);

    cache->node = calloc(NODE_BLOCKSIZE, sizeof(struct _xml_node *));
    cache->no_nodes = NODE_BLOCKSIZE;
}

void
cacheFree(const xmlCacheId *nc)
{
    struct _xml_node *cache = (struct _xml_node *)nc;

    assert(nc != 0);

    if (cache->first_free)
    {
        const struct _xml_node **node = cache->node;
        size_t i = 0;

        while(i < cache->first_free) {
             cacheFree((xmlCacheId*)node[i++]);
        }

        free(node);
    }
    free(cache);
}

const xmlCacheId*
cacheNodeGet(const xmlId *id)
{
    const struct _xml_id *xid = (const struct _xml_id *)id;
    const xmlCacheId *cache = 0;

    assert(xid != 0);

    if (xid->name) {
        cache = xid->node;
    }
    else
    {
        struct _root_id *rid = (struct _root_id *)xid;
        cache = rid->node;
    }

    return cache;
}

const xmlCacheId*
cacheNodeNew(const xmlCacheId *nc)
{
    struct _xml_node *cache = (struct _xml_node *)nc;
    struct _xml_node *rv = 0;
    size_t i = 0;

    assert(nc != 0);

    i = cache->first_free;
    if (i == cache->no_nodes)
    {
        size_t size, no_nodes;
        void *p;

        no_nodes = cache->no_nodes + NODE_BLOCKSIZE;
        size = no_nodes * sizeof(struct _xml_node *);
        p = realloc(cache->node, size);
        if (!p) return 0;

        cache->node = p;
        cache->no_nodes = no_nodes;
    }

    rv = calloc(1, sizeof(struct _xml_node));
    if (rv) rv->parent = cache;
    cache->node[i] = rv;
    cache->first_free++;

    return rv;
}

void
cacheDataSet(const xmlCacheId *n, const char *name, size_t namelen, const char *data, size_t datalen)
{
    struct _xml_node *node = (struct _xml_node *)n;

    assert(node != 0);
    assert(name != 0);
    assert(namelen != 0);
    assert(data != 0);

    node->name = name;
    node->name_len = namelen;
    node->data = data;
    node->data_len = datalen;
}

#endif
