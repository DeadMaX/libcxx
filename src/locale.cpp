//===------------------------- locale.cpp ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define _LIBCPP_EXTERN_TEMPLATE(...) extern template __VA_ARGS__;

// On Solaris, we need to define something to make the C99 parts of localeconv
// visible.
#ifdef __sun__
#define _LCONV_C99
#endif

#include "string"
#include "locale"
#include "codecvt"
#include "vector"
#include "algorithm"
#include "typeinfo"
#ifndef _LIBCPP_NO_EXCEPTIONS
#  include "type_traits"
#endif
#include "cwctype"
#include "__sso_allocator"
#if defined(_LIBCPP_MSVCRT) || defined(__MINGW32__)
#include <support/win32/locale_win32.h>
#else // _LIBCPP_MSVCRT
#include <langinfo.h>
#endif // !_LIBCPP_MSVCRT
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for strcoll_l and strxfrm_l

// On Linux, wint_t and wchar_t have different signed-ness, and this causes
// lots of noise in the build log, but no bugs that I know of. 
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wsign-conversion"
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#ifdef __cloc_defined
locale_t __cloc() {
  // In theory this could create a race condition. In practice
  // the race condition is non-fatal since it will just create
  // a little resource leak. Better approach would be appreciated.
  static locale_t result = newlocale(LC_ALL_MASK, "C", 0);
  return result;
}
#endif // __cloc_defined

namespace {

struct release
{
    void operator()(locale::facet* p) {p->__release_shared();}
};

template <class T, class A0>
inline
T&
make(A0 a0)
{
    static typename aligned_storage<sizeof(T)>::type buf;
    ::new (&buf) T(a0);
    return *reinterpret_cast<T*>(&buf);
}

template <class T, class A0, class A1>
inline
T&
make(A0 a0, A1 a1)
{
    static typename aligned_storage<sizeof(T)>::type buf;
    ::new (&buf) T(a0, a1);
    return *reinterpret_cast<T*>(&buf);
}

template <class T, class A0, class A1, class A2>
inline
T&
make(A0 a0, A1 a1, A2 a2)
{
    static typename aligned_storage<sizeof(T)>::type buf;
    ::new (&buf) T(a0, a1, a2);
    return *reinterpret_cast<T*>(&buf);
}

template <typename T, size_t N>
inline
_LIBCPP_CONSTEXPR
size_t
countof(const T (&)[N])
{
    return N;
}

template <typename T>
inline
_LIBCPP_CONSTEXPR
size_t
countof(const T * const begin, const T * const end)
{
    return static_cast<size_t>(end - begin);
}

}

#if defined(_AIX)
// Set priority to INT_MIN + 256 + 150
# pragma priority ( -2147483242 )
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif

class _LIBCPP_HIDDEN locale::__imp
    : public facet
{
    enum {N = 28};
#if defined(_LIBCPP_MSVC)
// FIXME: MSVC doesn't support aligned parameters by value.
// I can't get the __sso_allocator to work here
// for MSVC I think for this reason.
    vector<facet*> facets_;
#else
    vector<facet*, __sso_allocator<facet*, N> > facets_;
#endif
    string         name_;
public:
    explicit __imp(size_t refs = 0);
    explicit __imp(const string& name, size_t refs = 0);
    __imp(const __imp&);
    __imp(const __imp&, const string&, locale::category c);
    __imp(const __imp& other, const __imp& one, locale::category c);
    __imp(const __imp&, facet* f, long id);
    ~__imp();

    const string& name() const {return name_;}
    bool has_facet(long id) const
        {return static_cast<size_t>(id) < facets_.size() && facets_[static_cast<size_t>(id)];}
    const locale::facet* use_facet(long id) const;

    static const locale& make_classic();
    static       locale& make_global();
private:
    void install(facet* f, long id);
    template <class F> void install(F* f) {install(f, f->id.__get());}
    template <class F> void install_from(const __imp& other);
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

locale::__imp::__imp(size_t refs)
    : facet(refs),
      facets_(N),
      name_("C")
{
    facets_.clear();
    install(&make<_VSTD::collate<char> >(1u));
    install(&make<_VSTD::collate<wchar_t> >(1u));
    install(&make<_VSTD::ctype<char> >(nullptr, false, 1u));
    install(&make<_VSTD::ctype<wchar_t> >(1u));
    install(&make<codecvt<char, char, mbstate_t> >(1u));
    install(&make<codecvt<wchar_t, char, mbstate_t> >(1u));
    install(&make<codecvt<char16_t, char, mbstate_t> >(1u));
    install(&make<codecvt<char32_t, char, mbstate_t> >(1u));
    install(&make<numpunct<char> >(1u));
    install(&make<numpunct<wchar_t> >(1u));
    install(&make<num_get<char> >(1u));
    install(&make<num_get<wchar_t> >(1u));
    install(&make<num_put<char> >(1u));
    install(&make<num_put<wchar_t> >(1u));
    install(&make<moneypunct<char, false> >(1u));
    install(&make<moneypunct<char, true> >(1u));
    install(&make<moneypunct<wchar_t, false> >(1u));
    install(&make<moneypunct<wchar_t, true> >(1u));
    install(&make<money_get<char> >(1u));
    install(&make<money_get<wchar_t> >(1u));
    install(&make<money_put<char> >(1u));
    install(&make<money_put<wchar_t> >(1u));
    install(&make<time_get<char> >(1u));
    install(&make<time_get<wchar_t> >(1u));
    install(&make<time_put<char> >(1u));
    install(&make<time_put<wchar_t> >(1u));
    install(&make<_VSTD::messages<char> >(1u));
    install(&make<_VSTD::messages<wchar_t> >(1u));
}

locale::__imp::__imp(const string& name, size_t refs)
    : facet(refs),
      facets_(N),
      name_(name)
{
#ifndef _LIBCPP_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCPP_NO_EXCEPTIONS
        facets_ = locale::classic().__locale_->facets_;
        for (unsigned i = 0; i < facets_.size(); ++i)
            if (facets_[i])
                facets_[i]->__add_shared();
        install(new collate_byname<char>(name_));
        install(new collate_byname<wchar_t>(name_));
        install(new ctype_byname<char>(name_));
        install(new ctype_byname<wchar_t>(name_));
        install(new codecvt_byname<char, char, mbstate_t>(name_));
        install(new codecvt_byname<wchar_t, char, mbstate_t>(name_));
        install(new codecvt_byname<char16_t, char, mbstate_t>(name_));
        install(new codecvt_byname<char32_t, char, mbstate_t>(name_));
        install(new numpunct_byname<char>(name_));
        install(new numpunct_byname<wchar_t>(name_));
        install(new moneypunct_byname<char, false>(name_));
        install(new moneypunct_byname<char, true>(name_));
        install(new moneypunct_byname<wchar_t, false>(name_));
        install(new moneypunct_byname<wchar_t, true>(name_));
        install(new time_get_byname<char>(name_));
        install(new time_get_byname<wchar_t>(name_));
        install(new time_put_byname<char>(name_));
        install(new time_put_byname<wchar_t>(name_));
        install(new messages_byname<char>(name_));
        install(new messages_byname<wchar_t>(name_));
#ifndef _LIBCPP_NO_EXCEPTIONS
    }
    catch (...)
    {
        for (unsigned i = 0; i < facets_.size(); ++i)
            if (facets_[i])
                facets_[i]->__release_shared();
        throw;
    }
#endif  // _LIBCPP_NO_EXCEPTIONS
}

// NOTE avoid the `base class should be explicitly initialized in the
// copy constructor` warning emitted by GCC
#if defined(__clang__) || _GNUC_VER >= 406
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#endif

locale::__imp::__imp(const __imp& other)
    : facets_(max<size_t>(N, other.facets_.size())),
      name_(other.name_)
{
    facets_ = other.facets_;
    for (unsigned i = 0; i < facets_.size(); ++i)
        if (facets_[i])
            facets_[i]->__add_shared();
}

#if defined(__clang__) || _GNUC_VER >= 406
#pragma GCC diagnostic pop
#endif

locale::__imp::__imp(const __imp& other, const string& name, locale::category c)
    : facets_(N),
      name_("*")
{
    facets_ = other.facets_;
    for (unsigned i = 0; i < facets_.size(); ++i)
        if (facets_[i])
            facets_[i]->__add_shared();
#ifndef _LIBCPP_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCPP_NO_EXCEPTIONS
        if (c & locale::collate)
        {
            install(new collate_byname<char>(name));
            install(new collate_byname<wchar_t>(name));
        }
        if (c & locale::ctype)
        {
            install(new ctype_byname<char>(name));
            install(new ctype_byname<wchar_t>(name));
            install(new codecvt_byname<char, char, mbstate_t>(name));
            install(new codecvt_byname<wchar_t, char, mbstate_t>(name));
            install(new codecvt_byname<char16_t, char, mbstate_t>(name));
            install(new codecvt_byname<char32_t, char, mbstate_t>(name));
        }
        if (c & locale::monetary)
        {
            install(new moneypunct_byname<char, false>(name));
            install(new moneypunct_byname<char, true>(name));
            install(new moneypunct_byname<wchar_t, false>(name));
            install(new moneypunct_byname<wchar_t, true>(name));
        }
        if (c & locale::numeric)
        {
            install(new numpunct_byname<char>(name));
            install(new numpunct_byname<wchar_t>(name));
        }
        if (c & locale::time)
        {
            install(new time_get_byname<char>(name));
            install(new time_get_byname<wchar_t>(name));
            install(new time_put_byname<char>(name));
            install(new time_put_byname<wchar_t>(name));
        }
        if (c & locale::messages)
        {
            install(new messages_byname<char>(name));
            install(new messages_byname<wchar_t>(name));
        }
#ifndef _LIBCPP_NO_EXCEPTIONS
    }
    catch (...)
    {
        for (unsigned i = 0; i < facets_.size(); ++i)
            if (facets_[i])
                facets_[i]->__release_shared();
        throw;
    }
#endif  // _LIBCPP_NO_EXCEPTIONS
}

template<class F>
inline
void
locale::__imp::install_from(const locale::__imp& one)
{
    long id = F::id.__get();
    install(const_cast<F*>(static_cast<const F*>(one.use_facet(id))), id);
}

locale::__imp::__imp(const __imp& other, const __imp& one, locale::category c)
    : facets_(N),
      name_("*")
{
    facets_ = other.facets_;
    for (unsigned i = 0; i < facets_.size(); ++i)
        if (facets_[i])
            facets_[i]->__add_shared();
#ifndef _LIBCPP_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCPP_NO_EXCEPTIONS
        if (c & locale::collate)
        {
            install_from<_VSTD::collate<char> >(one);
            install_from<_VSTD::collate<wchar_t> >(one);
        }
        if (c & locale::ctype)
        {
            install_from<_VSTD::ctype<char> >(one);
            install_from<_VSTD::ctype<wchar_t> >(one);
            install_from<_VSTD::codecvt<char, char, mbstate_t> >(one);
            install_from<_VSTD::codecvt<char16_t, char, mbstate_t> >(one);
            install_from<_VSTD::codecvt<char32_t, char, mbstate_t> >(one);
            install_from<_VSTD::codecvt<wchar_t, char, mbstate_t> >(one);
        }
        if (c & locale::monetary)
        {
            install_from<moneypunct<char, false> >(one);
            install_from<moneypunct<char, true> >(one);
            install_from<moneypunct<wchar_t, false> >(one);
            install_from<moneypunct<wchar_t, true> >(one);
            install_from<money_get<char> >(one);
            install_from<money_get<wchar_t> >(one);
            install_from<money_put<char> >(one);
            install_from<money_put<wchar_t> >(one);
        }
        if (c & locale::numeric)
        {
            install_from<numpunct<char> >(one);
            install_from<numpunct<wchar_t> >(one);
            install_from<num_get<char> >(one);
            install_from<num_get<wchar_t> >(one);
            install_from<num_put<char> >(one);
            install_from<num_put<wchar_t> >(one);
        }
        if (c & locale::time)
        {
            install_from<time_get<char> >(one);
            install_from<time_get<wchar_t> >(one);
            install_from<time_put<char> >(one);
            install_from<time_put<wchar_t> >(one);
        }
        if (c & locale::messages)
        {
            install_from<_VSTD::messages<char> >(one);
            install_from<_VSTD::messages<wchar_t> >(one);
        }
#ifndef _LIBCPP_NO_EXCEPTIONS
    }
    catch (...)
    {
        for (unsigned i = 0; i < facets_.size(); ++i)
            if (facets_[i])
                facets_[i]->__release_shared();
        throw;
    }
#endif  // _LIBCPP_NO_EXCEPTIONS
}

locale::__imp::__imp(const __imp& other, facet* f, long id)
    : facets_(max<size_t>(N, other.facets_.size()+1)),
      name_("*")
{
    f->__add_shared();
    unique_ptr<facet, release> hold(f);
    facets_ = other.facets_;
    for (unsigned i = 0; i < other.facets_.size(); ++i)
        if (facets_[i])
            facets_[i]->__add_shared();
    install(hold.get(), id);
}

locale::__imp::~__imp()
{
    for (unsigned i = 0; i < facets_.size(); ++i)
        if (facets_[i])
            facets_[i]->__release_shared();
}

void
locale::__imp::install(facet* f, long id)
{
    f->__add_shared();
    unique_ptr<facet, release> hold(f);
    if (static_cast<size_t>(id) >= facets_.size())
        facets_.resize(static_cast<size_t>(id+1));
    if (facets_[static_cast<size_t>(id)])
        facets_[static_cast<size_t>(id)]->__release_shared();
    facets_[static_cast<size_t>(id)] = hold.release();
}

