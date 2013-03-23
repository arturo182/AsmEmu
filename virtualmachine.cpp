#include "virtualmachine.h"

#include "compiler.h"
#include "screen.h"

#include <QRegularExpression>
#include <QProgressDialog>
#include <QApplication>
#include <QStringList>
#include <QMessageBox>
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
	int bounded = val;
	while(bounded > 999)
		bounded -= 999;

	while(bounded < -999)
		bounded += 999;

	return(bounded >= 0) ? bounded : -bounded + 1000;
}

VirtualMachine::VirtualMachine(Screen *screen, QObject *parent) :
	QObject(parent),
	m_screen(screen),
	m_execCell(0)
{
	setMemorySize(100);
	setRegisterCount(6);
}

bool VirtualMachine::assemble(const QString &code, QStringList *msgs)
{
	Compiler compiler(code);
	connect(&compiler, &Compiler::memoryChanged, [=](const int &cellNo, const int &value)
	{
		if(cellNo < 0 || cellNo >= m_memory.size())
			return;

		m_memory[cellNo] = value;
	});

	const bool result = compiler.compile(msgs);
	if(!result)
		return false;

	m_labels = compiler.labelMap();
	emit memoryChanged(m_memory);
	emit labelsChanged();

	return true;
}

/**
 * @brief VirtualMachine::exec
 * @return True if there's any more isntructions to exec.
 *
 * Execs m_execCell
 */
bool VirtualMachine::exec()
{
	if((m_execCell > m_memory.size() - 1) || (m_execCell < 0))
		return false;

	m_registers[IP] = m_execCell;
	m_registers[SB] = m_memory.size() - 1;

	if(m_registers[SP] == 0)
		m_registers[SP] = m_registers[SB];

	//OO AA XXXX
	const int value = m_memory[m_execCell];
	const int oo = value / 1000000;
	const int aa = value / 10000 - oo * 100;
	const int xxxx = value - oo * 1000000 - aa * 10000;

	bool theEnd = false;

	switch(oo) {
		case CPA:
		{
			m_registers[AX] = valueFor(aa, xxxx);

			++m_execCell;
		}
		break;

		case STO:
		{
			addressFor(aa, xxxx) = m_registers[AX];

			++m_execCell;
		}
		break;

		case RSI:
		{
			m_registers[DI] = 0;

			++m_execCell;
		}
		break;

		case ADD:
		{
			const int intVal = memoryToInt(m_registers[AX]) + memoryToInt(valueFor(aa, xxxx));
			m_registers[AX] = intToMemory(intVal);

			updateFlags(intVal);
			++m_execCell;
		}
		break;

		case SUB:
		{
			const int intVal = memoryToInt(m_registers[AX]) - memoryToInt(valueFor(aa, xxxx));
			m_registers[AX] = intToMemory(intVal);

			updateFlags(intVal);
			++m_execCell;
		}
		break;

		case MUL:
		{
			const int intVal = memoryToInt(m_registers[AX]) * memoryToInt(valueFor(aa, xxxx));
			m_registers[AX] = intToMemory(intVal);

			updateFlags(intVal);
			++m_execCell;
		}
		break;

		case BRA:
		{
			m_execCell =  (aa == 0) ? memoryToInt(xxxx) : memoryToInt(valueFor(aa, xxxx));
		}
		break;

		case BRN:
		{
			const int targetCell = (aa == 0) ? memoryToInt(xxxx) : memoryToInt(valueFor(aa, xxxx));

			m_execCell = (memoryToInt(m_registers[AX]) < 0) ? targetCell : m_execCell + 1;
		}
		break;

		case BRZ:
		{
			const int targetCell = (aa == 0) ? memoryToInt(xxxx) : memoryToInt(valueFor(aa, xxxx));

			m_execCell = (memoryToInt(m_registers[AX]) == 0) ? targetCell : m_execCell + 1;
		}
		break;

		case BROF:
		{
			const bool overflowFlag = m_registers[FLAGS] / OF % 10;

			const int targetCell = (aa == 0) ? memoryToInt(xxxx) : memoryToInt(valueFor(aa, xxxx));
			m_execCell = overflowFlag ? targetCell : m_execCell + 1;
		}
		break;

		case BRNF:
		{
			const bool negativeFlag = m_registers[FLAGS] / NF % 10;
			const int targetCell = (aa == 0) ? memoryToInt(xxxx) : memoryToInt(valueFor(aa, xxxx));

			m_execCell = negativeFlag ? targetCell : m_execCell + 1;
		}
		break;

		case BRZF:
		{
			const bool zeroFlag = m_registers[FLAGS] / ZF % 10;
			const int targetCell = (aa == 0) ? memoryToInt(xxxx) : memoryToInt(valueFor(aa, xxxx));

			m_execCell = zeroFlag ? targetCell : m_execCell + 1;
		}
		break;

		case INC:
		{
			const int intVal = memoryToInt(valueFor(aa, xxxx)) + 1;
			addressFor(aa, xxxx) = intToMemory(intVal);

			updateFlags(intVal);

			++m_execCell;
		}
		break;

		case DEC:
		{
			const int intVal = memoryToInt(valueFor(aa, xxxx)) - 1;
			addressFor(aa, xxxx) = intToMemory(intVal);

			updateFlags(intVal);

			++m_execCell;
		}
		break;

		case POP:
		{
			if(m_registers[SP] != m_registers[SB])
				m_registers[SP] = intToMemory(memoryToInt(m_registers[SP]) + 1);

			const int intVal = memoryToInt(m_registers[SP]);
			addressFor(aa, xxxx) = m_memory[intVal];

			++m_execCell;
		}
		break;

		case PUSH:
		{
			const int stackPos = memoryToInt(m_registers[SP]);
			m_memory[stackPos] = valueFor(aa, xxxx);

			m_registers[SP] = intToMemory(stackPos - 1);

			++m_execCell;
		}
		break;

		case CALL:
		{
			const int stackPos = memoryToInt(m_registers[SP]);
			m_memory[stackPos] = intToMemory(m_execCell + 1);

			m_registers[SP] = intToMemory(stackPos - 1);

			const int targetCell = (aa == 0) ? memoryToInt(xxxx) : memoryToInt(valueFor(aa, xxxx));
			m_execCell = targetCell;
		}
		break;

		case RET:
		{
			const int popCount = memoryToInt(valueFor(aa, xxxx)) + 1;

			for(int i = 0; i < popCount; ++i) {
				if(m_registers[SP] == m_registers[SB])
					break;

				m_registers[SP] = intToMemory(memoryToInt(m_registers[SP]) + 1);
			}

			const int intVal = memoryToInt(m_registers[SP]) - popCount + 1;
			m_execCell = memoryToInt(m_memory[intVal]);
		}
		break;

		case SCRX:
		{
			m_screen->setCol(valueFor(aa, xxxx));

			++m_execCell;
		}
		break;

		case SCRY:
		{
			m_screen->setRow(valueFor(aa, xxxx));

			++m_execCell;
		}
		break;

		case SCRF:
		{
			m_screen->setFore(valueFor(aa, xxxx));

			++m_execCell;
		}
		break;

		case SCRB:
		{
			m_screen->setBack(valueFor(aa, xxxx));

			++m_execCell;
		}
		break;

		case SCR:
		{
			m_screen->setChar(valueFor(aa, xxxx));

			++m_execCell;
		}
		break;

		case HLT:
		default:
			theEnd = true;
		break;
	}

	emit memoryChanged(m_memory);
	emit registersChanged();
	emit labelsChanged();

	if(m_execCell > m_memory.size() - 1)
		return false;

	return !theEnd;
}

