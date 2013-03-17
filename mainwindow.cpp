#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "asmhighlighter.h"
#include "virtualmachine.h"
#include "compiler.h"
#include "contants.h"

#include <QRegularExpression>
#include <QDesktopServices>
#include <QSignalMapper>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>
#include <QSpinBox>
#include <QDebug>
#include <QLabel>

class TreeItem : public QTreeWidgetItem
{
	public:
		TreeItem(QTreeWidget *parent):
			QTreeWidgetItem(parent)
		{ }

	private:
		bool operator<(const QTreeWidgetItem &other) const
		{
			const int column = treeWidget()->sortColumn();
			switch(column) {
				case 0:
					return text(0).toLower() < other.text(0).toLower();
				break;

				case 1:
				case 2:
					return text(column).toInt() < other.text(column).toInt();
				break;
			}

			return true;
		}
};

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	m_ui(new Ui::MainWindow),
	m_mruMapper(new QSignalMapper(this)),
	m_virtualMachine(new VirtualMachine(this)),
	m_startCellSpinBox(new QSpinBox(this)),
	m_positionLabel(new QLabel(this))
{
	m_ui->setupUi(this);
	m_ui->canvasDock->hide();

	m_ui->vmToolBar->addWidget(new QLabel(tr("Start cell: "), this));
	m_ui->vmToolBar->addWidget(m_startCellSpinBox);
	m_ui->statusBar->addPermanentWidget(m_positionLabel);

	QAction *viewMenuAction = m_ui->menuBar->insertMenu(m_ui->helpMenu->menuAction(), createPopupMenu());
	viewMenuAction->setText(tr("&View"));


	m_asmHighlighter = new AsmHighlighter(m_ui->codeEdit->document());

	connect(m_ui->fileMenu, &QMenu::aboutToShow, this, &MainWindow::updateMruMenu);

	void (QSignalMapper:: *mapped)(const QString &) = &QSignalMapper::mapped;
	connect(m_mruMapper, mapped, this, &MainWindow::loadFile);

	connect(m_virtualMachine, &VirtualMachine::registersChanged, this, &MainWindow::updateRegisters);
	connect(m_virtualMachine, &VirtualMachine::labelsChanged, this, &MainWindow::updateLabels);
	connect(m_virtualMachine, &VirtualMachine::memoryChanged, m_ui->memoryView, &MemoryView::setMemory);
	connect(m_virtualMachine, &VirtualMachine::memoryChanged, [=](const QVector<int> &memory)
	{
		m_startCellSpinBox->setMaximum(memory.size() - 1);
	});

	void (QSpinBox:: *valueChanged)(int) = &QSpinBox::valueChanged;
	connect(m_startCellSpinBox, valueChanged, this, &MainWindow::setStartCell);
	connect(m_startCellSpinBox, valueChanged, m_virtualMachine, &VirtualMachine::setExecCell);

	connect(m_ui->codeEdit->document(), &QTextDocument::contentsChanged, this, &MainWindow::documentWasModified);
	connect(m_ui->codeEdit, &CodeEdit::copyAvailable, this, &MainWindow::updateActions);
	connect(m_ui->codeEdit, &CodeEdit::copyAvailable, this, &MainWindow::updateActions);
	connect(m_ui->codeEdit, &CodeEdit::undoAvailable, this, &MainWindow::updateActions);
	connect(m_ui->codeEdit, &CodeEdit::redoAvailable, this, &MainWindow::updateActions);
	connect(m_ui->codeEdit, &CodeEdit::cursorPositionChanged, this, &MainWindow::updateCursorPosition);
	connect(m_ui->codeEdit, &CodeEdit::gutterClicked, this, &MainWindow::setStartLine);
	connect(m_ui->codeEdit, &CodeEdit::focusChanged, this, &MainWindow::updateActions);
	connect(m_ui->codeEdit, &CodeEdit::fileDropped, [=](const QString &fileName)
	{
		if(!maybeSave())
			return;

		loadFile(fileName);
	});

	updateActions();
	updateRegisters();

	readSettings();
	setCurrentFile("");
}

MainWindow::~MainWindow()
{
	delete m_asmHighlighter;
	delete m_ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if(maybeSave()) {
		writeSettings();

		event->accept();
	} else {
		event->ignore();
	}
}

