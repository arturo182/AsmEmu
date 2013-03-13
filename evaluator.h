#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <QString>
#include <QStack>

class Evaluator
{
	public:
		int eval(const QString &expr)
		{
			m_index = 0;
			m_expr = expr;

			return parseExpr();
		}

	private:
		enum Operation
		{
			Null = 0,
			Add,
			Sub,
			Mul,
			Div
		};

		struct Operator
		{
			Operator() :
				op(Null),
				precedence(0)
			{ }

			Operator(const Operation &opr, const int &precedence) :
				op(opr),
				precedence(precedence)
			{ }

			Operation op;
			int precedence;
		};

	private:
		Operator parseOp();
		int parseValue();
		int parseExpr();

	private:
		QString m_expr;
		int m_index;
};

#endif // EVALUATOR_H
