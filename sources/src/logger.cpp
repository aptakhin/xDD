/** xDDTools */
#include "xdd/logger.hpp"

namespace xdd {
	
Logger* Logger::instance_ = nullptr;
	
Logger::Logger()
{
	log_.open("xdd.log", std::ios_base::app);

	XDD_ASSERT3(!Logger::instance_,
		"Singleton of logger is already created!",
			return);
	Logger::instance_ = this;
}

Logger::~Logger()
{
	XDD_ASSERT3(Logger::instance_ == this,
		"Problem while deleting logger! Another singleton was created!",
			return);

	Logger::instance_ = nullptr;
}

Logger& Logger::i() 
{
	XDD_ASSERT2(Logger::instance_,
		"Singleton of logger wasn't yet created!");
	return *Logger::instance_; 
}

void Logger::write_header(Message msg)
{
	log_ << std::setw(21);

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
	log_ << std::left << buf;

	log_ << std::setw(5);

	switch (msg)
	{
	case M_DEFAULT: log_ << "	 "; break;
	case M_ERROR:   log_ << "ERR  "; break;
	case M_WARNING: log_ << "warn "; break;
	}
}

std::ostream& Logger::write_content()
{
	return log_;
}

std::ostream& Logger::write_line() 
{
	return log_ << "--------------------------------------------------------------------------------" << std::endl;
}

std::ostream& operator << (std::ostream& out, const QString& str)
{
	return out << qPrintable(str);
}

}// namespace xdd
