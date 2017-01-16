//===------------------------- string.cpp ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "string"
#include "cstdlib"
#include "cwchar"
#include "cerrno"
#include "limits"
#include "stdexcept"
#include "locale"
#include "ios"
#ifdef _LIBCPP_MSVCRT
#include "support/win32/support.h"
#endif // _LIBCPP_MSVCRT

_LIBCPP_BEGIN_NAMESPACE_STD

template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS __basic_string_common<true>;

template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_string<char>;
template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_string<wchar_t>;

template
    string
    operator+<char, char_traits<char>, allocator<char> >(char const*, string const&);

namespace
{

template<typename T>
inline
void throw_helper( const string& msg )
{
#ifndef _LIBCPP_NO_EXCEPTIONS
    throw T( msg );
#else
    fprintf(stderr, "%s\n", msg.c_str());
    _VSTD::abort();
#endif
}

inline
void throw_from_string_out_of_range( const string& func )
{
    throw_helper<out_of_range>(func + ": out of range");
}

inline
void throw_from_string_invalid_arg( const string& func )
{
    throw_helper<invalid_argument>(func + ": no conversion");
}

// as_integer

template<typename V, typename S>
inline
V
as_signed_integer_helper(const string& func, const S& str, size_t* idx, int base)
{
    typename S::value_type* ptr = nullptr;
    const typename S::value_type* const p = str.c_str();
    bool error = false;
    long long int r = __libcpp_get_signed_intergral(p, &ptr, base, __libcpp_locale_t(), error);
    if (error || r < numeric_limits<V>::min() || numeric_limits<V>::max() < r)
	  throw_from_string_out_of_range(func);
    if (ptr == p)
	throw_from_string_invalid_arg(func);
    if (idx)
	*idx = static_cast<size_t>(ptr - p);
    static_cast<V>(r);
}

template<typename V, typename S>
inline
V
as_unsigned_integer_helper(const string& func, const S& str, size_t* idx, int base)
{
    typename S::value_type* ptr = nullptr;
    const typename S::value_type* const p = str.c_str();
    bool error = false;
    long long int r = __libcpp_get_unsigned_intergral(p, &ptr, base, __libcpp_locale_t(), error);
    if (error || numeric_limits<V>::max() < r)
	  throw_from_string_out_of_range(func);
    if (ptr == p)
	throw_from_string_invalid_arg(func);
    if (idx)
	*idx = static_cast<size_t>(ptr - p);
    static_cast<V>(r);
}

template<typename V, typename S>
inline
V
as_integer(const string& func, const S& s, size_t* idx, int base);

// string
template<>
inline
int
as_integer(const string& func, const string& s, size_t* idx, int base )
{
    // Use long as no Standard string to integer exists.
    return as_signed_integer_helper<int>( func, s, idx, base );
}

template<>
inline
long
as_integer(const string& func, const string& s, size_t* idx, int base )
{
    return as_signed_integer_helper<long>( func, s, idx, base );
}

template<>
inline
unsigned long
as_integer( const string& func, const string& s, size_t* idx, int base )
{
    return as_unsigned_integer_helper<unsigned long>( func, s, idx, base );
}

template<>
inline
long long
as_integer( const string& func, const string& s, size_t* idx, int base )
{
    return as_signed_integer_helper<long long>( func, s, idx, base );
}

template<>
inline
unsigned long long
as_integer( const string& func, const string& s, size_t* idx, int base )
{
    return as_unsigned_integer_helper<unsigned long long>( func, s, idx, base );
}

// wstring
template<>
inline
int
as_integer( const string& func, const wstring& s, size_t* idx, int base )
{
    // Use long as no Stantard string to integer exists.
    return as_signed_integer_helper<int>( func, s, idx, base );
}

template<>
inline
long
as_integer( const string& func, const wstring& s, size_t* idx, int base )
{
    return as_signed_integer_helper<long>( func, s, idx, base );
}

template<>
inline
unsigned long
as_integer( const string& func, const wstring& s, size_t* idx, int base )
{
    return as_unsigned_integer_helper<unsigned long>( func, s, idx, base );
}

template<>
inline
long long
as_integer( const string& func, const wstring& s, size_t* idx, int base )
{
    return as_signed_integer_helper<long long>( func, s, idx, base );
}

template<>
inline
unsigned long long
as_integer( const string& func, const wstring& s, size_t* idx, int base )
{
    return as_unsigned_integer_helper<unsigned long long>( func, s, idx, base );
}

// as_float

template<typename V, typename S, typename F> 
inline
V
as_float_helper(const string& func, const S& str, size_t* idx, F f )
{
    typename S::value_type* ptr = nullptr;
    const typename S::value_type* const p = str.c_str();
    bool error = false;
    V r = f(p, &ptr, __libcpp_locale_t(), error);
    if (error)
        throw_from_string_out_of_range(func);
    if (ptr == p)
        throw_from_string_invalid_arg(func);
    if (idx)
        *idx = static_cast<size_t>(ptr - p);
    return r;
}

template<typename V, typename S>
inline
V as_float( const string& func, const S& s, size_t* idx = nullptr );

float get_char_float(const char* __a, const char** __p2, const __libcpp_locale_t &__locale, bool &__range_error)
{
    return __libcpp_get_float(__a, __p2, __locale, __range_error);
}

double get_char_double(const char* __a, const char** __p2, const __libcpp_locale_t &__locale, bool &__range_error)
{
    return __libcpp_get_double(__a, __p2, __locale, __range_error);
}

long double get_char_long_double(const char* __a, const char** __p2, const __libcpp_locale_t &__locale,
									 bool &__range_error)
{
    return __libcpp_get_long_double(__a, __p2, __locale, __range_error);
}

float get_wchar_float(const wchar_t* __a, const wchar_t** __p2, const __libcpp_locale_t &__locale, bool &__range_error)
{
    return __libcpp_get_float(__a, __p2, __locale, __range_error);
}

double get_wchar_double(const wchar_t* __a, const wchar_t** __p2, const __libcpp_locale_t &__locale, bool &__range_error)
{
    return __libcpp_get_double(__a, __p2, __locale, __range_error);
}

long double get_wchar_long_double(const wchar_t* __a, const wchar_t** __p2, const __libcpp_locale_t &__locale,
									 bool &__range_error)
{
    return __libcpp_get_long_double(__a, __p2, __locale, __range_error);
}

template<>
inline
float
as_float( const string& func, const string& s, size_t* idx )
{
    return as_float_helper<float>( func, s, idx, get_char_float );
}

template<>
inline
double
as_float(const string& func, const string& s, size_t* idx )
{
    return as_float_helper<double>( func, s, idx, get_char_double );
}

template<>
inline
long double
as_float( const string& func, const string& s, size_t* idx )
{
    return as_float_helper<long double>( func, s, idx, get_char_long_double );
}

template<>
inline
float
as_float( const string& func, const wstring& s, size_t* idx )
{
    return as_float_helper<float>( func, s, idx, get_wchar_float );
}

template<>
inline
double
as_float( const string& func, const wstring& s, size_t* idx )
{
    return as_float_helper<double>( func, s, idx, get_wchar_double );
}

template<>
inline
long double
as_float( const string& func, const wstring& s, size_t* idx )
{
    return as_float_helper<long double>( func, s, idx, get_wchar_long_double );
}

}  // unnamed namespace

