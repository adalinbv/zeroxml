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

#include <stdio.h>
#include <malloc.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#include "xml.h"

int print_comment(xmlId *xid)
{
    int rv = 1;
    char *s;

    s = xmlGetStringRaw(xid);
    if (!s) {
        rv = 0;
    }
    else
    {
        printf("<!-- %s --", s);
        free(s);
    }
    return rv;
}

int print_xml(xmlId *id)
{
    static int level = 1;
    xmlId *xid = xmlMarkId(id);
    unsigned int num;
    int rv = 1;

    num = xmlNodeGetNum(xid, "*");
    if (num == 0)
    {
        char *s;
        s = xmlGetStringRaw(xid);
        if (!s) {
            rv = 0;
        }
        else
        {
            printf(">%s", s);
            free(s);
        }
    }
    else
    {
        /* recusively walk the sub-nodes */
        unsigned int i, j, q;
        for (i=0; i<num; i++)
        {
            if (xmlNodeGetPos(id, xid, "*", i) != 0)
            {
                char name[256];

                printf(">\n");
                for(q=0; q<level; q++) printf(" ");

                if (xmlNodeTest(xid, XML_COMMENT))
                {
                    print_comment(xid);
                    continue;
                }

                xmlNodeCopyName(xid, (char *)&name, 256);
                printf("<%s", name);

                /* print the nodes attributes */
                for (j=0; j<xmlAttributeGetNum(xid); ++j)
                {
                    char attr[256], value[256];
                    q = xmlAttributeCopyName(xid, (char *)&attr, 256, j);
                    if (q)
                    {
                        printf(" %s", attr);
                        q = xmlAttributeCopyString(xid, attr, value, 256);
                        if (q) printf("=\"%s\"", value);
                    }
                }

                level++;
                if (print_xml(xid)) {
                    printf("</%s", name);
                } else {
                    printf("/");
                }
                level--;
            }
            else printf("error\n");
        }
        printf(">\n");
        for(q=1; q<level; q++) printf(" ");
    }
    xmlFree(xid);
    return rv;
}


int main(int argc, char **argv)
{
#ifdef HAVE_LOCALE_H
    setlocale(LC_CTYPE, "");
#endif

    if (argc < 1)
    {
        printf("usage: printxml <filename>\n\n");
    }
    else
    {
        xmlId *rid;

        rid = xmlOpen(argv[1]);
        if (xmlErrorGetNo(rid, 0) != XML_NO_ERROR)
        {
             printf("%s\n", xmlErrorGetString(rid, 1));
        }
        else if (rid)
        {
            unsigned int i, num;
            xmlId *xid;

            printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n");

            xid = xmlMarkId(rid);
            num = xmlNodeGetNum(xid, "*");
            for (i=0; i<num; i++)
            {
                if (xmlNodeGetPos(rid, xid, "*", i) != 0)
                {
                    char name[256];

                    if (xmlNodeTest(xid, XML_COMMENT))
                    {
                        print_comment(xid);
                        printf(">\n");
                        continue;
                    }

                    xmlNodeCopyName(xid, (char *)&name, 256);
                    printf("<%s", name);
                    print_xml(xid);
                    printf("</%s>\n", name);
                }
            }
            free(xid);

            xmlClose(rid);
        }
        else
        {
            printf("Error while opening file for reading: '%s'\n", argv[1]);
        }
    }

    return 0;
}
