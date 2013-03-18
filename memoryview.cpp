#include "memoryview.h"

#include <QScrollBar>
#include <QDebug>

MemoryView::MemoryView(QWidget *parent) :
	CodeEdit(parent),
	m_updating(false)
{
	setMemory(QVector<int>(100));
}

void MemoryView::setMemory(const QVector<int> &memory)
{
	if(m_memory == memory)
		return;

	m_memory = memory;

	resizeEvent(new QResizeEvent(QSize(), QSize()));

	updateGutterWidth();
}

void MemoryView::resizeEvent(QResizeEvent *event)
{
	if(m_updating)
		return;

	CodeEdit::resizeEvent(event);

	const int yPos = verticalScrollBar()->value();

	if(!m_memory.size())
		return;

	m_updating = true;

	const int contentWidth = viewport()->width() - m_gutter->prefferedWidth();
	int cellCount = contentWidth / fontMetrics().width("         ");

	if(cellCount * fontMetrics().width("         ") + fontMetrics().width("       ") <= contentWidth)
		cellCount += 1;

	setLineStep(cellCount + 1);

	QString content;
	int j = 0;
	for(int i = 0; i < m_memory.size(); ++i) {
		content += QString::number(m_memory[i]).rightJustified(8, ' ');

		++j;

		if(j > cellCount) {
			content += "\n";
			j = 0;
		} else {
			content += " ";
		}
	}

	if(content.endsWith("\n"))
		content.chop(1);

	if(toPlainText() != content) {
		clear();
		setPlainText(content);
	}

	verticalScrollBar()->setValue(yPos);

	m_updating = false;
}
