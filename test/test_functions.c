
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>

#include "test_shared.h"
#include "xml.c"

#define cTb	"_cTb5"
#define ctb	"_ctb5"
#define nTB	"_nTB9"

int main(int argc, char **argv)
{
    static char buf[BUFLEN+1];
    char *s, *e, *p = buf;
    const char *cs, *c, *b;
    int i, hl, nl;
    xmlId *rid;
    double d;
    long l;

    s = "xmlOpen";
    rid = xmlInitBuffer("<x/>", 4);
    if (!rid)
    {
        printf("Error initializing the buffer.\n");
        return -1;
    }

    s = "33";
    e = s + strlen(s);
    l = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE);

    s = "0";
    e = s + strlen(s);
    i = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE);

    s = "true";
    e = s + strlen(s);
    l = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE);

    s = "false";
    e = s + strlen(s);
    i = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE);

    s = "on";
    e = s + strlen(s);
    l = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE);

    s = "off";
    e = s + strlen(s);
    i = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE);

    s = "yes";
    e = s + strlen(s);
    l = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE);

    s = "no";
    e = s + strlen(s);
    i = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE);

    s = "happy";
    e = s + strlen(s);
    i = __zeroxml_strtob(rid, s, e, XML_BOOL_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_BOOL_NONE)", s);
    TESTINT(p, i, XML_BOOL_NONE);

    s = "1024</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10, XML_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with decimal '%s'", s);
    TESTINT(p, l, 1024);

    s = "0d1024</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10, XML_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with decimal '%s'", s);
    TESTINT(p, l, 1024);

    s = "01000</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10, XML_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with octal '%s'", s);
    TESTINT(p, l, 512);

    s = "0o1000</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10, XML_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with octal '%s'", s);
    TESTINT(p, l, 512);

    s = "0x200</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10, XML_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with hexadecimal '%s'", s);
    TESTINT(p, l, 512);

    s = "0b100000000</" ;
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10, XML_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with binary '%s'", s);
    TESTINT(p, l, 256);

    s = "happy" ;
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10, XML_NONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with string '%s' (XML_NONE)", s);
    TESTINT(p, l, XML_NONE);


    s = "3.1415926536</";
    e = s + strlen(s);
    d = __zeroxml_strtod(s, &e, XML_FPNONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtod with double '%s'", s);
    TESTFLOAT(p, d, 3.1415926536);

    s = "314e-17</";
    e = s + strlen(s);
    d = __zeroxml_strtod(s, &e, XML_FPNONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtod with double '%s'", s);
    TESTFLOAT(p, d, 314e-17);

    s = "happy" ;
    e = s + strlen(s);
    d = __zeroxml_strtod(s, &e, XML_FPNONE);
    snprintf(buf, BUFLEN, "__zeroxml_strtod with string '%s' (XML_FPNONE)", s);
    TESTINT(p, isnan(d), isnan(d));


    s = "<?xml?><"ctb">1</"ctb">";
    c = __zeroxml_memmem(s, strlen(s), ctb, strlen(ctb));
    snprintf(buf, BUFLEN, "__zeroxml_memmem with '%s' and '%s'", s, ctb);
    TESTSTRNCASE(p, strncmp, c, ctb, strlen(ctb));

    s = "<?xml?><"cTb">1</"cTb">";
    c = __zeroxml_memmem(s, strlen(s), cTb, strlen(cTb));
    snprintf(buf, BUFLEN, "__zeroxml_memmem with '%s' and '%s'", s, ctb);
    TESTSTRNCASE(p, !strncmp, c, ctb, strlen(ctb));

    s = "<?xml?><"nTB">1</"nTB">";
    c = __zeroxml_memmem(s, strlen(s), cTb, strlen(cTb));
    snprintf(buf, BUFLEN, "__zeroxml_memmem with '%s' and '%s'", s, cTb);
    TESTPTR(p, c, NULL);


    s = "<?xml?><"cTb">1</"cTb">";
    c = __zeroxml_memncasestr(rid, s, strlen(s), cTb);
    snprintf(buf, BUFLEN, "__zeroxml_memncasestr with '%s' and '%s'", s, ctb);
    TESTSTRNCASE(p, !strncmp, c, ctb, strlen(ctb));

    s = "<?xml?><"cTb">1</"cTb">";
    c = __zeroxml_memncasestr(rid, s, strlen(s), nTB);
    snprintf(buf, BUFLEN, "__zeroxml_memncasestr with '%s' and '%s'", s, nTB);
    TESTPTR(p, c, NULL);


    cs = "<"ctb">1</"ctb">";
    b = "*"; nl = strlen(b);
    snprintf(buf, BUFLEN, "__zeroxml_memncasecmp with '%s' and '%s'", cs, b);
    hl = strlen(++cs);
    c = __zeroxml_memncasecmp(rid, &cs, &hl, &b, &nl);
    TESTSTRNCASE(p, !strncmp, c, ctb, strlen(ctb));

    cs = "<"cTb">1</"cTb">";
    b = ctb; nl = strlen(b);
    snprintf(buf, BUFLEN, "__zeroxml_memncasecmp with '%s' and '%s'", cs, b);
    hl = strlen(++cs);
    c = __zeroxml_memncasecmp(rid, &cs, &hl, &b, &nl);
#ifndef XML_CASE_INSENSITIVE
    TESTPTR(p, c, NULL);
#else
    TESTSTRNCASE(p, !strncmp, c, ctb, strlen(ctb));
#endif

    cs = "<"ctb">1</"ctb">";
    b = nTB; nl = strlen(b);
    snprintf(buf, BUFLEN, "__zeroxml_memncasecmp with '%s' and '%s'", cs, b);
    hl = strlen(++cs);
    c = __zeroxml_memncasecmp(rid, &cs, &hl, &b, &nl);
    TESTPTR(p, c, NULL);

    xmlClose(rid);
}
