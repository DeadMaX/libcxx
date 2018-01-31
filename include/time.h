// -*- C++ -*-
//===---------------------------- time.h --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_TIME_H
#define _LIBCPP_TIME_H

/*
    time.h synopsis

Macros:

    time

*/

#include <__config>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

#ifndef LIBCXX_STANDFREE
#include_next <time.h>
#else
#include <standfree/__time>
#endif

#endif  // _LIBCPP_TIME_H
