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
	filter_(filter),
	ui_()
{
	ui_.setupUi(this);

	ui_.simple_filter_activated->setChecked(true);

	const int Max_megabytes = 4096;

	ui_.min_file_sz_slider->setMinimum(0);
	ui_.min_file_sz_slider->setMaximum(Max_megabytes);
	ui_.min_file_sz_slider->setTickInterval(Max_megabytes / 8);

	QObject::connect(ui_.min_file_sz_slider, SIGNAL(valueChanged(int)),
		this, SLOT(min_file_size_changed(int)));

	ui_.min_file_sz_edit->setEnabled(false);

	ui_.mark_file_sz_slider->setMinimum(0);
	ui_.mark_file_sz_slider->setMaximum(Max_megabytes);
	ui_.mark_file_sz_slider->setTickInterval(Max_megabytes / 8);

	QObject::connect(ui_.mark_file_sz_slider, SIGNAL(valueChanged(int)),
		this, SLOT(mark_file_size_changed(int)));

	ui_.mark_file_sz_edit->setEnabled(false);

	const int Max_last_access_days = 365 * 4;

	ui_.last_access_slider->setMinimum(0);
	ui_.last_access_slider->setMaximum(Max_last_access_days);
	ui_.last_access_slider->setTickInterval(Max_last_access_days / 8);

	QObject::connect(ui_.last_access_slider, SIGNAL(valueChanged(int)),
		this, SLOT(file_last_access_changed(int)));
}

void Size_simple_filter_view::min_file_size_changed(int)
{
	ui_.min_file_sz_edit->setText(helper::format_size(min_file_size()));

	mark_file_size_changed(0);
}

void Size_simple_filter_view::mark_file_size_changed(int)
{
	ui_.mark_file_sz_slider->setValue((std::max)(ui_.mark_file_sz_slider->value(), 
		ui_.min_file_sz_slider->value()));
	ui_.mark_file_sz_edit->setText(helper::format_size(mark_file_size()));
}

void Size_simple_filter_view::file_last_access_changed(int)
{
	ui_.last_access_edit->setText(helper::format_time_s(file_last_access_seconds()));
}

uint64 Size_simple_filter_view::min_file_size() const
{
	uint64 result = (uint64)ui_.min_file_sz_slider->value();
	result *= MEGABYTE;
	return result;
}

uint64 Size_simple_filter_view::mark_file_size() const
{
	uint64 result = (uint64)ui_.mark_file_sz_slider->value();
	result *= MEGABYTE;
	return result;
}

uint32 Size_simple_filter_view::file_last_access_seconds() const
{
	uint64 result = (uint64)ui_.last_access_slider->value();
	result *= DAY_S;
	return result;
}

void Size_simple_filter_view::update_view()
{
	ui_.min_file_sz_slider->setValue(filter_->min_size() / MEGABYTE);
	min_file_size_changed(0);
	ui_.mark_file_sz_slider->setValue(filter_->pref_size() / MEGABYTE);
	mark_file_size_changed(0);
	ui_.last_access_slider->setValue(filter_->last_access_sec() / DAY_S);
	file_last_access_changed(0);
}

void Size_simple_filter_view::update_filter()
{
	filter_->set_min_size(min_file_size());
	filter_->set_pref_size(mark_file_size());
	filter_->set_last_access_sec(file_last_access_seconds());
}

void Size_simple_filter_view::set_filter(Size_simple_filter* filter) 
{
	filter_      = filter;
	base_filter_ = filter;
}

}// namespace xdd
