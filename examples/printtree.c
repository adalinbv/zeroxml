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
 *          this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in the
 *          documentation and/or other materials provided with the distribution.
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
 * ----------------------------------------------------------------------------- * ALTERNATIVE B - Public Domain (www.unlicense.org)
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

#include <types.h>
#include "xml.h"

#define MAX_BUF	4096
void print_xml(xmlId*, char*, unsigned int);

int main(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("usage: printtree <filename>\n\n");
    }
    else
    {
        xmlId *rid;

#ifdef HAVE_LOCALE_H
        setlocale(LC_ALL, "");
#endif

        rid = xmlOpen(argv[1]);
        if (xmlErrorGetNo(rid, 0) != XML_NO_ERROR)
        {
             printf("%s\n", xmlErrorGetString(rid, 1));
        }
        else if (rid)
        {
            int i, num, res;
            xmlId *xid;

            xid = xmlMarkId(rid);
            num = xmlNodeGetNum(xid, "*");
            for (i=0; i<num; i++)
            {
                char name[MAX_BUF+1] = "/";
                if (xmlNodeGetPos(rid, xid, "*", i) != 0)
                {
                    if (xmlNodeTest(xid, XML_COMMENT)) continue;

                    res = xmlNodeCopyName(xid, name+1, MAX_BUF-1);
                    print_xml(xid, name, res+1);
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

void print_xml(xmlId *id, char *name, unsigned int len)
{
    xmlId *xid = xmlMarkId(id);
    unsigned int num, i, q;

    num = xmlNodeGetNum(xid, "*");
    name[len] = 0;
    for (i=0; i<xmlAttributeGetNum(xid); ++i)
    {
        char attr[256], value[256];

        xmlAttributeCopyName(xid, (char *)&attr, 256, i);
        if (xmlErrorGetNo(xid, 0) != XML_NO_ERROR) {
            printf("Error for xmlAttributeCopyName: %s\n",
                    xmlErrorGetString(xid, XML_TRUE));
        }

        xmlAttributeCopyString(xid, attr, value, 256);
        printf("%s[@%s] = \"%s\"\n", name, attr, value);
        if (xmlErrorGetNo(xid, 0) != XML_NO_ERROR) {
            printf("Error for xmlAttributeCopyString: %s\n",
                    xmlErrorGetString(xid, XML_TRUE));
        }
    }

    if (num == 0)
    {
        char s[MAX_BUF+1];
        xmlCopyString(xid, s, MAX_BUF);
        printf("%s = \"%s\"\n", name, s);
        if (xmlErrorGetNo(xid, 0) != XML_NO_ERROR) {
            printf("Error for xmlCopyString: %s\n",
                    xmlErrorGetString(xid, XML_TRUE));
        }
    }
    else
    {
        unsigned int i;

        name[len++] = '/';
        for (i=0; i<num; i++)
        {
            if (xmlNodeGetPos(id, xid, "*", i) != 0)
            {
                if (xmlNodeTest(xid, XML_COMMENT)) continue;

                unsigned int res, i = MAX_BUF - len;
                if ((res = xmlNodeCopyName(xid, (char *)&name[len], i)) != NULL)
                {
                    unsigned int index = xmlAttributeGetInt(xid, "n");
                    if (index)
                    {
                        unsigned int pos = len+res;

                        name[pos++] = '[';
                        i = snprintf((char *)&name[pos], 4096-pos, "%i", index);
                        name[pos+i] = ']';
                        res += i+2;
                    }
                }
                else {
                    printf("Error for xmlNodeCopyName: %s\n",
                            xmlErrorGetString(xid, XML_TRUE));
                }
                print_xml(xid, name, len+res);
            }
            else printf("error\n");
        }
    }
}
