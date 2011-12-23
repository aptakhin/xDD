/** xDDTools */
#include "xdd/common.hpp"

namespace xdd {

namespace helper {

    uint64 quad_part(DWORD low, DWORD high)
    {
        LARGE_INTEGER large;
        large.LowPart  = low;
        large.HighPart = high;
        return large.QuadPart;
    }

	String format_size(uint64 sz)
	{
		std::wostringstream out;
		out << std::fixed;

		double first = (double)sz;
		
		if (first > GIGABYTE)
		{
			first /= (double)GIGABYTE;
			out << std::setprecision(1) << first << " Gb";
			return out.str();
		}

		if (first > MEGABYTE)
		{
			first /= (double)MEGABYTE;
			out << std::setprecision(0) << first << " Mb";
			return out.str();
		}

		if (first > KILOBYTE)
		{
			first /= (double)KILOBYTE;
			out << std::setprecision(0) << first << " Kb";
			return out.str();
		}
		out << sz << " b";
		return out.str();
	}

	String format_time_ms(uint32 milliseconds)
	{
		std::wostringstream out;

		if (milliseconds < SECOND_MS)
		{
			out << milliseconds << " ms";
			return out.str();
		}
		
		if (milliseconds >= DAY_MS)
		{
			uint32 days   = milliseconds / DAY_MS;
			milliseconds -= days * DAY_MS;
			out << days << " days ";
		}

		if (milliseconds >= HOUR_MS)
		{
			uint32 hours  = milliseconds / HOUR_MS;
			milliseconds -= hours * HOUR_MS;
			out << hours << " hours ";
		}

		if (milliseconds >= MINUTE_MS)
		{
			uint32 minutes = milliseconds / MINUTE_MS;
			milliseconds  -= minutes * MINUTE_MS;
			out << minutes << " minutes ";
		}

		if (milliseconds >= SECOND_MS)
		{
			uint32 seconds = milliseconds / SECOND_MS;
			milliseconds  -= seconds * SECOND_MS;
			out << seconds << " seconds";
		}

		return out.str();
	}

	String format_time_s(uint32 seconds)
	{
		std::wostringstream out;

		if (seconds >= AVG_YEAR_S)
		{
			uint32 years = seconds / AVG_YEAR_S;
			out << years << " years";
			return out.str();
		}

		if (seconds >= AVG_MONTH_S)
		{
			uint32 months = seconds / AVG_MONTH_S;
			out << months << " months";
			return out.str();
		}
		
		if (seconds >= DAY_S)
		{
			uint32 days = seconds / DAY_S;
			out << days << " days";
			return out.str();
		}

		if (seconds >= HOUR_S)
		{
			uint32 hours = seconds / HOUR_MS;
			out << hours << " hours";
			return out.str();
		}

		if (seconds >= MINUTE_S)
		{
			uint32 minutes = seconds / MINUTE_MS;
			out << minutes << " minutes";
			return out.str();
		}

		out << seconds << " seconds";
		return out.str();
	}

	String format_date(time_t rawtime)
	{
		wchar_t buffer[80];

		tm timeinfo, *timeinfo_ptr;
	#ifdef _MSC_VER
		localtime_s(&timeinfo, &rawtime);
		timeinfo_ptr = &timeinfo;
	#else
		timeinfo_ptr = localtime(&rawtime);
	#endif

		setlocale(LC_ALL, "russian");
		wcsftime(buffer, 80, L"%c", timeinfo_ptr);

		return String(buffer);
	}

	uint64 get_ms_time()
	{
	#ifdef Q_OS_WIN32
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
