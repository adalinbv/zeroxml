
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_shared.h"
#include "xml.h"

#define ROOTNODE	"/Configuration"
#define OUTPUTNODE	ROOTNODE"/output"
#define MENUNODE	OUTPUTNODE"/menu"
#define SAMPLENODE	OUTPUTNODE"/sample"
#define TESTNODE	"test"
#define TESTPATH	OUTPUTNODE"/"TESTNODE
#define NAMENODE	"name"
#define SPEAKERNODE	"speaker"
#define MENUPATH	MENUNODE"/"NAMENODE
#define SAMPLEPATH	SAMPLENODE"/"TESTNODE
#define BACKENDPATH	ROOTNODE"/backend/"NAMENODE
#define AUDIOFRAMEPATH	ROOTNODE"/audioframe"
#define NONVALIDNODE	MENUNODE"/"TESTNODE

#ifdef WIN32
# define NEWLINE	"\r\n"
#else
# define NEWLINE	"\n"
#endif
#define STRING1		"* Traffic, # taxiing to runway (."

int test(xmlId *rid)
{
    char buf[BUFLEN+1];
    xmlId *pid, *nid;
    char *p, *s;
    double f;
    int i, q;

    printf("\n");
    if (!rid)
    {
        printf("Invalid XML-id\n");
        return XML_TRUE;
    }

    p = "xmlNodeTest for "ROOTNODE"/nasal/YF23";
    i = xmlNodeTest(rid, ROOTNODE"/nasal/YF23");
    TESTINT(p, i, 1, "should be true");

    nid = xmlNodeGet(rid, OUTPUTNODE);
    p = "xmlNodeGetNum for "OUTPUTNODE"/boolean";
    i = xmlNodeGetNum(nid, "boolean");
    TESTINT(p, i, 5, "should be 5");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean: (0)";
    i = xmlNodeGetBool(rid, OUTPUTNODE"/boolean");
    TESTINT(p, i, 0, "should be false");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[1] (XML_TRUE)";
    i = xmlNodeGetBool(nid, "boolean[1]");
    TESTINT(p, i, XML_TRUE, "should be true");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[2] (on)";
    i = xmlNodeGetBool(nid, "boolean[2]");
    TESTINT(p, i, XML_TRUE, "should be true");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[3] (true)";
    i = xmlNodeGetBool(nid, "boolean[3]");
    TESTINT(p, i, XML_TRUE, "should be true");

    p = "xmlNodeGetBool for "OUTPUTNODE"/boolean[4] (yes)";
    i = xmlNodeGetBool(nid, "boolean[4]");
    TESTINT(p, i, XML_TRUE, "should be true");

    p = "xmlNodeGetInt for "OUTPUTNODE"/boolean[1] (-1)";
    i = xmlNodeGetInt(nid, "boolean[1]");
    TESTINT(p, i, -1, "should be -1");

    p = "xmlNodeGetInt for "OUTPUTNODE"/binary (0b1011)";
    i = xmlNodeGetInt(nid, "binary");
    TESTINT(p, i, 11, "should be 11");

    p = "xmlNodeGetInt for "OUTPUTNODE"/octal (070374)";
    i = xmlNodeGetInt(nid, "octal[0]");
    TESTINT(p, i, 28924, "should be 28924");

    p = "xmlNodeGetInt for "OUTPUTNODE"/octal (0o70374)";
    i = xmlNodeGetInt(nid, "octal[1]");
    TESTINT(p, i, 28924, "should be 28924");

    p = "xmlNodeGetInt for "OUTPUTNODE"/hexadecimal (0x7FA32)";
    i = xmlNodeGetInt(nid, "hexadecimal");
    TESTINT(p, i, 522802, "should be 522802");

    p = "xmlNodeGetDouble for "OUTPUTNODE"/interval-hz (20.0)";
    f = xmlNodeGetDouble(nid, "interval-hz");
    TESTFLOAT(p, f, 20.0, "should be 20.0");
    xmlFree(nid);

    p = "xmlNodeGetString for /*/*[1]/test";
    s = xmlNodeGetString(rid , "/*/*[1]/test");
    TESTPTR(p, s, NULL, "should be empty");
    xmlFree(s);

    p = "xmlGetString for "TESTPATH;
    pid = xmlNodeGet(rid, TESTPATH);
    if (!pid) PRINT_ERROR_AND_EXIT(rid);
    s = xmlGetString(pid);
    TESTPTR(p, s, NULL, "should be empty");
    xmlFree(pid);
    xmlFree(s);

    p = "xmlNodeGet for non-valid node "NONVALIDNODE;
    pid = xmlNodeGet(rid, NONVALIDNODE);
    TESTPTR(p, pid, NULL, "should be NULL");
    xmlErrorGetNo(rid, 1); /* clear the error */
    xmlFree(pid);

    pid = xmlNodeGet(rid, SAMPLEPATH);
    if (!pid) PRINT_ERROR_AND_EXIT(rid);

    p = "xmlNodeGetName for "SAMPLEPATH;
    s = xmlNodeGetName(pid);
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcoll, s, TESTNODE);

    p = "xmlNodeCopyName for "SAMPLEPATH;
    i = xmlNodeCopyName(pid, buf, BUFLEN);
    if (!i) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcoll, buf, s);

    p = "xmlNodeCompareName for "SAMPLEPATH;
    TESTSTRCMP(p, xmlNodeCompareName, pid, s, buf);
    xmlFree(s);

    xmlCopyString(pid, buf, BUFLEN);
    p = "xmlNodeCopyString against xmlGetString";
    s = xmlGetString(pid);
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);

    p = "xmlGetStringRaw against xmlNodeCopyString";
    nid = xmlNodeGet(rid, MENUPATH);
    if (!nid) PRINT_ERROR_AND_EXIT(rid);
    s = xmlGetStringRaw(nid);
    if (!s) PRINT_ERROR_AND_EXIT(nid);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(nid);

    p = "xmlGetBool for "OUTPUTNODE"/boolean[2] (on)";
    nid = xmlNodeGet(rid, OUTPUTNODE"/boolean[2]");
    i = xmlGetBool(nid);
    TESTINT(p, i, XML_TRUE, "should be true");
    xmlFree(nid);

    p = "xmlGetInt for "OUTPUTNODE"/boolean[1] (-1)";
    nid = xmlNodeGet(rid, OUTPUTNODE"/boolean[1]");
    i = xmlGetInt(nid);
    TESTINT(p, i, -1, "should be -1");
    xmlFree(nid);

    p = "xmlGetDouble for "OUTPUTNODE"/interval-hz (20.0)";
    nid = xmlNodeGet(rid, OUTPUTNODE"/interval-hz");
    f = xmlGetDouble(nid);
    TESTFLOAT(p, f, 20.0, "should be 20.0");
    xmlFree(nid);


    p = "xmlCopyString against xmlGetString";
    xmlCopyString(pid, buf, BUFLEN);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);

    p = "xmlCopyString against xmlCompareString";
    s = xmlGetString(pid);
    TESTSTRCMP(p, xmlCompareString, pid, s, buf);
    xmlFree(s);

    nid = xmlNodeGet(rid, MENUNODE);
    if (!nid) PRINT_ERROR_AND_EXIT(rid);

    p = "xmlCopyString against xmlNodeCompareString";
    TESTCMP(p, xmlNodeCompareString, nid, NAMENODE, buf);

    p = "xmlCopyString against xmlNodeGetString";
    s = xmlNodeGetString(nid, NAMENODE);
    if (!s) printf("failed.\n\t'%s' not found.\n", NAMENODE);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);
    xmlFree(nid);
    xmlFree(pid);

    p = "xmlAttributeExists on / for 'pan'";
    i = xmlAttributeExists(rid, "pan");
    TESTINT(p, i, XML_FALSE, "should be false");

    pid = xmlNodeGet(rid, BACKENDPATH);
    if (!pid) PRINT_ERROR_AND_EXIT(rid);

    p = "xmlAttributeGetNum for "BACKENDPATH;
    i = xmlAttributeGetNum(pid);
    TESTINT(p, i, 1, "should be 1");

    p = "xmlAttributeCopyString against xmlAttributeCompareString";
    xmlAttributeCopyString(pid, "type", buf, BUFLEN);
    TESTCMP(p, xmlAttributeCompareString, pid, "type", buf);

    nid = xmlNodeGet(rid, AUDIOFRAMEPATH);
    if (!nid) PRINT_ERROR_AND_EXIT(pid);

    p = "xmlAttributeExists on "AUDIOFRAMEPATH" for 'pan'";
    i = xmlAttributeExists(nid, "pan");
    TESTINT(p, i, XML_TRUE, "should be true");

    p = "xmlAttributeCompareName for "AUDIOFRAMEPATH" for 'pan'";
    TESTATTR(p, xmlAttributeCompareName, nid, 0, "pan");

    p = "xmlAttributeGetInt on "AUDIOFRAMEPATH" for 'emitters'";
    i = xmlAttributeGetInt(nid, "emitters");
    TESTINT(p, i, 64, "should be 64");

    p = "xmlAttributeGetDouble on "AUDIOFRAMEPATH" for 'pan'";
    f = xmlAttributeGetDouble(nid, "pan");
    TESTFLOAT(p, f, -0.2, "should be -0.2");

    xmlFree(nid);

    p = "xmlNodeCopy against the original";
    nid = xmlNodeCopy(rid, MENUNODE);
    if (!nid) PRINT_ERROR_AND_EXIT(pid);

    s = xmlAttributeGetString(pid, "type");
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);
    xmlClose(nid);

    p = "xmlAttributeCopyString against xmlAttributeGetString";
    s = xmlAttributeGetString(pid, "type");
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);

    p = "xmlAttributeGetName for attribute 0";
    s = xmlAttributeGetName(pid, 0);
    TESTSTR(p, strcoll, s, "type");

    p = "xmlAttributeCopyName against xmlAttributeGetName";
    i = xmlAttributeCopyName(pid, buf, BUFLEN, 0);
    if (!i) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);

    xmlFree(pid);

    pid = xmlNodeGet(rid, SAMPLEPATH);
    if (!pid) PRINT_ERROR_AND_EXIT(rid);

    p = "xmlNodeCopyString against xmlNodeCopyString";
    xmlNodeCopyString(rid , MENUPATH, buf, BUFLEN);
    s = xmlNodeGetString(pid, MENUPATH);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);

    p = "xmlCompareString against a fixed string";
    TESTSTRCMP(p, xmlCompareString, pid, buf, STRING1);

    p = "xmlGetString against xmlNodeCopyString";
    s = xmlGetString(pid);
    if (!s) PRINT_ERROR_AND_EXIT(pid);
    TESTSTR(p, strcoll, s, buf);
    xmlFree(s);

    p = "xmlCopyString gainst a fixed string";
    xmlCopyString(pid, buf, BUFLEN);
    TESTSTR(p, strcoll, buf, "* Traffic, # taxiing to runway (.");
    xmlFree(pid);

    pid = xmlNodeGet(rid, OUTPUTNODE);
    if (!pid) PRINT_ERROR_AND_EXIT(pid);

    p = "xmlMarkId and xmlNodeGetNum for "OUTPUTNODE"/"SPEAKERNODE;
    nid = xmlMarkId(pid);
    if (!nid) PRINT_ERROR_AND_EXIT(pid);

    i = xmlNodeGetNum(nid, SPEAKERNODE);
    TESTINT(p, i, 2, "should be 2");

    p = buf;
    for (q=0; q<i; ++q)
    {
        xmlId *sid = xmlNodeGetPos(pid, nid, SPEAKERNODE, q);
        if (!sid) PRINT_ERROR_AND_EXIT(pid);

        snprintf(buf, BUFLEN, "%s/%s/channel[%i]", OUTPUTNODE, SPEAKERNODE, q);
        TESTINT(p, q, (int)xmlNodeGetInt(sid, "channel"), "mismatch");

        // used later by xmlNodeCopyPos
        if (q == 1) s = xmlGetString(sid);
    }
    xmlFree(nid);

    // xmlNodeCopyPos
    p = "xmlNodeCopyPos for "OUTPUTNODE"/"SPEAKERNODE"[1]";
    nid = xmlMarkId(pid);
    if (!nid) PRINT_ERROR_AND_EXIT(pid);
    do
    {
        xmlId *cid;
        char *cs;

        cid = xmlNodeCopyPos(pid, nid, SPEAKERNODE, 1);
        if (!cid) PRINT_ERROR_AND_EXIT(pid);

        cs = xmlGetString(cid);
        if (cs == NULL) PRINT_ERROR_AND_EXIT(cid);

        TESTSTR(p, strcoll, s, cs);
        xmlFree(s);

        p = "- xmlAttributeExists on / for 'n'";
        i = xmlAttributeExists(cid, "n");
        TESTINT(p, i, XML_FALSE, "should be false");

        p = "- xmlNodeGetInt for /test/node (3)";
        i = xmlNodeGetInt(cid, "/test/node");
        TESTINT(p, i, 3, "should be 3");       

        xmlFree(cs);
        xmlClose(cid);
    } while(0);
    xmlFree(nid);
    xmlFree(pid);

    if (xmlErrorGetNo(rid, 0) != XML_NO_ERROR)
    {
        const char *errstr = xmlErrorGetString(rid, 0);
        int column = xmlErrorGetColumnNo(rid, 0);
        int lineno = xmlErrorGetLineNo(rid, 1);

        printf("Error at line %i, column %i: %s\n", lineno, column, errstr);
    }
    printf("\n");

    return 0;
}
