#ifndef MEMORYVIEW_H
#define MEMORYVIEW_H

#include "codeedit.h"

#include <QVector>

class MemoryView : public CodeEdit
{
	Q_OBJECT

	public:
		explicit MemoryView(QWidget *parent = 0);

	public slots:
		void setMemory(const QVector<int> &memory);

	protected:
		void resizeEvent(QResizeEvent *event);

	private:
		QVector<int> m_memory;
		bool m_updating;
};

#endif // MEMORYVIEW_H