const locale::facet*
locale::__imp::use_facet(long id) const
{
#ifndef _LIBCPP_NO_EXCEPTIONS
    if (!has_facet(id))
        throw bad_cast();
#endif  // _LIBCPP_NO_EXCEPTIONS
    return facets_[static_cast<size_t>(id)];
}

// locale

const locale&
locale::__imp::make_classic()
{
    // only one thread can get in here and it only gets in once
    static aligned_storage<sizeof(locale)>::type buf;
    locale* c = reinterpret_cast<locale*>(&buf);
    c->__locale_ = &make<__imp>(1u);
    return *c;
}

const locale&
locale::classic()
{
    static const locale& c = __imp::make_classic();
    return c;
}

locale&
locale::__imp::make_global()
{
    // only one thread can get in here and it only gets in once
    static aligned_storage<sizeof(locale)>::type buf;
    ::new (&buf) locale(locale::classic());
    return *reinterpret_cast<locale*>(&buf);
}

locale&
locale::__global()
{
    static locale& g = __imp::make_global();
    return g;
}

locale::locale()  _NOEXCEPT
    : __locale_(__global().__locale_)
{
    __locale_->__add_shared();
}

locale::locale(const locale& l)  _NOEXCEPT
    : __locale_(l.__locale_)
{
    __locale_->__add_shared();
}

locale::~locale()
{
    __locale_->__release_shared();
}

const locale&
locale::operator=(const locale& other)  _NOEXCEPT
{
    other.__locale_->__add_shared();
    __locale_->__release_shared();
    __locale_ = other.__locale_;
    return *this;
}

locale::locale(const char* name)
#ifndef _LIBCPP_NO_EXCEPTIONS
    : __locale_(name ? new __imp(name)
                     : throw runtime_error("locale constructed with null"))
#else  // _LIBCPP_NO_EXCEPTIONS
    : __locale_(new __imp(name))
#endif
{
    __locale_->__add_shared();
}

locale::locale(const string& name)
    : __locale_(new __imp(name))
{
    __locale_->__add_shared();
}

locale::locale(const locale& other, const char* name, category c)
#ifndef _LIBCPP_NO_EXCEPTIONS
    : __locale_(name ? new __imp(*other.__locale_, name, c)
                     : throw runtime_error("locale constructed with null"))
#else  // _LIBCPP_NO_EXCEPTIONS
    : __locale_(new __imp(*other.__locale_, name, c))
#endif
{
    __locale_->__add_shared();
}

locale::locale(const locale& other, const string& name, category c)
    : __locale_(new __imp(*other.__locale_, name, c))
{
    __locale_->__add_shared();
}

locale::locale(const locale& other, const locale& one, category c)
    : __locale_(new __imp(*other.__locale_, *one.__locale_, c))
{
    __locale_->__add_shared();
}

string
locale::name() const
{
    return __locale_->name();
}

void
locale::__install_ctor(const locale& other, facet* f, long id)
{
    if (f)
        __locale_ = new __imp(*other.__locale_, f, id);
    else
        __locale_ = other.__locale_;
    __locale_->__add_shared();
}

locale
locale::global(const locale& loc)
{
    locale& g = __global();
    locale r = g;
    g = loc;
    return r;
}

bool
locale::has_facet(id& x) const
{
    return __locale_->has_facet(x.__get());
}

const locale::facet*
locale::use_facet(id& x) const
{
    return __locale_->use_facet(x.__get());
}

bool
locale::operator==(const locale& y) const
{
    return (__locale_ == y.__locale_)
        || (__locale_->name() != "*" && __locale_->name() == y.__locale_->name());
}

// locale::facet

locale::facet::~facet()
{
}

void
locale::facet::__on_zero_shared() _NOEXCEPT
{
    delete this;
}

// locale::id

int32_t locale::id::__next_id = 0;

namespace
{

class __fake_bind
{
    locale::id* id_;
    void (locale::id::* pmf_)();
public:
    __fake_bind(void (locale::id::* pmf)(), locale::id* id)
        : id_(id), pmf_(pmf) {}

    void operator()() const
    {
        (id_->*pmf_)();
    }
};

}

long
locale::id::__get()
{
    call_once(__flag_, __fake_bind(&locale::id::__init, this));
    return __id_ - 1;
}

void
locale::id::__init()
{
    __id_ = __sync_add_and_fetch(&__next_id, 1);
}

// template <> class collate_byname<char>

collate_byname<char>::collate_byname(const char* n, size_t refs)
    : collate<char>(refs)
{
  // TODO: load locale definition
}

collate_byname<char>::collate_byname(const string& name, size_t refs)
    : collate<char>(refs)
{
  // TODO: load locale definition
}

collate_byname<char>::~collate_byname()
{
    // TODO: clear locale definition
}

int
collate_byname<char>::do_compare(const char_type* __lo1, const char_type* __hi1,
                                 const char_type* __lo2, const char_type* __hi2) const
{
  // TODO: Make comparision
  while (*__lo1 == *__lo2)
  {
    if (__lo1 == __hi1)
    {
      if (__lo2 == __hi2)
	return 0;
      return -1;
    }
    if (__lo2 == __hi2)
    {
      return 1;
    }
    __lo1++;
    __lo2++;
  }
  if (*__lo1 < *__lo2)
    return -1;
  return 1;
}

collate_byname<char>::string_type
collate_byname<char>::do_transform(const char_type* lo, const char_type* hi) const
{
    /* TODO do transformation */
    return string_type(lo, hi);
}

// template <> class collate_byname<wchar_t>

collate_byname<wchar_t>::collate_byname(const char* n, size_t refs)
    : collate<wchar_t>(refs)
{
  /* TODO load locale definition */
}

collate_byname<wchar_t>::collate_byname(const string& name, size_t refs)
    : collate<wchar_t>(refs)
{
  /* TODO load locale definition */
}

collate_byname<wchar_t>::~collate_byname()
{
  /* TODO free locale */
}

int
collate_byname<wchar_t>::do_compare(const char_type* __lo1, const char_type* __hi1,
                                 const char_type* __lo2, const char_type* __hi2) const
{
  // TODO: Make comparision
  while (*__lo1 == *__lo2)
  {
    if (__lo1 == __hi1)
    {
      if (__lo2 == __hi2)
	return 0;
      return -1;
    }
    if (__lo2 == __hi2)
    {
      return 1;
    }
    __lo1++;
    __lo2++;
  }
  if (*__lo1 < *__lo2)
    return -1;
  return 1;
}

collate_byname<wchar_t>::string_type
collate_byname<wchar_t>::do_transform(const char_type* lo, const char_type* hi) const
{
    /* TODO Do transformation */
    return string_type(lo, hi);
}

// template <> class ctype<wchar_t>;

const ctype_base::mask ctype_base::space;
const ctype_base::mask ctype_base::print;
const ctype_base::mask ctype_base::cntrl;
const ctype_base::mask ctype_base::upper;
const ctype_base::mask ctype_base::lower;
const ctype_base::mask ctype_base::alpha;
const ctype_base::mask ctype_base::digit;
const ctype_base::mask ctype_base::punct;
const ctype_base::mask ctype_base::xdigit;
const ctype_base::mask ctype_base::blank;
const ctype_base::mask ctype_base::alnum;
const ctype_base::mask ctype_base::graph;
    
locale::id ctype<wchar_t>::id;

ctype<wchar_t>::~ctype()
{
}

bool
ctype<wchar_t>::do_is(mask m, char_type c) const
{
    return isascii(c) ? (ctype<char>::classic_table()[c] & m) != 0 : false;
}

const wchar_t*
ctype<wchar_t>::do_is(const char_type* low, const char_type* high, mask* vec) const
{
    for (; low != high; ++low, ++vec)
        *vec = static_cast<mask>(isascii(*low) ?
                                   ctype<char>::classic_table()[*low] : 0);
    return low;
}

const wchar_t*
ctype<wchar_t>::do_scan_is(mask m, const char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
        if (isascii(*low) && (ctype<char>::classic_table()[*low] & m))
            break;
    return low;
}

const wchar_t*
ctype<wchar_t>::do_scan_not(mask m, const char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
        if (!(isascii(*low) && (ctype<char>::classic_table()[*low] & m)))
            break;
    return low;
}

wchar_t
ctype<wchar_t>::do_toupper(char_type c) const
{
	// TODO upperization
	if (c >= 'a' && c <= 'z')
		c = c - 'a' + 'A';
	return c;
}

const wchar_t*
ctype<wchar_t>::do_toupper(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
        *low = do_toupper(*low);
    return low;
}

wchar_t
ctype<wchar_t>::do_tolower(char_type c) const
{
	// TODO lowerization
	if (c >= 'A' && c <= 'Z')
		c = c - 'A' + 'a';
	return c;
}

const wchar_t*
ctype<wchar_t>::do_tolower(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
	*low = do_tolower(*low);
    return low;
}

wchar_t
ctype<wchar_t>::do_widen(char c) const
{
    return c;
}

const char*
ctype<wchar_t>::do_widen(const char* low, const char* high, char_type* dest) const
{
    for (; low != high; ++low, ++dest)
        *dest = *low;
    return low;
}

char
ctype<wchar_t>::do_narrow(char_type c, char dfault) const
{
    if (isascii(c))
        return static_cast<char>(c);
    return dfault;
}

const wchar_t*
ctype<wchar_t>::do_narrow(const char_type* low, const char_type* high, char dfault, char* dest) const
{
    for (; low != high; ++low, ++dest)
        if (isascii(*low))
            *dest = static_cast<char>(*low);
        else
            *dest = dfault;
    return low;
}

// template <> class ctype<char>;

locale::id ctype<char>::id;

ctype<char>::ctype(const mask* tab, bool del, size_t refs)
    : locale::facet(refs),
      __tab_(tab),
      __del_(del)
{
  if (__tab_ == 0)
      __tab_ = classic_table();
}

ctype<char>::~ctype()
{
    if (__tab_ && __del_)
        delete [] __tab_;
}

char
ctype<char>::do_toupper(char_type c) const
{
	if (c >= 'a' && c <= 'z')
		c = c - 'a' + 'A';
	return c;
}

const char*
ctype<char>::do_toupper(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
	*low = do_toupper(*low);
    return low;
}

char
ctype<char>::do_tolower(char_type c) const
{
	if (c >= 'A' && c <= 'Z')
		c = c - 'A' + 'a';
	return c;
}

const char*
ctype<char>::do_tolower(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
	*low = do_tolower(*low);
    return low;
}

char
ctype<char>::do_widen(char c) const
{
    return c;
}

const char*
ctype<char>::do_widen(const char* low, const char* high, char_type* dest) const
{
    for (; low != high; ++low, ++dest)
        *dest = *low;
    return low;
}

char
ctype<char>::do_narrow(char_type c, char dfault) const
{
    if (isascii(c))
        return static_cast<char>(c);
    return dfault;
}

const char*
ctype<char>::do_narrow(const char_type* low, const char_type* high, char dfault, char* dest) const
{
    for (; low != high; ++low, ++dest)
        if (isascii(*low))
            *dest = *low;
        else
            *dest = dfault;
    return low;
}

#ifdef __EMSCRIPTEN__
extern "C" const unsigned short ** __ctype_b_loc();
extern "C" const int ** __ctype_tolower_loc();
extern "C" const int ** __ctype_toupper_loc();
#endif

