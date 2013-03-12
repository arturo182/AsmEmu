#include "asmhighlighter.h"

#include <QDebug>

AsmHighlighter::AsmHighlighter(QTextDocument *parent) :
	QSyntaxHighlighter(parent),
	m_enabled(true)
{
	const QString mnemonics = "(hlt|cpa|sto|add|sub|bra|brn|mul|brz|inc|dec)";
	const QString label = "[a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ_][_a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ0-9]*";

	//the order here is important
	//memory cells
	m_rules << new HighlighterRule("\\s([0-9]+|\\[([0-9]+|[-\\+\\*\\/]|"+label+"|\\s)+\\])(\\s|;|$)", Qt::red);

	//const values
	m_rules << new HighlighterRule("\\s\\$[0-9]+(\\s|;|$)", Qt::darkYellow);

	//line numbers
	m_rules << new HighlighterRule("^[0-9]+\\s", Qt::darkMagenta);

	//labels
	m_rules << new HighlighterRule("(^" + label + ":\\s?)|(" + mnemonics + "\\s+" + label + ")|" + label, QColor(250, 130, 0), QRegularExpression::CaseInsensitiveOption);

	//mnemonics
	m_rules << new HighlighterRule("\\s?" + mnemonics + "(\\s|;|$)", Qt::blue, QRegularExpression::CaseInsensitiveOption);

	//directives
	m_rules << new HighlighterRule("^\\.(.*)$", Qt::darkGray);

	//comments
	m_rules << new HighlighterRule(";(.*)$", Qt::darkGreen);
}

AsmHighlighter::~AsmHighlighter()
{
	qDeleteAll(m_rules);
	m_rules.clear();
}

void AsmHighlighter::setEnabled(bool enabled)
{
	m_enabled = enabled;
	rehighlight();
}

void AsmHighlighter::highlightBlock(const QString &text)
{
	if(!m_enabled) {
		setFormat(0, text.size(), Qt::darkGray);
		return;
	}

	foreach(HighlighterRule *rule, m_rules) {
		const QRegularExpression expression = rule->pattern;

		QRegularExpressionMatchIterator it = expression.globalMatch(text);
		while(it.hasNext()) {
			const QRegularExpressionMatch match = it.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule->format);
		}
	}
}
