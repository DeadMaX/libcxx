//===-------------------------- math.cpp ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "cmath"

#ifdef _LIBCXX_DISABLE_C_LINKAGE

_LIBCPP_BEGIN_NAMESPACE_STD

namespace
{
	union IEEEf2bits {
		IEEEf2bits(float __f) : f(__f) {}
		float f;
		struct {
#if _LIBCPP_LITTLE_ENDIAN
			unsigned int    man     :23;
			unsigned int    exp     :8;
			bool            sign    :1;
#else /* _LIBCPP_BIG_ENDIAN */
			bool            sign    :1;
			unsigned int    exp     :8;
			unsigned int    man     :23;
#endif
		} bits;
	};
	static_assert(sizeof(IEEEf2bits) == sizeof(float), "Invalide float structure");

	union IEEEd2bits {
		IEEEd2bits(double __d) : d(__d) {}
		double d;
		struct {
#if _LIBCPP_LITTLE_ENDIAN
			uint64_t	    man     :52;
			unsigned int    exp     :11;
			bool            sign    :1;
#else /* _LIBCPP_BIG_ENDIAN */
			bool            sign    :1;
			unsigned int    exp     :11;
			uint64_t        man     :52;
#endif
		} bits;
	};
	static_assert(sizeof(IEEEd2bits) == sizeof(double), "Invalide double structure");

	union IEEEl2bits {
		IEEEl2bits(long double __l) : l(__l) {}
		long double l;
		struct {
#if _LIBCPP_LITTLE_ENDIAN
			uint64_t        man     :63;
			unsigned int    _zero   :1;
			unsigned int    exp     :15;
			bool            sign    :1;
#else /* _LIBCPP_BIG_ENDIAN */
			bool            sign    :1;
			unsigned int    exp     :15;
			unsigned int    _zero   :1;
			uint64_t        man     :63;
#endif
		} bits;
	};
	static_assert(sizeof(IEEEl2bits) == sizeof(long double), "Invalide long double structure");
}

bool signbit(float __f)
{
	IEEEf2bits f(__f);
	return f.bits.sign;
}

bool signbit(double __d)
{
	IEEEd2bits d(__d);
	return d.bits.sign;
}

bool signbit(long double __l)
{
	IEEEl2bits l(__l);
	return l.bits.sign;
}

float       exp( float __f )
{
}

double      exp( double __d )
{
}

long double exp( long double __l )
{
}

float       lgamma(float __f)
{
}

double      lgamma(double __d)
{
}

long double lgamma(long double __l)
{
}

float       log(float __f)
{
}

double      log(double __d)
{
}

long double log(long double __l)
{
}

float       sqrt(float __f)
{
}

double      sqrt(double __d)
{
}

long double sqrt(long double __l)
{
}

float       pow(float __f, float __exp)
{
}

double      pow(double __d, double __exp)
{
}

long double pow(long double __l, long double __exp)
{
}

float       pow(float __f, int __exp)
{
}

double      pow(double __d, int __exp)
{
}

long double pow(long double __l, int __exp)
{
}

int         abs(int __n)
{
}

long        abs(long __n)
{
}

long long   abs(long long __n)
{
}

float       abs(float __f)
{
}

double      abs(double __d)
{
}

long double abs(long double __l)
{
}

float       tan(float __f)
{
}

double      tan(double __d)
{
}

long double tan(long double __l)
{
}

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCXX_DISABLE_C_LINKAGE
