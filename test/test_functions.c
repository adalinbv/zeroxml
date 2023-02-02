
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

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
    long l;

    s = "33";
    e = s + strlen(s);
    l = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE, "should be XML_TRUE");

    s = "0";
    e = s + strlen(s);
    i = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE, "should be XML_FALSE");

    s = "true";
    e = s + strlen(s);
    l = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE, "should be XML_TRUE");

    s = "false";
    e = s + strlen(s);
    i = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE, "should be XML_FALSE");

    s = "on";
    e = s + strlen(s);
    l = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE, "should be XML_TRUE");

    s = "off";
    e = s + strlen(s);
    i = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE, "should be XML_FALSE");

    s = "yes";
    e = s + strlen(s);
    l = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_TRUE)", s);
    TESTINT(p, l, XML_TRUE, "should be XML_TRUE");

    s = "no";
    e = s + strlen(s);
    i = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_FALSE)", s);
    TESTINT(p, i, XML_FALSE, "should be XML_FALSE");

    s = "happy";
    e = s + strlen(s);
    i = __zeroxml_strtob(s, e);
    snprintf(buf, BUFLEN, "__zeroxml_strtob with '%s' (XML_BOOL_NONE)", s);
#ifdef XML_NONEVALUE
    TESTINT(p, i, XML_BOOL_NONE, "should be XML_BOOL_NONE");
#else
    TESTINT(p, i, XML_FALSE, "should be XML_FALSE");
#endif


    s = "1024</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with decimal '%s'", s);
    TESTINT(p, l, 1024, "should be 1024");

    s = "0d1024</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with decimal '%s'", s);
    TESTINT(p, l, 1024, "should be 1024");

    s = "01000</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with octal '%s'", s);
    TESTINT(p, l, 512, "should be 512");

    s = "0o1000</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with octal '%s'", s);
    TESTINT(p, l, 512, "should be 512");

    s = "0x200</";
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with hexadecimal '%s'", s);
    TESTINT(p, l, 512, "should be 512");

    s = "0b100000000</" ;
    e = s + strlen(s);
    l = __zeroxml_strtol(s, &e, 10);
    snprintf(buf, BUFLEN, "__zeroxml_strtol with binary '%s'", s);
    TESTINT(p, l, 256, "should be 256");


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
    TESTPTR(p, c, NULL, "should be NULL");


    s = "<?xml?><"cTb">1</"cTb">";
    c = __zeroxml_memncasestr(s, strlen(s), cTb);
    snprintf(buf, BUFLEN, "__zeroxml_memncasestr with '%s' and '%s'", s, ctb);
    TESTSTRNCASE(p, !strncmp, c, ctb, strlen(ctb));

    s = "<?xml?><"cTb">1</"cTb">";
    c = __zeroxml_memncasestr(s, strlen(s), nTB);
    snprintf(buf, BUFLEN, "__zeroxml_memncasestr with '%s' and '%s'", s, nTB);
    TESTPTR(p, c, NULL, "should be NULL");


    cs = "<"ctb">1</"ctb">";
    b = "*"; nl = strlen(b);
    snprintf(buf, BUFLEN, "__zeroxml_memncasecmp with '%s' and '%s'", cs, b);
    hl = strlen(++cs);
    c = __zeroxml_memncasecmp(&cs, &hl, &b, &nl);
    TESTSTRNCASE(p, !strncmp, c, ctb, strlen(ctb));

    cs = "<"cTb">1</"cTb">";
    b = ctb; nl = strlen(b);
    snprintf(buf, BUFLEN, "__zeroxml_memncasecmp with '%s' and '%s'", cs, b);
    hl = strlen(++cs);
    c = __zeroxml_memncasecmp(&cs, &hl, &b, &nl);
#ifndef XML_CASE_INSENSITIVE
    TESTPTR(p, c, NULL, "should be NULL");
#else
    TESTSTRNCASE(p, !strncmp, c, ctb, strlen(ctb));
#endif

    cs = "<"ctb">1</"ctb">";
    b = nTB; nl = strlen(b);
    snprintf(buf, BUFLEN, "__zeroxml_memncasecmp with '%s' and '%s'", cs, b);
    hl = strlen(++cs);
    c = __zeroxml_memncasecmp(&cs, &hl, &b, &nl);
    TESTPTR(p, c, NULL, "should be NULL");
}
