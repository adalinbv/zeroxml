
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
#define TESTNODE	"test"
#define TESTPATH	OUTPUTNODE"/"TESTNODE
#define NAMENODE	"name"
#define MENUPATH	MENUNODE"/"NAMENODE
#define BACKENDPATH	ROOTNODE"/backend/"NAMENODE
#define NONVALIDNODE	MENUNODE"/"TESTNODE
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

#define TESTMENUPATH(p, fn, a, b, c) \
  if (!fn(a, b, c)) printf("Testing %-63s: succes\n", p); \
  else printf("Testing %-63s: failed.\n\t'%s' differs from '%s'\n",p,a,c);

/* TODO:
 *
 * xmlMarkId
 * xmlNodeGetPos
 * xmlNodeCopyPos
 * xmlGetStringRaw
 * xmlGetBool
 * xmlGetInt
 * xmlNodeGetInt
 * xmlAttributeGetInt
 * xmlGetDouble
 * xmlNodeGetDouble
 * xmlAttributeGetDouble
 * xmlAttributeExists
 */
int test(xmlId *root_id)
{
    char buf[BUFLEN+1];
    xmlId *pid, *nid;
    char *p, *s;
    int i;

    printf("\n");
    if (!root_id)
    {
        printf("Invalid XML-id\n");
        return -1;
    }

    p = "xmlNodeTest for "ROOTNODE"/nasal/YF23";
    i = xmlNodeTest(root_id, ROOTNODE"/nasal/YF23");
    TESTINT(p, i, 1, "should be true");

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
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTINT(p, s, NULL, "should be empty");
    xmlFree(s);

    p = "xmlGetString for "TESTPATH;
    pid = xmlNodeGet(root_id, TESTPATH);
    if (!pid) PRINT_ERROR_AND_EXIT(root_id);
    s = xmlGetString(pid);
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTINT(p, s, NULL, "should be empty");
    xmlFree(pid);
    xmlFree(s);

    p = "xmlNodeGet for non-valid node "NONVALIDNODE;
    pid = xmlNodeGet(root_id, NONVALIDNODE);
    TESTINT(p, pid, NULL, "should be NULL");
    xmlErrorGetNo(root_id, 1); /* clear the error */
    xmlFree(pid);

    pid = xmlNodeGet(root_id, MENUPATH);
    if (!pid) PRINT_ERROR_AND_EXIT(root_id);

    nid = xmlNodeGet(root_id, MENUNODE);
    if (!nid) PRINT_ERROR_AND_EXIT(root_id);

    p = "xmlNodeGetName for "MENUPATH;
    s = xmlNodeGetName(pid);
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, s, NAMENODE);

    p = "xmlNodeCopyName for "MENUPATH;
    i = xmlNodeCopyName(pid, buf, BUFLEN);
    if (!i) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, buf, s);

    p = "xmlNodeCompareName for "MENUPATH;
    TESTSTR(p, xmlNodeCompareName, pid, buf);

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
    TESTMENUPATH(p, xmlNodeCompareString, nid, NAMENODE, buf);

    p = "xmlCopyString against xmlNodeGetString";
    s = xmlNodeGetString(nid, NAMENODE);
    if (!s) printf("failed.\n\t'%s' not found.\n", NAMENODE);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);
    xmlFree(pid);

    pid = xmlNodeGet(root_id, BACKENDPATH);
    if (!pid) PRINT_ERROR_AND_EXIT(root_id);

    p = "xmlAttributeGetNum for "BACKENDPATH;
    i = xmlAttributeGetNum(pid);
    TESTINT(p, i, 1, "should be 1");

    p = "xmlAttributeCopyString against xmlAttributeCompareString";
    xmlAttributeCopyString(pid, "type", buf, BUFLEN);
    TESTMENUPATH(p, xmlAttributeCompareString, pid, "type", buf);

    xmlFree(nid);

    p = "xmlNodeCopy against the original";
    nid = xmlNodeCopy(root_id, MENUNODE);
    if (!nid) PRINT_ERROR_AND_EXIT(pid);
    s = xmlAttributeGetString(pid, "type");
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);
    xmlFree(nid);

    p = "xmlAttributeCopyString against xmlAttributeGetString";
    s = xmlAttributeGetString(pid, "type");
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);

    p = "xmlAttributeGetName for attribute 0";
    s = xmlAttributeGetName(pid, 0);
    TESTSTR(p, strcmp, s, "type");

    p = "xmlAttributeCopyName against xmlAttributeGetName";
    i = xmlAttributeCopyName(pid, buf, BUFLEN, 0);
    if (!i) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcmp, s, buf);
    xmlFree(s);

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
