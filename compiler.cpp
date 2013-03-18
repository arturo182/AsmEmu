#include "compiler.h"

#include "virtualmachine.h"
#include "evaluator.h"

#include <QRegularExpression>
#include <QDebug>

struct InstructionDefine
{
	int code;
	int operands;
};

const QMap<QString, InstructionDefine> mnemonicMap(
		std::map<QString, InstructionDefine>({
			{ "HLT",  { VirtualMachine::HLT,  0 } },

			{ "CPA",  { VirtualMachine::CPA,  1 } },
			{ "STO",  { VirtualMachine::STO,  1 } },
			{ "RSI",  { VirtualMachine::RSI,  0 } },

			{ "ADD",  { VirtualMachine::ADD,  1 } },
			{ "SUB",  { VirtualMachine::SUB,  1 } },
			{ "MUL",  { VirtualMachine::MUL,  1 } },

			{ "BRA",  { VirtualMachine::BRA,  1 } },
			{ "BRN",  { VirtualMachine::BRN,  1 } },
			{ "BRZ",  { VirtualMachine::BRZ,  1 } },
			{ "BROF", { VirtualMachine::BRA,  1 } },
			{ "BRNF", { VirtualMachine::BRN,  1 } },
			{ "BRZF", { VirtualMachine::BRZ,  1 } },

			{ "INC",  { VirtualMachine::INC,  1 } },
			{ "DEC",  { VirtualMachine::DEC,  1 } },

			{ "POP",  { VirtualMachine::POP,  0 } },
			{ "PUSH", { VirtualMachine::PUSH, 0 } },

			{ "CALL", { VirtualMachine::CALL, 1 } },
			{ "RET",  { VirtualMachine::RET,  0 } },

			//screen (unofficial)
			{ "SCRX", { VirtualMachine::SCRX, 1 } },
			{ "SCRY", { VirtualMachine::SCRY, 1 } },
			{ "SCRF", { VirtualMachine::SCRF, 1 } },
			{ "SCRB", { VirtualMachine::SCRB, 1 } },
			{ "SCR",  { VirtualMachine::SCR,  1 } },

			// pseudo instruction for direct memory assignment
			{ "DAT",  { -1,					  0 } }
		})
);

const QStringList registerList = { "AX", "IP", "SP", "SB", "DI" };

const QRegularExpression labelPattern("^[a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ_][_a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ0-9]*$", QRegularExpression::CaseInsensitiveOption);

bool isWhitespace(const QChar &c)
{
	static QList<QChar> chars = { '\n', '\r', '\t', ' ', ',' };

	return chars.contains(c);
}

Compiler::Compiler(const QString &code, QObject *parent) :
	QObject(parent),
	m_startCell(-1),
	m_code(code)
{

}


QHash<QString, int> Compiler::labelMap() const
{
	QHash<QString, int> fixed;
	foreach(const QString &name, m_labelMap.keys())
		fixed.insert(name, m_addressMap.key(m_labelMap[name]));

	return fixed;
}

int Compiler::startCell() const
{
	return m_startCell;
}

int Compiler::cellToLine(const int &cellNo)
{
	return m_instructionMap[m_addressMap[cellNo]];
}

int Compiler::lineToCell(const int &lineNo)
{
	return m_addressMap.key(m_instructionMap.indexOf(lineNo));
}

QString Compiler::code() const
{
	return m_code;
}

void Compiler::setCode(const QString &code)
{
	m_startCell = -1;
	m_code = code;
}

