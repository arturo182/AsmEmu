#include "evaluator.h"

#include <QPair>

Evaluator::Operator Evaluator::parseOp()
{
	switch(m_expr[m_index].toLatin1()) {
		case '+':
			++m_index;
			return Operator(Add, 10);
		break;

		case '-':
			++m_index;
			return Operator(Sub, 10);
		break;

		case '/':
			++m_index;
			return Operator(Div, 20);
		break;

		case '*':
			++m_index;
			return Operator(Mul, 20);
		break;

		default:
			return Operator(Null, 0);
	}
}

int Evaluator::parseValue()
{
	int value = 0;

	switch(m_expr[m_index].toLatin1()) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			char c = m_expr[m_index].toLatin1();
			while((c >= '0') && (c <= '9')) {
				value *= 10;
				value += c - '0';

				++m_index;
				c = m_expr[m_index].toLatin1();
			}
		}
		break;

		case '(':
		{
			++m_index;
			value = parseExpr();

			if(m_expr[m_index] != ')')
				return 0;

			++m_index;
		}
		break;

		case '+':
			++m_index;
			value = parseValue();
		break;

		case '-':
			++m_index;
			value = -parseValue();
		break;

		default:
			return 0;
	}

	return value;
}

int Evaluator::parseExpr()
{
	QStack<QPair<Operator, int> > stack;
	stack.push(qMakePair(Operator(Null, 0), 0));

	int value = parseValue();

	while(!stack.empty()) {
		Operator op = parseOp();

		while(op.precedence <= stack.top().first.precedence) {
			QPair<Operator, int> top = stack.top();

			switch(top.first.op) {
				case Add:
					value = top.second + value;
				break;

				case Sub:
					value = top.second - value;
				break;

				case Mul:
					value = top.second * value;
				break;

				case Div:
					value = top.second / (value == 0) ? 1 : value;
				break;

				case Null:
					stack.pop();
					return value;
				break;
			}

			stack.pop();
		}

		stack.push(qMakePair(op, value));
		value = parseValue();
	}

	return 0;
}