const QList<int> &VirtualMachine::breakpoints() const
{
	return m_breakpoints;
}

void VirtualMachine::clearBreakpoints()
{
	m_breakpoints.clear();
}

bool VirtualMachine::toggleBreakpoint(const int &cell)
{
	if(m_breakpoints.contains(cell)) {
		m_breakpoints.removeOne(cell);
		return false;
	} else {
		m_breakpoints << cell;
		return true;
	}
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

	emit registersChanged();
}

QString VirtualMachine::registerName(const int &registerNo) const
{
	switch(registerNo) {
		case AX:
			return "AX";
		break;

		case SP:
			return "SP";
		break;

		case SB:
			return "SB";
		break;

		case IP:
			return "IP";
		break;

		case DI:
			return "DI";
		break;

		case FLAGS:
			return "FLAGS";
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

	emit registersChanged();
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

	emit registersChanged();
}

void VirtualMachine::resetMemory()
{
	for(int i = 0; i < m_memory.size(); ++i)
		m_memory[i] = 0;

	emit memoryChanged(m_memory);
}

void VirtualMachine::updateFlags(const int &intVal)
{
	const int zeroFlag = m_registers[FLAGS] / ZF % 10;
	const bool isZero = (intVal == 0);
	m_registers[FLAGS] += (isZero && !zeroFlag) ? ZF : ((!isZero && zeroFlag) ? -ZF : 0);

	const int negativeFlag = m_registers[FLAGS] / NF % 10;
	const bool isNegative = (intVal < 0);
	m_registers[FLAGS] += (isNegative && !negativeFlag) ? NF : ((!isNegative && negativeFlag) ? -NF : 0);

	const int overflowFlag = m_registers[FLAGS] / OF % 10;
	const bool isOverflowed = (intVal > 999 || intVal < -999);
	m_registers[FLAGS] += (isOverflowed && !overflowFlag) ? OF : ((!isOverflowed && overflowFlag) ? -OF : 0);

}

int &VirtualMachine::addressFor(const int &aa, const int &xxxx)
{
	const int xxxxInt = memoryToInt(xxxx);

	switch(aa) {
		case 0:
			return m_memory[xxxxInt];
		break;

		case 1:
			return m_memory[m_memory[xxxxInt]];
		break;

		case 3:
			return m_registers[AX];
		break;

		case 4:
			return m_registers[IP];
		break;

		case 5:
			return m_registers[SP];
		break;

		case 6:
			return m_registers[SB];
		break;

		case 7:
			return m_registers[DI];
		break;

		case 8:
			return m_memory[m_registers[AX] + memoryToInt(xxxx)];
		break;

		case 9:
			return m_memory[m_registers[IP] + memoryToInt(xxxx)];
		break;

		case 10:
			return m_memory[m_registers[SP] + memoryToInt(xxxx)];
		break;

		case 11:
			return m_memory[m_registers[SB] + memoryToInt(xxxx)];
		break;

		case 12:
			return m_memory[m_registers[DI] + memoryToInt(xxxx)];
		break;

		case 2:
			qDebug() << "No address for constant!";
		break;
	}
}

int VirtualMachine::valueFor(const int &aa, const int &xxxx)
{
	if(aa == 2) //const
		return memoryToInt(xxxx);

	return addressFor(aa, xxxx);
}
