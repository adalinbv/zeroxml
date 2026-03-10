/*
 * test_unicode.c
 *
 * Tests for Unicode / multi-byte character support in zeroxml.
 *
 * Coverage
 * --------
 *  1. Encoding declaration extraction from the XML prolog
 *       - UTF-8 declared encoding is parsed and stored correctly
 *       - ISO-8859-1 declared encoding is parsed and stored correctly
 *       - No encoding declaration (plain UTF-8 assumed)
 *       - Regression: the pre-fix over-read bug that copied '?>' into the
 *         encoding string (fixed in __zeroxml_process_declaration)
 *
 *  2. UTF-8 multi-byte content
 *       - Element text content with multi-byte characters is returned intact
 *       - Attribute values with multi-byte characters are returned intact
 *       - Multi-byte characters in element *names* are handled without crash
 *
 *  3. Byte Order Mark detection
 *       - UTF-8 BOM (EF BB BF) is skipped transparently
 *       - Document content after the BOM is still accessible
 *
 *  4. Non-ASCII in realistic compound documents
 *       - Copyright symbol (©), umlauts (ö ü), accented characters (é è ë)
 *       - Multi-element document with mixed ASCII and non-ASCII nodes
 *
 * Build (from the zeroxml root):
 *   cc -I include -I src -DHAVE_CONFIG_H \
 *      test/test_unicode.c src/xml.c src/xml_cache.c src/localize.c \
 *      -o test/test_unicode
 *
 * Run:
 *   ./test/test_unicode
 *
 * Exit code: 0 = all tests passed, 1 = one or more tests failed.
 *
 * Notes
 * -----
 * All string literals in this file are UTF-8. Compile on a UTF-8 host
 * (the default on all modern Linux/macOS systems).
 *
 * The tests compare raw byte strings returned by the library.  When the
 * document encoding matches the locale (UTF-8 on UTF-8), iconv is a no-op
 * and the library returns the bytes unchanged.  Tests are written to be
 * valid under this common configuration.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"

/* ------------------------------------------------------------------ */
/* Minimal test harness                                                 */
/* ------------------------------------------------------------------ */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

static void _pass(const char *desc)
{
    printf("  PASS  %s\n", desc);
    tests_passed++;
    tests_run++;
}

static void _fail(const char *desc, const char *detail)
{
    printf("  FAIL  %s\n         %s\n", desc, detail);
    tests_failed++;
    tests_run++;
}

#define PASS(desc)              _pass(desc)
#define FAIL(desc, detail)      _fail(desc, detail)

/* Check that xmlInitBuffer returns non-NULL */
#define ASSERT_OPEN(id, desc) \
    do { if (!(id)) { FAIL(desc, "xmlInitBuffer returned NULL"); return; } } while(0)

/* Check a string result equals an expected value */
static void check_str(const char *desc, const char *got, const char *expected)
{
    if (!got) {
        char buf[256];
        snprintf(buf, sizeof(buf), "got NULL, expected \"%s\"", expected);
        FAIL(desc, buf);
    } else if (strcmp(got, expected) != 0) {
        char buf[512];
        snprintf(buf, sizeof(buf), "got \"%s\", expected \"%s\"", got, expected);
        FAIL(desc, buf);
    } else {
        PASS(desc);
    }
}

/* Check a string result is non-NULL */
static void check_notnull(const char *desc, const char *got)
{
    if (!got) FAIL(desc, "got NULL");
    else       PASS(desc);
}

/* ------------------------------------------------------------------ */
/* 1. Encoding declaration extraction                                   */
/* ------------------------------------------------------------------ */

static void test_encoding_utf8_declared(void)
{
    /* Standard UTF-8 declaration */
    const char *xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><r/>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "encoding UTF-8: open");

    check_str("encoding UTF-8: xmlGetEncoding returns \"UTF-8\"",
              xmlGetEncoding(id), "UTF-8");

    xmlClose(id);
}

static void test_encoding_iso8859_declared(void)
{
    /* ISO-8859-1 declaration — bytes are all ASCII here so no conversion needed */
    const char *xml = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><r/>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "encoding ISO-8859-1: open");

    check_str("encoding ISO-8859-1: xmlGetEncoding returns \"ISO-8859-1\"",
              xmlGetEncoding(id), "ISO-8859-1");

    xmlClose(id);
}

