#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>

/*
 * ChatGPT: write a localization aware character compare function in C
 * Here's a sample implementation of a localization-aware character comparison
 * function in C:
 *
 * This implementation uses the mbrtowc function to convert sequences of
 * multibyte characters to wide characters, and compares the wide characters
 * using the wcscoll function, which performs a locale-specific comparison of
 * wide character strings. The function returns 0 if the two characters are
 * equal, a negative value if s1 is less than s2, or a positive value if s1
 * is greater than s2.
 */
int localized_char_cmp(const char s1, const char s2)
{
    mbstate_t mbs1, mbs2;
    wchar_t wc1, wc2;
    size_t mblen1, mblen2;
    int res;

    memset(&mbs1, 0, sizeof mbs1);
    memset(&mbs2, 0, sizeof mbs2);
    mblen1 = mbrtowc(&wc1, &s1, MB_CUR_MAX, &mbs1);
    mblen2 = mbrtowc(&wc2, &s2, MB_CUR_MAX, &mbs2);
    if (mblen1 == (size_t)-1 || mblen1 == (size_t)-2 ||
        mblen2 == (size_t)-1 || mblen2 == (size_t)-2) {
        return -1;
    }
    res = wcscoll(&wc1, &wc2);
    return res;
}

/*
 * ChatGPT: write a localization aware tolower function in C
 * Here's a sample implementation of a localization-aware tolower function in C:
 *
 * This implementation uses the mbrtowc function to convert a multibyte
 * character to a wide character, and converts the wide character to lowercase
 * using the towlower function, which performs a locale-specific case conversion
 * of wide characters. The function returns the lowercase equivalent of the
 * input character, or -1 if an error occurs.
 */
int localized_tolower(int c)
{
    mbstate_t mbs;
    wchar_t wc;
    size_t mblen;

    memset(&mbs, 0, sizeof mbs);
    mblen = mbrtowc(&wc, (const char*)&c, MB_CUR_MAX, &mbs);
    if (mblen == (size_t)-1 || mblen == (size_t)-2) {
        return -1;
    }
    return towlower(wc);
}

/* ChatGPT: write a localization aware memchr function in C
 * Here's a sample implementation of a localization-aware memchr function in C:
 *
 * This implementation uses the mbrtowc function to convert a sequence of
 * multibyte characters to a wide character, and compares the wide character
 * with the integer c passed as the second argument. If a match is found, the
 * function returns a pointer to the matching multibyte character in the input
 * string.
 */
void *localized_memchr(const void *s, int c, size_t n)
{
    mbstate_t mbs;
    wchar_t wc;
    size_t mblen;

    memset(&mbs, 0, sizeof mbs);
    while (n > 0) {
        mblen = mbrtowc(&wc, s, n, &mbs);
        if (mblen == (size_t)-1 || mblen == (size_t)-2) {
            return NULL;
        }
        if (wc == (wchar_t)c) {
            return (void *)s;
        }
        s = (char *)s + mblen;
        n -= mblen;
    }
    return NULL;
}

/*
 * ChatGPT: write a localization aware memcmp function
 * Here's an example implementation of a localization aware memcmp function in C
 *
 * This implementation takes into account the current locale set with setlocale
 * to perform a correct comparison of the strings. The mbrtowc function is used
 * to convert multi-byte characters to wide characters, and the wcscoll function * is used to compare the wide characters according to the current locale.
 */
int localized_memcmp(const void *s1, const void *s2, size_t n) {
  wchar_t wc1, wc2;
  size_t i, j;
  mbstate_t state1, state2;

  memset(&state1, 0, sizeof state1);
  memset(&state2, 0, sizeof state2);

  for (i = j = 0; i < n && j < n;) {
    size_t len1 = mbrtowc(&wc1, (const char *)s1 + i, n - i, &state1);
    size_t len2 = mbrtowc(&wc2, (const char *)s2 + j, n - j, &state2);

    if (len1 == (size_t) -1 || len1 == (size_t) -2)
      return len2 == 0 ? 0 : -1;
    if (len2 == (size_t) -1 || len2 == (size_t) -2)
      return len1 == 0 ? 0 : 1;

    int cmp = wcscoll(&wc1, &wc2);
    if (cmp != 0)
      return cmp;

    i += len1;
    j += len2;
  }

  if (i < n)
    return 1;
  if (j < n)
    return -1;

  return 0;
}

/* ChatGPT: write a localization aware strncmp function in C
 * Here's a sample implementation of a localization-aware strncmp function in C: *
 * This implementation uses the mbrtowc function to convert sequences of
 * multibyte characters to wide characters, and compares the wide characters
 * using the wcscoll function, which performs a locale-specific comparison of
 * wide character strings. The function returns 0 if the two strings are equal
 * (up to n characters), a negative value if s1 is less than s2, or a positive
 * value if s1 is greater than s2.
 */
int localized_strncmp(const char *s1, const char *s2, size_t n)
{
    mbstate_t mbs1, mbs2;
    wchar_t wc1, wc2;
    size_t mblen1, mblen2;
    int res;

    memset(&mbs1, 0, sizeof mbs1);
    memset(&mbs2, 0, sizeof mbs2);
    while (n > 0) {
        mblen1 = mbrtowc(&wc1, s1, n, &mbs1);
        mblen2 = mbrtowc(&wc2, s2, n, &mbs2);
        if (mblen1 == (size_t)-1 || mblen1 == (size_t)-2 ||
            mblen2 == (size_t)-1 || mblen2 == (size_t)-2) {
            return -1;
        }
        res = wcscoll(&wc1, &wc2);
        if (res != 0) {
            return res;
        }
        s1 += mblen1;
        s2 += mblen2;
        n -= mblen1;
        if (mblen1 != mblen2) {
            n = 0;
        }
    }
    return 0;
}

/* ChatGPT: write a localization aware strncasecmp function in C
 * Here's a sample implementation of a localization-aware strncasecmp function
 * in C:
 *
 * This implementation uses the mbrtowc function to convert sequences of
 * multibyte characters to wide characters, and compares the wide characters
 * after converting them to lowercase using the towlower function, which
 * performs a locale-specific case conversion of wide characters. The function
 * returns 0 if the two strings are equal (up to n characters), a negative value
 * if s1 is less than s2, or a positive value if s1 is greater than s2.
 */
int localized_strncasecmp(const char *s1, const char *s2, size_t n)
{
    mbstate_t mbs1, mbs2;
    wchar_t wc1, wc2;
    size_t mblen1, mblen2;
    int res;

    memset(&mbs1, 0, sizeof mbs1);
    memset(&mbs2, 0, sizeof mbs2);
    while (n > 0) {
        mblen1 = mbrtowc(&wc1, s1, n, &mbs1);
        mblen2 = mbrtowc(&wc2, s2, n, &mbs2);
        if (mblen1 == (size_t)-1 || mblen1 == (size_t)-2 ||
            mblen2 == (size_t)-1 || mblen2 == (size_t)-2) {
            return -1;
        }
        res = towlower(wc1) - towlower(wc2);
        if (res != 0) {
            return res;
        }
        s1 += mblen1;
        s2 += mblen2;
        n -= mblen1;
        if (mblen1 != mblen2) {
            n = 0;
        }
    }
    return 0;
}
