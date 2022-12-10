/*
 * Copyright (C) 2005-2011 by Erik Hofman.
 * Copyright (C) 2007-2011 by Adalin B.V.
 *
 * This file is part of OpenAL-AeonWave.
 *
 *  OpenAL-AeonWave is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenAL-AeonWave is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenAL-AeonWave.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __OAL_TYPES_H
#define __OAL_TYPES_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if __STDC_VERSION__ < 199901L
#  if __GNUC__ >= 2
#    define __func__ __FUNCTION__
#  elif defined(_MSC_VER)
#    if _MSC_VER < 1300
#      define __func__ "<unknown>"
#    else
#      define __func__ __FUNCTION__
#    endif
#  elif defined(__BORLANDC__)
#    define __func__ __FUNC__
#  else
#    define __func__ "<unknown>"
#  endif
#endif

#if _MSC_VER
# include <Windows.h>
# define strtoll _strtoi64
# define snprintf _snprintf
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
# define rintf(v) (int)(v+0.5f)
# define msecSleep(tms) SleepEx((DWORD)tms, FALSE)

struct timespec
{
  time_t tv_sec; /* seconds */
  long tv_nsec;  /* nanoseconds */
};

struct timezone
{
  int tz_minuteswest; /* of Greenwich */
  int tz_dsttime;     /* type of dst correction to apply */
};
int gettimeofday(struct timeval*, void*);

typedef long	off_t;
# if SIZEOF_SIZE_T == 4
typedef INT32	ssize_t;
#else
typedef INT64	ssize_t;
#endif

#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>		/* for gettimeofday */
# endif
int msecSleep(unsigned int);
#endif

#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif /* !__OAL_TYPES_H */

