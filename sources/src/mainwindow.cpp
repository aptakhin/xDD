#include "xdd/mainwindow.hpp"
#include "xdd/files_model.hpp"
#include "xdd/Clean_model.hpp"
#include "xdd/manager.hpp"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QResizeEvent>

namespace xdd {

MainWindow::MainWindow(QWidget *parent) 
:	QMainWindow(parent),
	ui(new ::Ui::MainWindow),
	_file_dlg(nullptr),
	_reset_files_model(false)
{
	ui->setupUi(this);

	_settings = new Settings_window(this);
	_settings->hide();
	
	_files_model = new Files_model();
	ui->files->setModel(_files_model);
	ui->files->setAnimated(true);
	ui->files->setAutoScroll(false);

	_clean_model = new Clean_model();
	ui->clean->setModel(_clean_model);
	ui->clean->setAnimated(true);
	ui->clean->setAutoScroll(false);
	ui->clean->setSortingEnabled(true);

	_file_dlg = new QFileDialog(this);
	_file_dlg->setFileMode(QFileDialog::DirectoryOnly);

	ui->root_file->setText("E:\\");
	root_file_changed("");

	enable_cleaning_tab(false);

#ifdef XDD_UNIVERSAL_CODE
	ui->move_to_recycle_opt->hide();
	ui->move_to_recycle_opt->setCheckable(false);
	ui->move_to_recycle_opt->setChecked(false);
	ui->remove_from_hd_opt->setChecked(true);
#endif

	bind_slots();

	XDD_LOG("Interface successully initalized");
}
	
MainWindow::~MainWindow()
{
	delete _file_dlg;
	delete _settings;
	delete ui;
}

void MainWindow::bind_slots()
{
	QObject::connect(_files_model, SIGNAL(update_clean()),
		this, SLOT(update_clean()));

	_file_dlg = new QFileDialog(this);
	_file_dlg->setFileMode(QFileDialog::DirectoryOnly);

	QObject::connect(ui->run_btn, SIGNAL(clicked()),
		this, SLOT(run_btn_clicked()));

	QObject::connect(ui->root_file, SIGNAL(textChanged(QString)),
		this, SLOT(root_file_changed(QString)));
	
	QObject::connect(ui->view_root_file, SIGNAL(clicked()),
		this, SLOT(show_file_dlg()));

	QObject::connect(_file_dlg, SIGNAL(fileSelected(QString)),
		this, SLOT(file_selected(QString)));

	QObject::connect(Scan_manager::i(), SIGNAL(scan_finished()),
		this, SLOT(scan_finished()));

	QObject::connect(Scan_manager::i(), SIGNAL(update_info()),
		this, SLOT(scan_updated()));

	QObject::connect(ui->scanner_tab_next, SIGNAL(clicked()),
		this, SLOT(select_cleaning_tab()));

	QObject::connect(ui->cleaning_tab_back, SIGNAL(clicked()),
		this, SLOT(select_scanner_tab()));

	QObject::connect(ui->clean_btn, SIGNAL(clicked()),
		this, SLOT(clean_btn_clicked()));

	QObject::connect(ui->settings_btn, SIGNAL(clicked()),
		this, SLOT(settings_btn_clicked()));

	QObject::connect(_clean_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
		this, SLOT(clean_updated()));

	QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(scan_updated()));

	QObject::connect(ui->clean, SIGNAL(clean_updated()),
		this, SLOT(clean_updated()));

	QObject::connect(ui->tab, SIGNAL(currentChanged(int)),
		this, SLOT(tab_selected(int)));
}

void MainWindow::run_btn_clicked()
{
	_timer.setSingleShot(false);
	_timer.start(500);

	Scan_manager::i()->prepare_for_scan();

	Scan_files_param params;

	params.start_path = ui->root_file->text();

	_files_model->notify_scan_started();
	_clean_model->notify_scan_started();
	ui->files->reset();
	ui->clean->reset();

	Scan_manager::i()->start_scan_thread(params);

	ui->status->setText("Processing");
}

void MainWindow::enable_cleaning_tab(bool enable)
{
	QString msg = "";
	if (!enable)
		msg = "Run the scanner before selecting files";

	ui->scanner_tab_next->setEnabled(enable);
	ui->scanner_tab_next->setToolTip(msg);
	ui->tab->setTabEnabled(T_CLEAN, enable);
	ui->tab->setTabToolTip(T_CLEAN, msg);
}

