/*
 * test_doctype_crash.c
 *
 * Regression test for the NULL pointer dereference in __zeroxmlProcessCDATA()
 * when a DOCTYPE declaration has no closing "]>" sequence.
 *
 * Bug summary
 * -----------
 * The DOCTYPE parsing loop called __zeroxml_memmem() to search for the "]>"
 * terminator.  When the terminator was absent (NULL return), execution fell
 * through to `new += 2` — arithmetic on a null pointer — and then
 * dereferenced the result, crashing the process.
 *
 * The fix adds an explicit `if (!new) break;` before that arithmetic.
 *
 * Test strategy
 * -------------
 * Each test case calls xmlInitBuffer() with a crafted payload and checks
 * that the library returns NULL (error) rather than crashing.  The test
 * also verifies that well-formed DOCTYPE input continues to parse correctly.
 *
 * Build (from the zeroxml root directory):
 *   cc -I include -I src -o test_doctype_crash test/test_doctype_crash.c \
 *      src/xml.c src/xml_cache.c src/localize.c \
 *      -DXML_NONVALIDATING -o test/test_doctype_crash
 *
 * Or, if a non-validating build without iconv is desired:
 *   cc -I include -I src -DXML_NONVALIDATING \
 *      test/test_doctype_crash.c src/xml.c src/xml_cache.c src/localize.c \
 *      -o test/test_doctype_crash
 *
 * Run:
 *   ./test/test_doctype_crash
 *
 * Exit code: 0 = all tests passed, 1 = one or more tests failed.
 */

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

#define PASS(desc) do { \
    printf("  PASS  %s\n", (desc)); \
    tests_passed++; tests_run++; \
} while(0)

#define FAIL(desc, got, expected) do { \
    printf("  FAIL  %s\n         got %p, expected %s\n", \
           (desc), (void*)(got), (expected)); \
    tests_failed++; tests_run++; \
} while(0)

/* ------------------------------------------------------------------ */
/* Helper: parse a buffer, expect NULL (should fail gracefully)        */
/* ------------------------------------------------------------------ */
static void expect_null(const char *desc, const char *xml, int len)
{
    xmlId *id = xmlInitBuffer(xml, len);
    if (id == NULL) {
        PASS(desc);
    } else {
        FAIL(desc, id, "NULL");
        xmlClose(id);
    }
}

/* ------------------------------------------------------------------ */
/* Helper: parse a buffer, expect a non-NULL xmlId (should succeed)    */
/* ------------------------------------------------------------------ */
static void expect_ok(const char *desc, const char *xml, int len)
{
    xmlId *id = xmlInitBuffer(xml, len);
    if (id != NULL) {
        PASS(desc);
        xmlClose(id);
    } else {
        FAIL(desc, id, "non-NULL xmlId*");
    }
}

/* ------------------------------------------------------------------ */
/* Tests                                                                */
/* ------------------------------------------------------------------ */

/*
 * Core crash reproducer — the minimal payload that triggered the bug.
 * A DOCTYPE with an internal subset opener "[" but no "]>" terminator.
 */
static void test_unclosed_doctype_minimal(void)
{
    /* "<!DOCTYPE x [" — no closing "]>" anywhere in the buffer */
    const char *xml = "<!DOCTYPE x [";
    expect_null("unclosed DOCTYPE (minimal payload, no ']>')",
                xml, (int)strlen(xml));
}

/*
 * DOCTYPE with content inside the internal subset but still no "]>".
 */
static void test_unclosed_doctype_with_content(void)
{
    const char *xml =
        "<!DOCTYPE note [\n"
        "  <!ELEMENT note (to,from,body)>\n"
        "  <!ELEMENT to (#PCDATA)>\n"
        /* deliberately omitted: "]>" */
        "<note><to>Alice</to></note>";
    expect_null("unclosed DOCTYPE (internal subset, no ']>')",
                xml, (int)strlen(xml));
}

/*
 * DOCTYPE that is completely empty after the keyword — no "[" at all,
 * no "]>".  The memmem still returns NULL; exercise that path too.
 */
static void test_doctype_no_subset_at_all(void)
{
    const char *xml = "<!DOCTYPE ";   /* truncated mid-keyword */
    expect_null("truncated DOCTYPE keyword",
                xml, (int)strlen(xml));
}

/*
 * Buffer truncated right at the start of the internal subset marker.
 */
static void test_doctype_truncated_at_bracket(void)
{
    const char *xml = "<!DOCTYPE note [";
    expect_null("DOCTYPE truncated at '['",
                xml, (int)strlen(xml));
}

/*
 * Regression guard: a well-formed DOCTYPE must still parse correctly
 * so we don't accidentally break the happy path with the fix.
 */
static void test_valid_doctype_still_works(void)
{
    /*
     * A document with a complete internal subset — both "]>" present.
     * xmlInitBuffer should return a valid id and the root element
     * should be accessible.
     */
    const char *xml =
        "<!DOCTYPE note [\n"
        "  <!ELEMENT note (to)>\n"
        "  <!ELEMENT to (#PCDATA)>\n"
        "]>\n"
        "<note><to>Alice</to></note>";
    expect_ok("valid DOCTYPE with internal subset parses correctly",
              xml, (int)strlen(xml));
}

/*
 * Regression guard: a document without any DOCTYPE at all must still
 * parse correctly.
 */
static void test_no_doctype_still_works(void)
{
    const char *xml = "<root><child>hello</child></root>";
    expect_ok("document without DOCTYPE parses correctly",
              xml, (int)strlen(xml));
}

/*
 * Stress: many nested "]" characters before the real "]>" to exercise
 * the loop-continuation path (where *(new-1) == ']').
 */
static void test_doctype_nested_brackets_unclosed(void)
{
    const char *xml =
        "<!DOCTYPE x [\n"
        "  <!ENTITY % data ']]]]]]'>\n"
        /* still no "]>" to close the subset */
        ;
    expect_null("unclosed DOCTYPE with nested ']' characters",
                xml, (int)strlen(xml));
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== test_doctype_crash: DOCTYPE null-deref regression tests ===\n\n");

    test_unclosed_doctype_minimal();
    test_unclosed_doctype_with_content();
    test_doctype_no_subset_at_all();
    test_doctype_truncated_at_bracket();
    test_valid_doctype_still_works();
    test_no_doctype_still_works();
    test_doctype_nested_brackets_unclosed();

    printf("\n--- Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed)
        printf(", %d FAILED", tests_failed);
    printf(" ---\n");

    return tests_failed ? 1 : 0;
}
