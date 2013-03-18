#ifndef SCREEN_H
#define SCREEN_H

#include <QWidget>

class Screen : public QWidget
{
	Q_OBJECT

	public:
		enum Color
		{
			Black = 0,
			Red,
			Green,
			Blue,
			Cyan,
			Magenta,
			Yellow,
			White
		};

		struct Char
		{
			Char(const Screen::Color &fore, const Screen::Color &back, const char &c):
				fore(fore),
				back(back),
				ch(c)
			{ }

			Screen::Color fore;
			Screen::Color back;
			char ch;
		};

	public:
		explicit Screen(QWidget *parent = 0);

		void setBack(const int &back);
		void setFore(const int &fore);
		void setCol(const int &col);
		void setRow(const int &row);
		void setChar(const char &ch);

		void reset();

	protected:
		void paintEvent(QPaintEvent *event);

	private:
		Color m_back;
		Color m_fore;
		int m_col;
		int m_row;
		QList<Char> m_buffer;
};

#endif // SCREEN_H