bool Compiler::compile(QStringList *msgs)
{
	if(!precompile(msgs))
		return false;

	QList<int> words;
	QList<LabelItem> labelQueue;
	int address = 0;

	auto packValue = [&](const int &value)
	{
		if(words.isEmpty())
			return;

		words.last() += value;
	};

	auto packOperand = [&](const int &operand)
	{
		if(words.isEmpty())
			return;

		words.last() += operand * 10000;
	};

	auto parseOperand = [&](QString operand, const int &n, const int &line)
	{
		operand.replace('\t', "").replace('\n', "");

		bool isPointer = false;
		if((operand[0] == '[') && (operand[operand.length() - 1] == ']')) {
			isPointer = true;
			operand = operand.mid(1, operand.length() - 2);
		}

		bool isConst = false;
		if(!isPointer && operand.startsWith('$')) {
			isConst = true;
			operand = operand.mid(1);
		}

		bool isNumber = false;
		operand.toInt(&isNumber);

		if((operand[0] == '"') && (operand[operand.length() - 1] == '"')) {
			//string literal
			operand = operand.mid(1, operand.length() - 2);

			words.clear();

			for(int i = 0; i < operand.length(); ++i) {
				char c = operand[i].toLatin1();

				if(c == '\\') {
					switch(operand[i + 1].toLatin1()) {
						case 'n':  c = 10; break;
						case 'r':  c = 13; break;
						case 'a':  c = 7;  break;
						case '\\': c = 92; break;
						case '"':  c = 34; break;
						case '0':  c = 0;  break;

						default:
						{
							if(msgs) *msgs << tr("%1:Unrecognised string escape \\%2").arg(line).arg(operand[i+1]);

							return false;
						}
					}

					++i;
				}

				words << c;
			}
		} else if(isPointer && (operand.split('+').size() == 2)) {
			//register + offset
			const QString errorStr = tr("%1:Invalid offset pointer, must be one literal/label and one register.");

			const QStringList split = operand.replace(' ', "").split('+');
			QString reg;
			QString offset;

			for(int i = 0; i < 2; ++i) {
				bool isOffset = false;
				int number = split[i].toInt(&isOffset);

				if(isOffset) {
					if(offset.isEmpty()) {
						offset = QString::number(number);
					} else {
						if(msgs) *msgs << errorStr.arg(line);

						return false;
					}

					//range check

					packValue(number);
				} else if(registerList.contains(split[i].toUpper())) {
					if(reg.isEmpty()) {
						reg = split[i].toUpper();
					} else {
						if(msgs) *msgs << errorStr.arg(line);

						return false;
					}
				} else if(offset.isEmpty() && labelPattern.match(split[i]).hasMatch()) {
					LabelItem label;
					label.name = split[i];
					label.address = address;
					labelQueue << label;
				} else {
					if(msgs) *msgs << errorStr.arg(line);

					return false;
				}
			}

			if(reg == "AX") {
				packOperand(8);
			} else if(reg == "IP") {
				packOperand(9);
			} else if(reg == "SP") {
				packOperand(10);
			} else if(reg == "SB") {
				packOperand(11);
			} else if(reg == "DI") {
				packOperand(12);
			}
		} else if(isNumber) {
			const int value = operand.toInt();

			//range check

			if(n > 0)
				words << 0;

			if(isPointer) {
				packOperand(1);
			} else if(isConst) {
				packOperand(2);
			} else {
				packOperand(0);
			}

			packValue(value);
		} else {
			if(registerList.contains(operand.toUpper())) {
				if(operand.toUpper() == "AX") {
					packOperand(isPointer ? 8 : 3);
				} else if(operand.toUpper() == "IP") {
					packOperand(isPointer ? 9 : 4);
				} else if(operand.toUpper() == "SP") {
					packOperand(isPointer ? 10 : 5);
				} else if(operand.toUpper() == "SB") {
					packOperand(isPointer ? 11 : 6);
				} else if(operand.toUpper() == "DI") {
					packOperand(isPointer ? 12 : 7);
				}
			} else {
				if(isPointer) {
					packOperand(1);
				} else if(isConst) {
					packOperand(2);
				} else {
					packOperand(0);
				}

				if(labelPattern.match(operand).hasMatch()) {
					LabelItem label;
					label.name = operand;
					label.address = address;
					labelQueue << label;
				} else {
					if(msgs) *msgs << tr("%1:Illegal symbol in label \"%2\"").arg(line).arg(operand);

					return false;
				}
			}
		}

		return true;
	};

	QHash<int, int> memory;

	for(int i = 0; i < m_instructions.size(); ++i) {
		const QString mnemonic = m_instructions[i].mnemonic.toUpper();
		const QStringList operands = m_instructions[i].operands;

		if((mnemonic == ".CODE") || (mnemonic == ".DATA")) {
			if(operands.size()) {
				address = operands[0].toInt();

				if((mnemonic == ".CODE") && (m_startCell == -1)) {
					m_startCell = address;
				}
			}

			//range check
		} else {
			if(mnemonicMap.value(mnemonic).operands > operands.size()) {
				if(msgs) *msgs << tr("%1:Invalid operand count for \"%2\" (expecting %3)").arg(m_instructionMap[i])
																						  .arg(mnemonic)
																						  .arg(mnemonicMap.value(mnemonic).operands);

				return false;
			}

			if(mnemonicMap.value(mnemonic.toUpper()).code > -1) {
				words = { mnemonicMap.value(mnemonic.toUpper()).code * 1000000 };
			} else {
				words = { 0 };
			}

			for(int j = 0; j < operands.size(); ++j) {
				if(!parseOperand(operands[j], j, m_instructionMap[i]))
					return false;
			}

			const int prevAddress = address;
			for(int j = 0; j < words.size(); ++j)
				memory.insert(address++, words[j]);

			for(int j = prevAddress; j <= address; ++j) {
				if(!m_addressMap.values().contains(i))
					m_addressMap.insert(j, i);
			}
		}
	}

	for(int i = 0; i < labelQueue.size(); ++i) {
		if(!m_labelMap.contains(labelQueue[i].name)) {
			if(msgs) *msgs << tr("%1:Undefined label \"%2\"").arg(-1).arg(labelQueue[i].name);

			return false;
		}

		int value = address;
		const int key = m_addressMap.key(m_labelMap.value(labelQueue[i].name), -1);
		if(key > -1)
			value = key;

		memory[labelQueue[i].address] += value;
	}

	QHashIterator<int, int> it(memory);
	while(it.hasNext()) {
		it.next();

		emit memoryChanged(it.key(), it.value());
	}

	return true;
}

