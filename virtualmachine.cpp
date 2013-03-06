#include "virtualmachine.h"

#include <QRegularExpression>
#include <QProgressDialog>
#include <QApplication>
#include <QStringList>
#include <QRegExp>
#include <QDebug>
#include <QMap>

const QMap<QString, int> mnemonicValues(std::map<QString, int>({{ "CPA", 1 }, { "STO", 2 }, { "ADD", 3 }, { "SUB", 4 }, { "BRA", 5 }, { "BRN", 6 }, { "MUL", 7 }, { "BRZ", 8 }}));

VirtualMachine::VirtualMachine(QObject *parent) :
	QObject(parent),
	m_execCell(0)
{
	setMemorySize(100);
	setRegisterCount(1);
}

/**
 * @brief VirtualMachine::assemble
 * @param code Assembler code
 *
 * Transforms asm code to memory cell values.
 */
void VirtualMachine::assemble(const QString &code)
{
	m_lineMap.clear();

	QStringList lines = code.split('\n', QString::SkipEmptyParts);

	for(int i = 0; i < lines.size(); ++i) {
		//strip comments
		lines[i].remove(QRegularExpression(";(.*)$"));

		//trim whitespace
		lines[i].replace(QRegularExpression("\\s+"), " ");
		lines[i] = lines[i].trimmed();

		//uppercase
		lines[i] = lines[i].toUpper();
	}

	for(int i = 0; i < lines.size(); ++i) {
		const QString line = lines[i];
		const QStringList parts = line.split(' ');
		if(!parts.size())
			continue;

		const int cellNumber = parts[0].toInt();
		if(cellNumber > m_memory.size())
			continue;

		m_lineMap.insert(i, cellNumber);

		if(parts.size() == 3) {
			const QString mnemonic = parts[1];
			const int value = parts[2].toInt();

			m_memory[cellNumber] = 1000 * mnemonicValues[mnemonic] + value;
		} else if(parts.size() == 2) {
			bool isNumber = false;
			const int value = parts[1].toInt(&isNumber);

			if(isNumber) {
				m_memory[cellNumber] = value;
			} else {
				if(parts[1] == "HLT") {
					m_memory[cellNumber] = 0;
				}
			}
		}
	}

	emit memoryChanged(m_memory);
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

	const int instr = m_memory[m_execCell] / 1000;
	const int value = m_memory[m_execCell] - instr * 1000;

	switch(instr) {
		case 1: //CPA
		{
			m_registers[ACU] = m_memory[value];
			++m_execCell;
		}
		break;

		case 2: //STO
		{
			m_memory[value] = m_registers[ACU];
			++m_execCell;
		}
		break;

		case 3: // ADD
		{
			m_registers[ACU] = qBound(0, m_registers[ACU] + m_memory[value], 9999);
			++m_execCell;
		}
		break;

		case 4: //SUB
		{
			m_registers[ACU] = qBound(0, m_registers[ACU] - m_memory[value], 9999);
			++m_execCell;
		}
		break;

		case 5: //BRA
		{
			m_execCell = value;
		}
		break;

		case 6: //BRN
		{
			if(m_registers[ACU] < 0) {
				m_execCell = value;
			} else {
				++m_execCell;
			}
		}
		break;

		case 7: //MUL
		{
			m_registers[ACU] = qBound(0, m_registers[ACU] * m_memory[value], 9999);
			++m_execCell;
		}
		break;

		case 8: //BRZ
		{
			if(m_registers[ACU] == 0) {
				m_execCell = value;
			} else {
				++m_execCell;
			}
		}
		break;

		case 0: //HLT
		default:
			return false;
	}

	emit memoryChanged(m_memory);
	emit registersChanged(m_registers);

	if(m_execCell > m_memory.size())
		return false;

	return true;
}

int VirtualMachine::cellToLine(const int &cell) const
{
	return m_lineMap.key(cell);
}

int VirtualMachine::lineToCell(const int &line) const
{
	return m_lineMap.value(line);
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
