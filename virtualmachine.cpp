#include "virtualmachine.h"

#include "compiler.h"

#include <QRegularExpression>
#include <QProgressDialog>
#include <QApplication>
#include <QStringList>
#include <QRegExp>
#include <QDebug>
#include <QMap>

int VirtualMachine::memoryToInt(const int &mem)
{
	const int sign = (mem / 1000);
	const int value = mem - sign * 1000;

	return (sign == 0) ? value : -value;
}

int VirtualMachine::intToMemory(const int &val)
{
	const int bounded = qBound(-999, val, 999);

	return(bounded >= 0) ? bounded : -bounded + 1000;
}

VirtualMachine::VirtualMachine(QObject *parent) :
	QObject(parent),
	m_execCell(0)
{
	setMemorySize(100);
	setRegisterCount(1);
}

void VirtualMachine::assemble(const QString &code)
{
	Compiler compiler(code);
	compiler.compile();

	connect(&compiler, &Compiler::memoryChanged, [=](const int &cellNo, const int &value)
	{
		m_memory[cellNo] = value;
	});

	compiler.compile();

	m_labels = compiler.labelMap();
	emit labelsChanged();
}

/**
 * @brief VirtualMachine::exec
 * @return True if there's any more isntructions to exec.
 *
 * Execs m_execCell
 */
bool VirtualMachine::exec()
{
	if((m_execCell > m_memory.size()) || (m_execCell < 0))
		return false;

	const int kilo = m_memory[m_execCell] / 1000;
	const int hecto = m_memory[m_execCell] / 100 % 10;
	const int deca = m_memory[m_execCell] / 10 % 10;

	const int hectoValue = m_memory[m_execCell] - kilo * 1000;
	const int decaValue = hectoValue - hecto * 100;
	const int oneValue = decaValue - 10 * deca;

	int value = -1;

	Instruction instr = HLT;
	bool isConst = false;

	if(kilo == 0) {
		if(hecto == 0) {
			instr = HLT;
		} else if(hecto == 1) {
			instr = INC;
			value = decaValue;
		} else if(hecto == 2) {
			instr = DEC;
			value = decaValue;
		}
	} else if(kilo == 1) {
		instr = CPA;
		value = hectoValue;
	} else if(kilo == 2) {
		instr = STO;
		value = hectoValue;
	} else if(kilo == 3) {
		instr = ADD;
		value = hectoValue;
	} else if(kilo == 4) {
		instr = SUB;
		value = hectoValue;
	} else if(kilo == 5) {
		instr = BRA;
		value = hectoValue;
	} else if(kilo == 6) {
		instr = BRN;
		value = hectoValue;
	} else if(kilo == 7) {
		instr = MUL;
		value = hectoValue;
	} else if(kilo == 8) {
		instr = BRZ;
		value = hectoValue;
	} else if(kilo == 9) {
		static QList<Instruction> instrList = { INC, CPA, STO, ADD, SUB, BRA, BRN, MUL, BRZ, DEC };
		instr = instrList[deca];

		if(hecto == 0) {
			++m_execCell;
			value = m_memory[m_execCell];
		} else if(hecto == 1) {
			isConst = true;
			value = oneValue;
		} else if(hecto == 2) {
			isConst = true;
			++m_execCell;
			value = m_memory[m_execCell];
		} else if(hecto == 3) {
			value = m_memory[oneValue];
		} else if(hecto == 4) {
			++m_execCell;
			value = memoryToInt(m_memory[m_execCell]);
			value = m_memory[value];
		}
	}

	switch(instr) {
		case CPA:
		{
			m_registers[ACU] = isConst ? value : m_memory[value];
			++m_execCell;
		}
		break;

		case STO:
		{
			m_memory[value] = m_registers[ACU];
			++m_execCell;
		}
		break;

		case ADD:
		{
			const int result = memoryToInt(m_registers[ACU]) + memoryToInt(isConst ? value : m_memory[value]);
			m_registers[ACU] = intToMemory(result);
			++m_execCell;
		}
		break;

		case SUB:
		{
			const int result = memoryToInt(m_registers[ACU]) - memoryToInt(isConst ? value : m_memory[value]);
			m_registers[ACU] = intToMemory(result);
			++m_execCell;
		}
		break;

		case BRA:
		{
			m_execCell = value;
		}
		break;

		case BRN:
		{
			if(memoryToInt(m_registers[ACU]) < 0) {
				m_execCell = value;
			} else {
				++m_execCell;
			}
		}
		break;

		case MUL:
		{
			const int result = memoryToInt(m_registers[ACU]) * memoryToInt(isConst ? value : m_memory[value]);
			m_registers[ACU] = intToMemory(result);
			++m_execCell;
		}
		break;

		case BRZ:
		{
			if(memoryToInt(m_registers[ACU]) == 0) {
				m_execCell = value;
			} else {
				++m_execCell;
			}
		}
		break;

		case INC:
		{
			if(value > -1)
				m_memory[value] = intToMemory(memoryToInt(m_memory[value]) + 1);

			++m_execCell;
		}
		break;

		case DEC:
		{
			if(value > -1)
				m_memory[value] = intToMemory(memoryToInt(m_memory[value]) - 1);

			++m_execCell;
		}
		break;

		case HLT: //HLT
		default:
			return false;
	}

	emit memoryChanged(m_memory);
	emit registersChanged(m_registers);
	emit labelsChanged();

	if(m_execCell > m_memory.size())
		return false;

	return true;
}