int
stoi(const string& str, size_t* idx, int base)
{
    return as_integer<int>( "stoi", str, idx, base );
}

int
stoi(const wstring& str, size_t* idx, int base)
{
    return as_integer<int>( "stoi", str, idx, base );
}

long
stol(const string& str, size_t* idx, int base)
{
    return as_integer<long>( "stol", str, idx, base );
}

long
stol(const wstring& str, size_t* idx, int base)
{
    return as_integer<long>( "stol", str, idx, base );
}

unsigned long
stoul(const string& str, size_t* idx, int base)
{
    return as_integer<unsigned long>( "stoul", str, idx, base );
}

unsigned long
stoul(const wstring& str, size_t* idx, int base)
{
    return as_integer<unsigned long>( "stoul", str, idx, base );
}

long long
stoll(const string& str, size_t* idx, int base)
{
    return as_integer<long long>( "stoll", str, idx, base );
}

long long
stoll(const wstring& str, size_t* idx, int base)
{
    return as_integer<long long>( "stoll", str, idx, base );
}

unsigned long long
stoull(const string& str, size_t* idx, int base)
{
    return as_integer<unsigned long long>( "stoull", str, idx, base );
}

unsigned long long
stoull(const wstring& str, size_t* idx, int base)
{
    return as_integer<unsigned long long>( "stoull", str, idx, base );
}

