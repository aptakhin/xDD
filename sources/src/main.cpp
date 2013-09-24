/** xDDTools */
#include "xdd/common.hpp"
#include "xdd/logger.hpp"
#include "xdd/mainwindow.hpp"
#include "xdd/manager.hpp"
#include "xdd/settings.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
	xdd::Logger l;
	QApplication app(argc, argv);
	xdd::Scan_manager mgr;
	xdd::MainWindow w;
	w.show();
	return app.exec();
}
