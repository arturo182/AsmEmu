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

		void setEnabled(bool enabled);

	protected:
		void highlightBlock(const QString &text);

	private:
		QList<HighlighterRule*> m_rules;
		bool m_enabled;
		bool m_readOnly;
};

#endif // ASMHIGHLIGHTER_H
