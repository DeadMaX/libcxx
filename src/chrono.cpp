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

_LIBCPP_BEGIN_NAMESPACE_STD

namespace chrono
{

// system_clock

const bool system_clock::is_steady;

system_clock::time_point
system_clock::now() _NOEXCEPT
{
	return __libcpp_get_current_time<system_clock::time_point>();
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

#ifdef CLOCK_MONOTONIC

steady_clock::time_point
steady_clock::now() _NOEXCEPT
{
	return __libcpp_get_current_time<steady_clock::time_point>();
}

#else
#error "Monotonic clock not implemented"
#endif

#endif // !_LIBCPP_HAS_NO_MONOTONIC_CLOCK

}

_LIBCPP_END_NAMESPACE_STD
