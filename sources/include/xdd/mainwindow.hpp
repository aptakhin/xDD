/** xDDTools */
#pragma once

#include "xdd/files_model.hpp"
#include "xdd/clean_model.hpp"

#include <QMainWindow>
#include <QThread>
#include <QThreadPool>
#include <QFileDialog>
#include <QTimer>

#include "xdd/settings_window.hpp"

namespace Ui {
	class MainWindow;
}

namespace xdd {

class MainWindow : public QMainWindow
{
	Q_OBJECT

	enum Main_button_state
	{
		MBS_RUN,
		MBS_STOP
	};

public:

	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	enum Tabs
	{
		T_SCAN,
		T_CLEAN,
	};

public slots:
	void runstop_btn_clicked();

	void scan_finished();

	void show_file_dlg();

	void file_selected(const QString& file);

	void root_file_changed(const QString&);

	void scan_updated();

	void update_clean(bool hint_do_rec_reset);
	void update_clean_rec();

	void enable_cleaning_tab(bool enable);

	void select_scanner_tab();
	void select_cleaning_tab();

	void clean_updated();

	void clean_btn_clicked();
	void settings_btn_clicked();

	void update_clean_btn_access();

	void tab_selected(int index);

	void update_main_btn(Main_button_state new_bs);

private /*overriden*/:

	void bind_slots();

	void init_filters();

private:
	std::unique_ptr<::Ui::MainWindow> ui_;

	std::unique_ptr<Settings_window> settings_;

	std::unique_ptr<QFileDialog> file_dlg_;

	std::unique_ptr<Files_model> files_model_;
	std::unique_ptr<Clean_model> clean_model_;

	QTimer timer_;

	bool reset_files_model_;

	Main_button_state mbs_;
};

} // namespace xdd