float
stof(const string& str, size_t* idx)
{
    return as_float<float>( "stof", str, idx );
}

float
stof(const wstring& str, size_t* idx)
{
    return as_float<float>( "stof", str, idx );
}

double
stod(const string& str, size_t* idx)
{
    return as_float<double>( "stod", str, idx );
}

double
stod(const wstring& str, size_t* idx)
{
    return as_float<double>( "stod", str, idx );
}

long double
stold(const string& str, size_t* idx)
{
    return as_float<long double>( "stold", str, idx );
}

long double
stold(const wstring& str, size_t* idx)
{
    return as_float<long double>( "stold", str, idx );
}

// to_string

namespace
{

// as_string

template<typename S, typename V >
inline
S
as_signed_string(V a)
{
    typedef typename S::value_type _type;
    const unsigned __nbuf = (numeric_limits<V>::digits / 3)
                            + ((numeric_limits<V>::digits % 3) != 0)
                            + 2;
    _type __nar[__nbuf];

    size_t len = __libcpp_put_signed_intergral(__nar, sizeof(__nar), __libcpp_locale_t(), ios_base::dec, a);
    return S(__nar, len);
}

template<typename S, typename V >
inline
S
as_unsigned_string(V a)
{
    typedef typename S::value_type _type;
    const unsigned __nbuf = (numeric_limits<V>::digits / 3)
                            + ((numeric_limits<V>::digits % 3) != 0)
                            + 2;
    _type __nar[__nbuf];

    size_t len = __libcpp_put_unsigned_intergral(__nar, sizeof(__nar), __libcpp_locale_t(), ios_base::dec, a);
    return S(__nar, len);
}

template<typename S>
inline
S
as_float_string(float __v)
{
    typedef typename S::value_type _type;
    const size_t __nbuf = 30;
    size_t __nc;
   _type __nar[__nbuf];
   _type *__nb = __nar;
    vector<_type> __nbh;
    __libcpp_locale_t __locale;
    
    __nc = __libcpp_put_float<ios_base>(__nb, __nc + 1, __locale, ios_base::dec, 6, __v);
    
	if (__nc > (__nbuf-1))
	{
	__nbh.resize(__nc + 1);
	__nb = __nbh.data();
	__nc = __libcpp_put_float<ios_base>(__nb, __nc + 1, __locale, ios_base::dec, 6, __v);
	}
    return S(__nb, __nc);
}

template<typename S>
inline
S
as_double_string(double __v)
{
    typedef typename S::value_type _type;
    const size_t __nbuf = 30;
    size_t __nc;
   _type __nar[__nbuf];
   _type *__nb = __nar;
    vector<_type> __nbh;
    __libcpp_locale_t __locale;
    
    __nc = __libcpp_put_double<ios_base>(__nb, __nc + 1, __locale, ios_base::dec, 6, __v);
    
	if (__nc > (__nbuf-1))
	{
	__nbh.resize(__nc + 1);
	__nb = __nbh.data();
	__nc = __libcpp_put_double<ios_base>(__nb, __nc + 1, __locale, ios_base::dec, 6, __v);
	}
    return S(__nb, __nc);
}

template<typename S>
inline
S
as_long_double_string(long double __v)
{
    typedef typename S::value_type _type;
    const size_t __nbuf = 30;
    size_t __nc;
   _type __nar[__nbuf];
   _type *__nb = __nar;
    vector<_type> __nbh;
    __libcpp_locale_t __locale;
    
    __nc = __libcpp_put_long_double<ios_base>(__nb, __nc + 1, __locale, ios_base::dec, 6, __v);
    
	if (__nc > (__nbuf-1))
	{
	__nbh.resize(__nc + 1);
	__nb = __nbh.data();
	__nc = __libcpp_put_long_double<ios_base>(__nb, __nc + 1, __locale, ios_base::dec, 6, __v);
	}
    return S(__nb, __nc);

}

template <class S, class V, bool = is_floating_point<V>::value>
struct initial_string;

template <class V, bool b>
struct initial_string<string, V, b>
{
    string
    operator()() const
    {
        string s;
        s.resize(s.capacity());
        return s;
    }
};

template <class V>
struct initial_string<wstring, V, false>
{
    wstring
    operator()() const
    {
        const size_t n = (numeric_limits<unsigned long long>::digits / 3)
          + ((numeric_limits<unsigned long long>::digits % 3) != 0)
          + 1;
        wstring s(n, wchar_t());
        s.resize(s.capacity());
        return s;
    }
};

template <class V>
struct initial_string<wstring, V, true>
{
    wstring
    operator()() const
    {
        wstring s(20, wchar_t());
        s.resize(s.capacity());
        return s;
    }
};

typedef int (*wide_printf)(wchar_t* __restrict, size_t, const wchar_t*__restrict, ...);

inline
wide_printf
get_swprintf()
{
#ifndef _LIBCPP_MSVCRT
    return swprintf;
#else
    return static_cast<int (__cdecl*)(wchar_t* __restrict, size_t, const wchar_t*__restrict, ...)>(swprintf);
#endif
}

}  // unnamed namespace

