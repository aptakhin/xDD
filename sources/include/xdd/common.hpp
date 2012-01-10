/** xDDTools */
#pragma once

#include <stdio.h>
#include <locale.h>
#include <tchar.h>
#include <time.h>

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <algorithm>
#include <iomanip>
#include <QString>
#include <Qt>

#ifdef Q_OS_WIN32
#	include <windows.h>
#
#	define NOMINMAX /* hate, hate, hate! */
#	undef min
#	undef max
#
#else
#
#	define XDD_UNIVERSAL_CODE
#
#endif//#ifdef Q_OS_WIN32

#ifdef XDD_UNIVERSAL_CODE
#
#	define XDD_UNIVERSAL_SCANNER
#
#else 
#
#	ifdef Q_OS_WIN32
#		define XDD_WIN32_CODE
#	endif
#
#endif

#ifdef XDD_WIN32_CODE
#	define XDD_WIN32_SCANNER
#endif

#ifdef XDD_WIN32_SCANNER
	typedef WIN32_FIND_DATA file_data;
#endif

#ifdef XDD_UNIVERSAL_SCANNER
#	include <QDir>
#
	typedef QFileInfo file_data;
#endif//#ifdef XDD_UNIVERSAL_SCANNER

#ifdef _MSC_VER
#	if _MSC_VER >= 1600 // Younger or equal than MSVS 10
#		include <functional>
#		// MSVS 10 doesn't support a few features of C++11, but we try not to use them
#		define XDD_CPP11
#	endif
#endif//#ifdef _MSC_VER

#ifdef _MSC_VER
#	define XDD_INLINE __forceinline
#endif

#ifndef XDD_INLINE
#	define XDD_INLINE inline
#endif

namespace xdd {

// Save 2 bytes per type - save a tree!
typedef uint32_t uint32;
// Save another 2 bytes per type - save another one tree!
typedef uint64_t uint64;
typedef unsigned int uint;

#define XDD_ASSERT3(expr, msg, ret) { if (!(expr)) { xdd::Logger::i().wh(xdd::Logger::M_ERROR); xdd::Logger::i().wc() << msg << std::endl; ret; } }
#define XDD_ASSERT2(expr, msg) { if (!(expr)) { xdd::Logger::i().wh(xdd::Logger::M_ERROR); xdd::Logger::i().wc() << msg << std::endl; } }
#define XDD_ERR2(msg, ret) { xdd::Logger::i().wh(xdd::Logger::M_ERROR); xdd::Logger::i().wc() << msg << std::endl; ret; }
#define XDD_ERR(msg) { xdd::Logger::i().wh(xdd::Logger::M_ERROR); xdd::Logger::i().wc() << msg << std::endl; }
#define XDD_LOG(msg) { xdd::Logger::i().wh(xdd::Logger::M_DEFAULT); xdd::Logger::i().wc() << msg << std::endl; }

const uint64 KILOBYTE = 1 << 10;
const uint64 MEGABYTE = 1 << 20;
const uint64 GIGABYTE = 1 << 30;

const uint32 SECOND_MS = 1000;
const uint32 MINUTE_MS = 60 * SECOND_MS;
const uint32 HOUR_MS   = 60 * MINUTE_MS;
const uint32 DAY_MS	= 24 * HOUR_MS;

const uint32 SECOND_S	 = 1;
const uint32 MINUTE_S	 = 60;
const uint32 HOUR_S	   = 60  * MINUTE_S;
const uint32 DAY_S		= 24  * HOUR_S;
const uint32 AVG_MONTH_S  = 30  * DAY_S;
const uint32 AVG_YEAR_S   = 365 * DAY_S;

class File;
class File_system;

enum Sort_order
{
	S_ASCENDING,
	S_DESCENDING
};

Sort_order from_qt(Qt::SortOrder order);

template <typename T>
XDD_INLINE bool relation(const T& a, const T& b, Sort_order order)
{
	return order == S_ASCENDING? a < b: a > b;
}

void not_implemented(const QString& msg = "yet");

namespace helper
{
	/// Unite parts to DDWORD
	uint64 quad_part(DWORD low, DWORD high);

	QString format_size(uint64 sz);

	QString format_time_ms(uint32 milliseconds);
	QString format_time_s(uint32 seconds);

	QString format_date(time_t rawtime);

	/// Returns any time in milliseconds
	uint64 get_ms_time();

	template <typename T>
	QString num2str(T val)
	{
		QString str;
		str.setNum(val);
		return str;
	}

	QString real2str(double val, int prec);

	template <typename T>
	QString format_percent(T part, T of)
	{
		double ratio = (double)part;
		double div   = (double)of;

		if (div == 0)
			ratio = 0;
		else
			ratio /= div;

		ratio *= 100;
		int prec = 0;

		if (ratio < 1)
			prec = 1;
		if (ratio < 0.1)
			prec = 2;
		if (ratio < 0.01)
			return QString("<0.01%");
		else
			return QString::number(ratio, 'f', prec).append("%");
	}

}// namespace helper

template <typename T>
class Numeric_accum
{
public:
	Numeric_accum() : _acc(0){};
	Numeric_accum(T t) : _acc(t){};

	T operator () () const { return _acc; }
	T operator () (T t) { _acc += t; return _acc; }
	
	T value() const { return _acc; }

protected:
	T _acc;
};

template <typename T>
class Object_accum
{
public:
	Object_accum() : _acc(){};
	Object_accum(const T& t) : _acc(t){};

	const T& operator () () const { return _acc; }
	T& operator () (const T& t) { _acc += t; return _acc; }
	
	T& value() { return _acc; }
	const T& value() const { return _acc; }

protected:
	T _acc;
};


}// namespace xdd

#include "xdd/logger.hpp"
#include "xdd/fun.hpp"
