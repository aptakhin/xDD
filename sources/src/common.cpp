/** xDDTools */
#include "xdd/common.hpp"
#include <QMessageBox>

namespace xdd {

void not_implemented(const QString& msg)
{
	QMessageBox::critical(nullptr, "Not implemented", msg);
}

namespace helper {

	uint64 quad_part(DWORD low, DWORD high)
	{
		LARGE_INTEGER large;
		large.LowPart  = low;
		large.HighPart = high;
		return large.QuadPart;
	}

	QString real2str(double val, int prec)
	{
		QString str;
		str.setNum(val, 'f', prec);
		return str;
	}

	QString format_size(uint64 size)
	{
		QString out;

		double sz = (double)size;
		
		if (size > GIGABYTE)
			return real2str(sz / double(GIGABYTE), 1).append(" Gb");

		if (size > MEGABYTE)
			return real2str(sz / double(MEGABYTE), 0).append(" Mb");

		if (size > KILOBYTE)
			return real2str(sz / double(KILOBYTE), 0).append(" Kb");

		return real2str(sz, 0).append(" b");
	}

	QString format_time_ms(uint32 milliseconds)
	{
		QString out;

		if (milliseconds < SECOND_MS)
			return num2str(milliseconds).append(" ms");
		
		if (milliseconds >= DAY_MS)
		{
			uint32 days   = milliseconds / DAY_MS;
			milliseconds -= days * DAY_MS;
			out += num2str(days).append(" days ");
		}

		if (milliseconds >= HOUR_MS)
		{
			uint32 hours  = milliseconds / HOUR_MS;
			milliseconds -= hours * HOUR_MS;
			out += num2str(hours).append(" hours ");
		}

		if (milliseconds >= MINUTE_MS)
		{
			uint32 minutes = milliseconds / MINUTE_MS;
			milliseconds  -= minutes * MINUTE_MS;
			out += num2str(minutes).append(" minutes ");
		}

		if (milliseconds >= SECOND_MS)
		{
			uint32 seconds = milliseconds / SECOND_MS;
			milliseconds  -= seconds * SECOND_MS;
			out += num2str(seconds).append(" seconds ");
		}

		return out.trimmed();
	}

	QString format_time_s(uint32 seconds)
	{
		QString out;

		if (seconds >= AVG_YEAR_S)
		{
			uint32 years = seconds / AVG_YEAR_S;
			return num2str(years).append(" years");
		}

		if (seconds >= AVG_MONTH_S)
		{
			uint32 months = seconds / AVG_MONTH_S;
			return num2str(months).append(" months");
		}
		
		if (seconds >= DAY_S)
		{
			uint32 days = seconds / DAY_S;
			return num2str(days).append(" days");
		}

		if (seconds >= HOUR_S)
		{
			uint32 hours = seconds / HOUR_MS;
			return num2str(hours).append(" hours");
		}

		if (seconds >= MINUTE_S)
		{
			uint32 minutes = seconds / MINUTE_MS;
			return num2str(minutes).append(" minutes");

		}

		return num2str(seconds).append(" seconds");
	}

	QString format_date(time_t rawtime)
	{
		not_implemented("format_date");
		return "";
	}

	uint64 get_ms_time()
	{
	#ifdef XDD_WIN32_CODE
		return (uint64)GetTickCount();
	#else
	#	error "get_ms_time isn't implemented on this platform"
	#endif
	}


}// namespace helper


Sort_order from_qt(Qt::SortOrder order)
{
	switch (order)
	{
	case Qt::AscendingOrder:	return S_ASCENDING;
	case Qt::DescendingOrder:	return S_DESCENDING;
	default: XDD_ERR2("Unbelievable!", return S_ASCENDING);
	}
}

}// namespace xdd
