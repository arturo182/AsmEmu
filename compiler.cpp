#include "compiler.h"

#include "virtualmachine.h"

#include <QRegularExpression>

const QMap<QString, VirtualMachine::Instruction> mnemonicMap(std::map<QString, VirtualMachine::Instruction>({{"HLT", VirtualMachine::HLT},
																											 {"INC", VirtualMachine::INC},
																											 {"DEC", VirtualMachine::DEC},
																											 {"CPA", VirtualMachine::CPA},
																											 {"STO", VirtualMachine::STO},
																											 {"ADD", VirtualMachine::ADD},
																											 {"SUB", VirtualMachine::SUB},
																											 {"BRA", VirtualMachine::BRA},
																											 {"BRN", VirtualMachine::BRN},
																											 {"MUL", VirtualMachine::MUL},
																											 {"BRZ", VirtualMachine::BRZ}}));


Compiler::Compiler(const QString &code)
{
	m_lines = code.split('\n', QString::KeepEmptyParts);

	const QRegularExpression labelPattern("^\\s?([a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ_][_a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ0-9]*:)\\s?$", QRegularExpression::CaseInsensitiveOption);


	for(int i = 0; i < m_lines.size(); ++i) {
		//strip comments
		m_lines[i].replace(QRegularExpression(";(.*)$"), "");

		//trim whitespace
		m_lines[i].replace(QRegularExpression("\\s+"), " ");
		m_lines[i] = m_lines[i].trimmed();

		//move single labels to next lines
		QRegularExpressionMatch labelMatch = labelPattern.match(m_lines[i]);
		if(labelMatch.hasMatch() && i < m_lines.size()) {
			m_lines[i + 1].prepend(labelMatch.captured(1) + " ");
		}
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

int Compiler::assembleInstruction(const int &cellNo, const QString &mnemonic, const QString &strValue)
{
	VirtualMachine::Instruction instr = mnemonicMap.value(mnemonic.toUpper());

	const bool isConst = strValue.contains('$');
	const bool isAddress = strValue.contains('[');
	const int value =  QString(strValue).remove(QRegularExpression("\\$|\\[|\\]")).toInt();

	switch(instr) {
		case VirtualMachine::HLT:
		{
			emit memoryChanged(cellNo, VirtualMachine::intToMemory(0));
			return 1;
		}
		break;

		case VirtualMachine::INC:
		{
			if(isConst) {
				return 0;
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9300 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9400);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 100) {
					emit memoryChanged(cellNo, 100 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9000);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::DEC:
		{
			if(isConst) {
				return 0;
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9390 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9490);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 100) {
					emit memoryChanged(cellNo, 200 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9010);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::CPA:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9110 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9210);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9310 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9410);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 1000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9010);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::STO:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9120 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9220);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9320 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9420);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 2000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9020);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::ADD:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9130 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9210);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9330 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9430);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 3000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9030);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::SUB:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9140 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9240);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9340 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9440);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 4000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9040);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::BRA:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9150 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9250);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9350 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9450);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 5000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9050);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::BRN:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9160 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9260);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9360 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9460);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 6000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9060);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::MUL:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9170 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9270);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9370 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9470);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 7000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9070);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;

		case VirtualMachine::BRZ:
		{
			if(isConst) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9180 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9280);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else if(isAddress) {
				if(value < 10) {
					emit memoryChanged(cellNo, 9380 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9480);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			} else {
				if(value < 1000) {
					emit memoryChanged(cellNo, 8000 + value);
					return 1;
				} else {
					emit memoryChanged(cellNo, 9080);
					emit memoryChanged(cellNo + 1, value);
					return 2;
				}
			}
		}
		break;
	}

	return 0;
}

bool Compiler::compile()
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

				m_labelMap.insert(label, codeStart);
				cellNumber = codeStart;
			}

			//labeled value
			QString realVal = parts[2];
			if(m_labelMap.contains(parts[2]))
				realVal = QString::number(m_labelMap.value(parts[2]));

			const int size = assembleInstruction(cellNumber, parts[1], realVal);

			m_lineMap.insert(i + 1, cellNumber);
			codeStart = cellNumber + size;
		} else if(parts.size() == 2) {
			/*2 parts so one of:
			* - directive value
			* - label value
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
				}
			} else if(parts[0].contains(':')) {
				const QString label = parts[0].left(parts[0].size() - 1);

				bool isNumber = false;
				const int value = parts[1].toInt(&isNumber);

				if(isNumber) {
					//variable definition

					m_lineMap.insert(i + 1, dataStart);
					m_labelMap.insert(label, dataStart);
					emit memoryChanged(dataStart, value);
					++dataStart;
				} else {
					//labeled HLT

					m_lineMap.insert(i + 1, codeStart);
					m_labelMap.insert(label, codeStart);
					emit memoryChanged(codeStart, 0);
					++codeStart;
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
					} else if(parts[1].toLower() == "hlt") {
						//a HLT mnemonic
						emit memoryChanged(cellNumber, VirtualMachine::intToMemory(0));
						codeStart = cellNumber + 1;
					}

					m_lineMap.insert(i + 1, cellNumber);
				} else {
					//mnemonic and value

					m_lineMap.insert(i + 1, codeStart);

					//labeled value
					QString realVal = parts[1];
					if(m_labelMap.contains(parts[1]))
						realVal = QString::number(m_labelMap.value(parts[1]));

					const int size = assembleInstruction(codeStart, parts[0], realVal);

					m_lineMap.insert(i + 1, codeStart);
					codeStart += size;
				}
			}
		} else if(parts.size() == 1) {
			/* 1 part is
			 * - (sequential) mnemonic (HLT)
			 * - (sequential) value
			 * - label:
			 */

			bool isNumber = false;
			const int value = parts[0].toInt(&isNumber);

			if(isNumber) {
				//sequential memory value
				emit memoryChanged(dataStart, value);
				++dataStart;
			} else if(parts[0].toLower() == "hlt") {
				//a HLT mnemonic
				emit memoryChanged(codeStart, VirtualMachine::intToMemory(0));
				m_lineMap.insert(i + 1, codeStart);

				++codeStart;
			} else if(parts[0].contains(':')) {
				//label
				m_lineMap.insert(i + 1, codeStart);
			}
		}
	}

	return true;
}
