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
	reset_files_model_(false),
	mbs_(MBS_RUN)
{
	ui_.reset(new ::Ui::MainWindow);
	ui_->setupUi(this);

	settings_.reset(new Settings_window(this));
	settings_->hide();

	ui_->hdd->setIcon(QIcon(":/icon-hdd.png"));
	ui_->hdd->setIconSize(QSize(32, 32));
	ui_->hdd->setText("C:\\");
	
	files_model_.reset(new Files_model());
	ui_->files->setModel(files_model_.get());
	ui_->files->setAnimated(true);
	ui_->files->setAutoScroll(false);

	clean_model_.reset(new Clean_model());
	ui_->clean->setModel(clean_model_.get());
	ui_->clean->setAnimated(true);
	ui_->clean->setAutoScroll(false);
	ui_->clean->setSortingEnabled(true);

	file_dlg_.reset(new QFileDialog(this));
	file_dlg_->setFileMode(QFileDialog::DirectoryOnly);

	ui_->tab->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));

	ui_->root_file->setText("C:\\");
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
}

void MainWindow::bind_slots()
{
	QObject::connect(files_model_.get(), SIGNAL(update_clean()),
		this, SLOT(update_clean_rec()));

	QObject::connect(ui_->runstop_btn, SIGNAL(clicked()),
		this, SLOT(runstop_btn_clicked()));

	QObject::connect(ui_->root_file, SIGNAL(textChanged(QString)),
		this, SLOT(root_file_changed(QString)));

	file_dlg_.reset(new QFileDialog(this));
	file_dlg_->setFileMode(QFileDialog::DirectoryOnly);
	
	QObject::connect(ui_->view_root_file, SIGNAL(clicked()),
		this, SLOT(show_file_dlg()));

	QObject::connect(file_dlg_.get(), SIGNAL(fileSelected(QString)),
		this, SLOT(file_selected(QString)));

	QObject::connect(Scan_manager::i(), SIGNAL(scan_finished()),
		this, SLOT(scan_finished()));

	QObject::connect(Scan_manager::i(), SIGNAL(update_info()),
		this, SLOT(scan_updated()));

	QObject::connect(ui_->scanner_tab_next, SIGNAL(clicked()),
		this, SLOT(select_cleaning_tab()));

	QObject::connect(ui_->cleaning_tab_back, SIGNAL(clicked()),
		this, SLOT(select_scanner_tab()));

	QObject::connect(ui_->clean_btn, SIGNAL(clicked()),
		this, SLOT(clean_btn_clicked()));

	QObject::connect(ui_->settings_btn, SIGNAL(clicked()),
		this, SLOT(settings_btn_clicked()));

	QObject::connect(clean_model_.get(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
		this, SLOT(clean_updated()));

	QObject::connect(&timer_, SIGNAL(timeout()), this, SLOT(scan_updated()));

	QObject::connect(ui_->clean, SIGNAL(clean_updated()),
		this, SLOT(clean_updated()));

	QObject::connect(ui_->tab, SIGNAL(currentChanged(int)),
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
		params.start_path = ui_->root_file->text();

		files_model_->notify_scan_started();
		clean_model_->notify_scan_started();
		ui_->files->reset();
		ui_->clean->reset();

		Scan_manager::i()->start_scan_thread(params);

		update_main_btn(MBS_STOP);

		ui_->status->setText("Processing");
	}
	else
	{
		timer_.stop();
		Scan_manager::i()->stop_scan_thread();
		ui_->status->setText("Stopped");
		update_main_btn(MBS_RUN);
	}
}

void MainWindow::enable_cleaning_tab(bool enable)
{
	QString msg = "";
	if (!enable)
		msg = "Run the scanner before selecting files";

	ui_->scanner_tab_next->setEnabled(enable);
	ui_->scanner_tab_next->setToolTip(msg);
	ui_->tab->setTabEnabled(T_CLEAN, enable);
	ui_->tab->setTabToolTip(T_CLEAN, msg);
}

void MainWindow::scan_finished()
{
	timer_.stop();
	ui_->status->setText("Finished");
	ui_->common_info->setText(
		QString("Execution time: ") + helper::format_time_ms(Scan_manager::i()->last_time_exec()) +
		QString("\nFull disk size: ") + helper::format_size(Scan_manager::i()->fs_stat()->full_disk_size()) +
		QString("\nFree disk size: ") + helper::format_size(Scan_manager::i()->fs_stat()->free_disk_size())
	);
	files_model_->notify_scan_finished();
	clean_model_->notify_scan_finished();
	update_clean_btn_access();

	enable_cleaning_tab(true);

	ui_->files->reset();
	ui_->files->setColumnWidth(Files_model::C_NAME, 400);
	ui_->files->setColumnWidth(Files_model::C_SIZE,  50);

	ui_->clean->reset();
	ui_->clean->setColumnWidth(Clean_model::C_NAME, 400);
	
	update_clean(true);

	update_main_btn(MBS_RUN);
}

void MainWindow::show_file_dlg()
{
	file_dlg_->show();
}

void MainWindow::file_selected(const QString& file)
{
	ui_->root_file->setText(file);
	root_file_changed("");
}

void MainWindow::root_file_changed(const QString&)
{
	if (!ui_->root_file->text().isEmpty())
		ui_->runstop_btn->setEnabled(true);
}

void MainWindow::scan_updated()
{
	// Very important time event. 
	// But we just draw dots to show that we don't sleep.
	int dots = 0;
	const int max_dots = 3;

	QString status = ui_->status->text();
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

	ui_->status->setText(status + QString(".").repeated(dots));

	ui_->common_info->setText("ETA: " + helper::format_time_ms(Scan_manager::i()->approx_scan_time_left()));
}

void MainWindow::update_clean(bool hint_do_rec_reset)
{
	clean_model_->flush(hint_do_rec_reset);
	ui_->clean->reset();
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
	ui_->new_stat->setText(new_stat);
	ui_->new_stat2->setText(new_stat);

	reset_files_model_ = true;// Have to update some cache

	if (ui_->tab->currentIndex() == T_SCAN)
	{
		files_model_->flush();// Can't delay. So update at moment
		reset_files_model_ = false;
	}
}

void MainWindow::select_scanner_tab()
{
	ui_->tab->setCurrentIndex(T_SCAN);
}

void MainWindow::select_cleaning_tab()
{
	ui_->tab->setCurrentIndex(T_CLEAN);
}

void MainWindow::update_clean_btn_access()
{
	ui_->clean_btn->setEnabled(!clean_model_->empty());
}

void MainWindow::clean_btn_clicked()
{
	// Are you sure? Are you really sure? Are you really sure of you real sure? 
	// Don't you stupid to format disc? No? Fuh, I'm glad!
	QMessageBox submit_box(QMessageBox::Question, "Cleaning", "Submit cleaning?", QMessageBox::NoButton, this);
	QString what_really = ui_->move_to_recycle_opt->isChecked()? 
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
		if (ui_->move_to_recycle_opt->isChecked())
			clean.make_clean(Clean_manager::A_MOVE_TO_RECYCLE_BIN);
		else if (ui_->remove_from_hd_opt->isChecked())// or to recycle bin
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
	ui_->runstop_btn->setText(mbs_ == MBS_RUN? QString("Run") : QString("Stop"));	
}

} // namespace xdd

