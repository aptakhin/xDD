/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include <fstream>

namespace xdd {

class Logger
{
public:
    Logger();
	~Logger();

	enum Message
	{
		M_DEFAULT,
		M_ERROR,
		M_WARNING
	};

	static Logger& i();

	void write_header(Message msg);
	void wh(Message msg = M_DEFAULT) { write_header(msg); } 
	std::ofstream& write_content();
	std::ofstream& wc() { return write_content(); }

	std::ofstream& write_line();
	std::ofstream& wl() { return write_line(); }

protected:
	static Logger* _instance;

	std::ofstream _log;
};

std::ostream& operator << (std::ostream& out, const QString& str);

}// namespace xdd
