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
	std::ostream& write_content();
	std::ostream& wc() { return write_content(); }

	std::ostream& write_line();
	std::ostream& wl() { return write_line(); }

protected:
	static Logger* instance_;

	std::ofstream log_;
};

std::ostream& operator << (std::ostream& out, const QString& str);

}// namespace xdd