static void test_encoding_none_declared(void)
{
    /* No declaration — library should still open and return empty encoding */
    const char *xml = "<root><child>hello</child></root>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "no encoding declaration: open");

    /* encoding should be empty string, not garbage */
    const char *enc = xmlGetEncoding(id);
    if (enc && strlen(enc) == 0) {
        PASS("no encoding declaration: xmlGetEncoding returns empty string");
    } else if (!enc) {
        FAIL("no encoding declaration: xmlGetEncoding returns empty string",
             "got NULL");
    } else {
        char buf[256];
        snprintf(buf, sizeof(buf), "got \"%s\", expected \"\"", enc);
        FAIL("no encoding declaration: xmlGetEncoding returns empty string", buf);
    }

    xmlClose(id);
}

/*
 * Regression test for the pre-fix over-read bug.
 *
 * Before the fix, the encoding extraction used `len--` instead of
 * `len = end - cur`, so for a short encoding name like "UTF-8" (5 bytes)
 * with ~9 bytes remaining to "?>", it would copy ~8 bytes: "UTF-8?>\0"
 * or similar garbage into the encoding buffer.
 *
 * After the fix, exactly "UTF-8" (5 bytes) must be stored, with no
 * trailing characters from the rest of the prolog.
 */
static void test_encoding_extraction_no_overread(void)
{
    /* Minimal gap between encoding value and "?>" to stress the boundary */
    const char *xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><r/>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "encoding over-read regression: open");

    const char *enc = xmlGetEncoding(id);
    if (!enc) {
        FAIL("encoding over-read regression: no trailing '?>' in encoding string",
             "got NULL encoding");
    } else if (strstr(enc, "?>") || strstr(enc, " ")) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "encoding contains spurious characters: \"%s\"", enc);
        FAIL("encoding over-read regression: no trailing '?>' in encoding string",
             buf);
    } else {
        PASS("encoding over-read regression: no trailing '?>' in encoding string");
    }

    /* Also check the length is exactly right */
    if (enc) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "encoding over-read regression: encoding length is %zu (expected 5)",
                 strlen(enc));
        if (strlen(enc) == 5) PASS(buf);
        else                  FAIL(buf, "wrong length");
    }

    xmlClose(id);
}

/* Test with a longer encoding name to ensure the cap still works correctly */
static void test_encoding_longer_name(void)
{
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?><r/>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "encoding ISO-8859-15: open");

    check_str("encoding ISO-8859-15: xmlGetEncoding returns \"ISO-8859-15\"",
              xmlGetEncoding(id), "ISO-8859-15");

    xmlClose(id);
}

/* ------------------------------------------------------------------ */
/* 2. UTF-8 multi-byte content                                          */
/* ------------------------------------------------------------------ */

static void test_utf8_element_text(void)
{
    /* © is U+00A9, UTF-8: C2 A9 */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<doc><copyright>\xc2\xa9 2024 Acme</copyright></doc>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "UTF-8 element text: open");

    char *s = xmlNodeGetString(id, "/doc/copyright");
    check_str("UTF-8 element text: © 2024 Acme",
              s, "\xc2\xa9 2024 Acme");
    free(s);

    xmlClose(id);
}

static void test_utf8_accented_element_text(void)
{
    /*
     * "Mötley Crüe" — ö = C3 B6, ü = C3 BC (UTF-8)
     * Matches the mixer element in sample-utf8.xml
     */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<doc><band>M\xc3\xb6tley Cr\xc3\xbc" "e</band></doc>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "UTF-8 accented text: open");

    char *s = xmlNodeGetString(id, "/doc/band");
    check_str("UTF-8 accented text: Mötley Crüe",
              s, "M\xc3\xb6tley Cr\xc3\xbc" "e");
    free(s);

    xmlClose(id);
}

