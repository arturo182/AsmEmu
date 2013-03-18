#include "asmhighlighter.h"

#include <QDebug>

AsmHighlighter::AsmHighlighter(QTextDocument *parent) :
	QSyntaxHighlighter(parent),
	m_enabled(true)
{
	const QString mnemonics = "(hlt|cpa|sto|rsi|add|sub|mul|bra|brn|brz|brof|brnf|brzf|inc|dec|pop|push|call|ret|dat|scrx|scry|scrf|scrb|scr)";
	const QString registers = "(ax|sp|sb|ip|di|flags)";
	const QString label = "[a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ_][_a-zęóąśłżźćńĘÓĄŚŁŻŹĆŃ0-9]*";

	//the order here is important
	//memory cells
	m_rules << new HighlighterRule("\\s([0-9]+|\\[([0-9]+|[-\\+\\*\\/]|" + label + "|\\s)+\\])(\\s|;|$)", Qt::red, QRegularExpression::CaseInsensitiveOption);

	//const values
	m_rules << new HighlighterRule("\\s\\$[0-9]+(\\s|;|$)", Qt::darkYellow);

	//line numbers
	m_rules << new HighlighterRule("^[0-9]+\\s", Qt::darkMagenta);

	//labels
	m_rules << new HighlighterRule("(^" + label + ":\\s?)|(" + mnemonics + "\\s+" + label + ")|" + label, QColor(250, 130, 0), QRegularExpression::CaseInsensitiveOption);

	//registers
	m_rules << new HighlighterRule("\\b"+registers+"\\b", Qt::darkCyan, QRegularExpression::CaseInsensitiveOption);

	//mnemonics
	m_rules << new HighlighterRule("\\b" + mnemonics + "\\b", Qt::blue, QRegularExpression::CaseInsensitiveOption);

	//directives
	m_rules << new HighlighterRule("^\\.(.*)$", Qt::darkGray);

	//strings
	m_rules << new HighlighterRule("\\s\\\".*\\\"(\\s|;|$)", Qt::red, QRegularExpression::CaseInsensitiveOption);

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
