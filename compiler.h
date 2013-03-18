#ifndef COMPILER_H
#define COMPILER_H

#include <QStringList>
#include <QObject>
#include <QHash>

class Compiler : public QObject
{
	Q_OBJECT

	public:
		struct Instruction
		{
			QString mnemonic;
			QStringList operands;
		};

		struct LabelItem
		{
			QString name;
			int address;
		};

	public:
		Compiler(const QString &code, QObject *parent = 0);
		
		bool compile(QStringList *msgs = 0);
		bool precompile(QStringList *msgs = 0);

		QHash<QString, int> labelMap() const;
		int startCell() const;

		int cellToLine(const int &cellNo);
		int lineToCell(const int &lineNo);

		QString code() const;
		void setCode(const QString &code);

	signals:
		void memoryChanged(const int &cellNo, const int &value);

	private:
		QList<Instruction> m_instructions;
		QHash<QString, int> m_labelMap;
		QHash<int, int> m_addressMap;
		QList<int> m_instructionMap;
		int m_startCell;
		QString m_code;
};

#endif // COMPILER_H