string to_string(int val)
{
    return as_signed_string<string, int>(val);
}

string to_string(unsigned val)
{
    return as_unsigned_string<string, unsigned>(val);
}

string to_string(long val)
{
    return as_signed_string<string, long>(val);
}

string to_string(unsigned long val)
{
    return as_unsigned_string<string, unsigned long>(val);
}

string to_string(long long val)
{
    return as_signed_string<string, long long>(val);
}

string to_string(unsigned long long val)
{
    return as_unsigned_string<string, unsigned long long>(val);
}

string to_string(float val)
{
    return as_float_string<string>(val);
}

string to_string(double val)
{
    return as_double_string<string>(val);
}

string to_string(long double val)
{
    return as_long_double_string<string>(val);
}

wstring to_wstring(int val)
{
    return as_signed_string<wstring, int>(val);
}

wstring to_wstring(unsigned val)
{
    return as_unsigned_string<wstring, unsigned>(val);
}

wstring to_wstring(long val)
{
    return as_signed_string<wstring, long>(val);
}

wstring to_wstring(unsigned long val)
{
    return as_unsigned_string<wstring, unsigned long>(val);
}

wstring to_wstring(long long val)
{
    return as_signed_string<wstring, long long>(val);
}

wstring to_wstring(unsigned long long val)
{
    return as_unsigned_string<wstring, unsigned long long>(val);
}

wstring to_wstring(float val)
{
    return as_float_string<wstring>(val);
}

wstring to_wstring(double val)
{
    return as_double_string<wstring>(val);
}

wstring to_wstring(long double val)
{
    return as_long_double_string<wstring>(val);
}
_LIBCPP_END_NAMESPACE_STD