void MainWindow::scan_finished()
{
	_timer.stop();
	ui->status->setText("Finished");
	ui->common_info->setText(
		QString("Execution time: ") + helper::format_time_ms(Scan_manager::i()->last_time_exec()) +
		QString("\nFull disk size: ") + helper::format_size(Scan_manager::i()->fs_stat()->full_disk_size()) +
		QString("\nFree disk size: ") + helper::format_size(Scan_manager::i()->fs_stat()->free_disk_size())
	);
	_files_model->notify_scan_finished();
	_clean_model->notify_scan_finished();
	update_clean_btn_access();

	enable_cleaning_tab(true);

	//ui->files->setColumnWidth(Files_model::C_NAME, 400);
	//ui->files->setColumnWidth(Files_model::C_SIZE,  50);
	ui->files->resizeColumnToContents(Files_model::C_NAME);
	ui->files->resizeColumnToContents(Files_model::C_NAME);
	ui->files->reset();

	ui->clean->setColumnWidth(Files_model::C_NAME, 400);
	ui->clean->reset();

	update_clean();
}

void MainWindow::show_file_dlg()
{
	_file_dlg->show();
}

void MainWindow::file_selected(const QString& file)
{
	ui->root_file->setText(file);
	root_file_changed("");
}

void MainWindow::root_file_changed(const QString&)
{
	if (!ui->root_file->text().isEmpty())
		ui->run_btn->setEnabled(true);
}

void MainWindow::scan_updated()
{
	// Very important time event. 
	// But we just draw dots to show that we don't sleep.
	int dots = 0;
	const int max_dots = 3;

	QString status = ui->status->text();
	int l = (int)status.size() - 1;
	for (; l >= 0; --l, ++dots)
	{
		if (status[l] != L'.')
			break;
	}
	status.chop(dots);

	++dots;

	if (dots > max_dots)
		dots = 1;

	ui->status->setText(status + QString(".").repeated(dots));
}

void MainWindow::update_clean()
{
	_clean_model->reset();
	ui->clean->reset();
	clean_updated();
}

void MainWindow::clean_updated()
{
	update_clean_btn_access();// Enable clean button or disable it

	// Show some statistics
	QString new_stat = QString() +
		"Bytes will be free: " + helper::format_size(_clean_model->calculate_free_size()) +
		"\nTotal free disk size will be: " + 
		helper::format_size(Scan_manager::i()->fs_stat()->free_disk_size() + _clean_model->calculate_free_size());

	// update info in both tabs
	ui->new_stat->setText(new_stat);
	ui->new_stat2->setText(new_stat);


	_reset_files_model = true;// Have to update some cache

	if (ui->tab->currentIndex() == T_SCAN)
	{
		_files_model->reset();// Can't delay. So update at moment
		_reset_files_model = false;
	}
}

void MainWindow::select_scanner_tab()
{
	ui->tab->setCurrentIndex(T_SCAN);
}

void MainWindow::select_cleaning_tab()
{
	ui->tab->setCurrentIndex(T_CLEAN);
}

void MainWindow::update_clean_btn_access()
{
	ui->clean_btn->setEnabled(!_clean_model->empty());
}

void MainWindow::clean_btn_clicked()
{
	// Are you sure? Are you really sure? Are you really sure of you real sure? 
	// Don't you stupid to format disc? No? Fuh, I'm glad!
	QMessageBox submitBox(QMessageBox::Question, "Cleaning", "Submit cleaning?", QMessageBox::NoButton, this);
	QString what_really = ui->move_to_recycle_opt->isChecked()? 
		"move files to Recycle Bin" : "erase files";
	submitBox.setInformativeText(QString() + "Do you realy want to " + what_really + " with " +
		helper::format_size(_clean_model->calculate_free_size()) + " volume?");
	submitBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	submitBox.setDefaultButton(QMessageBox::No);
	int button = submitBox.exec();
	
	switch (button) 
	{
	case QMessageBox::Yes:// Move some data to null
	{
		Clean_manager clean;
		if (ui->move_to_recycle_opt->isChecked())
			clean.make_clean(Clean_manager::A_MOVE_TO_RECYCLE_BIN);
		else if (ui->remove_from_hd_opt->isChecked())// or to recycle bin
			clean.make_clean(Clean_manager::A_REMOVE);
	}
		break;
	}
}

void MainWindow::settings_btn_clicked()
{
	_settings->show();
}

void MainWindow::tab_selected(int tab)
{
	if (_reset_files_model && tab == T_SCAN)
	{
		_files_model->reset();
		_reset_files_model = false;
	}
}

} // namespace xdd

