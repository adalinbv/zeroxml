/*
 * Copyright (C) 2008-2011 by Erik Hofman.
 * Copyright (C) 2009-2011 by Adalin B.V.
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
 */

#include <stdio.h>
#include <malloc.h>

#include <types.h>
#include "xml.h"

void print_xml(void *, char *, unsigned int);

int main(int argc, char **argv)
{
  if (argc < 1)
  {
    printf("usage: printtree <filename>\n\n");
  }
  else
  {
    void *rid;

    rid = xmlOpen(argv[1]);
    if (xmlErrorGetNo(rid, 0) != XML_NO_ERROR)
    {
       printf("%s\n", xmlErrorGetString(rid, 1));
    }
    else if (rid)
    {
      unsigned int i, num;
      void *xid;
 
      xid = xmlMarkId(rid);
      num = xmlNodeGetNum(xid, "*");
      for (i=0; i<num; i++)
      {
        if (xmlNodeGetPos(rid, xid, "*", i) != 0)
        {
          char name[4096] = "";
          print_xml(xid, (char *)&name, 0);
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

void print_xml(void *id, char *name, unsigned int len)
{
  void *xid = xmlMarkId(id);
  unsigned int num;
  
  num = xmlNodeGetNum(xid, "*");
  if (num == 0)
  {
    char *s;
    s = xmlGetString(xid);
    if (s)
    {
      name[len] = 0;
      printf("%s = %s\n", name, s);
      free(s);
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
        unsigned int res, i = 4096 - len;
        res = xmlNodeCopyName(xid, (char *)&name[len], i);
        if (res)
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
        print_xml(xid, name, len+res);
      }
      else printf("error\n");
    }
  }
}
