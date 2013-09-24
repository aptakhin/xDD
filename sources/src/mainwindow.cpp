#include "xdd/mainwindow.hpp"
#include "xdd/files_model.hpp"
#include "xdd/clean_model.hpp"
#include "xdd/manager.hpp"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QResizeEvent>

namespace xdd {

MainWindow::MainWindow(QWidget *parent) 
:	QMainWindow(parent),
	ui(new ::Ui::MainWindow),
	file_dlg_(nullptr),
	reset_files_model_(false),
	mbs_(MBS_RUN)
{
	ui->setupUi(this);

	settings_ = new Settings_window(this);
	settings_->hide();
	
	files_model_ = new Files_model();
	ui->files->setModel(files_model_);
	ui->files->setAnimated(true);
	ui->files->setAutoScroll(false);

	clean_model_ = new Clean_model();
	ui->clean->setModel(clean_model_);
	ui->clean->setAnimated(true);
	ui->clean->setAutoScroll(false);
	ui->clean->setSortingEnabled(true);

	file_dlg_ = new QFileDialog(this);
	file_dlg_->setFileMode(QFileDialog::DirectoryOnly);

	ui->tab->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));

	ui->root_file->setText("C:\\");
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
	delete file_dlg_;
	delete settings_;
	delete ui;
}

void MainWindow::bind_slots()
{
	QObject::connect(files_model_, SIGNAL(update_clean()),
		this, SLOT(update_clean_rec()));

	QObject::connect(ui->runstop_btn, SIGNAL(clicked()),
		this, SLOT(runstop_btn_clicked()));

	QObject::connect(ui->root_file, SIGNAL(textChanged(QString)),
		this, SLOT(root_file_changed(QString)));

	file_dlg_ = new QFileDialog(this);
	file_dlg_->setFileMode(QFileDialog::DirectoryOnly);
	
	QObject::connect(ui->view_root_file, SIGNAL(clicked()),
		this, SLOT(show_file_dlg()));

	QObject::connect(file_dlg_, SIGNAL(fileSelected(QString)),
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

	QObject::connect(clean_model_, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
		this, SLOT(clean_updated()));

	QObject::connect(&timer_, SIGNAL(timeout()), this, SLOT(scan_updated()));

	QObject::connect(ui->clean, SIGNAL(clean_updated()),
		this, SLOT(clean_updated()));

	QObject::connect(ui->tab, SIGNAL(currentChanged(int)),
		this, SLOT(tab_selected(int)));
}

void MainWindow::runstop_btn_clicked()
{
	if (mbs_ == MBS_RUN)
	{
		timer_.setSingleShot(false);
		timer_.start(500);

		Scan_manager::i()->prepare_for_scan();

		Scan_files_param params;
		params.start_path = ui->root_file->text();

		files_model_->notify_scan_started();
		clean_model_->notify_scan_started();
		ui->files->reset();
		ui->clean->reset();

		Scan_manager::i()->start_scan_thread(params);

		update_main_btn(MBS_STOP);

		ui->status->setText("Processing");
	}
	else
	{
		timer_.stop();
		Scan_manager::i()->stop_scan_thread();
		ui->status->setText("Stopped");
		update_main_btn(MBS_RUN);
	}
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
	timer_.stop();
	ui->status->setText("Finished");
	ui->common_info->setText(
		QString("Execution time: ") + helper::format_time_ms(Scan_manager::i()->last_time_exec()) +
		QString("\nFull disk size: ") + helper::format_size(Scan_manager::i()->fs_stat()->full_disk_size()) +
		QString("\nFree disk size: ") + helper::format_size(Scan_manager::i()->fs_stat()->free_disk_size())
	);
	files_model_->notify_scan_finished();
	clean_model_->notify_scan_finished();
	update_clean_btn_access();

	enable_cleaning_tab(true);

	ui->files->reset();
	ui->files->setColumnWidth(Files_model::C_NAME, 400);
	ui->files->setColumnWidth(Files_model::C_SIZE,  50);

	ui->clean->reset();
	ui->clean->setColumnWidth(Clean_model::C_NAME, 400);
	
	update_clean(true);

	update_main_btn(MBS_RUN);
}

void MainWindow::show_file_dlg()
{
	file_dlg_->show();
}

void MainWindow::file_selected(const QString& file)
{
	ui->root_file->setText(file);
	root_file_changed("");
}

void MainWindow::root_file_changed(const QString&)
{
	if (!ui->root_file->text().isEmpty())
		ui->runstop_btn->setEnabled(true);
}

void MainWindow::scan_updated()
{
	// Very important time event. 
	// But we just draw dots to show that we don't sleep.
	int dots = 0;
	const int max_dots = 3;

	QString status = ui->status->text();
	int l = status.size() - 1;
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

	ui->common_info->setText("ETA: " + helper::format_time_ms(Scan_manager::i()->approx_scan_time_left()));
}

void MainWindow::update_clean(bool hint_do_rec_reset)
{
	clean_model_->flush(hint_do_rec_reset);
	ui->clean->reset();
	clean_updated();
}

void MainWindow::update_clean_rec()
{
	update_clean(true);
}

void MainWindow::clean_updated()
{
	update_clean_btn_access();// Enable clean button or disable it

	// Show some statistics
	QString new_stat = QString() +
		"Bytes will be free: " + helper::format_size(clean_model_->calculate_free_size()) +
		"\nTotal free disk size will be: " + 
		helper::format_size(Scan_manager::i()->fs_stat()->free_disk_size() + clean_model_->calculate_free_size());

	// update info in both tabs
	ui->new_stat->setText(new_stat);
	ui->new_stat2->setText(new_stat);

	reset_files_model_ = true;// Have to update some cache

	if (ui->tab->currentIndex() == T_SCAN)
	{
		files_model_->flush();// Can't delay. So update at moment
		reset_files_model_ = false;
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
	ui->clean_btn->setEnabled(!clean_model_->empty());
}

void MainWindow::clean_btn_clicked()
{
	// Are you sure? Are you really sure? Are you really sure of you real sure? 
	// Don't you stupid to format disc? No? Fuh, I'm glad!
	QMessageBox submit_box(QMessageBox::Question, "Cleaning", "Submit cleaning?", QMessageBox::NoButton, this);
	QString what_really = ui->move_to_recycle_opt->isChecked()? 
		"move files to Recycle Bin" : "erase files";
	submit_box.setInformativeText(QString() + "Do you realy want to " + what_really + " with " +
		helper::format_size(clean_model_->calculate_free_size()) + " volume?");
	submit_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	submit_box.setDefaultButton(QMessageBox::No);
	int button = submit_box.exec();
	
	switch (button) 
	{
	case QMessageBox::Yes:// Move some data to null
	{
		Clean_manager clean;
		if (ui->move_to_recycle_opt->isChecked())
			clean.make_clean(Clean_manager::A_MOVE_TO_RECYCLE_BIN);
		else if (ui->remove_from_hd_opt->isChecked())// or to recycle bin
			clean.make_clean(Clean_manager::A_REMOVE);

		files_model_->remove_deleted();

		update_clean(false);
	}
		break;
	}
}

void MainWindow::settings_btn_clicked()
{
	settings_->show();
}

void MainWindow::tab_selected(int tab)
{
	if (reset_files_model_ && tab == T_SCAN)
	{
		files_model_->flush();
		reset_files_model_ = false;
	}
}

void MainWindow::update_main_btn(Main_button_state new_bs)
{
	mbs_ = new_bs;
	ui->runstop_btn->setText(mbs_ == MBS_RUN? QString("Run") : QString("Stop"));	
}

} // namespace xdd

