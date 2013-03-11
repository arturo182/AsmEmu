#ifndef COMPILER_H
#define COMPILER_H

#include <QStringList>
#include <QObject>
#include <QMap>

class Compiler : public QObject
{
	Q_OBJECT

	public:
		Compiler(const QString &code);
		
		bool compile();

		const QMap<int, int> &lineMap() const;
		int startCell() const;

	signals:
		void memoryChanged(const int &cellNo, const int &value);

	private:
		int assembleInstruction(const int &cellNo, const QString &mnemonic, const QString &strValue);

	private:
		QMap<QString, int> m_labelMap;
		QMap<int, int> m_lineMap;
		QStringList m_lines;
		int m_startCell;
};

#endif // COMPILER_H
