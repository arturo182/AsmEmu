#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "asmhighlighter.h"
#include "virtualmachine.h"

#include <QRegularExpression>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>
#include <QSpinBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	m_ui(new Ui::MainWindow),
	m_virtualMachine(new VirtualMachine(this)),
	m_startCellSpinBox(new QSpinBox(this)),
	m_positionLabel(new QLabel(this))
{
	m_ui->setupUi(this);

	m_ui->vmToolBar->addWidget(new QLabel(tr("Start cell: "), this));
	m_ui->vmToolBar->addWidget(m_startCellSpinBox);
	m_ui->statusBar->addPermanentWidget(m_positionLabel);

	m_asmHighlighter = new AsmHighlighter(m_ui->codeEdit->document());

	connect(m_virtualMachine, &VirtualMachine::registersChanged, this, &MainWindow::updateRegisters);
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
	if(maybeSave()) {
		const QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Assembler files (*.asm)"));

		if(!fileName.isEmpty())
			loadFile(fileName);
	}
}

bool MainWindow::save()
{
	if(m_currentFile.isEmpty()) {
		return saveAs();
	} else {
		return saveFile(m_currentFile);
	}
}

bool MainWindow::saveAs()
{
	const QString fileName = QFileDialog::getSaveFileName(this, QString(), QString(), tr("Assembler files (*.asm)"));
	if(fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

void MainWindow::assemble()
{
	m_virtualMachine->assemble(m_ui->codeEdit->toPlainText());
}

void MainWindow::startExec()
{
	m_ui->codeEdit->setEnabled(false);
	m_startCellSpinBox->setEnabled(false);
	m_ui->startAction->setEnabled(false);
	m_ui->singleStepAction->setEnabled(false);
	m_ui->memorySizeAction->setEnabled(false);
	m_ui->stopAction->setEnabled(true);
	updateActions();

	m_prevMemory = m_virtualMachine->memory();
	m_prevRegisters = m_virtualMachine->registers();
	m_prevStartCell = m_startCellSpinBox->value();

	m_virtualMachine->assemble(m_ui->codeEdit->toPlainText());

	while(m_virtualMachine->exec()) {
		m_startCellSpinBox->setValue(m_virtualMachine->execCell());
		setStartCell(m_virtualMachine->execCell());

		qApp->processEvents();
	}

	m_ui->stopAction->setEnabled(false);
	m_ui->rewindAction->setEnabled(true);
}

void MainWindow::singleExec()
{
	if(m_ui->startAction->isEnabled()) {
		m_ui->codeEdit->setEnabled(false);
		m_startCellSpinBox->setEnabled(false);
		m_ui->startAction->setEnabled(false);
		m_ui->memorySizeAction->setEnabled(false);
		m_ui->rewindAction->setEnabled(true);
		updateActions();

		m_prevMemory = m_virtualMachine->memory();
		m_prevRegisters = m_virtualMachine->registers();
		m_prevStartCell = m_startCellSpinBox->value();

		m_virtualMachine->assemble(m_ui->codeEdit->toPlainText());
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

	m_ui->codeEdit->setEnabled(true);
	m_startCellSpinBox->setEnabled(true);
	m_ui->startAction->setEnabled(true);
	m_ui->singleStepAction->setEnabled(true);
	m_ui->memorySizeAction->setEnabled(true);
	m_ui->rewindAction->setEnabled(false);
	updateActions();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("AsmEmu"), tr("Assembler Emulator<br>Copyright &copy; 2013 Artur Pacholec"));
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
			QTreeWidgetItem *registerItem = new QTreeWidgetItem(m_ui->registersTree);
			registerItem->setText(0, m_virtualMachine->registerName(i));
			registerItem->setText(1, QString::number(m_virtualMachine->registers().at(i)));
			m_registerItems.insert(i, registerItem);

			continue;
		}

		QTreeWidgetItem *registerItem = m_registerItems.value(i);
		registerItem->setText(1, QString::number(m_virtualMachine->registers().at(i)));
	}
}

void MainWindow::updateCursorPosition()
{
	m_positionLabel->setText(tr("Line %1 Col %2").arg(m_ui->codeEdit->textCursor().block().blockNumber()).arg(m_ui->codeEdit->textCursor().positionInBlock()));
}
void MainWindow::setStartLine(const int &lineNo)
{
	const QString line = m_ui->codeEdit->document()->findBlockByLineNumber(lineNo - 1).text().replace(QRegularExpression("\\s+"), " ");
	const QStringList parts = line.split(' ', QString::SkipEmptyParts);

	if(!parts.size())
		return;

	bool isNumber = false;
	const int cellNo = parts[0].toInt(&isNumber);
	if(!isNumber)
		return;

	m_startCellSpinBox->setValue(cellNo);
	m_ui->codeEdit->setSpecialLine(lineNo);
}

void MainWindow::setStartCell(const int &cellNo)
{
	m_ui->codeEdit->setSpecialLine(-1);

	for(int i = 0; i < m_ui->codeEdit->blockCount(); ++i) {
		const QString line = m_ui->codeEdit->document()->findBlockByNumber(i).text().replace(QRegularExpression("\\s+"), " ");
		const QStringList parts = line.split(' ', QString::SkipEmptyParts);

		if(!parts.size())
			continue;

		bool isNumber = false;
		const int lineCellNo = parts[0].toInt(&isNumber);
		if(!isNumber)
			continue;

		if(lineCellNo == cellNo) {
			m_ui->codeEdit->setSpecialLine(i + 1);
			return;
		}
	}
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

	if(dlgAccepted) {
		set.setValue("lang", langs.at(fullLangs.indexOf(newLang)));

		QMessageBox::information(this, tr("Restart needed"), tr("The language change will take effect after you restart the application."));
	}
}

void MainWindow::toggleAsmHelp()
{
	m_ui->asmHelpEdit->setVisible(!m_ui->asmHelpEdit->isVisible());
}

void MainWindow::readSettings()
{
	QSettings set;
	restoreGeometry(set.value("geometry").toByteArray());
	restoreState(set.value("state").toByteArray());
	m_ui->registersTree->header()->restoreState(set.value("regCols").toByteArray());
	m_ui->splitter->restoreState(set.value("splitter").toByteArray());
	m_ui->asmHelpEdit->setVisible(set.value("asmHelp", false).toBool());
	m_virtualMachine->setMemorySize(set.value("memorySize", 100).toInt());
}

void MainWindow::writeSettings()
{
	QSettings set;
	set.setValue("geometry", saveGeometry());
	set.setValue("state", saveState());
	set.setValue("regCols", m_ui->registersTree->header()->saveState());
	set.setValue("splitter", m_ui->splitter->saveState());
	set.setValue("asmHelp", m_ui->asmHelpEdit->isVisible());
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

	setCurrentFile(fileName);
}

bool MainWindow::saveFile(const QString &fileName)
{
	QFile file(fileName);
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
	m_currentFile = fileName;

	m_ui->codeEdit->document()->setModified(false);

	updateCursorPosition();
	setWindowModified(false);
	setWindowTitle(QString("%1[*] - %2").arg(m_currentFile.isEmpty() ? tr("untitled.asm") : QFileInfo(m_currentFile).fileName()).arg(tr("AsmEmu")));
}