const ctype<char>::mask*
ctype<char>::classic_table()  _NOEXCEPT
{
  static const ctype<char>::mask table[256] =
  {
      /* 0x00 */ cntrl,
      /* 0x01 */ cntrl,
      /* 0x02 */ cntrl,
      /* 0x03 */ cntrl,
      /* 0x04 */ cntrl,
      /* 0x05 */ cntrl,
      /* 0x06 */ cntrl,
      /* 0x07 */ cntrl,
      /* 0x08 */ cntrl,
      /* TAB  */ cntrl | blank | space,
      /* 0x0A */ cntrl | space,
      /* 0x0B */ cntrl | space,
      /* 0x0C */ cntrl | space,
      /* 0x0D */ cntrl | space,
      /* 0x0E */ cntrl,
      /* 0x0F */ cntrl,
      /* 0x10 */ cntrl,
      /* 0x11 */ cntrl,
      /* 0x12 */ cntrl,
      /* 0x13 */ cntrl,
      /* 0x14 */ cntrl,
      /* 0x15 */ cntrl,
      /* 0x16 */ cntrl,
      /* 0x17 */ cntrl,
      /* 0x18 */ cntrl,
      /* 0x19 */ cntrl,
      /* 0x1A */ cntrl,
      /* 0x1B */ cntrl,
      /* 0x1C */ cntrl,
      /* 0x1D */ cntrl,
      /* 0x1E */ cntrl,
      /* 0x1F */ cntrl,
      /* SPCE */ blank | space | print,
      /* 0x21 */ punct | graph | print,
      /* 0x22 */ punct | graph | print,
      /* 0x23 */ punct | graph | print,
      /* 0x24 */ punct | graph | print,
      /* 0x25 */ punct | graph | print,
      /* 0x26 */ punct | graph | print,
      /* 0x27 */ punct | graph | print,
      /* 0x28 */ punct | graph | print,
      /* 0x29 */ punct | graph | print,
      /* 0x2A */ punct | graph | print,
      /* 0x2B */ punct | graph | print,
      /* 0x2C */ punct | graph | print,
      /* 0x2D */ punct | graph | print,
      /* 0x2E */ punct | graph | print,
      /* 0x2F */ punct | graph | print,
      /* '0'  */ digit | xdigit | alnum | graph | print,
      /* '1'  */ digit | xdigit | alnum | graph | print,
      /* '2'  */ digit | xdigit | alnum | graph | print,
      /* '3'  */ digit | xdigit | alnum | graph | print,
      /* '4'  */ digit | xdigit | alnum | graph | print,
      /* '5'  */ digit | xdigit | alnum | graph | print,
      /* '6'  */ digit | xdigit | alnum | graph | print,
      /* '7'  */ digit | xdigit | alnum | graph | print,
      /* '8'  */ digit | xdigit | alnum | graph | print,
      /* '9'  */ digit | xdigit | alnum | graph | print,
      /* 0x3A */ punct | graph | print,
      /* 0x3B */ punct | graph | print,
      /* 0x3C */ punct | graph | print,
      /* 0x3D */ punct | graph | print,
      /* 0x3E */ punct | graph | print,
      /* 0x3F */ punct | graph | print,
      /* 'A'  */ upper | alpha | xdigit | alnum | graph | print,
      /* 'B'  */ upper | alpha | xdigit | alnum | graph | print,
      /* 'C'  */ upper | alpha | xdigit | alnum | graph | print,
      /* 'D'  */ upper | alpha | xdigit | alnum | graph | print,
      /* 'E'  */ upper | alpha | xdigit | alnum | graph | print,
      /* 'F'  */ upper | alpha | xdigit | alnum | graph | print,
      /* 'G'  */ upper | alpha | alnum | graph | print,
      /* 'H'  */ upper | alpha | alnum | graph | print,
      /* 'I'  */ upper | alpha | alnum | graph | print,
      /* 'J'  */ upper | alpha | alnum | graph | print,
      /* 'K'  */ upper | alpha | alnum | graph | print,
      /* 'L'  */ upper | alpha | alnum | graph | print,
      /* 'M'  */ upper | alpha | alnum | graph | print,
      /* 'N'  */ upper | alpha | alnum | graph | print,
      /* 'O'  */ upper | alpha | alnum | graph | print,
      /* 'P'  */ upper | alpha | alnum | graph | print,
      /* 'Q'  */ upper | alpha | alnum | graph | print,
      /* 'R'  */ upper | alpha | alnum | graph | print,
      /* 'S'  */ upper | alpha | alnum | graph | print,
      /* 'T'  */ upper | alpha | alnum | graph | print,
      /* 'U'  */ upper | alpha | alnum | graph | print,
      /* 'V'  */ upper | alpha | alnum | graph | print,
      /* 'W'  */ upper | alpha | alnum | graph | print,
      /* 'X'  */ upper | alpha | alnum | graph | print,
      /* 'Y'  */ upper | alpha | alnum | graph | print,
      /* 'Z'  */ upper | alpha | alnum | graph | print,
      /* 0x5B */ punct | graph | print,
      /* 0x5C */ punct | graph | print,
      /* 0x5D */ punct | graph | print,
      /* 0x5E */ punct | graph | print,
      /* 0x5F */ punct | graph | print,
      /* 0x60 */ punct | graph | print,
      /* 'a'  */ lower | alpha | xdigit | alnum | graph | print,
      /* 'b'  */ lower | alpha | xdigit | alnum | graph | print,
      /* 'c'  */ lower | alpha | xdigit | alnum | graph | print,
      /* 'd'  */ lower | alpha | xdigit | alnum | graph | print,
      /* 'e'  */ lower | alpha | xdigit | alnum | graph | print,
      /* 'f'  */ lower | alpha | xdigit | alnum | graph | print,
      /* 'g'  */ lower | alpha | alnum | graph | print,
      /* 'h'  */ lower | alpha | alnum | graph | print,
      /* 'i'  */ lower | alpha | alnum | graph | print,
      /* 'j'  */ lower | alpha | alnum | graph | print,
      /* 'k'  */ lower | alpha | alnum | graph | print,
      /* 'l'  */ lower | alpha | alnum | graph | print,
      /* 'm'  */ lower | alpha | alnum | graph | print,
      /* 'n'  */ lower | alpha | alnum | graph | print,
      /* 'o'  */ lower | alpha | alnum | graph | print,
      /* 'p'  */ lower | alpha | alnum | graph | print,
      /* 'q'  */ lower | alpha | alnum | graph | print,
      /* 'r'  */ lower | alpha | alnum | graph | print,
      /* 's'  */ lower | alpha | alnum | graph | print,
      /* 't'  */ lower | alpha | alnum | graph | print,
      /* 'u'  */ lower | alpha | alnum | graph | print,
      /* 'v'  */ lower | alpha | alnum | graph | print,
      /* 'w'  */ lower | alpha | alnum | graph | print,
      /* 'x'  */ lower | alpha | alnum | graph | print,
      /* 'y'  */ lower | alpha | alnum | graph | print,
      /* 'z'  */ lower | alpha | alnum | graph | print,
      /* 0x7B */ punct | graph | print,
      /* 0x7C */ punct | graph | print,
      /* 0x7D */ punct | graph | print,
      /* 0x7E */ punct | graph | print,
      /* 0x7F */ cntrl,
      
      /* Remain to 0 */
  };
  return table;
}

// template <> class ctype_byname<char>

ctype_byname<char>::ctype_byname(const char* name, size_t refs)
    : ctype<char>(0, false, refs)
{
  // TODO: load locale definition
}

ctype_byname<char>::ctype_byname(const string& name, size_t refs)
    : ctype<char>(0, false, refs)
{
  // TODO: load locale definition
}

ctype_byname<char>::~ctype_byname()
{
    // TODO: free locale definition
}

char
ctype_byname<char>::do_toupper(char_type c) const
{
    // TODO localized upper
    if (c >= 'a' && c <= 'z')
	c += 'A' - 'a';
    return c;
}

const char*
ctype_byname<char>::do_toupper(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
        *low = do_toupper(*low);
    return low;
}

char
ctype_byname<char>::do_tolower(char_type c) const
{
    // TODO localized upper
    if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
    return c;

}

const char*
ctype_byname<char>::do_tolower(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
        *low = do_tolower(*low);
    return low;
}

// template <> class ctype_byname<wchar_t>

ctype_byname<wchar_t>::ctype_byname(const char* name, size_t refs)
    : ctype<wchar_t>(refs)
{
  // TODO load locale
}

ctype_byname<wchar_t>::ctype_byname(const string& name, size_t refs)
    : ctype<wchar_t>(refs)
{
    // TODO load locale
}

ctype_byname<wchar_t>::~ctype_byname()
{
    // TODO free locale
}

bool
ctype_byname<wchar_t>::do_is(mask m, char_type c) const
{
  // TODO wchar support ?
  return isascii(c) ? (ctype<char>::classic_table()[c] & m) != 0 : false;
}

const wchar_t*
ctype_byname<wchar_t>::do_is(const char_type* low, const char_type* high, mask* vec) const
{
    for (; low != high; ++low, ++vec)
    {
        if (isascii(*low))
            *vec = ctype<char>::classic_table()[*low];
        else
	  // TODO wchar support ?
            *vec = 0;
    }
    return low;
}

const wchar_t*
ctype_byname<wchar_t>::do_scan_is(mask m, const char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
    {
      if (do_is(m, *low))
	break;
    }
    return low;
}

const wchar_t*
ctype_byname<wchar_t>::do_scan_not(mask m, const char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
    {
      if (!do_is(m, *low))
	break;
    }
    return low;
}

wchar_t
ctype_byname<wchar_t>::do_toupper(char_type c) const
{
  // TODO wchar support ?
  if (c >= 'a' && c <= 'z')
	c += 'A' - 'a';
  return c;
}

const wchar_t*
ctype_byname<wchar_t>::do_toupper(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
        *low = do_toupper(*low);
    return low;
}

wchar_t
ctype_byname<wchar_t>::do_tolower(char_type c) const
{
  // TODO wchar support ?
  if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
  return c;
}

const wchar_t*
ctype_byname<wchar_t>::do_tolower(char_type* low, const char_type* high) const
{
    for (; low != high; ++low)
        *low = do_tolower(*low);
    return low;
}

wchar_t
ctype_byname<wchar_t>::do_widen(char c) const
{
  // TODO charset conversion
  return c;
}

const char*
ctype_byname<wchar_t>::do_widen(const char* low, const char* high, char_type* dest) const
{
    for (; low != high; ++low, ++dest)
      *dest = do_widen(*low);
    return low;
}

char
ctype_byname<wchar_t>::do_narrow(char_type c, char dfault) const
{
  // TODO charset conversion
    if (c > 127)
      return dfault;
    return c;
}

const wchar_t*
ctype_byname<wchar_t>::do_narrow(const char_type* low, const char_type* high, char dfault, char* dest) const
{
    for (; low != high; ++low, ++dest)
    {
        *dest = do_narrow(*low, dfault);
    }
    return low;
}

// template <> class codecvt<char, char, mbstate_t>

locale::id codecvt<char, char, mbstate_t>::id;

codecvt<char, char, mbstate_t>::~codecvt()
{
}

codecvt<char, char, mbstate_t>::result
codecvt<char, char, mbstate_t>::do_out(state_type&,
    const intern_type* frm, const intern_type*, const intern_type*& frm_nxt,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    frm_nxt = frm;
    to_nxt = to;
    return noconv;
}

codecvt<char, char, mbstate_t>::result
codecvt<char, char, mbstate_t>::do_in(state_type&,
    const extern_type* frm, const extern_type*, const extern_type*& frm_nxt,
    intern_type* to, intern_type*, intern_type*& to_nxt) const
{
    frm_nxt = frm;
    to_nxt = to;
    return noconv;
}

codecvt<char, char, mbstate_t>::result
codecvt<char, char, mbstate_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
codecvt<char, char, mbstate_t>::do_encoding() const  _NOEXCEPT
{
    return 1;
}

bool
codecvt<char, char, mbstate_t>::do_always_noconv() const  _NOEXCEPT
{
    return true;
}

int
codecvt<char, char, mbstate_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* end, size_t mx) const
{
    return static_cast<int>(min<size_t>(mx, static_cast<size_t>(end-frm)));
}

int
codecvt<char, char, mbstate_t>::do_max_length() const  _NOEXCEPT
{
    return 1;
}

// template <> class codecvt<wchar_t, char, mbstate_t>

locale::id codecvt<wchar_t, char, mbstate_t>::id;

codecvt<wchar_t, char, mbstate_t>::codecvt(size_t refs)
    : locale::facet(refs)
{
  // TODO load locale
}

codecvt<wchar_t, char, mbstate_t>::codecvt(const char* nm, size_t refs)
    : locale::facet(refs)
{
  // TODO load locale
}

codecvt<wchar_t, char, mbstate_t>::~codecvt()
{
  // TODO free locale
}

codecvt<wchar_t, char, mbstate_t>::result
codecvt<wchar_t, char, mbstate_t>::do_out(state_type& st,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
  // TODO charset conversion
    return noconv;
}

codecvt<wchar_t, char, mbstate_t>::result
codecvt<wchar_t, char, mbstate_t>::do_in(state_type& st,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
  // TODO charset conversion
    return noconv;
}

codecvt<wchar_t, char, mbstate_t>::result
codecvt<wchar_t, char, mbstate_t>::do_unshift(state_type& st,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
  // TODO charset conversion
    to_nxt = to;
    return noconv;
}

int
codecvt<wchar_t, char, mbstate_t>::do_encoding() const  _NOEXCEPT
{
  // TODO check that ...
   return 1;                // which take more than 1 char to form a wchar_t
}

bool
codecvt<wchar_t, char, mbstate_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
codecvt<wchar_t, char, mbstate_t>::do_length(state_type& st,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
  // TODO
    return mx;
}

int
codecvt<wchar_t, char, mbstate_t>::do_max_length() const  _NOEXCEPT
{
  // TODO
  return 1;
}

//                                     Valid UTF ranges
//     UTF-32               UTF-16                          UTF-8               # of code points
//                     first      second       first   second    third   fourth
// 000000 - 00007F  0000 - 007F               00 - 7F                                 127
// 000080 - 0007FF  0080 - 07FF               C2 - DF, 80 - BF                       1920
// 000800 - 000FFF  0800 - 0FFF               E0 - E0, A0 - BF, 80 - BF              2048
// 001000 - 00CFFF  1000 - CFFF               E1 - EC, 80 - BF, 80 - BF             49152
// 00D000 - 00D7FF  D000 - D7FF               ED - ED, 80 - 9F, 80 - BF              2048
// 00D800 - 00DFFF                invalid
// 00E000 - 00FFFF  E000 - FFFF               EE - EF, 80 - BF, 80 - BF              8192
// 010000 - 03FFFF  D800 - D8BF, DC00 - DFFF  F0 - F0, 90 - BF, 80 - BF, 80 - BF   196608
// 040000 - 0FFFFF  D8C0 - DBBF, DC00 - DFFF  F1 - F3, 80 - BF, 80 - BF, 80 - BF   786432
// 100000 - 10FFFF  DBC0 - DBFF, DC00 - DFFF  F4 - F4, 80 - 8F, 80 - BF, 80 - BF    65536