void MainWindow::newFile()
{
	if(maybeSave()) {
		m_ui->codeEdit->clear();

		setCurrentFile("");
	}
}

void MainWindow::open()
{
	if(!maybeSave())
		return;

	QSettings set;

	const QString fileName = QFileDialog::getOpenFileName(this, QString(), set.value("lastDir").toString(), tr("Assembler files (*.asm)"));

	//save directory
	set.setValue("lastDir", QFileInfo(fileName).absolutePath());

	if(!fileName.isEmpty())
		loadFile(fileName);
}

bool MainWindow::save()
{
	if(m_currentFile.isEmpty())
		return saveAs();

	return saveFile(m_currentFile);
}

bool MainWindow::saveAs()
{
	QSettings set;

	const QString fileName = QFileDialog::getSaveFileName(this, QString(), set.value("lastDir").toString(), tr("Assembler files (*.asm)"));
	if(fileName.isEmpty())
		return false;

	//save directory
	set.setValue("lastDir", QFileInfo(fileName).absolutePath());

	return saveFile(fileName);
}

bool MainWindow::assemble()
{
	m_ui->messagesTree->clear();

	QStringList msgs;
	const bool result = m_virtualMachine->assemble(m_ui->codeEdit->toPlainText(), &msgs);
	if(result)
		return true;

	foreach(const QString &message, msgs) {
		QTreeWidgetItem *messageItem = new QTreeWidgetItem(m_ui->messagesTree);
		messageItem->setIcon(0, QIcon(":/icons/cancel2.png"));
		messageItem->setText(0, message.mid(0, message.indexOf(':')));
		messageItem->setText(1, message.mid(message.indexOf(':') + 1));
	}

	rewindExec();

	return false;
}

void MainWindow::startExec()
{
	m_ui->codeEdit->setReadOnly(true);
	m_asmHighlighter->setEnabled(false);
	m_startCellSpinBox->setEnabled(false);
	m_ui->startAction->setEnabled(false);
	m_ui->singleStepAction->setEnabled(false);
	m_ui->memorySizeAction->setEnabled(false);
	m_ui->stopAction->setEnabled(true);
	updateActions();

	m_prevMemory = m_virtualMachine->memory();
	m_prevRegisters = m_virtualMachine->registers();
	m_prevStartCell = m_startCellSpinBox->value();

	if(!assemble())
		return;

	int execCell = 0;
	while(m_virtualMachine->exec()) {
		execCell = m_virtualMachine->execCell();

		m_startCellSpinBox->setValue(execCell);
		setStartCell(execCell);

		qApp->processEvents();
	}

	m_ui->stopAction->setEnabled(false);
	m_ui->rewindAction->setEnabled(true);
}

void MainWindow::singleExec()
{
	if(m_ui->startAction->isEnabled()) {
		m_ui->codeEdit->setReadOnly(true);
		m_asmHighlighter->setEnabled(false);
		m_startCellSpinBox->setEnabled(false);
		m_ui->startAction->setEnabled(false);
		m_ui->memorySizeAction->setEnabled(false);
		m_ui->rewindAction->setEnabled(true);
		updateActions();

		m_prevMemory = m_virtualMachine->memory();
		m_prevRegisters = m_virtualMachine->registers();
		m_prevStartCell = m_startCellSpinBox->value();

		if(!assemble())
			return;
	}

	if(!m_ui->singleStepAction->isEnabled())
		return;

	bool hasNext = m_virtualMachine->exec();
	m_startCellSpinBox->setValue(m_virtualMachine->execCell());
	setStartCell(m_virtualMachine->execCell());

	if(!hasNext)
		m_ui->singleStepAction->setEnabled(false);
}

void MainWindow::stopExec()
{
	m_virtualMachine->setExecCell(-1);
}

void MainWindow::rewindExec()
{
	m_virtualMachine->setMemory(m_prevMemory);
	m_virtualMachine->setRegisters(m_prevRegisters);
	m_startCellSpinBox->setValue(m_prevStartCell);

	m_ui->codeEdit->setReadOnly(false);
	m_asmHighlighter->setEnabled(true);
	m_startCellSpinBox->setEnabled(true);
	m_ui->startAction->setEnabled(true);
	m_ui->singleStepAction->setEnabled(true);
	m_ui->memorySizeAction->setEnabled(true);
	m_ui->rewindAction->setEnabled(false);
	m_ui->stopAction->setEnabled(false);

	m_ui->labelsTree->clear();
	m_labelItems.clear();

	updateActions();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("AsmEmu"), tr("Assembler Emulator v%1<br>Copyright &copy; 2013 <a href=\"mailto: arturo182@tlen.pl\">Artur Pacholec</a>").arg(Constants::Version));
}

