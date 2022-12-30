
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"

#define ROOTNODE	"/Configuration"
#define OUTPUTNODE	ROOTNODE"/output"
#define MENUNODE	OUTPUTNODE"/menu"
#define	LEAFNODE	"name"
#define PATH		MENUNODE"/"LEAFNODE
#define BUFLEN		4096

#define PRINT_ERROR_AND_EXIT(id) \
  if (xmlErrorGetNo(id, 0) != XML_NO_ERROR) { \
      const char *errstr = xmlErrorGetString(id, 0); \
      int column = xmlErrorGetColumnNo(id, 0); \
      int lineno = xmlErrorGetLineNo(id, 1); \
      printf("\n\tError at line %i, column %i: %s\n", lineno, column, errstr);\
      exit(-1); \
  }

#define TESTINT(p, a, b, c) \
  if (a == b) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed\n\t'%i' %s.\n", p, a, c);

#define TESTSTR(p, fn, a, b) \
  if (!fn(a, b)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,a,b);

#define TESTPATH(p, fn, a, b, c) \
  if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,a,c);

int test(xmlId *root_id)
{
    char buf[BUFLEN];
    xmlId *pid, *nid;
    char *p, *s;
    int i;

    printf("\n");
    if (!root_id)
    {
        printf("Invalid XML-id\n");
        return -1;
    }

    nid = xmlNodeGet(root_id, OUTPUTNODE);
    p = "xmlNodeGetNum for "OUTPUTNODE"/boolean";
    i = xmlNodeGetNum(nid, "boolean");
    TESTINT(p, i, 5, "should be 5");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean: (0)";
    i = xmlNodeGetBool(root_id, OUTPUTNODE"/boolean");
    TESTINT(p, i, 0, "should be false");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[1]: (-1)";
    i = xmlNodeGetBool(nid, "boolean[1]");
    TESTINT(p, i, -1, "should be true");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[2]: (on)";
    i = xmlNodeGetBool(nid, "boolean[2]");
    TESTINT(p, i, -1, "should be true");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[3]: (true)";
    i = xmlNodeGetBool(nid, "boolean[3]");
    TESTINT(p, i, -1, "should be true");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[4]: (yes)";
    i = xmlNodeGetBool(nid, "boolean[4]");
    TESTINT(p, i, -1, "should be true");

    p = "xmlNodeGetString for /*/*/test";
    s = xmlNodeGetString(root_id , "/*/*/test");
    TESTINT(p, s, NULL, "should be empty");
    xmlFree(s);

    p = "xmlGetString for /Configuration/output/test";
    pid = xmlNodeGet(root_id, "/Configuration/output/test");
    if (!pid) PRINT_ERROR_AND_EXIT(root_id);
    s = xmlGetString(pid);
    TESTINT(p, s, NULL, "should be empty");
    xmlFree(s);

    pid = xmlNodeGet(root_id, PATH);
    if (!pid) PRINT_ERROR_AND_EXIT(root_id);

    nid = xmlNodeGet(root_id, MENUNODE);
    if (!nid) PRINT_ERROR_AND_EXIT(root_id);

    xmlCopyString(pid, buf, BUFLEN);
    p = "xmlNodeCopyString against xmlGetString";
    s = xmlGetString(pid);
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, s, buf);

    p = "xmlCopyString against xmlGetString";
    xmlCopyString(pid, buf, BUFLEN);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);

    p = "xmlCopyString against xmlCompareString";
    TESTSTR(p, xmlCompareString, pid, buf);

    p = "xmlCopyString against xmlNodeCompareString";
    TESTPATH(p, xmlNodeCompareString, nid, LEAFNODE, buf);

    p = "xmlCopyString against xmlNodeGetString";
    s = xmlNodeGetString(nid, LEAFNODE);
    if (!s) printf("failed.\n\t'%s' not found.\n", LEAFNODE);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);
    xmlFree(pid);

    pid = xmlNodeGet(root_id, "/Configuration/backend/name");
    if (!pid) PRINT_ERROR_AND_EXIT(root_id);

    p = "xmlAttributeCopyString against xmlAttributeCompareString";
    xmlAttributeCopyString(pid, "type", buf, BUFLEN);
    TESTPATH(p, xmlAttributeCompareString, pid, "type", buf);

    p = "xmlAttributeCopyString against xmlAttributeGetString";
    s = xmlAttributeGetString(pid, "type");
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);

    xmlFree(nid);
    xmlFree(pid);

    pid = xmlNodeGet(root_id, "Configuration/output/sample/test");
    if (!pid) PRINT_ERROR_AND_EXIT(root_id);

    xmlNodeCopyString(root_id ,"Configuration/output/menu/name", buf, BUFLEN);
    p = "xmlCompareString against a fixed string";
    TESTSTR(p, xmlCompareString, pid, buf);

    s = xmlGetString(pid);
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, s, buf);

    p = "xmlCopyString gainst a fixed string";
    xmlCopyString(pid, buf, BUFLEN);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);

    xmlFree(pid);

    if (xmlErrorGetNo(root_id, 0) != XML_NO_ERROR)
    {
        const char *errstr = xmlErrorGetString(root_id, 0);
        int column = xmlErrorGetColumnNo(root_id, 0);
        int lineno = xmlErrorGetLineNo(root_id, 1);

        printf("Error at line %i, column %i: %s\n", lineno, column, errstr);
    }
    printf("\n");

    return 0;
}
