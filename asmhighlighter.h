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
			public:
				HighlighterRule(const QString &patternStr, const QBrush &brush, const QRegularExpression::PatternOptions &options = QRegularExpression::NoPatternOption) :
					pattern(QRegularExpression(patternStr))
				{
					format.setForeground(brush);
					pattern.setPatternOptions(options);
				}

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
