/** xDDTools */
#include "xdd/filters.hpp"
#include "xdd/file.hpp"
#include "xdd/settings_window.hpp"

namespace xdd{
/*---------------------------------------------------------
 * Size_simple_filter
 */
const QString Size_simple_filter::BIG_AND_OUT_OF_DATE = "Big and out of date [Size_simple_filter]";

Size_simple_filter::Size_simple_filter()
:	Filter("Size_simple_filter"),
	min_size_(0),
	pref_size_(0),
	last_access_(0)
{
}

void Size_simple_filter::set_min_size(uint64 min_size) 
{
	min_size_ = min_size; 
}

void Size_simple_filter::set_pref_size(uint64 pref_size) 
{
	pref_size_ = pref_size; 
}

void Size_simple_filter::set_last_access_sec(uint32 last_access) 
{
	last_access_ = last_access; 
}
	
const QString* Size_simple_filter::look(const File_data& data)
{
#ifdef XDD_WIN32_SCANNER
	uint64 file_size   = helper::quad_part(data.nFileSizeLow, data.nFileSizeHigh);
	uint64 last_access = helper::quad_part(data.ftLastAccessTime.dwLowDateTime, data.ftLastAccessTime.dwHighDateTime);

	if (file_size > pref_size_ && last_access > last_access_)
		return &BIG_AND_OUT_OF_DATE;
	else if (file_size > min_size_ || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return &EMPTY_STR;
	else
		return nullptr;
#endif

#ifdef XDD_UNIVERSAL_SCANNER
	uint64 file_size = (uint64)data.size();
	if (file_size > pref_size_ && data.lastRead().toTime_t() > last_access_)
		return &BIG_AND_OUT_OF_DATE;
	else if (file_size > min_size_ || data.isDir())
		return &EMPTY_STR;
	else
		return nullptr;
#endif
}

/*---------------------------------------------------------
 * Mark_all_filter
 */
Mark_all_filter::Mark_all_filter()
	: Filter("Mark_all_filter")
{
}

const QString* Mark_all_filter::look(const File_data&)
{
	return &EMPTY_STR;
}

/*---------------------------------------------------------
 * Mark_nothing_filter
 */
Mark_nothing_filter::Mark_nothing_filter()
	: Filter("Mark_nothing_filter")
{
}

const QString* Mark_nothing_filter::look(const File_data&)
{
	return nullptr;
}


}// namespace xdd