void MainWindow::documentWasModified()
{
	setWindowModified(m_ui->codeEdit->document()->isModified());
	setStartCell(m_startCellSpinBox->value());
}

void MainWindow::updateActions()
{
	bool isFocused = m_ui->codeEdit->hasFocus();

	m_ui->undoAction->setEnabled(isFocused && m_ui->codeEdit->canUndo());
	m_ui->redoAction->setEnabled(isFocused && m_ui->codeEdit->canRedo());
	m_ui->pasteAction->setEnabled(isFocused && m_ui->codeEdit->canPaste());
	m_ui->cutAction->setEnabled(isFocused && m_ui->codeEdit->textCursor().hasSelection());
	m_ui->copyAction->setEnabled(isFocused && m_ui->codeEdit->textCursor().hasSelection());
}

void MainWindow::updateRegisters()
{
	for(int i = 0; i < m_virtualMachine->registerCount(); ++i) {
		if(!m_registerItems.contains(i)) {
			TreeItem *registerItem = new TreeItem(m_ui->registersTree);
			registerItem->setText(0, m_virtualMachine->registerName(i));
			registerItem->setText(1, QString::number(m_virtualMachine->registers().at(i)));
			m_registerItems.insert(i, registerItem);

			continue;
		}

		QTreeWidgetItem *registerItem = m_registerItems.value(i);
		registerItem->setText(1, QString::number(m_virtualMachine->registers().at(i)));
	}
}

void MainWindow::updateLabels()
{
	for(int i = 0; i < m_virtualMachine->labelCount(); ++i) {
		if(!m_labelItems.contains(i)) {
			TreeItem *labelItem = new TreeItem(m_ui->labelsTree);
			labelItem->setText(0, m_virtualMachine->labelName(i));
			labelItem->setText(1, QString::number(m_virtualMachine->labelCellNo(i)));
			labelItem->setText(2, QString::number(m_virtualMachine->labels().at(i)));
			m_labelItems.insert(i, labelItem);

			continue;
		}

		QTreeWidgetItem *labelItem = m_labelItems.value(i);
		labelItem->setText(2, QString::number(m_virtualMachine->labels().at(i)));
	}
}

void MainWindow::updateCursorPosition()
{
	m_positionLabel->setText(tr("Line %1 Col %2").arg(m_ui->codeEdit->textCursor().block().blockNumber()).arg(m_ui->codeEdit->textCursor().positionInBlock()));
}

void MainWindow::updateMruMenu()
{
	m_ui->recentFilesMenu->clear();

	QSettings set;
	const QStringList files = set.value("mru").toStringList();
	foreach(const QString &fileName, files) {
		QAction *action = m_ui->recentFilesMenu->addAction(fileName);
		connect(action, SIGNAL(triggered()), m_mruMapper, SLOT(map()));
		m_mruMapper->setMapping(action, fileName);
	}

	if(files.count()) {
		m_ui->recentFilesMenu->setEnabled(true);
		m_ui->recentFilesMenu->addSeparator();
		m_ui->recentFilesMenu->addAction(m_ui->clearMruAction);
	} else {
		m_ui->recentFilesMenu->setEnabled(false);
	}
}

void MainWindow::clearMru()
{
	QSettings set;
	set.setValue("mru", QStringList());
}

void MainWindow::setStartLine(const int &lineNo)
{
	Compiler compiler(m_ui->codeEdit->toPlainText());
	compiler.compile();

	const int cellNo = compiler.lineMap().value(lineNo);

	m_startCellSpinBox->setValue(cellNo);
	m_ui->codeEdit->setSpecialLine(lineNo);
}

void MainWindow::setStartCell(const int &cellNo)
{
	m_ui->codeEdit->setSpecialLine(-1);

	Compiler compiler(m_ui->codeEdit->toPlainText());
	compiler.compile();

	const int lineNo = compiler.lineMap().key(cellNo);
	m_ui->codeEdit->setSpecialLine(lineNo);
}

