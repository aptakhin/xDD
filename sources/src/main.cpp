/** xDDTools */
#include "xdd/common.hpp"
#include "xdd/logger.hpp"
#include "xdd/mainwindow.hpp"
#include "xdd/manager.hpp"
#include "xdd/settings.hpp"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	xdd::Logger l;
	xdd::Settings_manager smgr;
	QApplication app(argc, argv);
	xdd::Scan_manager mgr;
	xdd::MainWindow w;
	smgr.notify_everything_initialized();
	w.show();
	return app.exec();
}
