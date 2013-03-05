#include "memoryview.h"

#include <QDebug>

MemoryView::MemoryView(QWidget *parent) :
	CodeEdit(parent),
	m_updating(false)
{
}

void MemoryView::setMemory(const QVector<int> &memory)
{
	m_memory = memory;

	resizeEvent(new QResizeEvent(QSize(), QSize()));

	updateGutterWidth();
}

void MemoryView::resizeEvent(QResizeEvent *event)
{
	if(m_updating)
		return;

	CodeEdit::resizeEvent(event);

	if(!m_memory.size())
		return;

	m_updating = true;

	const int contentWidth = viewport()->width() - m_gutter->prefferedWidth();
	int cellCount = contentWidth / fontMetrics().width("0000 ");

	if(cellCount * fontMetrics().width("0000 ") + fontMetrics().width("0000") <= contentWidth)
		cellCount += 1;

	setLineStep(cellCount + 1);

	QString content;
	int j = 0;
	for(int i = 0; i < m_memory.size(); ++i) {
		content += QString::number(m_memory[i]).rightJustified(4, '0');

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

	m_updating = false;
}
