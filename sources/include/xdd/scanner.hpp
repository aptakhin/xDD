/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include "xdd/filters.hpp"
#include "xdd/file_system.hpp"

namespace xdd {
	
class Scanner
{
public:

	Scanner() : _path_len(0), _all_looked_size(0), _soft_stop(false) {}

	void start(const QString& path);

	/// Fast filter have to use only file data, which they will get. They have not use more file information and analysis.
	void add_fast_filter(Filter* filter);

	uint64 get_current_looked_size() const { return _all_looked_size; } 

	void sig_soft_stop() { _soft_stop = true; }

protected:
#ifdef XDD_WIN32_SCANNER
	/// Fastest scanner for Win32. WTF-code and global variables:).
	uint64 _start(wchar_t* path, File* file, int depth);
#endif

#ifdef XDD_UNIVERSAL_SCANNER
	/// Universal Scanner code based on Qt-framework. 
	uint64 _start(File* file, const QDir& cur_dir);	
#endif

protected:

	typedef std::vector<Filter*> Filters;

	Filters _fast_filters;

	Filters _filters;

	size_t _path_len;

	uint64 _all_looked_size;

	bool _soft_stop;
};

}// namespace xdd
