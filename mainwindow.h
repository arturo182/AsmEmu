#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class QTreeWidgetItem;
class AsmHighlighter;
class VirtualMachine;
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
		void assemble();
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
		QMap<int, QTreeWidgetItem*> m_registerItems;
		QMap<int, QTreeWidgetItem*> m_labelItems;
		AsmHighlighter *m_asmHighlighter;
		VirtualMachine *m_virtualMachine;
		QVector<int> m_prevRegisters;
		QVector<int> m_prevMemory;
		int m_prevStartCell;
		QSpinBox *m_startCellSpinBox;
		QLabel *m_positionLabel;
		QString m_currentFile;
};

#endif // MAINWINDOW_H
