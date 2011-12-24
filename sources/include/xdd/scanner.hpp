/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include "xdd/filters.hpp"
#include "xdd/File_system.hpp"

namespace xdd {
	
class Scanner
{
public:

    void start(const QString& path);

	/// Fast filter have use only file data, which they will get and no more file reads and analysis.
    void add_fast_filter(Filter* filter);

protected:
#ifdef XDD_WIN32_SCANNER
	/// Fastest Scanner in Win32. WTF code and global variables:).
    uint64 _start(wchar_t* path, File* file, int depth);
#endif

#ifdef XDD_UNIVERSAL_SCANNER
	/// Universal Scanner code based on Qt-framework. It works compared to Win32 version 10 times slowler.
	uint64 _start(File* file, const QDir& cur_dir);	
#endif

    bool does_look_at(const file_data& data, int depth);

protected:

    typedef std::vector<Filter*> Filters;

    Filters _fast_filters;

    Filters _filters;

	size_t _path_len;
};

}// namespace xdd
