//===-------------------------- ios.cpp -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "__config"

#include "ios"

#include "__locale"
#include "algorithm"
#include "include/config_elast.h"
#include "istream"
#include "limits"
#include "memory"
#include "new"
#include "streambuf"
#include "string"

_LIBCPP_BEGIN_NAMESPACE_STD

template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ios<char>;
template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ios<wchar_t>;

template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_streambuf<char>;
template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_streambuf<wchar_t>;

template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_istream<char>;
template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_istream<wchar_t>;

template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ostream<char>;
template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ostream<wchar_t>;

template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_iostream<char>;

class _LIBCPP_HIDDEN __iostream_category
    : public __do_message
{
public:
    virtual const char* name() const _NOEXCEPT;
    virtual string message(int ev) const;
};

const char*
__iostream_category::name() const _NOEXCEPT
{
    return "iostream";
}

string
__iostream_category::message(int ev) const
{
    if (ev != static_cast<int>(io_errc::stream)
#ifdef _LIBCPP_ELAST
        && ev <= _LIBCPP_ELAST
#endif  // _LIBCPP_ELAST
        )
        return __do_message::message(ev);
    return string("unspecified iostream_category error");
}

const error_category&
iostream_category() _NOEXCEPT
{
    static __iostream_category s;
    return s;
}

// ios_base::failure

ios_base::failure::failure(const string& msg, const error_code& ec)
    : system_error(ec, msg)
{
}

ios_base::failure::failure(const char* msg, const error_code& ec)
    : system_error(ec, msg)
{
}

ios_base::failure::~failure() throw()
{
}

// ios_base locale

const ios_base::fmtflags ios_base::boolalpha;
const ios_base::fmtflags ios_base::dec;
const ios_base::fmtflags ios_base::fixed;
const ios_base::fmtflags ios_base::hex;
const ios_base::fmtflags ios_base::internal;
const ios_base::fmtflags ios_base::left;
const ios_base::fmtflags ios_base::oct;
const ios_base::fmtflags ios_base::right;
const ios_base::fmtflags ios_base::scientific;
const ios_base::fmtflags ios_base::showbase;
const ios_base::fmtflags ios_base::showpoint;
const ios_base::fmtflags ios_base::showpos;
const ios_base::fmtflags ios_base::skipws;
const ios_base::fmtflags ios_base::unitbuf;
const ios_base::fmtflags ios_base::uppercase;
const ios_base::fmtflags ios_base::adjustfield;
const ios_base::fmtflags ios_base::basefield;
const ios_base::fmtflags ios_base::floatfield;

const ios_base::iostate ios_base::badbit;
const ios_base::iostate ios_base::eofbit;
const ios_base::iostate ios_base::failbit;
const ios_base::iostate ios_base::goodbit;

const ios_base::openmode ios_base::app;
const ios_base::openmode ios_base::ate;
const ios_base::openmode ios_base::binary;
const ios_base::openmode ios_base::in;
const ios_base::openmode ios_base::out;
const ios_base::openmode ios_base::trunc;

void
ios_base::__call_callbacks(event ev)
{
    for (size_t i = __fn_.size(); i;)
    {
        --i;
        __fn_[i](ev, *this, __index_[i]);
    }
}

// locale

locale
ios_base::imbue(const locale& newloc)
{
    static_assert(sizeof(locale) == sizeof(__loc_), "");
    locale& loc_storage = *reinterpret_cast<locale*>(&__loc_);
    locale oldloc = loc_storage;
    loc_storage = newloc;
    __call_callbacks(imbue_event);
    return oldloc;
}

locale
ios_base::getloc() const
{
    const locale& loc_storage = *reinterpret_cast<const locale*>(&__loc_);
    return loc_storage;
}

// xalloc
#if defined(_LIBCPP_HAS_C_ATOMIC_IMP) && !defined(_LIBCPP_HAS_NO_THREADS)
atomic<int> ios_base::__xindex_ = ATOMIC_VAR_INIT(0);
#else
int ios_base::__xindex_ = 0;
#endif

template <typename _Tp>
static size_t __ios_new_cap(size_t __req_size, size_t __current_cap)
{ // Precondition: __req_size > __current_cap
	const size_t mx = std::numeric_limits<size_t>::max() / sizeof(_Tp);
	if (__req_size < mx/2)
		return _VSTD::max(2 * __current_cap, __req_size);
	else
		return mx;
}

int
ios_base::xalloc()
{
    return __xindex_++;
}

long&
ios_base::iword(int index)
{
    size_t req_size = static_cast<size_t>(index)+1;
	size_t cur_cap = __iarray_.capacity();
    if (req_size > cur_cap)
    {
        size_t newcap = __ios_new_cap<long>(req_size, cur_cap);
        __iarray_.resize(newcap, 0);
        if (req_size > __iarray_.capacity())
        {
            setstate(badbit);
            static long error;
            error = 0;
            return error;
        }
    }
    return __iarray_[index];
}

void*&
ios_base::pword(int index)
{
    size_t req_size = static_cast<size_t>(index)+1;
	size_t cur_cap = __parray_.capacity();
    if (req_size > cur_cap)
    {
        size_t newcap = __ios_new_cap<void *>(req_size, cur_cap);
        __parray_.resize(newcap, 0);
        if (req_size > __parray_.capacity())
        {
            setstate(badbit);
            static void* error;
            error = 0;
            return error;
        }
    }
    return __parray_[index];
}

// register_callback

