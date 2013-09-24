/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include "xdd/filter_views.hpp"

#include <QDialog>

namespace Ui {
	class SettingsWindow;
}

namespace xdd {

class Settings_window : public QDialog
{
	Q_OBJECT

public:

	explicit Settings_window(QWidget* parent = 0);
	~Settings_window();

	void init_filters();

	void install_filter_view(Filter_view* view);

	Size_simple_filter_view* simple_filter_view() { return &simple_size_filter_view_; }

public /*overriden*/:
	void show();

public slots:
	void apply_btn_clicked();
	void cancel_btn_clicked();

private:
	::Ui::SettingsWindow* ui_;

	Size_simple_filter_view simple_size_filter_view_;
};

} // namespace xdd