static
codecvt_base::result
utf16_to_utf8(const uint16_t* frm, const uint16_t* frm_end, const uint16_t*& frm_nxt,
              uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
              unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 3)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(0xEF);
        *to_nxt++ = static_cast<uint8_t>(0xBB);
        *to_nxt++ = static_cast<uint8_t>(0xBF);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint16_t wc1 = *frm_nxt;
        if (wc1 > Maxcode)
            return codecvt_base::error;
        if (wc1 < 0x0080)
        {
            if (to_end-to_nxt < 1)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(wc1);
        }
        else if (wc1 < 0x0800)
        {
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xC0 | (wc1 >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 | (wc1 & 0x03F));
        }
        else if (wc1 < 0xD800)
        {
            if (to_end-to_nxt < 3)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc1 >> 12));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc1 & 0x0FC0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc1 & 0x003F));
        }
        else if (wc1 < 0xDC00)
        {
            if (frm_end-frm_nxt < 2)
                return codecvt_base::partial;
            uint16_t wc2 = frm_nxt[1];
            if ((wc2 & 0xFC00) != 0xDC00)
                return codecvt_base::error;
            if (to_end-to_nxt < 4)
                return codecvt_base::partial;
            if (((((wc1 & 0x03C0UL) >> 6) + 1) << 16) +
                ((wc1 & 0x003FUL) << 10) + (wc2 & 0x03FF) > Maxcode)
                return codecvt_base::error;
            ++frm_nxt;
            uint8_t z = ((wc1 & 0x03C0) >> 6) + 1;
            *to_nxt++ = static_cast<uint8_t>(0xF0 | (z >> 2));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((z & 0x03) << 4)     | ((wc1 & 0x003C) >> 2));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc1 & 0x0003) << 4) | ((wc2 & 0x03C0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc2 & 0x003F));
        }
        else if (wc1 < 0xE000)
        {
            return codecvt_base::error;
        }
        else
        {
            if (to_end-to_nxt < 3)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc1 >> 12));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc1 & 0x0FC0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc1 & 0x003F));
        }
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf16_to_utf8(const uint32_t* frm, const uint32_t* frm_end, const uint32_t*& frm_nxt,
              uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
              unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 3)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(0xEF);
        *to_nxt++ = static_cast<uint8_t>(0xBB);
        *to_nxt++ = static_cast<uint8_t>(0xBF);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint16_t wc1 = static_cast<uint16_t>(*frm_nxt);
        if (wc1 > Maxcode)
            return codecvt_base::error;
        if (wc1 < 0x0080)
        {
            if (to_end-to_nxt < 1)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(wc1);
        }
        else if (wc1 < 0x0800)
        {
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xC0 | (wc1 >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 | (wc1 & 0x03F));
        }
        else if (wc1 < 0xD800)
        {
            if (to_end-to_nxt < 3)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc1 >> 12));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc1 & 0x0FC0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc1 & 0x003F));
        }
        else if (wc1 < 0xDC00)
        {
            if (frm_end-frm_nxt < 2)
                return codecvt_base::partial;
            uint16_t wc2 = static_cast<uint16_t>(frm_nxt[1]);
            if ((wc2 & 0xFC00) != 0xDC00)
                return codecvt_base::error;
            if (to_end-to_nxt < 4)
                return codecvt_base::partial;
            if (((((wc1 & 0x03C0UL) >> 6) + 1) << 16) +
                ((wc1 & 0x003FUL) << 10) + (wc2 & 0x03FF) > Maxcode)
                return codecvt_base::error;
            ++frm_nxt;
            uint8_t z = ((wc1 & 0x03C0) >> 6) + 1;
            *to_nxt++ = static_cast<uint8_t>(0xF0 | (z >> 2));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((z & 0x03) << 4)     | ((wc1 & 0x003C) >> 2));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc1 & 0x0003) << 4) | ((wc2 & 0x03C0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc2 & 0x003F));
        }
        else if (wc1 < 0xE000)
        {
            return codecvt_base::error;
        }
        else
        {
            if (to_end-to_nxt < 3)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc1 >> 12));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc1 & 0x0FC0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc1 & 0x003F));
        }
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf8_to_utf16(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
              uint16_t* to, uint16_t* to_end, uint16_t*& to_nxt,
              unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 3 && frm_nxt[0] == 0xEF && frm_nxt[1] == 0xBB &&
                                                          frm_nxt[2] == 0xBF)
            frm_nxt += 3;
    }
    for (; frm_nxt < frm_end && to_nxt < to_end; ++to_nxt)
    {
        uint8_t c1 = *frm_nxt;
        if (c1 > Maxcode)
            return codecvt_base::error;
        if (c1 < 0x80)
        {
            *to_nxt = static_cast<uint16_t>(c1);
            ++frm_nxt;
        }
        else if (c1 < 0xC2)
        {
            return codecvt_base::error;
        }
        else if (c1 < 0xE0)
        {
            if (frm_end-frm_nxt < 2)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            if ((c2 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint16_t t = static_cast<uint16_t>(((c1 & 0x1F) << 6) | (c2 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 2;
        }
        else if (c1 < 0xF0)
        {
            if (frm_end-frm_nxt < 3)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            switch (c1)
            {
            case 0xE0:
                if ((c2 & 0xE0) != 0xA0)
                    return codecvt_base::error;
                 break;
            case 0xED:
                if ((c2 & 0xE0) != 0x80)
                    return codecvt_base::error;
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return codecvt_base::error;
                 break;
            }
            if ((c3 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint16_t t = static_cast<uint16_t>(((c1 & 0x0F) << 12)
                                             | ((c2 & 0x3F) << 6)
                                             |  (c3 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 3;
        }
        else if (c1 < 0xF5)
        {
            if (frm_end-frm_nxt < 4)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            uint8_t c4 = frm_nxt[3];
            switch (c1)
            {
            case 0xF0:
                if (!(0x90 <= c2 && c2 <= 0xBF))
                    return codecvt_base::error;
                 break;
            case 0xF4:
                if ((c2 & 0xF0) != 0x80)
                    return codecvt_base::error;
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return codecvt_base::error;
                 break;
            }
            if ((c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80)
                return codecvt_base::error;
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            if ((((c1 & 7UL) << 18) +
                ((c2 & 0x3FUL) << 12) +
                ((c3 & 0x3FUL) << 6) + (c4 & 0x3F)) > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint16_t>(
                    0xD800
                  | (((((c1 & 0x07) << 2) | ((c2 & 0x30) >> 4)) - 1) << 6)
                  | ((c2 & 0x0F) << 2)
                  | ((c3 & 0x30) >> 4));
            *++to_nxt = static_cast<uint16_t>(
                    0xDC00
                  | ((c3 & 0x0F) << 6)
                  |  (c4 & 0x3F));
            frm_nxt += 4;
        }
        else
        {
            return codecvt_base::error;
        }
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
codecvt_base::result
utf8_to_utf16(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
              uint32_t* to, uint32_t* to_end, uint32_t*& to_nxt,
              unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 3 && frm_nxt[0] == 0xEF && frm_nxt[1] == 0xBB &&
                                                          frm_nxt[2] == 0xBF)
            frm_nxt += 3;
    }
    for (; frm_nxt < frm_end && to_nxt < to_end; ++to_nxt)
    {
        uint8_t c1 = *frm_nxt;
        if (c1 > Maxcode)
            return codecvt_base::error;
        if (c1 < 0x80)
        {
            *to_nxt = static_cast<uint32_t>(c1);
            ++frm_nxt;
        }
        else if (c1 < 0xC2)
        {
            return codecvt_base::error;
        }
        else if (c1 < 0xE0)
        {
            if (frm_end-frm_nxt < 2)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            if ((c2 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint16_t t = static_cast<uint16_t>(((c1 & 0x1F) << 6) | (c2 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint32_t>(t);
            frm_nxt += 2;
        }
        else if (c1 < 0xF0)
        {
            if (frm_end-frm_nxt < 3)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            switch (c1)
            {
            case 0xE0:
                if ((c2 & 0xE0) != 0xA0)
                    return codecvt_base::error;
                 break;
            case 0xED:
                if ((c2 & 0xE0) != 0x80)
                    return codecvt_base::error;
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return codecvt_base::error;
                 break;
            }
            if ((c3 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint16_t t = static_cast<uint16_t>(((c1 & 0x0F) << 12)
                                             | ((c2 & 0x3F) << 6)
                                             |  (c3 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint32_t>(t);
            frm_nxt += 3;
        }
        else if (c1 < 0xF5)
        {
            if (frm_end-frm_nxt < 4)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            uint8_t c4 = frm_nxt[3];
            switch (c1)
            {
            case 0xF0:
                if (!(0x90 <= c2 && c2 <= 0xBF))
                    return codecvt_base::error;
                 break;
            case 0xF4:
                if ((c2 & 0xF0) != 0x80)
                    return codecvt_base::error;
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return codecvt_base::error;
                 break;
            }
            if ((c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80)
                return codecvt_base::error;
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            if ((((c1 & 7UL) << 18) +
                ((c2 & 0x3FUL) << 12) +
                ((c3 & 0x3FUL) << 6) + (c4 & 0x3F)) > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint32_t>(
                    0xD800
                  | (((((c1 & 0x07) << 2) | ((c2 & 0x30) >> 4)) - 1) << 6)
                  | ((c2 & 0x0F) << 2)
                  | ((c3 & 0x30) >> 4));
            *++to_nxt = static_cast<uint32_t>(
                    0xDC00
                  | ((c3 & 0x0F) << 6)
                  |  (c4 & 0x3F));
            frm_nxt += 4;
        }
        else
        {
            return codecvt_base::error;
        }
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
int
utf8_to_utf16_length(const uint8_t* frm, const uint8_t* frm_end,
                     size_t mx, unsigned long Maxcode = 0x10FFFF,
                     codecvt_mode mode = codecvt_mode(0))
{
    const uint8_t* frm_nxt = frm;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 3 && frm_nxt[0] == 0xEF && frm_nxt[1] == 0xBB &&
                                                          frm_nxt[2] == 0xBF)
            frm_nxt += 3;
    }
    for (size_t nchar16_t = 0; frm_nxt < frm_end && nchar16_t < mx; ++nchar16_t)
    {
        uint8_t c1 = *frm_nxt;
        if (c1 > Maxcode)
            break;
        if (c1 < 0x80)
        {
            ++frm_nxt;
        }
        else if (c1 < 0xC2)
        {
            break;
        }
        else if (c1 < 0xE0)
        {
            if ((frm_end-frm_nxt < 2) || (frm_nxt[1] & 0xC0) != 0x80)
                break;
            uint16_t t = static_cast<uint16_t>(((c1 & 0x1F) << 6) | (frm_nxt[1] & 0x3F));
            if (t > Maxcode)
                break;
            frm_nxt += 2;
        }
        else if (c1 < 0xF0)
        {
            if (frm_end-frm_nxt < 3)
                break;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            switch (c1)
            {
            case 0xE0:
                if ((c2 & 0xE0) != 0xA0)
                    return static_cast<int>(frm_nxt - frm);
                break;
            case 0xED:
                if ((c2 & 0xE0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            }
            if ((c3 & 0xC0) != 0x80)
                break;
            if ((((c1 & 0x0Fu) << 12) | ((c2 & 0x3Fu) << 6) | (c3 & 0x3Fu)) > Maxcode)
                break;
            frm_nxt += 3;
        }
        else if (c1 < 0xF5)
        {
            if (frm_end-frm_nxt < 4 || mx-nchar16_t < 2)
                break;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            uint8_t c4 = frm_nxt[3];
            switch (c1)
            {
            case 0xF0:
                if (!(0x90 <= c2 && c2 <= 0xBF))
                    return static_cast<int>(frm_nxt - frm);
                 break;
            case 0xF4:
                if ((c2 & 0xF0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            }
            if ((c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80)
                break;
            if ((((c1 & 7UL) << 18) +
                ((c2 & 0x3FUL) << 12) +
                ((c3 & 0x3FUL) << 6) + (c4 & 0x3F)) > Maxcode)
                break;
            ++nchar16_t;
            frm_nxt += 4;
        }
        else
        {
            break;
        }
    }
    return static_cast<int>(frm_nxt - frm);
}

static
codecvt_base::result
ucs4_to_utf8(const uint32_t* frm, const uint32_t* frm_end, const uint32_t*& frm_nxt,
             uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
             unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 3)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(0xEF);
        *to_nxt++ = static_cast<uint8_t>(0xBB);
        *to_nxt++ = static_cast<uint8_t>(0xBF);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint32_t wc = *frm_nxt;
        if ((wc & 0xFFFFF800) == 0x00D800 || wc > Maxcode)
            return codecvt_base::error;
        if (wc < 0x000080)
        {
            if (to_end-to_nxt < 1)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(wc);
        }
        else if (wc < 0x000800)
        {
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xC0 | (wc >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 | (wc & 0x03F));
        }
        else if (wc < 0x010000)
        {
            if (to_end-to_nxt < 3)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc >> 12));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x0FC0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc & 0x003F));
        }
        else // if (wc < 0x110000)
        {
            if (to_end-to_nxt < 4)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xF0 |  (wc >> 18));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x03F000) >> 12));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x000FC0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc & 0x00003F));
        }
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf8_to_ucs4(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
             uint32_t* to, uint32_t* to_end, uint32_t*& to_nxt,
             unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 3 && frm_nxt[0] == 0xEF && frm_nxt[1] == 0xBB &&
                                                          frm_nxt[2] == 0xBF)
            frm_nxt += 3;
    }
    for (; frm_nxt < frm_end && to_nxt < to_end; ++to_nxt)
    {
        uint8_t c1 = static_cast<uint8_t>(*frm_nxt);
        if (c1 < 0x80)
        {
            if (c1 > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint32_t>(c1);
            ++frm_nxt;
        }
        else if (c1 < 0xC2)
        {
            return codecvt_base::error;
        }
        else if (c1 < 0xE0)
        {
            if (frm_end-frm_nxt < 2)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            if ((c2 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint32_t t = static_cast<uint32_t>(((c1 & 0x1F) << 6)
                                              | (c2 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 2;
        }
        else if (c1 < 0xF0)
        {
            if (frm_end-frm_nxt < 3)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            switch (c1)
            {
            case 0xE0:
                if ((c2 & 0xE0) != 0xA0)
                    return codecvt_base::error;
                 break;
            case 0xED:
                if ((c2 & 0xE0) != 0x80)
                    return codecvt_base::error;
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return codecvt_base::error;
                 break;
            }
            if ((c3 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint32_t t = static_cast<uint32_t>(((c1 & 0x0F) << 12)
                                             | ((c2 & 0x3F) << 6)
                                             |  (c3 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 3;
        }
        else if (c1 < 0xF5)
        {
            if (frm_end-frm_nxt < 4)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            uint8_t c4 = frm_nxt[3];
            switch (c1)
            {
            case 0xF0:
                if (!(0x90 <= c2 && c2 <= 0xBF))
                    return codecvt_base::error;
                 break;
            case 0xF4:
                if ((c2 & 0xF0) != 0x80)
                    return codecvt_base::error;
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return codecvt_base::error;
                 break;
            }
            if ((c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint32_t t = static_cast<uint32_t>(((c1 & 0x07) << 18)
                                             | ((c2 & 0x3F) << 12)
                                             | ((c3 & 0x3F) << 6)
                                             |  (c4 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 4;
        }
        else
        {
            return codecvt_base::error;
        }
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
int
utf8_to_ucs4_length(const uint8_t* frm, const uint8_t* frm_end,
                    size_t mx, unsigned long Maxcode = 0x10FFFF,
                    codecvt_mode mode = codecvt_mode(0))
{
    const uint8_t* frm_nxt = frm;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 3 && frm_nxt[0] == 0xEF && frm_nxt[1] == 0xBB &&
                                                          frm_nxt[2] == 0xBF)
            frm_nxt += 3;
    }
    for (size_t nchar32_t = 0; frm_nxt < frm_end && nchar32_t < mx; ++nchar32_t)
    {
        uint8_t c1 = static_cast<uint8_t>(*frm_nxt);
        if (c1 < 0x80)
        {
            if (c1 > Maxcode)
                break;
            ++frm_nxt;
        }
        else if (c1 < 0xC2)
        {
            break;
        }
        else if (c1 < 0xE0)
        {
            if ((frm_end-frm_nxt < 2) || ((frm_nxt[1] & 0xC0) != 0x80))
                break;
            if ((((c1 & 0x1Fu) << 6) | (frm_nxt[1] & 0x3Fu)) > Maxcode)
                break;
            frm_nxt += 2;
        }
        else if (c1 < 0xF0)
        {
            if (frm_end-frm_nxt < 3)
                break;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            switch (c1)
            {
            case 0xE0:
                if ((c2 & 0xE0) != 0xA0)
                    return static_cast<int>(frm_nxt - frm);
                break;
            case 0xED:
                if ((c2 & 0xE0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            }
            if ((c3 & 0xC0) != 0x80)
                break;
            if ((((c1 & 0x0Fu) << 12) | ((c2 & 0x3Fu) << 6) | (c3 & 0x3Fu)) > Maxcode)
                break;
            frm_nxt += 3;
        }
        else if (c1 < 0xF5)
        {
            if (frm_end-frm_nxt < 4)
                break;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            uint8_t c4 = frm_nxt[3];
            switch (c1)
            {
            case 0xF0:
                if (!(0x90 <= c2 && c2 <= 0xBF))
                    return static_cast<int>(frm_nxt - frm);
                 break;
            case 0xF4:
                if ((c2 & 0xF0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            }
            if ((c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80)
                break;
            if ((((c1 & 0x07u) << 18) | ((c2 & 0x3Fu) << 12) |
                 ((c3 & 0x3Fu) << 6)  |  (c4 & 0x3Fu)) > Maxcode)
                break;
            frm_nxt += 4;
        }
        else
        {
            break;
        }
    }
    return static_cast<int>(frm_nxt - frm);
}

static
codecvt_base::result
ucs2_to_utf8(const uint16_t* frm, const uint16_t* frm_end, const uint16_t*& frm_nxt,
             uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
             unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 3)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(0xEF);
        *to_nxt++ = static_cast<uint8_t>(0xBB);
        *to_nxt++ = static_cast<uint8_t>(0xBF);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint16_t wc = *frm_nxt;
        if ((wc & 0xF800) == 0xD800 || wc > Maxcode)
            return codecvt_base::error;
        if (wc < 0x0080)
        {
            if (to_end-to_nxt < 1)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(wc);
        }
        else if (wc < 0x0800)
        {
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xC0 | (wc >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 | (wc & 0x03F));
        }
        else // if (wc <= 0xFFFF)
        {
            if (to_end-to_nxt < 3)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc >> 12));
            *to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x0FC0) >> 6));
            *to_nxt++ = static_cast<uint8_t>(0x80 |  (wc & 0x003F));
        }
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf8_to_ucs2(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
             uint16_t* to, uint16_t* to_end, uint16_t*& to_nxt,
             unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 3 && frm_nxt[0] == 0xEF && frm_nxt[1] == 0xBB &&
                                                          frm_nxt[2] == 0xBF)
            frm_nxt += 3;
    }
    for (; frm_nxt < frm_end && to_nxt < to_end; ++to_nxt)
    {
        uint8_t c1 = static_cast<uint8_t>(*frm_nxt);
        if (c1 < 0x80)
        {
            if (c1 > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint16_t>(c1);
            ++frm_nxt;
        }
        else if (c1 < 0xC2)
        {
            return codecvt_base::error;
        }
        else if (c1 < 0xE0)
        {
            if (frm_end-frm_nxt < 2)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            if ((c2 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint16_t t = static_cast<uint16_t>(((c1 & 0x1F) << 6)
                                              | (c2 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 2;
        }
        else if (c1 < 0xF0)
        {
            if (frm_end-frm_nxt < 3)
                return codecvt_base::partial;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            switch (c1)
            {
            case 0xE0:
                if ((c2 & 0xE0) != 0xA0)
                    return codecvt_base::error;
                 break;
            case 0xED:
                if ((c2 & 0xE0) != 0x80)
                    return codecvt_base::error;
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return codecvt_base::error;
                 break;
            }
            if ((c3 & 0xC0) != 0x80)
                return codecvt_base::error;
            uint16_t t = static_cast<uint16_t>(((c1 & 0x0F) << 12)
                                             | ((c2 & 0x3F) << 6)
                                             |  (c3 & 0x3F));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 3;
        }
        else
        {
            return codecvt_base::error;
        }
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
int
utf8_to_ucs2_length(const uint8_t* frm, const uint8_t* frm_end,
                    size_t mx, unsigned long Maxcode = 0x10FFFF,
                    codecvt_mode mode = codecvt_mode(0))
{
    const uint8_t* frm_nxt = frm;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 3 && frm_nxt[0] == 0xEF && frm_nxt[1] == 0xBB &&
                                                          frm_nxt[2] == 0xBF)
            frm_nxt += 3;
    }
    for (size_t nchar32_t = 0; frm_nxt < frm_end && nchar32_t < mx; ++nchar32_t)
    {
        uint8_t c1 = static_cast<uint8_t>(*frm_nxt);
        if (c1 < 0x80)
        {
            if (c1 > Maxcode)
                break;
            ++frm_nxt;
        }
        else if (c1 < 0xC2)
        {
            break;
        }
        else if (c1 < 0xE0)
        {
            if ((frm_end-frm_nxt < 2) || ((frm_nxt[1] & 0xC0) != 0x80))
                break;
            if ((((c1 & 0x1Fu) << 6) | (frm_nxt[1] & 0x3Fu)) > Maxcode)
                break;
            frm_nxt += 2;
        }
        else if (c1 < 0xF0)
        {
            if (frm_end-frm_nxt < 3)
                break;
            uint8_t c2 = frm_nxt[1];
            uint8_t c3 = frm_nxt[2];
            switch (c1)
            {
            case 0xE0:
                if ((c2 & 0xE0) != 0xA0)
                    return static_cast<int>(frm_nxt - frm);
                break;
            case 0xED:
                if ((c2 & 0xE0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            default:
                if ((c2 & 0xC0) != 0x80)
                    return static_cast<int>(frm_nxt - frm);
                 break;
            }
            if ((c3 & 0xC0) != 0x80)
                break;
            if ((((c1 & 0x0Fu) << 12) | ((c2 & 0x3Fu) << 6) | (c3 & 0x3Fu)) > Maxcode)
                break;
            frm_nxt += 3;
        }
        else
        {
            break;
        }
    }
    return static_cast<int>(frm_nxt - frm);
}

static
codecvt_base::result
ucs4_to_utf16be(const uint32_t* frm, const uint32_t* frm_end, const uint32_t*& frm_nxt,
                uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 2)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(0xFE);
        *to_nxt++ = static_cast<uint8_t>(0xFF);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint32_t wc = *frm_nxt;
        if ((wc & 0xFFFFF800) == 0x00D800 || wc > Maxcode)
            return codecvt_base::error;
        if (wc < 0x010000)
        {
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(wc >> 8);
            *to_nxt++ = static_cast<uint8_t>(wc);
        }
        else
        {
            if (to_end-to_nxt < 4)
                return codecvt_base::partial;
            uint16_t t = static_cast<uint16_t>(
                    0xD800
                  | ((((wc & 0x1F0000) >> 16) - 1) << 6)
                  |   ((wc & 0x00FC00) >> 10));
            *to_nxt++ = static_cast<uint8_t>(t >> 8);
            *to_nxt++ = static_cast<uint8_t>(t);
            t = static_cast<uint16_t>(0xDC00 | (wc & 0x03FF));
            *to_nxt++ = static_cast<uint8_t>(t >> 8);
            *to_nxt++ = static_cast<uint8_t>(t);
        }
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf16be_to_ucs4(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
                uint32_t* to, uint32_t* to_end, uint32_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFE && frm_nxt[1] == 0xFF)
            frm_nxt += 2;
    }
    for (; frm_nxt < frm_end - 1 && to_nxt < to_end; ++to_nxt)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[0] << 8 | frm_nxt[1]);
        if ((c1 & 0xFC00) == 0xDC00)
            return codecvt_base::error;
        if ((c1 & 0xFC00) != 0xD800)
        {
            if (c1 > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint32_t>(c1);
            frm_nxt += 2;
        }
        else
        {
            if (frm_end-frm_nxt < 4)
                return codecvt_base::partial;
            uint16_t c2 = static_cast<uint16_t>(frm_nxt[2] << 8 | frm_nxt[3]);
            if ((c2 & 0xFC00) != 0xDC00)
                return codecvt_base::error;
            uint32_t t = static_cast<uint32_t>(
                    ((((c1 & 0x03C0) >> 6) + 1) << 16)
                  |   ((c1 & 0x003F) << 10)
                  |    (c2 & 0x03FF));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 4;
        }
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
int
utf16be_to_ucs4_length(const uint8_t* frm, const uint8_t* frm_end,
                       size_t mx, unsigned long Maxcode = 0x10FFFF,
                       codecvt_mode mode = codecvt_mode(0))
{
    const uint8_t* frm_nxt = frm;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFE && frm_nxt[1] == 0xFF)
            frm_nxt += 2;
    }
    for (size_t nchar32_t = 0; frm_nxt < frm_end - 1 && nchar32_t < mx; ++nchar32_t)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[0] << 8 | frm_nxt[1]);
        if ((c1 & 0xFC00) == 0xDC00)
            break;
        if ((c1 & 0xFC00) != 0xD800)
        {
            if (c1 > Maxcode)
                break;
            frm_nxt += 2;
        }
        else
        {
            if (frm_end-frm_nxt < 4)
                break;
            uint16_t c2 = static_cast<uint16_t>(frm_nxt[2] << 8 | frm_nxt[3]);
            if ((c2 & 0xFC00) != 0xDC00)
                break;
            uint32_t t = static_cast<uint32_t>(
                    ((((c1 & 0x03C0) >> 6) + 1) << 16)
                  |   ((c1 & 0x003F) << 10)
                  |    (c2 & 0x03FF));
            if (t > Maxcode)
                break;
            frm_nxt += 4;
        }
    }
    return static_cast<int>(frm_nxt - frm);
}

static
codecvt_base::result
ucs4_to_utf16le(const uint32_t* frm, const uint32_t* frm_end, const uint32_t*& frm_nxt,
                uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 2)
            return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(0xFF);
            *to_nxt++ = static_cast<uint8_t>(0xFE);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint32_t wc = *frm_nxt;
        if ((wc & 0xFFFFF800) == 0x00D800 || wc > Maxcode)
            return codecvt_base::error;
        if (wc < 0x010000)
        {
            if (to_end-to_nxt < 2)
                return codecvt_base::partial;
            *to_nxt++ = static_cast<uint8_t>(wc);
            *to_nxt++ = static_cast<uint8_t>(wc >> 8);
        }
        else
        {
            if (to_end-to_nxt < 4)
                return codecvt_base::partial;
            uint16_t t = static_cast<uint16_t>(
                    0xD800
                  | ((((wc & 0x1F0000) >> 16) - 1) << 6)
                  |   ((wc & 0x00FC00) >> 10));
            *to_nxt++ = static_cast<uint8_t>(t);
            *to_nxt++ = static_cast<uint8_t>(t >> 8);
            t = static_cast<uint16_t>(0xDC00 | (wc & 0x03FF));
            *to_nxt++ = static_cast<uint8_t>(t);
            *to_nxt++ = static_cast<uint8_t>(t >> 8);
        }
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf16le_to_ucs4(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
                uint32_t* to, uint32_t* to_end, uint32_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFF && frm_nxt[1] == 0xFE)
            frm_nxt += 2;
    }
    for (; frm_nxt < frm_end - 1 && to_nxt < to_end; ++to_nxt)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[1] << 8 | frm_nxt[0]);
        if ((c1 & 0xFC00) == 0xDC00)
            return codecvt_base::error;
        if ((c1 & 0xFC00) != 0xD800)
        {
            if (c1 > Maxcode)
                return codecvt_base::error;
            *to_nxt = static_cast<uint32_t>(c1);
            frm_nxt += 2;
        }
        else
        {
            if (frm_end-frm_nxt < 4)
                return codecvt_base::partial;
            uint16_t c2 = static_cast<uint16_t>(frm_nxt[3] << 8 | frm_nxt[2]);
            if ((c2 & 0xFC00) != 0xDC00)
                return codecvt_base::error;
            uint32_t t = static_cast<uint32_t>(
                    ((((c1 & 0x03C0) >> 6) + 1) << 16)
                  |   ((c1 & 0x003F) << 10)
                  |    (c2 & 0x03FF));
            if (t > Maxcode)
                return codecvt_base::error;
            *to_nxt = t;
            frm_nxt += 4;
        }
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
int
utf16le_to_ucs4_length(const uint8_t* frm, const uint8_t* frm_end,
                       size_t mx, unsigned long Maxcode = 0x10FFFF,
                       codecvt_mode mode = codecvt_mode(0))
{
    const uint8_t* frm_nxt = frm;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFF && frm_nxt[1] == 0xFE)
            frm_nxt += 2;
    }
    for (size_t nchar32_t = 0; frm_nxt < frm_end - 1 && nchar32_t < mx; ++nchar32_t)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[1] << 8 | frm_nxt[0]);
        if ((c1 & 0xFC00) == 0xDC00)
            break;
        if ((c1 & 0xFC00) != 0xD800)
        {
            if (c1 > Maxcode)
                break;
            frm_nxt += 2;
        }
        else
        {
            if (frm_end-frm_nxt < 4)
                break;
            uint16_t c2 = static_cast<uint16_t>(frm_nxt[3] << 8 | frm_nxt[2]);
            if ((c2 & 0xFC00) != 0xDC00)
                break;
            uint32_t t = static_cast<uint32_t>(
                    ((((c1 & 0x03C0) >> 6) + 1) << 16)
                  |   ((c1 & 0x003F) << 10)
                  |    (c2 & 0x03FF));
            if (t > Maxcode)
                break;
            frm_nxt += 4;
        }
    }
    return static_cast<int>(frm_nxt - frm);
}

static
codecvt_base::result
ucs2_to_utf16be(const uint16_t* frm, const uint16_t* frm_end, const uint16_t*& frm_nxt,
                uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 2)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(0xFE);
        *to_nxt++ = static_cast<uint8_t>(0xFF);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint16_t wc = *frm_nxt;
        if ((wc & 0xF800) == 0xD800 || wc > Maxcode)
            return codecvt_base::error;
        if (to_end-to_nxt < 2)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(wc >> 8);
        *to_nxt++ = static_cast<uint8_t>(wc);
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf16be_to_ucs2(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
                uint16_t* to, uint16_t* to_end, uint16_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFE && frm_nxt[1] == 0xFF)
            frm_nxt += 2;
    }
    for (; frm_nxt < frm_end - 1 && to_nxt < to_end; ++to_nxt)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[0] << 8 | frm_nxt[1]);
        if ((c1 & 0xF800) == 0xD800 || c1 > Maxcode)
            return codecvt_base::error;
        *to_nxt = c1;
        frm_nxt += 2;
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
int
utf16be_to_ucs2_length(const uint8_t* frm, const uint8_t* frm_end,
                       size_t mx, unsigned long Maxcode = 0x10FFFF,
                       codecvt_mode mode = codecvt_mode(0))
{
    const uint8_t* frm_nxt = frm;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFE && frm_nxt[1] == 0xFF)
            frm_nxt += 2;
    }
    for (size_t nchar16_t = 0; frm_nxt < frm_end - 1 && nchar16_t < mx; ++nchar16_t)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[0] << 8 | frm_nxt[1]);
        if ((c1 & 0xF800) == 0xD800 || c1 > Maxcode)
            break;
        frm_nxt += 2;
    }
    return static_cast<int>(frm_nxt - frm);
}

static
codecvt_base::result
ucs2_to_utf16le(const uint16_t* frm, const uint16_t* frm_end, const uint16_t*& frm_nxt,
                uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & generate_header)
    {
        if (to_end-to_nxt < 2)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(0xFF);
        *to_nxt++ = static_cast<uint8_t>(0xFE);
    }
    for (; frm_nxt < frm_end; ++frm_nxt)
    {
        uint16_t wc = *frm_nxt;
        if ((wc & 0xF800) == 0xD800 || wc > Maxcode)
            return codecvt_base::error;
        if (to_end-to_nxt < 2)
            return codecvt_base::partial;
        *to_nxt++ = static_cast<uint8_t>(wc);
        *to_nxt++ = static_cast<uint8_t>(wc >> 8);
    }
    return codecvt_base::ok;
}

static
codecvt_base::result
utf16le_to_ucs2(const uint8_t* frm, const uint8_t* frm_end, const uint8_t*& frm_nxt,
                uint16_t* to, uint16_t* to_end, uint16_t*& to_nxt,
                unsigned long Maxcode = 0x10FFFF, codecvt_mode mode = codecvt_mode(0))
{
    frm_nxt = frm;
    to_nxt = to;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFF && frm_nxt[1] == 0xFE)
            frm_nxt += 2;
    }
    for (; frm_nxt < frm_end - 1 && to_nxt < to_end; ++to_nxt)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[1] << 8 | frm_nxt[0]);
        if ((c1 & 0xF800) == 0xD800 || c1 > Maxcode)
            return codecvt_base::error;
        *to_nxt = c1;
        frm_nxt += 2;
    }
    return frm_nxt < frm_end ? codecvt_base::partial : codecvt_base::ok;
}

static
int
utf16le_to_ucs2_length(const uint8_t* frm, const uint8_t* frm_end,
                       size_t mx, unsigned long Maxcode = 0x10FFFF,
                       codecvt_mode mode = codecvt_mode(0))
{
    const uint8_t* frm_nxt = frm;
    frm_nxt = frm;
    if (mode & consume_header)
    {
        if (frm_end-frm_nxt >= 2 && frm_nxt[0] == 0xFF && frm_nxt[1] == 0xFE)
            frm_nxt += 2;
    }
    for (size_t nchar16_t = 0; frm_nxt < frm_end - 1 && nchar16_t < mx; ++nchar16_t)
    {
        uint16_t c1 = static_cast<uint16_t>(frm_nxt[1] << 8 | frm_nxt[0]);
        if ((c1 & 0xF800) == 0xD800 || c1 > Maxcode)
            break;
        frm_nxt += 2;
    }
    return static_cast<int>(frm_nxt - frm);
}

// template <> class codecvt<char16_t, char, mbstate_t>

locale::id codecvt<char16_t, char, mbstate_t>::id;

codecvt<char16_t, char, mbstate_t>::~codecvt()
{
}

codecvt<char16_t, char, mbstate_t>::result
codecvt<char16_t, char, mbstate_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint16_t* _frm = reinterpret_cast<const uint16_t*>(frm);
    const uint16_t* _frm_end = reinterpret_cast<const uint16_t*>(frm_end);
    const uint16_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = utf16_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

codecvt<char16_t, char, mbstate_t>::result
codecvt<char16_t, char, mbstate_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint16_t* _to = reinterpret_cast<uint16_t*>(to);
    uint16_t* _to_end = reinterpret_cast<uint16_t*>(to_end);
    uint16_t* _to_nxt = _to;
    result r = utf8_to_utf16(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

codecvt<char16_t, char, mbstate_t>::result
codecvt<char16_t, char, mbstate_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
codecvt<char16_t, char, mbstate_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
codecvt<char16_t, char, mbstate_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
codecvt<char16_t, char, mbstate_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_utf16_length(_frm, _frm_end, mx);
}

int
codecvt<char16_t, char, mbstate_t>::do_max_length() const  _NOEXCEPT
{
    return 4;
}

// template <> class codecvt<char32_t, char, mbstate_t>

locale::id codecvt<char32_t, char, mbstate_t>::id;

codecvt<char32_t, char, mbstate_t>::~codecvt()
{
}

codecvt<char32_t, char, mbstate_t>::result
codecvt<char32_t, char, mbstate_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs4_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

codecvt<char32_t, char, mbstate_t>::result
codecvt<char32_t, char, mbstate_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf8_to_ucs4(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

codecvt<char32_t, char, mbstate_t>::result
codecvt<char32_t, char, mbstate_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
codecvt<char32_t, char, mbstate_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
codecvt<char32_t, char, mbstate_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
codecvt<char32_t, char, mbstate_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_ucs4_length(_frm, _frm_end, mx);
}

int
codecvt<char32_t, char, mbstate_t>::do_max_length() const  _NOEXCEPT
{
    return 4;
}

// __codecvt_utf8<wchar_t>

__codecvt_utf8<wchar_t>::result
__codecvt_utf8<wchar_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
#if _WIN32
    const uint16_t* _frm = reinterpret_cast<const uint16_t*>(frm);
    const uint16_t* _frm_end = reinterpret_cast<const uint16_t*>(frm_end);
    const uint16_t* _frm_nxt = _frm;
#else
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
#endif
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
#if _WIN32
    result r = ucs2_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
#else
    result r = ucs4_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
#endif
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8<wchar_t>::result
__codecvt_utf8<wchar_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
#if _WIN32
    uint16_t* _to = reinterpret_cast<uint16_t*>(to);
    uint16_t* _to_end = reinterpret_cast<uint16_t*>(to_end);
    uint16_t* _to_nxt = _to;
    result r = utf8_to_ucs2(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
#else
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf8_to_ucs4(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
#endif
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8<wchar_t>::result
__codecvt_utf8<wchar_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf8<wchar_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf8<wchar_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf8<wchar_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_ucs4_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf8<wchar_t>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 7;
    return 4;
}

// __codecvt_utf8<char16_t>

__codecvt_utf8<char16_t>::result
__codecvt_utf8<char16_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint16_t* _frm = reinterpret_cast<const uint16_t*>(frm);
    const uint16_t* _frm_end = reinterpret_cast<const uint16_t*>(frm_end);
    const uint16_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs2_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8<char16_t>::result
__codecvt_utf8<char16_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint16_t* _to = reinterpret_cast<uint16_t*>(to);
    uint16_t* _to_end = reinterpret_cast<uint16_t*>(to_end);
    uint16_t* _to_nxt = _to;
    result r = utf8_to_ucs2(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8<char16_t>::result
__codecvt_utf8<char16_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf8<char16_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf8<char16_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf8<char16_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_ucs2_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf8<char16_t>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 6;
    return 3;
}

// __codecvt_utf8<char32_t>

__codecvt_utf8<char32_t>::result
__codecvt_utf8<char32_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs4_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8<char32_t>::result
__codecvt_utf8<char32_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf8_to_ucs4(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                            _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8<char32_t>::result
__codecvt_utf8<char32_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf8<char32_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf8<char32_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf8<char32_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_ucs4_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf8<char32_t>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 7;
    return 4;
}

// __codecvt_utf16<wchar_t, false>

__codecvt_utf16<wchar_t, false>::result
__codecvt_utf16<wchar_t, false>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs4_to_utf16be(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<wchar_t, false>::result
__codecvt_utf16<wchar_t, false>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf16be_to_ucs4(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<wchar_t, false>::result
__codecvt_utf16<wchar_t, false>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf16<wchar_t, false>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf16<wchar_t, false>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf16<wchar_t, false>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf16be_to_ucs4_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf16<wchar_t, false>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 6;
    return 4;
}

// __codecvt_utf16<wchar_t, true>

__codecvt_utf16<wchar_t, true>::result
__codecvt_utf16<wchar_t, true>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs4_to_utf16le(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<wchar_t, true>::result
__codecvt_utf16<wchar_t, true>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf16le_to_ucs4(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<wchar_t, true>::result
__codecvt_utf16<wchar_t, true>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf16<wchar_t, true>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf16<wchar_t, true>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf16<wchar_t, true>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf16le_to_ucs4_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf16<wchar_t, true>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 6;
    return 4;
}

// __codecvt_utf16<char16_t, false>

__codecvt_utf16<char16_t, false>::result
__codecvt_utf16<char16_t, false>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint16_t* _frm = reinterpret_cast<const uint16_t*>(frm);
    const uint16_t* _frm_end = reinterpret_cast<const uint16_t*>(frm_end);
    const uint16_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs2_to_utf16be(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char16_t, false>::result
__codecvt_utf16<char16_t, false>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint16_t* _to = reinterpret_cast<uint16_t*>(to);
    uint16_t* _to_end = reinterpret_cast<uint16_t*>(to_end);
    uint16_t* _to_nxt = _to;
    result r = utf16be_to_ucs2(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char16_t, false>::result
__codecvt_utf16<char16_t, false>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf16<char16_t, false>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf16<char16_t, false>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf16<char16_t, false>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf16be_to_ucs2_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf16<char16_t, false>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 4;
    return 2;
}

// __codecvt_utf16<char16_t, true>

__codecvt_utf16<char16_t, true>::result
__codecvt_utf16<char16_t, true>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint16_t* _frm = reinterpret_cast<const uint16_t*>(frm);
    const uint16_t* _frm_end = reinterpret_cast<const uint16_t*>(frm_end);
    const uint16_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs2_to_utf16le(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char16_t, true>::result
__codecvt_utf16<char16_t, true>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint16_t* _to = reinterpret_cast<uint16_t*>(to);
    uint16_t* _to_end = reinterpret_cast<uint16_t*>(to_end);
    uint16_t* _to_nxt = _to;
    result r = utf16le_to_ucs2(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char16_t, true>::result
__codecvt_utf16<char16_t, true>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf16<char16_t, true>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf16<char16_t, true>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf16<char16_t, true>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf16le_to_ucs2_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf16<char16_t, true>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 4;
    return 2;
}

// __codecvt_utf16<char32_t, false>

__codecvt_utf16<char32_t, false>::result
__codecvt_utf16<char32_t, false>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs4_to_utf16be(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char32_t, false>::result
__codecvt_utf16<char32_t, false>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf16be_to_ucs4(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char32_t, false>::result
__codecvt_utf16<char32_t, false>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf16<char32_t, false>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf16<char32_t, false>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf16<char32_t, false>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf16be_to_ucs4_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf16<char32_t, false>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 6;
    return 4;
}

// __codecvt_utf16<char32_t, true>

__codecvt_utf16<char32_t, true>::result
__codecvt_utf16<char32_t, true>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = ucs4_to_utf16le(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char32_t, true>::result
__codecvt_utf16<char32_t, true>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf16le_to_ucs4(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                               _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf16<char32_t, true>::result
__codecvt_utf16<char32_t, true>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf16<char32_t, true>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf16<char32_t, true>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf16<char32_t, true>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf16le_to_ucs4_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf16<char32_t, true>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 6;
    return 4;
}

// __codecvt_utf8_utf16<wchar_t>

__codecvt_utf8_utf16<wchar_t>::result
__codecvt_utf8_utf16<wchar_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = utf16_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                             _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8_utf16<wchar_t>::result
__codecvt_utf8_utf16<wchar_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf8_to_utf16(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                             _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8_utf16<wchar_t>::result
__codecvt_utf8_utf16<wchar_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf8_utf16<wchar_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf8_utf16<wchar_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf8_utf16<wchar_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_utf16_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf8_utf16<wchar_t>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 7;
    return 4;
}

// __codecvt_utf8_utf16<char16_t>

__codecvt_utf8_utf16<char16_t>::result
__codecvt_utf8_utf16<char16_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint16_t* _frm = reinterpret_cast<const uint16_t*>(frm);
    const uint16_t* _frm_end = reinterpret_cast<const uint16_t*>(frm_end);
    const uint16_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = utf16_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                             _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8_utf16<char16_t>::result
__codecvt_utf8_utf16<char16_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint16_t* _to = reinterpret_cast<uint16_t*>(to);
    uint16_t* _to_end = reinterpret_cast<uint16_t*>(to_end);
    uint16_t* _to_nxt = _to;
    result r = utf8_to_utf16(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                             _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8_utf16<char16_t>::result
__codecvt_utf8_utf16<char16_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf8_utf16<char16_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf8_utf16<char16_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf8_utf16<char16_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_utf16_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf8_utf16<char16_t>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 7;
    return 4;
}

// __codecvt_utf8_utf16<char32_t>

__codecvt_utf8_utf16<char32_t>::result
__codecvt_utf8_utf16<char32_t>::do_out(state_type&,
    const intern_type* frm, const intern_type* frm_end, const intern_type*& frm_nxt,
    extern_type* to, extern_type* to_end, extern_type*& to_nxt) const
{
    const uint32_t* _frm = reinterpret_cast<const uint32_t*>(frm);
    const uint32_t* _frm_end = reinterpret_cast<const uint32_t*>(frm_end);
    const uint32_t* _frm_nxt = _frm;
    uint8_t* _to = reinterpret_cast<uint8_t*>(to);
    uint8_t* _to_end = reinterpret_cast<uint8_t*>(to_end);
    uint8_t* _to_nxt = _to;
    result r = utf16_to_utf8(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                             _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8_utf16<char32_t>::result
__codecvt_utf8_utf16<char32_t>::do_in(state_type&,
    const extern_type* frm, const extern_type* frm_end, const extern_type*& frm_nxt,
    intern_type* to, intern_type* to_end, intern_type*& to_nxt) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    const uint8_t* _frm_nxt = _frm;
    uint32_t* _to = reinterpret_cast<uint32_t*>(to);
    uint32_t* _to_end = reinterpret_cast<uint32_t*>(to_end);
    uint32_t* _to_nxt = _to;
    result r = utf8_to_utf16(_frm, _frm_end, _frm_nxt, _to, _to_end, _to_nxt,
                             _Maxcode_, _Mode_);
    frm_nxt = frm + (_frm_nxt - _frm);
    to_nxt = to + (_to_nxt - _to);
    return r;
}

__codecvt_utf8_utf16<char32_t>::result
__codecvt_utf8_utf16<char32_t>::do_unshift(state_type&,
    extern_type* to, extern_type*, extern_type*& to_nxt) const
{
    to_nxt = to;
    return noconv;
}

int
__codecvt_utf8_utf16<char32_t>::do_encoding() const  _NOEXCEPT
{
    return 0;
}

bool
__codecvt_utf8_utf16<char32_t>::do_always_noconv() const  _NOEXCEPT
{
    return false;
}

int
__codecvt_utf8_utf16<char32_t>::do_length(state_type&,
    const extern_type* frm, const extern_type* frm_end, size_t mx) const
{
    const uint8_t* _frm = reinterpret_cast<const uint8_t*>(frm);
    const uint8_t* _frm_end = reinterpret_cast<const uint8_t*>(frm_end);
    return utf8_to_utf16_length(_frm, _frm_end, mx, _Maxcode_, _Mode_);
}

int
__codecvt_utf8_utf16<char32_t>::do_max_length() const  _NOEXCEPT
{
    if (_Mode_ & consume_header)
        return 7;
    return 4;
}

// __narrow_to_utf8<16>

__narrow_to_utf8<16>::~__narrow_to_utf8()
{
}

// __narrow_to_utf8<32>

__narrow_to_utf8<32>::~__narrow_to_utf8()
{
}

// __widen_from_utf8<16>

__widen_from_utf8<16>::~__widen_from_utf8()
{
}

// __widen_from_utf8<32>

__widen_from_utf8<32>::~__widen_from_utf8()
{
}

// numpunct<char> && numpunct<wchar_t>

locale::id numpunct< char  >::id;
locale::id numpunct<wchar_t>::id;

numpunct<char>::numpunct(size_t refs)
    : locale::facet(refs),
      __decimal_point_('.'),
      __thousands_sep_(',')
{
}

numpunct<wchar_t>::numpunct(size_t refs)
    : locale::facet(refs),
      __decimal_point_(L'.'),
      __thousands_sep_(L',')
{
}

numpunct<char>::~numpunct()
{
}

numpunct<wchar_t>::~numpunct()
{
}

 char   numpunct< char  >::do_decimal_point() const {return __decimal_point_;}
wchar_t numpunct<wchar_t>::do_decimal_point() const {return __decimal_point_;}

 char   numpunct< char  >::do_thousands_sep() const {return __thousands_sep_;}
wchar_t numpunct<wchar_t>::do_thousands_sep() const {return __thousands_sep_;}

string numpunct< char  >::do_grouping() const {return __grouping_;}
string numpunct<wchar_t>::do_grouping() const {return __grouping_;}

 string numpunct< char  >::do_truename() const {return "true";}
wstring numpunct<wchar_t>::do_truename() const {return L"true";}

 string numpunct< char  >::do_falsename() const {return "false";}
wstring numpunct<wchar_t>::do_falsename() const {return L"false";}

// numpunct_byname<char>

numpunct_byname<char>::numpunct_byname(const char* nm, size_t refs)
    : numpunct<char>(refs)
{
    __init(nm);
}

numpunct_byname<char>::numpunct_byname(const string& nm, size_t refs)
    : numpunct<char>(refs)
{
    __init(nm.c_str());
}

numpunct_byname<char>::~numpunct_byname()
{
}

void
numpunct_byname<char>::__init(const char* nm)
{
  // TODO locale loading
    __decimal_point_ = '.';
    __thousands_sep_ = '\0';
    __grouping_ = '\0';
}

// numpunct_byname<wchar_t>

numpunct_byname<wchar_t>::numpunct_byname(const char* nm, size_t refs)
    : numpunct<wchar_t>(refs)
{
    __init(nm);
}

numpunct_byname<wchar_t>::numpunct_byname(const string& nm, size_t refs)
    : numpunct<wchar_t>(refs)
{
    __init(nm.c_str());
}

numpunct_byname<wchar_t>::~numpunct_byname()
{
}

void
numpunct_byname<wchar_t>::__init(const char* nm)
{
  // TODO locale loading
   __decimal_point_ = L'.';
   __thousands_sep_ = L'\0';
   __grouping_ = '\0';
}

// num_get helpers

int
__num_get_base::__get_base(ios_base& iob)
{
    ios_base::fmtflags __basefield = iob.flags() & ios_base::basefield;
    if (__basefield == ios_base::oct)
        return 8;
    else if (__basefield == ios_base::hex)
        return 16;
    else if (__basefield == 0)
        return 0;
    return 10;
}

const char __num_get_base::__src[33] = "0123456789abcdefABCDEFxX+-pPiInN";

void
__check_grouping(const string& __grouping, unsigned* __g, unsigned* __g_end,
                 ios_base::iostate& __err)
{
    if (__grouping.size() != 0)
    {
        reverse(__g, __g_end);
        const char* __ig = __grouping.data();
        const char* __eg = __ig + __grouping.size();
        for (unsigned* __r = __g; __r < __g_end-1; ++__r)
        {
            if (0 < *__ig && *__ig < numeric_limits<char>::max())
            {
                if (static_cast<unsigned>(*__ig) != *__r)
                {
                    __err = ios_base::failbit;
                    return;
                }
            }
            if (__eg - __ig > 1)
                ++__ig;
        }
        if (0 < *__ig && *__ig < numeric_limits<char>::max())
        {
            if (static_cast<unsigned>(*__ig) < __g_end[-1] || __g_end[-1] == 0)
                __err = ios_base::failbit;
        }
    }
}

char*
__num_put_base::__identify_padding(char* __nb, char* __ne,
                                   const ios_base& __iob)
{
    switch (__iob.flags() & ios_base::adjustfield)
    {
    case ios_base::internal:
        if (__nb[0] == '-' || __nb[0] == '+')
            return __nb+1;
        if (__ne - __nb >= 2 && __nb[0] == '0'
                            && (__nb[1] == 'x' || __nb[1] == 'X'))
            return __nb+2;
        break;
    case ios_base::left:
        return __ne;
    case ios_base::right:
    default:
        break;
    }
    return __nb;
}

// time_get

static
string*
init_weeks()
{
    static string weeks[14];
    weeks[0]  = "Sunday";
    weeks[1]  = "Monday";
    weeks[2]  = "Tuesday";
    weeks[3]  = "Wednesday";
    weeks[4]  = "Thursday";
    weeks[5]  = "Friday";
    weeks[6]  = "Saturday";
    weeks[7]  = "Sun";
    weeks[8]  = "Mon";
    weeks[9]  = "Tue";
    weeks[10] = "Wed";
    weeks[11] = "Thu";
    weeks[12] = "Fri";
    weeks[13] = "Sat";
    return weeks;
}

static
wstring*
init_wweeks()
{
    static wstring weeks[14];
    weeks[0]  = L"Sunday";
    weeks[1]  = L"Monday";
    weeks[2]  = L"Tuesday";
    weeks[3]  = L"Wednesday";
    weeks[4]  = L"Thursday";
    weeks[5]  = L"Friday";
    weeks[6]  = L"Saturday";
    weeks[7]  = L"Sun";
    weeks[8]  = L"Mon";
    weeks[9]  = L"Tue";
    weeks[10] = L"Wed";
    weeks[11] = L"Thu";
    weeks[12] = L"Fri";
    weeks[13] = L"Sat";
    return weeks;
}

template <>
const string*
__time_get_c_storage<char>::__weeks() const
{
    static const string* weeks = init_weeks();
    return weeks;
}

template <>
const wstring*
__time_get_c_storage<wchar_t>::__weeks() const
{
    static const wstring* weeks = init_wweeks();
    return weeks;
}

static
string*
init_months()
{
    static string months[24];
    months[0]  = "January";
    months[1]  = "February";
    months[2]  = "March";
    months[3]  = "April";
    months[4]  = "May";
    months[5]  = "June";
    months[6]  = "July";
    months[7]  = "August";
    months[8]  = "September";
    months[9]  = "October";
    months[10] = "November";
    months[11] = "December";
    months[12] = "Jan";
    months[13] = "Feb";
    months[14] = "Mar";
    months[15] = "Apr";
    months[16] = "May";
    months[17] = "Jun";
    months[18] = "Jul";
    months[19] = "Aug";
    months[20] = "Sep";
    months[21] = "Oct";
    months[22] = "Nov";
    months[23] = "Dec";
    return months;
}

static
wstring*
init_wmonths()
{
    static wstring months[24];
    months[0]  = L"January";
    months[1]  = L"February";
    months[2]  = L"March";
    months[3]  = L"April";
    months[4]  = L"May";
    months[5]  = L"June";
    months[6]  = L"July";
    months[7]  = L"August";
    months[8]  = L"September";
    months[9]  = L"October";
    months[10] = L"November";
    months[11] = L"December";
    months[12] = L"Jan";
    months[13] = L"Feb";
    months[14] = L"Mar";
    months[15] = L"Apr";
    months[16] = L"May";
    months[17] = L"Jun";
    months[18] = L"Jul";
    months[19] = L"Aug";
    months[20] = L"Sep";
    months[21] = L"Oct";
    months[22] = L"Nov";
    months[23] = L"Dec";
    return months;
}

template <>
const string*
__time_get_c_storage<char>::__months() const
{
    static const string* months = init_months();
    return months;
}

template <>
const wstring*
__time_get_c_storage<wchar_t>::__months() const
{
    static const wstring* months = init_wmonths();
    return months;
}

static
string*
init_am_pm()
{
    static string am_pm[24];
    am_pm[0]  = "AM";
    am_pm[1]  = "PM";
    return am_pm;
}

static
wstring*
init_wam_pm()
{
    static wstring am_pm[24];
    am_pm[0]  = L"AM";
    am_pm[1]  = L"PM";
    return am_pm;
}

template <>
const string*
__time_get_c_storage<char>::__am_pm() const
{
    static const string* am_pm = init_am_pm();
    return am_pm;
}

template <>
const wstring*
__time_get_c_storage<wchar_t>::__am_pm() const
{
    static const wstring* am_pm = init_wam_pm();
    return am_pm;
}

template <>
const string&
__time_get_c_storage<char>::__x() const
{
    static string s("%m/%d/%y");
    return s;
}

template <>
const wstring&
__time_get_c_storage<wchar_t>::__x() const
{
    static wstring s(L"%m/%d/%y");
    return s;
}

template <>
const string&
__time_get_c_storage<char>::__X() const
{
    static string s("%H:%M:%S");
    return s;
}

template <>
const wstring&
__time_get_c_storage<wchar_t>::__X() const
{
    static wstring s(L"%H:%M:%S");
    return s;
}

template <>
const string&
__time_get_c_storage<char>::__c() const
{
    static string s("%a %b %d %H:%M:%S %Y");
    return s;
}

template <>
const wstring&
__time_get_c_storage<wchar_t>::__c() const
{
    static wstring s(L"%a %b %d %H:%M:%S %Y");
    return s;
}

template <>
const string&
__time_get_c_storage<char>::__r() const
{
    static string s("%I:%M:%S %p");
    return s;
}

template <>
const wstring&
__time_get_c_storage<wchar_t>::__r() const
{
    static wstring s(L"%I:%M:%S %p");
    return s;
}

template <class CharT>
struct _LIBCPP_HIDDEN __time_get_temp
    : public ctype_byname<CharT>
{
    explicit __time_get_temp(const char* nm)
        : ctype_byname<CharT>(nm, 1) {}
    explicit __time_get_temp(const string& nm)
        : ctype_byname<CharT>(nm, 1) {}
};

// moneypunct_byname

template <class charT>
static
void
__init_pat(money_base::pattern& pat, basic_string<charT>& __curr_symbol_,
           bool intl, char cs_precedes, char sep_by_space, char sign_posn,
           charT space_char)
{
    const char sign = static_cast<char>(money_base::sign);
    const char space = static_cast<char>(money_base::space);
    const char none = static_cast<char>(money_base::none);
    const char symbol = static_cast<char>(money_base::symbol);
    const char value = static_cast<char>(money_base::value);
    const bool symbol_contains_sep = intl && __curr_symbol_.size() == 4;

    // Comments on case branches reflect 'C11 7.11.2.1 The localeconv
    // function'. "Space between sign and symbol or value" means that
    // if the sign is adjacent to the symbol, there's a space between
    // them, and otherwise there's a space between the sign and value.
    //
    // C11's localeconv specifies that the fourth character of an
    // international curr_symbol is used to separate the sign and
    // value when sep_by_space says to do so. C++ can't represent
    // that, so we just use a space.  When sep_by_space says to
    // separate the symbol and value-or-sign with a space, we rearrange the
    // curr_symbol to put its spacing character on the correct side of
    // the symbol.
    //
    // We also need to avoid adding an extra space between the sign
    // and value when the currency symbol is suppressed (by not
    // setting showbase).  We match glibc's strfmon by interpreting
    // sep_by_space==1 as "omit the space when the currency symbol is
    // absent".
    //
    // Users who want to get this right should use ICU instead.

    switch (cs_precedes)
    {
    case 0:  // value before curr_symbol
        if (symbol_contains_sep) {
            // Move the separator to before the symbol, to place it
            // between the value and symbol.
            rotate(__curr_symbol_.begin(), __curr_symbol_.begin() + 3,
                   __curr_symbol_.end());
        }
        switch (sign_posn)
        {
        case 0:  // Parentheses surround the quantity and currency symbol.
            pat.field[0] = sign;
            pat.field[1] = value;
            pat.field[2] = none;  // Any space appears in the symbol.
            pat.field[3] = symbol;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                // This case may have changed between C99 and C11;
                // assume the currency symbol matches the intention.
            case 2:  // Space between sign and currency or value.
                // The "sign" is two parentheses, so no space here either.
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[2]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.insert(0, 1, space_char);
                }
                return;
            default:
                break;
            }
            break;
        case 1:  // The sign string precedes the quantity and currency symbol.
            pat.field[0] = sign;
            pat.field[3] = symbol;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = value;
                pat.field[2] = none;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                pat.field[1] = value;
                pat.field[2] = none;
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[2]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.insert(0, 1, space_char);
                }
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = space;
                pat.field[2] = value;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // has already appeared after the sign.
                    __curr_symbol_.erase(__curr_symbol_.begin());
                }
                return;
            default:
                break;
            }
            break;
        case 2:  // The sign string succeeds the quantity and currency symbol.
            pat.field[0] = value;
            pat.field[3] = sign;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = none;
                pat.field[2] = symbol;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[1]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.insert(0, 1, space_char);
                }
                pat.field[1] = none;
                pat.field[2] = symbol;
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = symbol;
                pat.field[2] = space;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // should not be removed if showbase is absent.
                    __curr_symbol_.erase(__curr_symbol_.begin());
                }
                return;
            default:
                break;
            }
            break;
        case 3:  // The sign string immediately precedes the currency symbol.
            pat.field[0] = value;
            pat.field[3] = symbol;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = none;
                pat.field[2] = sign;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                pat.field[1] = space;
                pat.field[2] = sign;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // has already appeared before the sign.
                    __curr_symbol_.erase(__curr_symbol_.begin());
                }
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = sign;
                pat.field[2] = none;
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[2]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.insert(0, 1, space_char);
                }
                return;
            default:
                break;
            }
            break;
        case 4:  // The sign string immediately succeeds the currency symbol.
            pat.field[0] = value;
            pat.field[3] = sign;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = none;
                pat.field[2] = symbol;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                pat.field[1] = none;
                pat.field[2] = symbol;
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[1]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.insert(0, 1, space_char);
                }
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = symbol;
                pat.field[2] = space;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // should not disappear when showbase is absent.
                    __curr_symbol_.erase(__curr_symbol_.begin());
                }
                return;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    case 1:  // curr_symbol before value
        switch (sign_posn)
        {
        case 0:  // Parentheses surround the quantity and currency symbol.
            pat.field[0] = sign;
            pat.field[1] = symbol;
            pat.field[2] = none;  // Any space appears in the symbol.
            pat.field[3] = value;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                // This case may have changed between C99 and C11;
                // assume the currency symbol matches the intention.
            case 2:  // Space between sign and currency or value.
                // The "sign" is two parentheses, so no space here either.
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[2]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.insert(0, 1, space_char);
                }
                return;
            default:
                break;
            }
            break;
        case 1:  // The sign string precedes the quantity and currency symbol.
            pat.field[0] = sign;
            pat.field[3] = value;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = symbol;
                pat.field[2] = none;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                pat.field[1] = symbol;
                pat.field[2] = none;
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[2]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.push_back(space_char);
                }
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = space;
                pat.field[2] = symbol;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // has already appeared after the sign.
                    __curr_symbol_.pop_back();
                }
                return;
            default:
                break;
            }
            break;
        case 2:  // The sign string succeeds the quantity and currency symbol.
            pat.field[0] = symbol;
            pat.field[3] = sign;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = none;
                pat.field[2] = value;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                pat.field[1] = none;
                pat.field[2] = value;
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[1]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.push_back(space_char);
                }
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = value;
                pat.field[2] = space;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // will appear before the sign.
                    __curr_symbol_.pop_back();
                }
                return;
            default:
                break;
            }
            break;
        case 3:  // The sign string immediately precedes the currency symbol.
            pat.field[0] = sign;
            pat.field[3] = value;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = symbol;
                pat.field[2] = none;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                pat.field[1] = symbol;
                pat.field[2] = none;
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[2]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.push_back(space_char);
                }
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = space;
                pat.field[2] = symbol;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // has already appeared after the sign.
                    __curr_symbol_.pop_back();
                }
                return;
            default:
                break;
            }
            break;
        case 4:  // The sign string immediately succeeds the currency symbol.
            pat.field[0] = symbol;
            pat.field[3] = value;
            switch (sep_by_space)
            {
            case 0:  // No space separates the currency symbol and value.
                pat.field[1] = sign;
                pat.field[2] = none;
                return;
            case 1:  // Space between currency-and-sign or currency and value.
                pat.field[1] = sign;
                pat.field[2] = space;
                if (symbol_contains_sep) {
                    // Remove the separator from the symbol, since it
                    // should not disappear when showbase is absent.
                    __curr_symbol_.pop_back();
                }
                return;
            case 2:  // Space between sign and currency or value.
                pat.field[1] = none;
                pat.field[2] = sign;
                if (!symbol_contains_sep) {
                    // We insert the space into the symbol instead of
                    // setting pat.field[1]=space so that when
                    // showbase is not set, the space goes away too.
                    __curr_symbol_.push_back(space_char);
                }
                return;
           default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    pat.field[0] = symbol;
    pat.field[1] = sign;
    pat.field[2] = none;
    pat.field[3] = value;
}

