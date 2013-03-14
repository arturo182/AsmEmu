#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <QObject>
#include <QVector>
#include <QMap>

class VirtualMachine : public QObject
{
	Q_OBJECT

	public:
		enum Register
		{
			AX = 0,
			SP,
			SB,
			IP,
			FLAGS
		};

		enum Flags
		{
			ZF = 1,
			NF = 10,
			OF = 100
		};

		enum Instruction
		{
			HLT = 0,
			INC,
			DEC,
			CALL,
			CPA,
			STO,
			ADD,
			SUB,
			BRA,
			BRN,
			MUL,
			BRZ,
			POP,
			PUSH,
			RET
		};

	public:
		static int memoryToInt(const int &mem);
		static int intToMemory(const int &val);

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

		QVector<int> labels();
		int labelCellNo(const int &labelNo) const;
		QString labelName(const int &labelNo) const;
		int labelCount() const;

		void setExecCell(const int &execCell);
		int execCell() const;

	public slots:
		void resetRegisters();
		void resetMemory();

	signals:
		void memoryChanged(const QVector<int> &memory);
		void registersChanged(const QVector<int> &registers);
		void labelsChanged();

	private:
		void updateFlags(const int &intVal);

	private:
		QMap<QString, int> m_labels;
		QVector<int> m_registers;
		QVector<int> m_memory;
		int m_execCell;
};

#endif // VIRTUALMACHINE_H