void
ios_base::register_callback(event_callback fn, int index)
{
    size_t req_size = __fn_.size() + 1;
	size_t cur_cap = __fn_.capacity();
    if (req_size > cur_cap)
    {
        size_t newcap = __ios_new_cap<_callbacks>(req_size, cur_cap);
        __fn_.reserve(newcap);
        if (__fn_.capacity() == cur_cap)
		{
            setstate(badbit);
			return;
		}
    }
    __callbacks &cb = __fn_[req_size];
    cb.__cb = fn;
	cb.__index = index;
}

ios_base::~ios_base()
{
    __call_callbacks(erase_event);
    locale& loc_storage = *reinterpret_cast<locale*>(&__loc_);
    loc_storage.~locale();
}

// iostate

void
ios_base::clear(iostate state)
{
    if (__rdbuf_)
        __rdstate_ = state;
    else
        __rdstate_ = state | badbit;
#ifndef _LIBCPP_NO_EXCEPTIONS
    if (((state | (__rdbuf_ ? goodbit : badbit)) & __exceptions_) != 0)
        throw failure("ios_base::clear");
#endif  // _LIBCPP_NO_EXCEPTIONS
}

// init

void
ios_base::init(void* sb)
{
    __rdbuf_ = sb;
    __rdstate_ = __rdbuf_ ? goodbit : badbit;
    __exceptions_ = goodbit;
    __fmtflags_ = skipws | dec;
    __width_ = 0;
    __precision_ = 6;
    __fn_.clear();
    __iarray_.clear();
    __parray_.clear();
    ::new(&__loc_) locale;
}

void
ios_base::copyfmt(const ios_base& rhs)
{
    // If we can't acquire the needed resources, throw bad_alloc (can't set badbit)
    // Don't alter *this until all needed resources are acquired
    vector<_callbacks> new_fn;
    vector<long>    new_iarray;
    vector<void*>   new_parray;

    if (__fn_.capacity() < rhs.__fn_.size())
    {
        new_fn = rhs.__fn_;
    }
    if (__iarray_.capacity() < rhs.__iarray_.size())
	{
		new_iarray = rhs.__iarray_;
    }
    if (__parray_.capacity() < rhs.__parray_.size())
    {
		new_parray = rhs.__parray_;
    }
    // Got everything we need.  Copy everything but __rdstate_, __rdbuf_ and __exceptions_
    __fmtflags_ = rhs.__fmtflags_;
    __precision_ = rhs.__precision_;
    __width_ = rhs.__width_;
    locale& lhs_loc = *reinterpret_cast<locale*>(&__loc_);
    const locale& rhs_loc = *reinterpret_cast<const locale*>(&rhs.__loc_);
    lhs_loc = rhs_loc;
    if (new_fn.size())
    {
		__fn_.swap(new_fn);
    }
    else
	{
		__fn_ = rhs.__fn_;
	}
	if (new_iarray.size())
	{
		__iarray_.swap(new_iarray);
	}
	else
	{
		__iarray_ = rhs.__iarray_;
	}
	if (new_parray.size())
	{
		__parray_.swap(new_parray);
	}
	else
	{
		__parray_ = rhs.__parray_;
	}
}

void
ios_base::move(ios_base& rhs)
{
    // *this is uninitialized
    __fmtflags_ = rhs.__fmtflags_;
    __precision_ = rhs.__precision_;
    __width_ = rhs.__width_;
    __rdstate_ = rhs.__rdstate_;
    __exceptions_ = rhs.__exceptions_;
    __rdbuf_ = 0;
    locale& rhs_loc = *reinterpret_cast<locale*>(&rhs.__loc_);
    ::new(&__loc_) locale(rhs_loc);
    __fn_.swap(rhs.__fn_);
    rhs.__fn_.clear();
    __iarray_.swap(rhs.__iarray_);
    rhs.__iarray_.clear();
    __parray_.swap(rhs.__parray_);
    rhs.__parray_.clear();
}

void
ios_base::swap(ios_base& rhs) _NOEXCEPT
{
    _VSTD::swap(__fmtflags_, rhs.__fmtflags_);
    _VSTD::swap(__precision_, rhs.__precision_);
    _VSTD::swap(__width_, rhs.__width_);
    _VSTD::swap(__rdstate_, rhs.__rdstate_);
    _VSTD::swap(__exceptions_, rhs.__exceptions_);
    locale& lhs_loc = *reinterpret_cast<locale*>(&__loc_);
    locale& rhs_loc = *reinterpret_cast<locale*>(&rhs.__loc_);
    _VSTD::swap(lhs_loc, rhs_loc);
    _VSTD::swap(__fn_, rhs.__fn_);
    _VSTD::swap(__iarray_, rhs.__iarray_);
    _VSTD::swap(__parray_, rhs.__parray_);
}

void
ios_base::__set_badbit_and_consider_rethrow()
{
    __rdstate_ |= badbit;
#ifndef _LIBCPP_NO_EXCEPTIONS
    if (__exceptions_ & badbit)
        throw;
#endif  // _LIBCPP_NO_EXCEPTIONS
}

void
ios_base::__set_failbit_and_consider_rethrow()
{
    __rdstate_ |= failbit;
#ifndef _LIBCPP_NO_EXCEPTIONS
    if (__exceptions_ & failbit)
        throw;
#endif  // _LIBCPP_NO_EXCEPTIONS
}

bool
ios_base::sync_with_stdio(bool sync)
{
    static bool previous_state = true;
    bool r = previous_state;
    previous_state = sync;
    return r;
}

_LIBCPP_END_NAMESPACE_STD
