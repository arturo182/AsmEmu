#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <QObject>
#include <QVector>

class VirtualMachine : public QObject
{
	Q_OBJECT

	public:
		enum Registers
		{
			ACU = 0
		};

	public:
		explicit VirtualMachine(QObject *parent = 0);

		void assemble(const QString &code);
		bool exec();

		const QVector<int> &memory() const;
		void setMemory(const QVector<int> &memory);

		void setMemorySize(const int &memorySize);
		int memorySize() const;

		const QVector<int> &registers();
		void setRegisters(const QVector<int> &registers);

		QString registerName(const int &registerNo) const;
		void setRegisterCount(const int &registerCount);
		int registerCount() const;

		void setExecCell(const int &execCell);
		int execCell() const;

	public slots:
		void resetRegisters();
		void resetMemory();

	signals:
		void memoryChanged(const QVector<int> &memory);
		void registersChanged(const QVector<int> &registers);

	private:
		QVector<int> m_registers;
		QVector<int> m_memory;
		int m_execCell;
};

#endif // VIRTUALMACHINE_H
