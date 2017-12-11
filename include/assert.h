// -*- C++ -*-
//===---------------------------- locale.h --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_ASSERT_H
#define _LIBCPP_ASSERT_H

/*
    assert.h synopsis

Macros:

    assert

*/

#include <__config>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

#ifndef LIBCXX_STANDFREE
#include_next <assert.h>
#else
#include <standfree/__assert>
#endif

#endif  // _LIBCPP_ASSERT_H
