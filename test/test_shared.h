
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "xml.h"

#define BUFLEN		4096

#define PRINT_ERROR_AND_EXIT(id) \
  do { \
      const char *errstr = xmlErrorGetString(id, 0); \
      int column = xmlErrorGetColumnNo(id, 0); \
      int lineno = xmlErrorGetLineNo(id, 1); \
      printf("\n\tXML error at line %i, column %i: %s", lineno, column,errstr);\
      printf("\n\t\tat line: %i\n", __LINE__); \
      exit(-1); \
  } while(0);

#define TESTINT(p, a, b, c) \
  if (a == b) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed\n\t'%i' %s.\n", p, b, c);

#define TESTFLOAT(p, a, b, c) \
  if (a == b) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed\n\t'%f' %s.\n", p, b, c);

#define TESTSTR(p, fn, a, b) \
  if (!fn(a, b)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,a,b);

#define TESTSTRN(p, fn, a, b, c) \
  if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,a,b);

#define TESTSTRCMP(p, fn, a, b, c) \
  if (!fn(a, b)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,b,c);

#define TESTCMP(p, fn, a, b, c) \
  if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,b,c);

#define TESTPTR(p, a, b, c) \
  if (a == b) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed\n\t'%p' %s.\n", p, b, c);

#define TESTATTR(p, fn, a, b, c) \
  if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\tattribute '%i' differs from '%s'\n",p,b,c);

#define PRINT(s, b, c) { \
  int l1 = (b), l2 = (c); \
  if (s) { \
    int q, len = l2; \
    if (l1 < l2) len = l1; \
    if (len < 50000) { \
        printf("(%i) '", len); \
        for (q=0; q<len; q++) printf("%c", s[q]); \
        printf("'\n"); \
    } else printf("Length (%i) seems too large at line %i\n",len, __LINE__); \
  } else printf("NULL pointer at line %i\n", __LINE__); \
}

