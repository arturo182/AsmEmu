#include "codeedit.h"

#include "asmhighlighter.h"

#include <QTextBlock>
#include <QMimeData>
#include <QPainter>
#include <qmath.h>
#include <QDebug>

CodeEdit::Gutter::Gutter(CodeEdit *textEdit):
	QWidget(textEdit),
	m_textEdit(textEdit)
{ }

void CodeEdit::Gutter::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.fillRect(event->rect(), palette().color(QPalette::Window));
	painter.setPen(Qt::darkGray);

	const int marginRight = 3;

	QTextBlock block = m_textEdit->firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = m_textEdit->blockBoundingGeometry(block).translated(m_textEdit->contentOffset()).top();
	int bottom = top + m_textEdit->blockBoundingRect(block).height();

	while(block.isValid() && top <= event->rect().bottom()) {
		int number = blockNumber * m_textEdit->m_lineStep;
		if(!m_textEdit->m_zeroStart)
			number += m_textEdit->m_lineStep;

		if(block.isVisible() && bottom >= event->rect().top()) {
			if((m_textEdit->m_specialLine == number) && !m_textEdit->m_specialPixmap.isNull()) {
				painter.drawPixmap(0, top, m_textEdit->m_specialPixmap);
			}

			painter.drawText(0, top, width() - marginRight, m_textEdit->fontMetrics().height(), Qt::AlignRight, QString::number(number).rightJustified(m_textEdit->m_zeroPadding, '0'));
		}

		block = block.next();
		top = bottom;
		bottom = top + m_textEdit->blockBoundingRect(block).height();
		++blockNumber;
	}
}

void CodeEdit::Gutter::mousePressEvent(QMouseEvent *event)
{
	if(m_textEdit->isReadOnly())
		return;

	QTextBlock block = m_textEdit->firstVisibleBlock();
	int line = qFloor(event->pos().y() / m_textEdit->fontMetrics().height()) + block.blockNumber();
	if(!m_textEdit->m_zeroStart)
		line += 1;

	emit m_textEdit->gutterClicked(line);
}


int CodeEdit::Gutter::prefferedWidth() const
{
	int digits = 3;
	int max = qMax(1, m_textEdit->blockCount());
	while(max >= 10) {
		max /= 10;
		++digits;
	}

	digits = qMax(digits, m_textEdit->m_zeroPadding + 1);

	const int marginLeft = 3;
	const int marginRight = 3;

	return marginLeft + (m_textEdit->fontMetrics().width('9') * digits) + marginRight + ((!m_textEdit->m_specialPixmap.isNull() && m_textEdit->m_specialLine > -1) ? m_textEdit->m_specialPixmap.width() : 0);
}


QSize CodeEdit::Gutter::sizeHint() const
{
	return QSize(prefferedWidth(), 0);
}

CodeEdit::CodeEdit(QWidget *parent):
	QPlainTextEdit(parent),
	m_gutter(new Gutter(this)),
	m_lineStep(1),
	m_zeroPadding(0),
	m_zeroStart(false),
	m_undoAvailable(false),
	m_redoAvailable(false)
{
	connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEdit::updateGutterWidth);
	connect(this, &QPlainTextEdit::updateRequest, this, &CodeEdit::updateGutter);

	connect(this, &QPlainTextEdit::redoAvailable, this, &CodeEdit::setRedoAvailable);
	connect(this, &QPlainTextEdit::undoAvailable, this, &CodeEdit::setUndoAvailable);

	updateGutterWidth();
}

CodeEdit::~CodeEdit()
{
	delete m_gutter;
}

void CodeEdit::setLineStep(const int &lineStep)
{
	m_lineStep = lineStep;
}

int CodeEdit::lineStep() const
{
	return m_lineStep;
}

void CodeEdit::setZeroPadding(const int &zeroPadding)
{
	m_zeroPadding = zeroPadding;
}

int CodeEdit::zeroPadding() const
{
	return m_zeroPadding;
}

void CodeEdit::setZeroStart(bool zeroStart)
{
	m_zeroStart = zeroStart;

	m_gutter->repaint();
}

bool CodeEdit::zeroStart() const
{
	return m_zeroStart;
}

void CodeEdit::setSpecialLine(const int &specialLine)
{
	m_specialLine = specialLine;

	m_gutter->repaint();
}

int CodeEdit::specialLine() const
{
	return m_specialLine;
}

void CodeEdit::setSpecialPixmap(const QPixmap &specialPixmap)
{
	m_specialPixmap = specialPixmap;
}

QPixmap CodeEdit::specialPixmap() const
{
	return m_specialPixmap;
}

bool CodeEdit::canRedo()
{
	return m_redoAvailable;
}

bool CodeEdit::canUndo()
{
	return m_undoAvailable;
}

void CodeEdit::setRedoAvailable(bool redoAvailable)
{
	m_redoAvailable = redoAvailable;
}

void CodeEdit::setUndoAvailable(bool undoAvailable)
{
	m_undoAvailable = undoAvailable;
}

void CodeEdit::resizeEvent(QResizeEvent *event)
{
	QPlainTextEdit::resizeEvent(event);

	const QRect cr = contentsRect();
	m_gutter->setGeometry(QRect(cr.left(), cr.top(), m_gutter->prefferedWidth(), cr.height()));
}

void CodeEdit::focusInEvent(QFocusEvent *event)
{
	QPlainTextEdit::focusInEvent(event);

	emit focusChanged(true);
}

void CodeEdit::focusOutEvent(QFocusEvent *event)
{
	QPlainTextEdit::focusOutEvent(event);

	emit focusChanged(false);
}

bool CodeEdit::canInsertFromMimeData(const QMimeData *source) const
{
	return source->hasUrls() || QPlainTextEdit::canInsertFromMimeData(source);
}

void CodeEdit::insertFromMimeData(const QMimeData *source)
{
	if(source->hasUrls()) {
		const QString fileName = source->urls().first().toLocalFile();
		if(QFile::exists(fileName)) {
			emit fileDropped(fileName);
			return;
		}
	}

	QPlainTextEdit::insertFromMimeData(source);
}

void CodeEdit::updateGutterWidth()
{
	setViewportMargins(m_gutter->prefferedWidth(), 0, 0, 0);
}

void CodeEdit::updateGutter(const QRect &rect, const int &yDistance)
{
	if(yDistance) {
		m_gutter->scroll(0, yDistance);
	} else {
		m_gutter->update(0, rect.y(), m_gutter->width(), rect.height());
	}

	if(rect.contains(viewport()->rect()))
		updateGutterWidth();
}
