/** xDDTools */
#pragma once

#include "xdd/files_model.hpp"
#include "xdd/Clean_model.hpp"

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

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	enum Tabs
	{
		T_SCAN,
		T_CLEAN,
	};

public slots:
    void run_btn_clicked();

	void scan_finished();

	void show_file_dlg();

	void file_selected(const QString& file);

	void root_file_changed(const QString&);

	void scan_updated();

	void update_clean();

	void enable_cleaning_tab(bool enable);

	void select_scanner_tab();
	void select_cleaning_tab();

	void clean_updated();

	void clean_btn_clicked();
	void settings_btn_clicked();

	void update_clean_btn_access();

	void tab_selected(int index);

private /*overriden*/:

	void bind_slots();

	void init_filters();


private:
    ::Ui::MainWindow* ui;

	Settings_window* _settings;

	QFileDialog* _file_dlg;

    Files_model* _files_model;
	Clean_model* _clean_model;

	QTimer _timer;

	bool _reset_files_model;
};

} // namespace xdd