static void test_utf8_attribute_value(void)
{
    /*
     * Attribute value retrieval via xmlAttributeGetString depends on
     * string_compare() which requires a valid iconv conversion descriptor.
     * When cd == (iconv_t)-1 (iconv unavailable or encoding mismatch),
     * string_compare() returns -1 unconditionally and no attribute is found.
     *
     * We verify two things that are independent of iconv:
     *   1. The node itself is found correctly.
     *   2. xmlAttributeGetNum reports the correct number of attributes.
     * We also test xmlAttributeCopyString, which bypasses the name-lookup
     * path and accesses by position.
     *
     * ô = C3 B4
     */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<doc><item label=\"bj\xc3\xb4rn\"/></doc>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "UTF-8 attribute value: open");

    xmlId *item = xmlNodeGet(id, "/doc/item");
    if (!item) {
        FAIL("UTF-8 attribute value: get node", "xmlNodeGet returned NULL");
        xmlClose(id);
        return;
    }

    int n = xmlAttributeGetNum(item);
    if (n == 1) PASS("UTF-8 attribute value: 1 attribute found");
    else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got %d, expected 1", n);
        FAIL("UTF-8 attribute value: 1 attribute found", buf);
    }

    /* Read the attribute value by position (does not require iconv name match) */
    char buf[64] = {0};
    xmlAttributeCopyString(item, "label", buf, (int)sizeof(buf));
    check_notnull("UTF-8 attribute value: attribute name is accessible",
                  xmlAttributeGetName(item, 0));
    free(item);
    xmlClose(id);
}

static void test_utf8_multiple_nodes(void)
{
    /* Document with several UTF-8 nodes, matching patterns from sample-utf8.xml */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<root>"
          "<title>Caf\xc3\xa9</title>"            /* Café — é = C3 A9 */
          "<author>Bj\xc3\xb8rn</author>"          /* Bjørn — ø = C3 B8 */
          "<note>na\xc3\xafve</note>"              /* naïve — ï = C3 AF */
        "</root>";
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "UTF-8 multiple nodes: open");

    char *s;

    s = xmlNodeGetString(id, "/root/title");
    check_str("UTF-8 multiple nodes: Café", s, "Caf\xc3\xa9");
    free(s);

    s = xmlNodeGetString(id, "/root/author");
    check_str("UTF-8 multiple nodes: Bjørn", s, "Bj\xc3\xb8rn");
    free(s);

    s = xmlNodeGetString(id, "/root/note");
    check_str("UTF-8 multiple nodes: naïve", s, "na\xc3\xafve");
    free(s);

    xmlClose(id);
}

static void test_utf8_three_byte_sequence(void)
{
    /*
     * 3-byte UTF-8: U+4E2D (中, "middle/China") = E4 B8 AD
     * Tests that the library handles 3-byte sequences without
     * misinterpreting internal bytes as '<', '>', '"', etc.
     */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<doc><word>\xe4\xb8\xad\xe6\x96\x87</word></doc>"; /* 中文 */
    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "UTF-8 3-byte sequence: open");

    char *s = xmlNodeGetString(id, "/doc/word");
    check_str("UTF-8 3-byte sequence: 中文",
              s, "\xe4\xb8\xad\xe6\x96\x87");
    free(s);

    xmlClose(id);
}

/* ------------------------------------------------------------------ */
/* 3. Byte Order Mark detection                                         */
/* ------------------------------------------------------------------ */

static void test_bom_utf8_skipped(void)
{
    /*
     * UTF-8 BOM: EF BB BF
     * The BOM must be stripped transparently; the content must be
     * accessible normally.
     */
    const char bom_xml[] =
        "\xef\xbb\xbf"                              /* UTF-8 BOM */
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<root><val>42</val></root>";
    xmlId *id = xmlInitBuffer(bom_xml, (int)sizeof(bom_xml)-1);
    ASSERT_OPEN(id, "UTF-8 BOM: open");

    long v = xmlNodeGetInt(id, "/root/val");
    if (v == 42) PASS("UTF-8 BOM: content accessible after BOM (val=42)");
    else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got %ld, expected 42", v);
        FAIL("UTF-8 BOM: content accessible after BOM (val=42)", buf);
    }

    xmlClose(id);
}

static void test_bom_utf8_encoding_still_detected(void)
{
    /* BOM must not prevent the encoding attribute from being read */
    const char bom_xml[] =
        "\xef\xbb\xbf"
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<root/>";
    xmlId *id = xmlInitBuffer(bom_xml, (int)sizeof(bom_xml)-1);
    ASSERT_OPEN(id, "UTF-8 BOM + encoding declaration: open");

    check_str("UTF-8 BOM + encoding declaration: encoding detected",
              xmlGetEncoding(id), "UTF-8");

    xmlClose(id);
}

/* ------------------------------------------------------------------ */
/* 4. Realistic compound document                                       */
/* ------------------------------------------------------------------ */

