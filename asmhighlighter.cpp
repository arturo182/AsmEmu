#include "asmhighlighter.h"

AsmHighlighter::AsmHighlighter(QTextDocument *parent) :
	QSyntaxHighlighter(parent)
{
	HighlighterRule *memory = new HighlighterRule;
	memory->pattern = QRegularExpression("\\s([0-9]+|\\[[0-9]+\\])(\\s|;|$)");
	memory->format.setForeground(Qt::red);
	m_rules << memory;

	HighlighterRule *value = new HighlighterRule;
	value->pattern = QRegularExpression("\\s\\$[0-9]+(\\s|;|$)");
	value->format.setForeground(Qt::darkYellow);
	m_rules << value;

	HighlighterRule *line = new HighlighterRule;
	line->pattern = QRegularExpression("^[0-9]+\\s");
	line->format.setForeground(Qt::darkMagenta);
	m_rules << line;

	HighlighterRule *mnemonic = new HighlighterRule;
	mnemonic->pattern = QRegularExpression("\\s(hlt|cpa|sto|add|sub|bra|brn|mul|brz)(\\s|;|$)");
	mnemonic->pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	mnemonic->format.setForeground(Qt::blue);
	m_rules << mnemonic;

	HighlighterRule *comment = new HighlighterRule;
	comment->pattern = QRegularExpression(";(.*)$");
	comment->format.setForeground(Qt::darkGreen);
	m_rules << comment;
}

AsmHighlighter::~AsmHighlighter()
{
	qDeleteAll(m_rules);
	m_rules.clear();
}

void AsmHighlighter::highlightBlock(const QString &text)
{
	foreach(HighlighterRule *rule, m_rules) {
		const QRegularExpression expression = rule->pattern;

		QRegularExpressionMatchIterator it = expression.globalMatch(text);
		while(it.hasNext()) {
			const QRegularExpressionMatch match = it.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule->format);
		}
	}
}
