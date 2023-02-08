
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

#define TESTINT(p, a, b) \
  if (a == b) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed\n\t%i should be %i.\n", p, (int)a, (int)b);

#define TESTFLOAT(p, a, b) \
  if (a == b) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed\n\t%.1f should be %0.1f.\n", p, a, b);

#define TESTSTR(p, fn, a, b) \
  if (!fn(a, b)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,a,b);

#define TESTSTRNCASE(p, fn, a, b, c) \
  if (!(a)) printf("NULL returned at line %i\n", __LINE__); \
  else if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,a,b);

#define TESTSTRCMP(p, fn, a, b, c) \
  if (!(a)) printf("NULL returned at line %i\n", __LINE__); \
  else if (!fn(a, b)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,b,c);

#define TESTCMP(p, fn, a, b, c) \
  if (!(a)) printf("NULL returned at line %i\n", __LINE__); \
  else if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,b,c);

#define TESTPTR(p, a, b) \
  if (a == b) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed\n\t'%p' should be '%p'.\n", p, a, b);

#define TESTATTR(p, fn, a, b, c) \
  if (!(a)) printf("NULL returned at line %i\n", __LINE__); \
  else if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\tattribute '%i' differs from '%s'\n",p,b,c);

