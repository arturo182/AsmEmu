#include "compiler.h"

#include "virtualmachine.h"
#include "evaluator.h"

#include <QRegularExpression>
#include <QDebug>

const QMap<QString, VirtualMachine::Instruction> mnemonicMap(std::map<QString, VirtualMachine::Instruction>({
																												{"HLT", VirtualMachine::HLT},
																												{"INC", VirtualMachine::INC},
																												{"DEC", VirtualMachine::DEC},
																												{"CPA", VirtualMachine::CPA},
																												{"STO", VirtualMachine::STO},
																												{"ADD", VirtualMachine::ADD},
																												{"SUB", VirtualMachine::SUB},
																												{"BRA", VirtualMachine::BRA},
																												{"BRN", VirtualMachine::BRN},
																												{"MUL", VirtualMachine::MUL},
																												{"BRZ", VirtualMachine::BRZ},
																												{"POP", VirtualMachine::POP},
																												{"PUSH", VirtualMachine::PUSH},
																												{"CALL", VirtualMachine::CALL},
																												{"RET", VirtualMachine::RET}
																											}));


bool isValidLabel(const QString &label)
{
	const QRegularExpression labelPattern("^[a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ_][_a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ0-9]*$", QRegularExpression::CaseInsensitiveOption);
	const QRegularExpressionMatch match = labelPattern.match(label);

	return match.hasMatch();
}

bool isKeyword(const QString &label)
{
	static QStringList keywords = {
		"AX", "SP", "SB", "IP", "FLAGS",			//registers
		"HLT", "INC", "DEC", "CPA", "STO", "ADD",	//mnemonics
		"SUB", "BRA", "BRN", "MUL", "BRZ", "PUSH",
		"POP", "CALL", "RET"
	};

	return keywords.contains(label.toUpper());
}