template<>
void
moneypunct_byname<char, false>::init(const char* nm)
{
  // TODO locale loading
    __decimal_point_ = '.';
    __thousands_sep_ = '\0';
    __grouping_ = '\0';
    __curr_symbol_ = '\0';
    __frac_digits_ = '\0';
    __positive_sign_ = "()";
    __negative_sign_ = "()";
    // Assume the positive and negative formats will want spaces in
    // the same places in curr_symbol since there's no way to
    // represent anything else.
    string_type __dummy_curr_symbol = __curr_symbol_;
    __init_pat(__pos_format_, __dummy_curr_symbol, false,
               '\0', '\0', '\0', ' ');
    __init_pat(__neg_format_, __curr_symbol_, false,
               '\0', '\0', '-', ' ');
}

template<>
void
moneypunct_byname<char, true>::init(const char* nm)
{
  // TODO locale loading
    __decimal_point_ = '.';
    __thousands_sep_ = '\0';
    __grouping_ = '\0';
    __curr_symbol_ = '\0';
    __frac_digits_ = '\0';
    __positive_sign_ = "()";
    __negative_sign_ = "()";
    // Assume the positive and negative formats will want spaces in
    // the same places in curr_symbol since there's no way to
    // represent anything else.
    string_type __dummy_curr_symbol = __curr_symbol_;
    __init_pat(__pos_format_, __dummy_curr_symbol, false,
               '\0', '\0', '\0', ' ');
    __init_pat(__neg_format_, __curr_symbol_, false,
               '\0', '\0', '-', ' ');
}

