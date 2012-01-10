/** xDDTools */
#include "xdd/logger.hpp"

namespace xdd {
	
Logger* Logger::_instance = nullptr;
	
Logger::Logger()
{
	_log.open("xdd.log", std::ios_base::app);

	XDD_ASSERT3(!Logger::_instance,
		"Singleton of logger is already created!",
			return);
	Logger::_instance = this;
}

Logger::~Logger()
{
	XDD_ASSERT3(Logger::_instance == this,
		"Problem while deleting logger! Another singleton was created!",
			return);

	Logger::_instance = 0;
}

Logger& Logger::i() 
{
	XDD_ASSERT2(Logger::_instance,
		"Singleton of logger wasn't yet created!");
	return *Logger::_instance; 
}

void Logger::write_header(Message msg)
{
	_log << std::setw(21);

	char buf[100];

	tm timeinfo, *timeinfo_ptr;
	time_t seconds = time(NULL);
#ifdef _MSC_VER
	localtime_s(&timeinfo, &seconds);
	timeinfo_ptr = &timeinfo;
#else
	timeinfo_ptr = localtime(&seconds);
#endif
	
	strftime(buf, 100, "%d %b %Y %H:%M:%S", timeinfo_ptr);
	_log << std::left << buf;

	_log << std::setw(5);

	switch (msg)
	{
	case M_DEFAULT: _log << "	 "; break;
	case M_ERROR:   _log << "ERR  "; break;
	case M_WARNING: _log << "warn "; break;
	}
}

std::ostream& Logger::write_content()
{
	return _log;
}

std::ostream& Logger::write_line() 
{
	return _log << "--------------------------------------------------------------------------------" << std::endl;
}

std::ostream& operator << (std::ostream& out, const QString& str)
{
	return out << qPrintable(str);
}

}// namespace xdd