bool Compiler::precompile(QStringList *msgs)
{
	m_instructionMap.clear();
	m_instructions.clear();
	m_addressMap.clear();
	m_labelMap.clear();

	QStringList operands;
	QString operand;
	bool inString = false;
	bool inPointer = false;
	bool inEscape = false;

	auto finishOperand = [&](const int &line)
	{
		if(operand.isEmpty())
			return;

		if(operand.endsWith(':')) {
			const QString label = operand.mid(0, operand.length() - 1);
			if(!labelPattern.match(label).hasMatch()) {
				if(msgs) *msgs << tr("%1:Invalid label name \"%2\"").arg(line).arg(label);

				return false;
			}

			if(m_labelMap.contains(label)) {
				if(msgs) *msgs << tr("%1:Label already defined \"%2\"").arg(line).arg(label);

				return false;
			}

			m_labelMap.insert(label, m_instructions.size());
		} else {
			operands << operand;
		}

		operand.clear();

		return true;
	};

	QStringList lines = m_code.split('\n', QString::KeepEmptyParts);
	for(int i = 0; i < lines.count(); ++i) {
		QString line = lines[i];

		operands.clear();

		inString = false;
		inPointer = false;
		inEscape = false;

		for(int j = 0; j < line.size(); ++j) {
			QChar c = line[j];

			if(inString) {
				operand += c;

				if(!inEscape) {
					if(c == '"') {
						inString = false;
					} else if(c == '\\') {
						inEscape = true;
					}
				} else {
					inEscape = false;
				}
			} else if(inPointer) {
				operand += c;

				if(c == ']')
					inPointer = false;
			} else if(isWhitespace(c)) {
				if(!finishOperand(i + 1))
					return false;
			} else if(c == ';') {
				//comment, we are done here
				break;
			} else {
				operand += c;

				inString  = (c == '"');
				inPointer = (c == '[');
			}
		}

		if(!finishOperand(i + 1))
			return false;

		if(inString) {
			if(msgs) *msgs << tr("%1:Unterminated string").arg(i + 1);
			return false;
		}

		if(inPointer) {
			if(msgs) *msgs << tr("%1:Unclosed pointer brackets").arg(i + 1);
			return false;
		}

		if(operands.isEmpty())
			continue;

		QString firstItem = operands.takeFirst();
		QString mnemonic = firstItem.toUpper();
		if(!mnemonicMap.contains(mnemonic) && (mnemonic != ".CODE") && (mnemonic != ".DATA")) {
			bool isNumber = false;
			mnemonic.toInt(&isNumber);

			if(isNumber && operands.size()) {
				//10	1
				//10	CPA $1

				Instruction instr;
				instr.mnemonic = ".data";
				instr.operands = { mnemonic };
				m_instructions << instr;

				m_instructionMap << i + 1;

				bool isNumber = false;
				operands[0].toInt(&isNumber);
				if(!isNumber) {
					//10	CPA $1
					mnemonic = operands.takeFirst().toUpper();

					if(!mnemonicMap.contains(mnemonic)) {
						if(msgs) *msgs << tr("%1:Invalid mnemonic \"%2\"").arg(i + 1).arg(mnemonic);
						return false;
					}
				}
			} else if(isNumber || labelPattern.match(mnemonic).hasMatch()) {
				//10
				//label:	5
				//label:	label2

				operands.push_front(firstItem);
				mnemonic = "DAT";
			} else {
				if(msgs) *msgs << tr("%1:Invalid mnemonic \"%2\"").arg(i + 1).arg(mnemonic);
				return false;
			}
		}

		//operandless stack instructions operate on AX
		if((mnemonic == "POP") || (mnemonic == "PUSH"))
			if(!operands.size())
				operands << "ax";

		Instruction instr;
		instr.mnemonic = mnemonic;
		instr.operands = operands;
		m_instructions << instr;

		m_instructionMap << i + 1;
	}

	return true;
}

