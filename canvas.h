#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>

class Canvas : public QWidget
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
			Char(const Canvas::Color &fore, const Canvas::Color &back, const char &c):
				fore(fore),
				back(back),
				ch(c)
			{ }

			Canvas::Color fore;
			Canvas::Color back;
			char ch;
		};

	public:
		explicit Canvas(QWidget *parent = 0);

		void setBack(const int &back);
		void setFore(const int &fore);
		void setCol(const int &col);
		void setRow(const int &row);
		void setChar(const char &ch);

	protected:
		void paintEvent(QPaintEvent *event);

	private:
		Color m_back;
		Color m_fore;
		int m_col;
		int m_row;
		QList<Char> m_buffer;
};

#endif // CANVAS_H
