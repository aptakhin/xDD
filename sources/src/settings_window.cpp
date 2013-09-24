/** xDDTools */
#include "xdd/settings_window.hpp"
#include "xdd/manager.hpp"
#include "ui_settings.h"

#include <QMessageBox>

namespace xdd {

Settings_window::Settings_window(QWidget *parent) 
:	QDialog(parent),
	ui_(new ::Ui::SettingsWindow),
	simple_size_filter_view_()
{
	ui_->setupUi(this);
	ui_->filters_box->removeItem(0);// It was obligatory in Qt Designer. Remove it.

	QObject::connect(ui_->apply_btn, SIGNAL(clicked()),
		this, SLOT(apply_btn_clicked()));

	QObject::connect(ui_->cancel_btn, SIGNAL(clicked()),
		this, SLOT(cancel_btn_clicked()));

	init_filters();
}

void Settings_window::init_filters()
{
	Size_simple_filter* ssf = new Size_simple_filter;

	simple_size_filter_view_.set_filter(ssf);
	install_filter_view(&simple_size_filter_view_);
	
	Scan_manager::i()->scanner().add_fast_filter(ssf);
}

void Settings_window::install_filter_view(Filter_view* view)
{
	ui_->filters_box->addItem(static_cast<QWidget*>(view), view->filter()->name());
}
	
Settings_window::~Settings_window()
{
	delete ui_;
}

void Settings_window::show()
{
	simple_size_filter_view_.update_view();
	QDialog::show();
}

void Settings_window::apply_btn_clicked()
{
	simple_size_filter_view_.update_filter();
	hide();
}

void Settings_window::cancel_btn_clicked()
{
	hide();
}

} // namespace xdd

