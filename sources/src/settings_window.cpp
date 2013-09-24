/** xDDTools */
#include "xdd/settings_window.hpp"
#include "xdd/manager.hpp"
#include "ui_settings.h"

#include <QMessageBox>

namespace xdd {

Settings_window::Settings_window(QWidget *parent) 
:	QDialog(parent),
	_ui(new ::Ui::SettingsWindow),
	_simple_size_filter_view()
{
	_ui->setupUi(this);
	_ui->filters_box->removeItem(0);// It was obligatory in Qt Designer. Remove it.

	QObject::connect(_ui->apply_btn, SIGNAL(clicked()),
		this, SLOT(apply_btn_clicked()));

	QObject::connect(_ui->cancel_btn, SIGNAL(clicked()),
		this, SLOT(cancel_btn_clicked()));

	init_filters();
}

void Settings_window::init_filters()
{
	Size_simple_filter* ssf = new Size_simple_filter;

	_simple_size_filter_view.set_filter(ssf);
	install_filter_view(&_simple_size_filter_view);
	
	Scan_manager::i()->scanner().add_fast_filter(ssf);
}

void Settings_window::install_filter_view(Filter_view* view)
{
	_ui->filters_box->addItem(static_cast<QWidget*>(view), view->filter()->name());
}
	
Settings_window::~Settings_window()
{
	delete _ui;
}

void Settings_window::show()
{
	_simple_size_filter_view.update_view();
	QDialog::show();
}

void Settings_window::apply_btn_clicked()
{
	_simple_size_filter_view.update_filter();
	hide();
}

void Settings_window::cancel_btn_clicked()
{
	hide();
}

} // namespace xdd

