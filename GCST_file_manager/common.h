#pragma once
#include <vector>
#include <iostream>

//pop the first item from an array
template<class T>
void pop_front(std::vector<T>& v)
{
	if (v.size() > 0) {
		v.erase(v.begin());
	}
}

//define logging type
#ifdef NDEBUG
#define RELEASE
#endif
#ifdef _DEBUG
#define VERBOSE
#endif

#ifdef RELEASE
#define _LOG_MANDATORY
#define _LOG_ERROR
#define _LOG_WARN
#endif
#ifdef DEBUG
#define _LOG_MANDATORY
#define _LOG_ERROR
#define _LOG_INFO
#define _LOG_WARN
#endif
#ifdef VERBOSE
#define _LOG_MANDATORY
#define _LOG_ERROR
#define _LOG_INFO
#define _LOG_EXTRA
#define _LOG_WARN
#endif


#ifdef _LOG_WARN
#define LOG_WARN(x) std::cout << std::dec <<  "SYSTEM : " << x << std::endl;
#else
#define LOG_WARN(x)
#endif
#ifdef _LOG_MANDATORY
#define LOG_MANDATORY(x) std::cout << std::dec <<  "SYSTEM : " << x << std::endl;
#else
#define LOG_MANDATORY(x)
#endif
#ifdef _LOG_ERROR
#define LOG_ERROR(x) std::cout << std::dec <<      "ERROR  : " << x << std::endl;
#else
#define LOG_ERROR(x)
#endif
#ifdef _LOG_INFO
#define LOG_INFO(x) std::cout << std::dec <<       "INFO   : " << x << std::endl;
#else
#define LOG_INFO(x)
#endif
#ifdef _LOG_EXTRA
#define LOG_EXTRA(x) std::cout << std::dec <<      "VERBOSE: " << x << std::endl;
#else
#define LOG_EXTRA(x)
#endif