template<>
void
moneypunct_byname<wchar_t, false>::init(const char* nm)
{
  // TODO locale loading
    __decimal_point_ = '.';
    __thousands_sep_ = '\0';
    __grouping_ = '\0';
    __curr_symbol_ = L"";
    __frac_digits_ = '\0';
    __positive_sign_ = L"()";
    __negative_sign_ = L"()";
    // Assume the positive and negative formats will want spaces in
    // the same places in curr_symbol since there's no way to
    // represent anything else.
    string_type __dummy_curr_symbol = __curr_symbol_;
    __init_pat(__pos_format_, __dummy_curr_symbol, false,
               L'\0', L'\0', L'\0', L' ');
    __init_pat(__neg_format_, __curr_symbol_, false,
               L'\0', L'\0', L'-', L' ');
}

template<>
void
moneypunct_byname<wchar_t, true>::init(const char* nm)
{
  // TODO locale loading
    __decimal_point_ = '.';
    __thousands_sep_ = '\0';
    __grouping_ = '\0';
    __curr_symbol_ = L"";
    __frac_digits_ = '\0';
    __positive_sign_ = L"()";
    __negative_sign_ = L"()";
    // Assume the positive and negative formats will want spaces in
    // the same places in curr_symbol since there's no way to
    // represent anything else.
    string_type __dummy_curr_symbol = __curr_symbol_;
    __init_pat(__pos_format_, __dummy_curr_symbol, false,
               L'\0', L'\0', L'\0', L' ');
    __init_pat(__neg_format_, __curr_symbol_, false,
               L'\0', L'\0', L'-', L' ');
}

