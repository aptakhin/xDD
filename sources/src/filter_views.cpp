/** xDDTools */
#include "xdd/filter_views.hpp"
#include "xdd/settings_window.hpp"

#include "ui_simple_filter.h"

namespace xdd{

/*---------------------------------------------------------
 * Size_simple_filter_view
 */
Size_simple_filter_view::Size_simple_filter_view(Size_simple_filter* filter)
:   Filter_view(static_cast<Filter*>(filter)),
	_filter(filter),
	_ui()
{
	_ui.setupUi(this);

	_ui.simple_filter_activated->setChecked(true);

	const int Max_megabytes = 4096;

	_ui.min_file_sz_slider->setMinimum(0);
	_ui.min_file_sz_slider->setMaximum(Max_megabytes);
	_ui.min_file_sz_slider->setTickInterval(Max_megabytes / 8);

	QObject::connect(_ui.min_file_sz_slider, SIGNAL(valueChanged(int)),
		this, SLOT(min_file_size_changed(int)));

	_ui.min_file_sz_edit->setEnabled(false);

	_ui.mark_file_sz_slider->setMinimum(0);
	_ui.mark_file_sz_slider->setMaximum(Max_megabytes);
	_ui.mark_file_sz_slider->setTickInterval(Max_megabytes / 8);

	QObject::connect(_ui.mark_file_sz_slider, SIGNAL(valueChanged(int)),
		this, SLOT(mark_file_size_changed(int)));

	_ui.mark_file_sz_edit->setEnabled(false);

	const int Max_last_access_days = 365 * 4;

	_ui.last_access_slider->setMinimum(0);
	_ui.last_access_slider->setMaximum(Max_last_access_days);
	_ui.last_access_slider->setTickInterval(Max_last_access_days / 8);

	QObject::connect(_ui.last_access_slider, SIGNAL(valueChanged(int)),
		this, SLOT(file_last_access_changed(int)));
}

void Size_simple_filter_view::min_file_size_changed(int)
{
	_ui.min_file_sz_edit->setText(helper::format_size(min_file_size()));

	mark_file_size_changed(0);
}

void Size_simple_filter_view::mark_file_size_changed(int)
{
	_ui.mark_file_sz_slider->setValue((std::max)(_ui.mark_file_sz_slider->value(), 
		_ui.min_file_sz_slider->value()));
	_ui.mark_file_sz_edit->setText(helper::format_size(mark_file_size()));
}

void Size_simple_filter_view::file_last_access_changed(int)
{
	_ui.last_access_edit->setText(helper::format_time_s(file_last_access_seconds()));
}

uint64 Size_simple_filter_view::min_file_size() const
{
	uint64 result = (uint64)_ui.min_file_sz_slider->value();
	result *= MEGABYTE;
	return result;
}

uint64 Size_simple_filter_view::mark_file_size() const
{
	uint64 result = (uint64)_ui.mark_file_sz_slider->value();
	result *= MEGABYTE;
	return result;
}

uint32 Size_simple_filter_view::file_last_access_seconds() const
{
	uint64 result = (uint64)_ui.last_access_slider->value();
	result *= DAY_S;
	return result;
}

void Size_simple_filter_view::update_view()
{
	_ui.min_file_sz_slider->setValue(_filter->min_size() / MEGABYTE);
	_ui.mark_file_sz_slider->setValue(_filter->Pref_size() / MEGABYTE);
	_ui.last_access_slider->setValue(_filter->Last_access_sec() / DAY_S);
}

void Size_simple_filter_view::update_filter()
{
	_filter->set_min_size(min_file_size());
	_filter->set_pref_size(mark_file_size());
	_filter->set_last_access_sec(file_last_access_seconds());
}

void Size_simple_filter_view::set_filter(Size_simple_filter* filter) 
{
	_filter = filter;
	_base_filter = filter;
}

}// namespace xdd
