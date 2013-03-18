#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <QObject>
#include <QVector>
#include <QHash>

class Screen;

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
			DI,
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

			//indexes
			CPA,
			STO,
			RSI,

			//arithmetic
			ADD,
			SUB,
			MUL,

			//branching
			BRA,
			BRN,
			BRZ,
			BROF,
			BRNF,
			BRZF,

			//helpers
			INC,
			DEC,

			//stack
			POP,
			PUSH,

			//subroutines
			RET,
			CALL,

			//screen
			SCRX,
			SCRY,
			SCRF,
			SCRB,
			SCR
		};

	public:
		static int memoryToInt(const int &mem);
		static int intToMemory(const int &val);

	public:
		explicit VirtualMachine(Screen *screen, QObject *parent = 0);

		bool assemble(const QString &code, QStringList *msgs = 0);
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
		void registersChanged();
		void labelsChanged();

	private:
		int &addressFor(const int &aa, const int &xxxx);
		int valueFor(const int &aa, const int &xxxx);
		void updateFlags(const int &intVal);

	private:
		QHash<QString, int> m_labels;
		QVector<int> m_registers;
		QVector<int> m_memory;
		Screen *m_screen;
		int m_execCell;
};

#endif // VIRTUALMACHINE_H