void MainWindow::changeMemorySize()
{
	bool dlgAccepted = false;
	const int memorySize = QInputDialog::getInt(this, tr("Set memory size"), tr("Memory size:"), m_virtualMachine->memorySize(), 1, INT_MAX, 1, &dlgAccepted);

	if(dlgAccepted)
		m_virtualMachine->setMemorySize(memorySize);
}

void MainWindow::changeLanguage()
{
	QSettings set;

	const QStringList langs = { "", "pl" };
	const QStringList fullLangs = { "English", "Polski" };
	int current = langs.indexOf(set.value("lang", "pl").toString());

	bool dlgAccepted = false;
	const QString newLang = QInputDialog::getItem(this, tr("Change language"), tr("Choose language:"), fullLangs, current, false, &dlgAccepted);

	if(!dlgAccepted)
		return;

	set.setValue("lang", langs.at(fullLangs.indexOf(newLang)));
	QMessageBox::information(this, tr("Restart needed"), tr("The language change will take effect after you restart the application."));
}

void MainWindow::toggleAsmHelp()
{
	QSettings set;
	QString lang = set.value("lang", "pl").toString();
	if(lang.isEmpty())
		lang = "en";

	QDesktopServices::openUrl(qApp->applicationDirPath() + "/help/help_" + lang + ".html");
}

void MainWindow::readSettings()
{
	QSettings set;
	restoreGeometry(set.value("geometry").toByteArray());
	restoreState(set.value("state").toByteArray());
	m_ui->registersTree->header()->restoreState(set.value("regCols").toByteArray());
	m_ui->labelsTree->header()->restoreState(set.value("labelCols").toByteArray());
	m_ui->messagesTree->header()->restoreState(set.value("msgsCols").toByteArray());
	m_virtualMachine->setMemorySize(set.value("memorySize", 100).toInt());
}

void MainWindow::writeSettings()
{
	QSettings set;
	set.setValue("geometry", saveGeometry());
	set.setValue("state", saveState());
	set.setValue("regCols", m_ui->registersTree->header()->saveState());
	set.setValue("labelCols", m_ui->labelsTree->header()->saveState());
	set.setValue("msgsCols", m_ui->messagesTree->header()->saveState());
	set.setValue("memorySize", m_virtualMachine->memorySize());
}

bool MainWindow::maybeSave()
{
	if(m_ui->codeEdit->document()->isModified()) {
		const QMessageBox::StandardButton result = QMessageBox::warning(this, tr("Save changes?"), tr("Would you like to save changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

		if(result == QMessageBox::Save) {
			return save();
		} else if(result == QMessageBox::Cancel) {
			return false;
		}
	}

	return true;
}

void MainWindow::loadFile(const QString &fileName)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly | QFile::Text))
		return;

	QTextStream in(&file);
	m_ui->codeEdit->setPlainText(in.readAll());
	file.close();

	Compiler compiler(m_ui->codeEdit->toPlainText());
	compiler.compile();
	m_startCellSpinBox->setValue(compiler.startCell());

	setCurrentFile(fileName);
}

bool MainWindow::saveFile(const QString &fileName)
{
	QFileInfo fileInfo(fileName);
	if(fileInfo.suffix().toLower() != "asm")
		fileInfo.setFile(fileName + ".asm");

	QFile file(fileInfo.absoluteFilePath());
	if(!file.open(QFile::WriteOnly | QFile::Text))
		return false;

	QTextStream out(&file);
	out << m_ui->codeEdit->toPlainText();
	file.close();

	setCurrentFile(fileName);

	return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	//save mru
	if(!fileName.isEmpty()) {
		QSettings set;
		QStringList files = set.value("mru").toStringList();

		files.removeAll(fileName);
		files.prepend(fileName);

		while(files.size() > MaxMru)
			files.removeLast();

		set.setValue("mru", files);
	}

	//set current file
	m_currentFile = fileName;

	m_ui->codeEdit->document()->setModified(false);

	updateCursorPosition();
	setWindowModified(false);
	setWindowTitle(QString("%1[*] - %2").arg(m_currentFile.isEmpty() ? tr("untitled.asm") : QFileInfo(m_currentFile).fileName()).arg(tr("AsmEmu")));
}
