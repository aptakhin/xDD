/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include "xdd/filters.hpp"
#include "xdd/file_system.hpp"

namespace xdd {
	
class Scanner
{
public:

	Scanner() : path_len_(0), all_looked_size_(0), soft_stop_(false) {}

	void start(const QString& path);

	/// Fast filter have to use only file data, which they will get. They have not use more file information and analysis.
	void add_fast_filter(Filter* filter);

	uint64 get_current_looked_size() const { return all_looked_size_; } 

	void sig_soft_stop() { soft_stop_ = true; }

protected:
#ifdef XDD_WIN32_SCANNER
	/// Fastest scanner for Win32. WTF-code and global variables:).
	uint64 start_impl(wchar_t* path, File* file, int depth);
#endif

#ifdef XDD_UNIVERSAL_SCANNER
	/// Universal Scanner code based on Qt-framework. 
	uint64 start_impl(File* file, const QDir& cur_dir);	
#endif

protected:

	typedef std::vector<Filter*> Filters;

	Filters fast_filters_;

	Filters filters_;

	size_t path_len_;

	uint64 all_looked_size_;

	bool soft_stop_;
};

}// namespace xdd