const QVector<int> &VirtualMachine::memory() const
{
	return m_memory;
}

void VirtualMachine::setMemory(const QVector<int> &memory)
{
	if(m_memory == memory)
		return;

	m_memory = memory;

	emit memoryChanged(m_memory);
}

void VirtualMachine::setMemorySize(const int &memorySize)
{
	if(m_memory.size() == memorySize)
		return;

	m_memory.resize(memorySize);

	emit memoryChanged(m_memory);
}

int VirtualMachine::memorySize() const
{
	return m_memory.size();
}

const QVector<int> &VirtualMachine::registers()
{
	return m_registers;
}

void VirtualMachine::setRegisters(const QVector<int> &registers)
{
	if(m_registers == registers)
		return;

	m_registers = registers;

	emit registersChanged(m_registers);
}

QString VirtualMachine::registerName(const int &registerNo) const
{
	switch(registerNo) {
		case 0:
		return "ACU";
		break;

		default:
		return tr("Unknown");
	}
}

void VirtualMachine::setRegisterCount(const int &registerCount)
{
	if(m_registers.size() == registerCount)
		return;

	m_registers.resize(registerCount);

	emit registersChanged(m_registers);
}

int VirtualMachine::registerCount() const
{
	return m_registers.size();
}

QVector<int> VirtualMachine::labels()
{
	QVector<int> labels;

	foreach(const int &cellNo, m_labels.values())
		labels << m_memory[cellNo];

	return labels;
}

int VirtualMachine::labelCellNo(const int &labelNo) const
{
	return m_labels.values().at(labelNo);
}

QString VirtualMachine::labelName(const int &labelNo) const
{
	return m_labels.keys().at(labelNo);
}

int VirtualMachine::labelCount() const
{
	return m_labels.count();
}

void VirtualMachine::setExecCell(const int &execCell)
{
	m_execCell = execCell;
}

int VirtualMachine::execCell() const
{
	return m_execCell;
}

void VirtualMachine::resetRegisters()
{
	for(int i = 0; i < m_registers.size(); ++i)
		m_registers[i] = 0;

	emit registersChanged(m_registers);
}

void VirtualMachine::resetMemory()
{
	for(int i = 0; i < m_memory.size(); ++i)
		m_memory[i] = 0;

	emit memoryChanged(m_memory);
}
