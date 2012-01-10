/** xDDTools */
#pragma once

#include "xdd/filters.hpp"

#include "ui_simple_filter.h"

namespace xdd {

	namespace Ui {
	class Simple_filter_form;
}


class Settings_window;

class Filter_view : public QWidget
{
	Q_OBJECT

public:

	Filter_view(Filter* filter) : _base_filter(filter), QWidget() {}

	/// Called before presetings settings view
	virtual void update_view() = 0;

	/// Called after applying settings
	virtual void update_filter() = 0;

	const Filter* filter() const { return _base_filter; }

protected:
	Filter* _base_filter;
};

class Size_simple_filter_view : public Filter_view
{
	Q_OBJECT

public:
	Size_simple_filter_view(Size_simple_filter* filter = nullptr);

	void update_view();

	void update_filter();

	uint64 min_file_size() const;
	uint64 mark_file_size() const;
	uint32 file_last_access_seconds() const;

	void set_filter(Size_simple_filter* filter);

protected slots:
	void min_file_size_changed(int);
	void mark_file_size_changed(int);
	void file_last_access_changed(int);

protected:
	Size_simple_filter* _filter;

	::Ui::Simple_filter_form _ui;
};


}// namespace xdd
