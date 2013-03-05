#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QDebug>

int main(int argc, char *argv[])
{
	QApplication::setApplicationName("AsmEmu");
	QApplication::setOrganizationName("arturo182");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication app(argc, argv);

	QSettings set;
	if(!set.value("lang", "pl").isNull()) {
		QTranslator *translator = new QTranslator(&app);
		translator->load(app.applicationDirPath() + "/lang/asmemu_" + set.value("lang", "pl").toString() + ".qm");
		app.installTranslator(translator);

		QTranslator *qtTranslator = new QTranslator(&app);
		qtTranslator->load(app.applicationDirPath() + "/lang/qt_" + set.value("lang", "pl").toString() + ".qm");
		app.installTranslator(qtTranslator);
	}

	MainWindow wnd;
	wnd.show();
	
	return app.exec();
}
