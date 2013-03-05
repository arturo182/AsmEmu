#ifndef ASMHIGHLIGHTER_H
#define ASMHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

class AsmHighlighter: public QSyntaxHighlighter
{
	Q_OBJECT

	private:
		struct HighlighterRule
		{
			QRegularExpression pattern;
			QTextCharFormat format;
		};

	public:
		explicit AsmHighlighter(QTextDocument *parent = 0);
		~AsmHighlighter();

	protected:
		void highlightBlock(const QString &text);

	private:
		QList<HighlighterRule*> m_rules;
};

#endif // ASMHIGHLIGHTER_H