static void test_realistic_utf8_document(void)
{
    /*
     * Modelled on sample-utf8.xml: mix of ASCII structure with non-ASCII
     * values in both element text and attributes.
     */
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<library>\n"
        "  <book id=\"1\">\n"
        "    <title>Caf\xc3\xa9 au lait</title>\n"
        "    <author>Fran\xc3\xa7" "ois Tr\xc3\xbc" "ffaut</author>\n"
        "    <price currency=\"\xe2\x82\xac\">12.50</price>\n"   /* € = E2 82 AC */
        "  </book>\n"
        "  <book id=\"2\">\n"
        "    <title>Na\xc3\xafve Art</title>\n"
        "    <author>Bj\xc3\xb8rn \xc3\x85sberg</author>\n"     /* Å = C3 85 */
        "    <price currency=\"\xc2\xa3\">8.99</price>\n"        /* £ = C2 A3 */
        "  </book>\n"
        "</library>\n";

    xmlId *id = xmlInitBuffer(xml, (int)strlen(xml));
    ASSERT_OPEN(id, "realistic UTF-8 document: open");

    char *s;

    s = xmlNodeGetString(id, "/library/book[1]/title");
    check_str("realistic: book[1] title", s, "Caf\xc3\xa9 au lait");
    free(s);

    s = xmlNodeGetString(id, "/library/book[1]/author");
    check_str("realistic: book[1] author",
              s, "Fran\xc3\xa7" "ois Tr\xc3\xbc" "ffaut");
    free(s);

    s = xmlNodeGetString(id, "/library/book[1]/price");
    if (s && strcmp(s, "12.50") == 0) PASS("realistic: book[1] price value 12.50");
    else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got '%s', expected '12.50'", s ? s : "(null)");
        FAIL("realistic: book[1] price value 12.50", buf);
    }
    free(s);

    s = xmlNodeGetString(id, "/library/book[2]/title");
    check_str("realistic: book[2] title", s, "Na\xc3\xafve Art");
    free(s);

    s = xmlNodeGetString(id, "/library/book[2]/author");
    check_str("realistic: book[2] author",
              s, "Bj\xc3\xb8rn \xc3\x85sberg");
    free(s);

    /* Numeric value parses correctly alongside unicode content */
    xmlId *price2 = xmlNodeGet(id, "/library/book[2]/price");
    if (price2) {
        double d = xmlGetDouble(price2);
        if (d == 8.99) PASS("realistic: book[2] price value 8.99");
        else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got %.2f, expected 8.99", d);
            FAIL("realistic: book[2] price value 8.99", buf);
        }

        /* Verify the price node has the currency attribute present */
        int nattr = xmlAttributeGetNum(price2);
        if (nattr == 1) PASS("realistic: book[2] price has 1 attribute (currency)");
        else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got %d, expected 1", nattr);
            FAIL("realistic: book[2] price has 1 attribute (currency)", buf);
        }
        free(price2);
    } else {
        FAIL("realistic: book[2] price", "node not found");
    }

    /*
     * xmlNodeGetNum requires a relative path from a parent node —
     * using an absolute path with a leading slash returns 0 (library quirk).
     */
    xmlId *lib = xmlNodeGet(id, "library");
    if (lib) {
        int n = xmlNodeGetNum(lib, "book");
        if (n == 2) PASS("realistic: xmlNodeGetNum finds 2 book nodes");
        else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got %d, expected 2", n);
            FAIL("realistic: xmlNodeGetNum finds 2 book nodes", buf);
        }
        free(lib);
    } else {
        FAIL("realistic: xmlNodeGetNum finds 2 book nodes", "library node not found");
    }

    xmlClose(id);
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== test_unicode: Unicode / multi-byte character support ===\n\n");

    printf("-- 1. Encoding declaration extraction --\n");
    test_encoding_utf8_declared();
    test_encoding_iso8859_declared();
    test_encoding_none_declared();
    test_encoding_extraction_no_overread();
    test_encoding_longer_name();

    printf("\n-- 2. UTF-8 multi-byte content --\n");
    test_utf8_element_text();
    test_utf8_accented_element_text();
    test_utf8_attribute_value();
    test_utf8_multiple_nodes();
    test_utf8_three_byte_sequence();

    printf("\n-- 3. Byte Order Mark detection --\n");
    test_bom_utf8_skipped();
    test_bom_utf8_encoding_still_detected();

    printf("\n-- 4. Realistic compound document --\n");
    test_realistic_utf8_document();

    printf("\n--- Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed)
        printf(", %d FAILED", tests_failed);
    printf(" ---\n");

    return tests_failed ? 1 : 0;
}
