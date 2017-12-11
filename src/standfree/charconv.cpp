// NOTE: standfree doesn't support other locales than C.

// atoX

double atof(const char *__v)
{
	double res = 0.0;
	std::from_chars(__v, __v + std::char_traits<char>::length(__v), res);
	return res;
}

int atoi(const char *__v)
{
	int res = 0;
	std::from_chars(__v, __v + std::char_traits<char>::length(__v), res);
	return res;
}

long atol(const char *__v)
{
	long res = 0;
	std::from_chars(__v, __v + std::char_traits<char>::length(__v), res);
	return res;
}

#ifndef _LIBCPP_HAS_NO_LONG_LONG
long long atoll(const char *__v)
{
	long long res = 0;
	std::from_chars(__v, __v + std::char_traits<char>::length(__v), res);
	return res;
}
#endif

// strtoX

float strtof(const char* __v, char** __e)
{
	float res = 0.0;
	auto out = std::from_chars(__v, __v + std::char_traits<char>::length(__v), res);
	*__e = const_cast<char *>(out.ptr);
	return res;
}

double strtod(const char* str, char** str_end)
{
}

long double strtold(const char* str, char** str_end)
{
}

long strtol(const char* str, char** str_end, int base)
{
}

#ifndef _LIBCPP_HAS_NO_LONG_LONG
long long strtoll(const char* str, char** str_end, int base)
{
}
#endif

unsigned long strtoul(const char* str, char** str_end, int base)
{
}

#ifndef _LIBCPP_HAS_NO_LONG_LONG
unsigned long long strtoull(const char* str, char** str_end, int base)
{
}
#endif

