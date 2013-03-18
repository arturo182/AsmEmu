#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMap>

class AsmHighlighter;
class VirtualMachine;
class Compiler;

class QTreeWidgetItem;
class QSignalMapper;
class QSpinBox;
class QLabel;

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		enum { MaxMru = 5 };
		
	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	protected:
		void closeEvent(QCloseEvent *event);

	private slots:
		void newFile();
		void open();
		bool save();
		bool saveAs();
		bool assemble();
		void startExec();
		void singleExec();
		void stopExec();
		void rewindExec();
		void about();
		void documentWasModified();
		void updateActions();
		void updateRegisters();
		void updateLabels();
		void updateCursorPosition();
		void updateMruMenu();
		void clearMru();
		void setStartLine(const int &lineNo);
		void setStartCell(const int &cellNo);
		void changeMemorySize();
		void changeLanguage();
		void toggleAsmHelp();

	private:
		void readSettings();
		void writeSettings();
		bool maybeSave();
		void loadFile(const QString &fileName);
		bool saveFile(const QString &fileName);
		void setCurrentFile(const QString &fileName);
		
	private:
		Ui::MainWindow *m_ui;
		QSignalMapper *m_mruMapper;
		QHash<int, QTreeWidgetItem*> m_registerItems;
		QHash<int, QTreeWidgetItem*> m_labelItems;
		AsmHighlighter *m_asmHighlighter;
		VirtualMachine *m_virtualMachine;
		QTimer m_tickTimer;
		Compiler *m_compiler;
		QVector<int> m_prevRegisters;
		QVector<int> m_prevMemory;
		int m_prevStartCell;
		QSpinBox *m_startCellSpinBox;
		QLabel *m_positionLabel;
		QString m_currentFile;
};

#endif // MAINWINDOW_H
