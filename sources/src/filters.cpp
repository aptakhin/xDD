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
	_min_size(0),
	_pref_size(0),
	_last_access(0)
	//_sett_group("Size_simple_filter", Value::T_GROUP, this),
	//_sett_min_size(&_sett_group, "min_size", Value::T_UINT64),
	//_sett_pref_size(&_sett_group, "pref_size", Value::T_UINT64),
	//_sett_last_access(&_sett_group, "last_access", Value::T_UINT32)
{
	//_sett_min_size.set_uint64(10000);
	//_sett_pref_size.set_uint64(50000);
	//_sett_pref_size.set_uint32(0);

	//Settings_manager::bind_group(&_sett_group);
}

void Size_simple_filter::set_min_size(uint64 min_size) 
{
	_min_size = min_size; 
	//_sett_min_size.set_uint64(_min_size);
}

void Size_simple_filter::set_pref_size(uint64 pref_size) 
{
	_pref_size= pref_size; 
	//_sett_pref_size.set_uint64(_pref_size);
}

void Size_simple_filter::set_last_access_sec(uint32 last_access) 
{
	_last_access = last_access; 
	//_sett_last_access.set_uint32(_last_access);
}
	
const QString* Size_simple_filter::look(const File_data& data)
{
#ifdef XDD_WIN32_SCANNER
	uint64 file_size   = helper::quad_part(data.nFileSizeLow, data.nFileSizeHigh);
	uint64 last_access = helper::quad_part(data.ftLastAccessTime.dwLowDateTime, data.ftLastAccessTime.dwHighDateTime);

	if (file_size > _pref_size && last_access > _last_access)
		return &BIG_AND_OUT_OF_DATE;
	else if (file_size > _min_size || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return &EMPTY_STR;
	else
		return nullptr;
#endif

#ifdef XDD_UNIVERSAL_SCANNER
	uint64 file_size = (uint64)data.size();
	if (file_size > _pref_size && data.lastRead().toTime_t() > _last_access)
		return &BIG_AND_OUT_OF_DATE;
	else if (file_size > _min_size || data.isDir())
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
