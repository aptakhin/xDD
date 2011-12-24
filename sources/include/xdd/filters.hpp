/** xDDTools */
#pragma once

#include "xdd/common.hpp"

namespace xdd {

/**
Filters just say mark file for delete or not. Can be `fast` and... and the others.
Suggested that fast filters use file_data only and not more. They've to placed in fast filters in Scanner.
*/
class Filter
{
public:
    Filter(const QString& name) : _name(name) {}

    Filter(const Filter& cpy) : _name(cpy._name) {}

	/**
	nullptr means not show file in files and clean views
	Pointer to empty string means show file in views, but not mark it as deleted.
	Other values show file in views and marks it as deleted with this reason.
	*/
    virtual const QString* look(const file_data& data) = 0;

    const QString& name() const { return _name; }


protected:
    const QString _name;
};

class Size_simple_filter : public Filter
{
public:
    Size_simple_filter();

    const QString* look(const file_data& data);

	void set_min_size(uint64 min_size) { _min_size = min_size; }
	uint64 min_size() const { return _min_size; }

	void set_pref_size(uint64 pref_size) { _pref_size= pref_size; }
	uint64 Pref_size() const { return _pref_size; }

	void set_last_access_sec(uint32 last_access) { _last_access = last_access; }
	uint32 Last_access_sec() const { return _last_access; }

	static const QString BIG_AND_OUT_OF_DATE;

protected:
	uint64 _min_size;
	uint64 _pref_size;
	uint32 _last_access;
};

/// Marks everything to delete
class Mark_all_filter : public Filter
{
public:
    Mark_all_filter();

    const QString* look(const file_data& data);
};

/// It doesn't want to delete anything
class Mark_nothing_filter : public Filter
{
public:
    Mark_nothing_filter();

    const QString* look(const file_data& data);
};

class Filter_view;

class Filters_factory
{
public:

	typedef std::pair<Filter*, Filter_view*> Filter_and_view;

	Filter* filter(const QString& name);
	Filter_view* filter_view(const QString& name);
	Filter_and_view filter_and_view(const QString& name);

protected:
	struct Filter_item
	{
		Filter* filter;
		Filter_view* view;
	};

	typedef std::vector<Filter_item> Filters;
	Filters _filters;
};


}// namespace xdd