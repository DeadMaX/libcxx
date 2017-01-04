//===------------------------- chrono.cpp ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "chrono"
#include "cerrno"        // errno
#include "system_error"  // __throw_system_error
#include <__ctime_support>// clock_gettime, CLOCK_MONOTONIC and CLOCK_REALTIME
#include <__time_support>
#if defined(_LIBCPP_WIN32API)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN
#include <Windows.h>
#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
#include <winapifamily.h>
#endif
#else
#endif // !defined(CLOCK_REALTIME)
#endif // defined(_LIBCPP_WIN32API)
#if !defined(_LIBCPP_HAS_NO_MONOTONIC_CLOCK)
#elif !defined(_LIBCPP_WIN32API) && !defined(CLOCK_MONOTONIC)

_LIBCPP_BEGIN_NAMESPACE_STD

namespace chrono
{

// system_clock

const bool system_clock::is_steady;

system_clock::time_point
system_clock::now() _NOEXCEPT
{
	return __libcpp_get_current_time<system_clock::time_point>();
  // FILETIME is in 100ns units
  using filetime_duration =
      _VSTD::chrono::duration<__int64,
                              _VSTD::ratio_multiply<_VSTD::ratio<100, 1>,
                                                    nanoseconds::period>>;

  // The Windows epoch is Jan 1 1601, the Unix epoch Jan 1 1970.
  static _LIBCPP_CONSTEXPR const seconds nt_to_unix_epoch{11644473600};

  FILETIME ft;
#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  GetSystemTimePreciseAsFileTime(&ft);
#else
  GetSystemTimeAsFileTime(&ft);
#endif
#else
  GetSystemTimeAsFileTime(&ft);
#endif

  filetime_duration d{(static_cast<__int64>(ft.dwHighDateTime) << 32) |
                       static_cast<__int64>(ft.dwLowDateTime)};
  return time_point(duration_cast<duration>(d - nt_to_unix_epoch));
#else
#endif
}

#ifndef _LIBCXX_DISABLE_C_LINKAGE
time_t
system_clock::to_time_t(const time_point& t) _NOEXCEPT
{
    return time_t(duration_cast<seconds>(t.time_since_epoch()).count());
}

system_clock::time_point
system_clock::from_time_t(time_t t) _NOEXCEPT
{
    return system_clock::time_point(seconds(t));
}
#endif

#ifndef _LIBCPP_HAS_NO_MONOTONIC_CLOCK
// steady_clock
//
// Warning:  If this is not truly steady, then it is non-conforming.  It is
//  better for it to not exist and have the rest of libc++ use system_clock
//  instead.

const bool steady_clock::is_steady;

#if defined(__APPLE__)

	return __libcpp_get_current_time<steady_clock::time_point>();
}

#elif defined(_LIBCPP_WIN32API)

steady_clock::time_point
steady_clock::now() _NOEXCEPT
{
  static LARGE_INTEGER freq;
  static BOOL initialized = FALSE;
  if (!initialized)
    initialized = QueryPerformanceFrequency(&freq); // always succceeds

  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return time_point(duration(counter.QuadPart * nano::den / freq.QuadPart));
}

#elif defined(CLOCK_MONOTONIC)

steady_clock::time_point
steady_clock::now() _NOEXCEPT
{
    struct timespec tp;
    if (0 != clock_gettime(CLOCK_MONOTONIC, &tp))
        __throw_system_error(errno, "clock_gettime(CLOCK_MONOTONIC) failed");
    return time_point(seconds(tp.tv_sec) + nanoseconds(tp.tv_nsec));
}

#else
#error "Monotonic clock not implemented"
#endif

#endif // !_LIBCPP_HAS_NO_MONOTONIC_CLOCK

}

_LIBCPP_END_NAMESPACE_STD
