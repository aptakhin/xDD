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
	std::wofstream& write_content();
	std::wofstream& wc() { return write_content(); }

	std::wofstream& write_line();
	std::wofstream& wl() { return write_line(); }

protected:
	static Logger* _instance;

	std::wofstream _log;
};

}// namespace xdd