void __do_nothing(void*) {}

void __throw_runtime_error(const char* msg)
{
#ifndef _LIBCPP_NO_EXCEPTIONS
    throw runtime_error(msg);
#else
    (void)msg;
#endif
}

template class collate<char>;
template class collate<wchar_t>;

template class num_get<char>;
template class num_get<wchar_t>;

template struct __num_get<char>;
template struct __num_get<wchar_t>;

template class num_put<char>;
template class num_put<wchar_t>;

template struct __num_put<char>;
template struct __num_put<wchar_t>;

template class time_get<char>;
template class time_get<wchar_t>;

template class time_get_byname<char>;
template class time_get_byname<wchar_t>;

template class time_put<char>;
template class time_put<wchar_t>;

template class time_put_byname<char>;
template class time_put_byname<wchar_t>;

template class moneypunct<char, false>;
template class moneypunct<char, true>;
template class moneypunct<wchar_t, false>;
template class moneypunct<wchar_t, true>;

template class moneypunct_byname<char, false>;
template class moneypunct_byname<char, true>;
template class moneypunct_byname<wchar_t, false>;
template class moneypunct_byname<wchar_t, true>;

template class money_get<char>;
template class money_get<wchar_t>;

template class __money_get<char>;
template class __money_get<wchar_t>;

template class money_put<char>;
template class money_put<wchar_t>;

template class __money_put<char>;
template class __money_put<wchar_t>;

template class messages<char>;
template class messages<wchar_t>;

template class messages_byname<char>;
template class messages_byname<wchar_t>;

template class codecvt_byname<char, char, mbstate_t>;
template class codecvt_byname<wchar_t, char, mbstate_t>;
template class codecvt_byname<char16_t, char, mbstate_t>;
template class codecvt_byname<char32_t, char, mbstate_t>;

template class __vector_base_common<true>;

_LIBCPP_END_NAMESPACE_STD
