/* =====================================================================
   File:	rmalloc.h
   Author:	Rammi
   URL:		http://www.hexco.de/rmdebug/
   Date:	11/16/1995

   License:	(This is the open source ISC license, see 
                 http://en.wikipedia.org/wiki/ISC_license 
                 for more info)

	 Copyright © 2010  Andreas M. Rammelt <rammi@hexco.de>

	 Permission to use, copy, modify, and/or distribute this software for any
	 purpose with or without fee is hereby granted, provided that the above
	 copyright notice and this permission notice appear in all copies.

	 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
	 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
	 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
	 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
	 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
	 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
	 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


   Content:	Switching on/off of malloc debug library. This is done
		by preprocessor switch MALLOC_DEBUG.

		If MALLOC_DEBUG is defined, special debug versions for
		the functions of the standard malloc lib are used.
		They try to get hold of the following things:

		* Overwrite borders of allocated block
		* Multiple free of a block
		* free of an unallocated block

		All sources must include this file. No more changes necessary.

		Special macros:
		RM_TEST      Force testing of all allocated blocks.

		RM_STAT      Show a statistic of allocated blocks.

		RM_RETAG(p)  change position for allocated block to pos of
			     this macro

		RM_SET(p, f) Set flag f for block p. f can be a combination of
			     the following:
		   RM_STATIC Block is considered static. Is only summarized
                             in statistics.

		   RM_STRING Block is meant to hold a string.

		The global behaviour can be changed by editing, recompiling and
		linking rmalloc.c. See information there.

   Changes:
                See rmalloc.c

   ===================================================================== */


#ifndef R_MALLOC_H
#define R_MALLOC_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
# define RMALLOC_APIENTRY
# define RMALLOC_API extern
#else
# define RMALLOC_APIENTRY
# define RMALLOC_API extern
#endif

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export on
#endif


/* =========
   INCLUDEs:
   ========= */

#include <stdlib.h>
#include <string.h>

/* ========
   DEFINEs:
   ======== */

/* #define RM_UPPERSTYLE */	/* this is only used for backward compatibility */

/* once again useful: INT2STRING(prepro_macro_containing_number)-->"number" */
#define FUNCTIONIZE(a,b)  a(b)
#define STRINGIZE(a)      #a
#define INT2STRING(i)     FUNCTIONIZE(STRINGIZE,i)

/* Flags in header (with WITH_FLAGS defined, see rmalloc.c) */
#define RM_STATIC         (0x0001 << 0)         /* static memory */
#define RM_STRING         (0x0001 << 1)         /* contains string */

#ifdef MALLOC_DEBUG
/* Useful, to build 1 string from __FILE__ & __LINE__ in compile time: */
#define RM_FILE_POS       __FILE__ ":" INT2STRING(__LINE__)

#ifdef RM_UPPERSTYLE
/* Deprecated: Only used for backward compatibility: */
#  define MALLOC(s)       Rmalloc((s), RM_FILE_POS)
#  define CALLOC(n,s)     Rcalloc((n), (s), RM_FILE_POS)
#  define REALLOC(p,s)    Rrealloc((p), (s), RM_FILE_POS)
#  define FREE(p)         Rfree((p), RM_FILE_POS)
#  define FREE0(p)        Rfree((p), RM_FILE_POS),(p)=NULL
#  define STRDUP(s)       Rstrdup((s), RM_FILE_POS)
#else /* !RM_UPPERSTYLE */
/* Wrap with our stuff: */
#  ifdef malloc
#    undef malloc
#  endif
#  define malloc(s)       Rmalloc((s), RM_FILE_POS)

#  ifdef calloc
#    undef calloc
#  endif
#  define calloc(n,s)     Rcalloc((n), (s), RM_FILE_POS)

#  ifdef aligned_alloc
#    undef aligned_alloc
#  endif
#  define aligned_alloc(n,s) Raligned_alloc((n), (s), RM_FILE_POS)

#  ifdef _aligned_malloc
#    undef _aligned_malloc
#  endif
#  define _aligned_malloc(n,s) Raligned_alloc((n), (s), RM_FILE_POS)

#  ifdef realloc
#    undef realloc
#  endif
#  define realloc(p,s)    Rrealloc((p), (s), RM_FILE_POS)

#  ifdef free
#    undef free
#  endif
#  define free(p)         Rfree((p), RM_FILE_POS)

#  ifdef free0
#    undef free0
#  endif
#  define free0(p)        Rfree((p), RM_FILE_POS),(p)=NULL

#  ifdef strdup
#    undef strdup
#  endif
#  define strdup(s)       Rstrdup((s), RM_FILE_POS)

#  ifdef getcwd
#    undef getcwd
#  endif
#  define getcwd(b,s)	  Rgetcwd((b), (s), RM_FILE_POS)
#endif /* RM_UPPERSTYLE */
#define RM_TEST           Rmalloc_test(RM_FILE_POS)
#define RM_STAT           Rmalloc_stat(RM_FILE_POS)
#define RM_RETAG(p)       Rmalloc_retag((p), RM_FILE_POS)
#define RM_SET(p, f)      Rmalloc_set_flags((p), (f), RM_FILE_POS)

#else /* ! MALLOC_DEBUG */

#ifdef RM_UPPERSTYLE
/* The normal stuff (backward compatibility): */
#  define MALLOC(s)	malloc((s))
#  define CALLOC(n,s)	calloc((n), (s))
#  define REALLOC(p,s)	realloc((p), (s))
#  define FREE(p)	free((p))
#  define FREE0(p)	free((p)), (p)=NULL
#  define STRDUP(s)	(char *)strdup((s))
#else /* !RM_UPPERSTYLE */
#  define free0(p)	free((p)), (p)=NULL
#endif /* RM_UPPERSTYLE */
#define RM_TEST
#define RM_STAT
#define RM_RETAG(p)	(p)
#define RM_SET(p, f)	(p)

#endif /* ! MALLOC_DEBUG */

/* ===========
   PROTOTYPES:
   =========== */

#if defined(MALLOC_DEBUG) || defined(RM_NEED_PROTOTYPES)
RMALLOC_API void* RMALLOC_APIENTRY Rmalloc(size_t size, const char *file);
RMALLOC_API void* RMALLOC_APIENTRY Rcalloc(size_t nelem, size_t size, const char *file);
RMALLOC_API void* RMALLOC_APIENTRY Rrealloc(void *p, size_t size, const char *file);
RMALLOC_API void  RMALLOC_APIENTRY Rfree(void *p, const char *file);
RMALLOC_API char* RMALLOC_APIENTRY Rstrdup(const char *str, const char *file);
RMALLOC_API char* RMALLOC_APIENTRY Rgetcwd(char *buffer, size_t size, const char *file);
RMALLOC_API void  RMALLOC_APIENTRY Rtest_malloc(const char *file);
RMALLOC_API void  RMALLOC_APIENTRY Rmalloc_test(const char *file);
RMALLOC_API void  RMALLOC_APIENTRY Rmalloc_stat(const char *file);
RMALLOC_API void* RMALLOC_APIENTRY Rmalloc_retag(void *p, const char *file);
RMALLOC_API void* RMALLOC_APIENTRY Rmalloc_set_flags(void *p, unsigned flags, const char *file);
#endif /* MALLOC_DEBUG || RM_NEED_PROTOTYPES */

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export off
#endif

#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif /* !R_MALLOC_H */