Compiler::Compiler(const QString &code)
{
	m_lines = code.split('\n', QString::KeepEmptyParts);

	const QRegularExpression labelPattern("^\\s?([a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ_][_a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ0-9]*:)\\s?$", QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression pointerPattern("\\[.*\\]");

	for(int i = 0; i < m_lines.size(); ++i) {
		//strip comments
		m_lines[i].replace(QRegularExpression(";(.*)$"), "");

		//trim whitespace
		m_lines[i].replace(QRegularExpression("\\s+"), " ");
		m_lines[i] = m_lines[i].trimmed();

		//remove whitespace in pointer expressions
		QRegularExpressionMatch pointerMatch = pointerPattern.match(m_lines[i]);
		if(pointerMatch.hasMatch()) {
			const QString cleared = pointerMatch.captured().remove(QRegularExpression("\\s+"));
			m_lines[i].replace(pointerMatch.captured(), cleared);
		}

		//move single labels to next lines
		QRegularExpressionMatch labelMatch = labelPattern.match(m_lines[i]);
		if(labelMatch.hasMatch() && i < m_lines.size() - 1)
			m_lines[i + 1].prepend(labelMatch.captured(1) + " ");
	}
}

const QMap<int, int> &Compiler::lineMap() const
{
	return m_lineMap;
}

const QMap<QString, int> &Compiler::labelMap() const
{
	return m_labelMap;
}

int Compiler::startCell() const
{
	return m_startCell;
}

int Compiler::assembleInstruction(const int &cellNo, const QString &mnemonic, const QString &strValue, QString *error)
{
	if(!mnemonicMap.contains(mnemonic.toUpper())) {
		if(error) *error = tr("Invalid mnemonic \"%1\"").arg(mnemonic.toUpper());
		return -1;
	}

	const VirtualMachine::Instruction instr = mnemonicMap.value(mnemonic.toUpper());
	const bool isConst = strValue.contains('$');
	const bool isPointer = strValue.contains('[');

	if((isPointer && !strValue.contains(']')) || (!isPointer && strValue.contains(']'))) {
		if(error) *error = tr("Pointer symbol missing");
		return -1;
	}

	int value = QString(strValue).remove(QRegularExpression("\\$|\\[|\\]")).toInt();
	if(isPointer)
		value = evalExpression(QString(strValue).remove(QRegularExpression("\\[|\\]")));

	switch(instr) {
		case VirtualMachine::HLT:
		case VirtualMachine::RET:
		{
			static QMap<VirtualMachine::Instruction, int> varMap(std::map<VirtualMachine::Instruction, int>(
			{
				{ VirtualMachine::HLT,    0 },
				{ VirtualMachine::RET,  600 }
			}));

			emit memoryChanged(cellNo, varMap.value(instr));
			return 1;
		}
		break;

		case VirtualMachine::POP:
		case VirtualMachine::PUSH:
		{
			static QMap<VirtualMachine::Instruction, QList<int> > varMap(std::map<VirtualMachine::Instruction, QList<int> >(
			{
				{ VirtualMachine::POP,  {9610, 9620, 300, 9630, 9640, 9650, 9660} },
				{ VirtualMachine::PUSH, {9740, 9720, 400, 9730, 9740, 9750, 9760} },
			}));

			const QList<int> varList = varMap.value(instr);

			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, varList[0] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[1]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(strValue.isEmpty()) {
				emit memoryChanged(cellNo, varList[2]);
				return 1;
			} else if(isPointer) {
				if(value < 10) {
					emit memoryChanged(cellNo, varList[3] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[4]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 10) {
					emit memoryChanged(cellNo, varList[5] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[6]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::INC:
		case VirtualMachine::DEC:
		case VirtualMachine::CALL:
		{
			static QMap<VirtualMachine::Instruction, QList<int> > varMap(std::map<VirtualMachine::Instruction, QList<int> >(
			{
				{ VirtualMachine::INC,  {9300, 9400, 100, 9000} },
				{ VirtualMachine::DEC,  {9390, 9490, 200, 9090} },
				{ VirtualMachine::CALL, {9500, 9510, 500, 9520} }
			}));

			const QList<int> varList = varMap.value(instr);

			if(isConst) {
				if(error) *error = tr("\"%1\" doesn't work on const values").arg(mnemonic.toUpper());
				return -1;
			} else if(isPointer) {
				if(value < 10) {
					emit memoryChanged(cellNo, varList[0] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[1]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 100) {
					emit memoryChanged(cellNo, varList[2] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[3]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::CPA:
		case VirtualMachine::STO:
		case VirtualMachine::ADD:
		case VirtualMachine::SUB:
		case VirtualMachine::BRA:
		case VirtualMachine::BRN:
		case VirtualMachine::MUL:
		case VirtualMachine::BRZ:
		{
			const QList<int> varList = {
			//	CPA   STO   ADD   SUB   BRA   BRN   MUL   BRZ
				9110, 9120, 9130, 9140, 9150, 9160, 9170, 9180, // const < 10
				9210, 9220, 9210, 9240, 9250, 9260, 9270, 9280, // const >= 10
				9310, 9320, 9330, 9340, 9350, 9360, 9370, 9380, // pointer < 10
				9410, 9420, 9430, 9440, 9450, 9460, 9470, 9480, // pointer >= 10
				1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, // value < 1000
				9010, 9020, 9030, 9040, 9050, 9060, 9070, 9080  // value >= 1000
			};

			const int diff = instr - VirtualMachine::CPA;

			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, varList[0 + diff] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[8 + diff]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isPointer) {
				if(value < 10) {
					emit memoryChanged(cellNo, varList[16 + diff] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[24 + diff]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, varList[32 + diff] + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, varList[40 + diff]);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;
	}

	return 0;
}

int Compiler::evalExpression(const QString &expr)
{
	QString result = expr;
	QMapIterator<QString, int> it(m_labelMap);
	while(it.hasNext()) {
		it.next();
		result.replace(QRegularExpression("\\b" + it.key() + "\\b"), QString::number(it.value()));
	}

	result.remove('$');

	Evaluator evaluator;
	return evaluator.eval(result);
}

bool Compiler::compile(QStringList *msgs)
{
	int dataStart = 0;
	int codeStart = 0;

	for(int i = 0; i < m_lines.size(); ++i) {
		const QString line = m_lines[i];
		const QStringList parts = line.split(' ');
		if(!parts.size())
			continue;

		if(parts.size() == 3) {
			/* 3 parts so it's one of:
			 * - label mnemonic value
			 * - cell mnemonic value
			 */

			int cellNumber = parts[0].toInt();

			//labeled instruction
			if(parts[0].contains(':')) {
				const QString label = parts[0].left(parts[0].size() - 1);

				if(!isValidLabel(label)) {
					if(msgs) msgs->append(tr("%1:Invalid label name \"%2\"").arg(i + 1).arg(label));
					return false;
				}

				if(isKeyword(label)) {
					if(msgs) msgs->append(tr("%1:Reserved word used \"%2\"").arg(i + 1).arg(label));
					return false;
				}

				m_labelMap.insert(label, codeStart);
				cellNumber = codeStart;
			}

			//labeled value
			QString realVal = parts[2];
			if(m_labelMap.contains(parts[2]))
				realVal = QString::number(m_labelMap.value(parts[2]));

			QString error;
			const int size = assembleInstruction(cellNumber, parts[1], realVal, &error);
			if(size == -1) {
				if(msgs) msgs->append(QString("%1:%2").arg(i + 1).arg(error));
				return false;
			}

			m_lineMap.insert(i + 1, cellNumber);
			codeStart = cellNumber + size;
		} else if(parts.size() == 2) {
			/*2 parts so one of:
			* - directive value
			* - label value
			* - label label
			* - (sequential) mnemonic value
			* - cell value
			* - cell mnemonic
			*/

			if(parts[0].startsWith('.')) {
				//directive

				if(parts[0].toLower() == ".data") {
					dataStart = parts[1].toInt();
				} else if(parts[0].toLower() == ".code") {
					codeStart = parts[1].toInt();
					m_startCell = codeStart;
				} else {
					if(msgs) msgs->append(tr("%1:Unknown directive \"%2\"").arg(i + 1).arg(parts[0]));
					return false;
				}
			} else if(parts[0].contains(':')) {
				const QString label = parts[0].left(parts[0].size() - 1);

				if(!isValidLabel(label)) {
					if(msgs) msgs->append(tr("%1:Invalid label name \"%2\"").arg(i + 1).arg(label));
					return false;
				}

				if(isKeyword(label)) {
					if(msgs) msgs->append(tr("%1:Reserved word used \"%2\"").arg(i + 1).arg(label));
					return false;
				}

				bool isNumber = false;
				const int value = parts[1].toInt(&isNumber);

				if(isNumber) {
					//variable definition

					m_lineMap.insert(i + 1, dataStart);
					m_labelMap.insert(label, dataStart);
					emit memoryChanged(dataStart, value);
					++dataStart;
				} else {
					m_lineMap.insert(i + 1, codeStart);

					if(isValidLabel(parts[1])) {
						//labeled label

						m_labelMap.insert(label, dataStart);

						emit memoryChanged(dataStart, m_labelMap.value(parts[1]));
						++dataStart;
					} else {
						//labeled mnemonic

						m_labelMap.insert(label, codeStart);

						QString error;
						const int size = assembleInstruction(codeStart, parts[1], "", &error);
						if(size == -1) {
							if(msgs) msgs->append(QString("%1:%2").arg(i + 1).arg(error));
							return false;
						}

						codeStart += size;
					}
				}
			} else {
				bool hasCellNumber = false;
				const int cellNumber = parts[0].toInt(&hasCellNumber);

				if(hasCellNumber) {
					//has cell number

					bool isNumber = false;
					const int value = parts[1].toInt(&isNumber);

					if(isNumber) {
						//memory value
						emit memoryChanged(cellNumber, value);
						dataStart = cellNumber + 1;
					} else {
						//a mnemonic

						QString error;
						const int size = assembleInstruction(cellNumber, parts[1], "", &error);
						if(size == -1) {
							if(msgs) msgs->append(QString("%1:%2").arg(i + 1).arg(error));
							return false;
						}

						codeStart = cellNumber + size;
					}

					m_lineMap.insert(i + 1, cellNumber);
				} else {
					//mnemonic and value

					m_lineMap.insert(i + 1, codeStart);

					//labeled value
					QString realVal = parts[1];
					if(m_labelMap.contains(parts[1]))
						realVal = QString::number(m_labelMap.value(parts[1]));

					QString error;
					const int size = assembleInstruction(codeStart, parts[0], realVal, &error);
					if(size == -1) {
						if(msgs) msgs->append(QString("%1:%2").arg(i + 1).arg(error));
						return false;
					}

					m_lineMap.insert(i + 1, codeStart);
					codeStart += size;
				}
			}
		} else if(parts.size() == 1) {
			/* 1 part is
			 * - (sequential) mnemonic
			 * - (sequential) value
			 * - label:
			 */

			bool isNumber = false;
			const int value = parts[0].toInt(&isNumber);

			if(isNumber) {
				//sequential memory value
				emit memoryChanged(dataStart, value);
				++dataStart;
			} else if(parts[0].contains(':')) {
				//label
				m_lineMap.insert(i + 1, codeStart);
			} else if(parts[0].size()) {
				//(sequential) mnemonic

				m_lineMap.insert(i + 1, codeStart);

				QString error;
				const int size = assembleInstruction(codeStart, parts[0], 0, &error);
				if(size == -1) {
					if(msgs) msgs->append(QString("%1:%2").arg(i + 1).arg(error));
					return false;
				}

				codeStart += size;
			}
		}
	}

	return true;
}
