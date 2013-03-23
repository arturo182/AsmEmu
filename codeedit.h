#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QPlainTextEdit>
#include <QHash>
#include <QMap>

class CodeEdit: public QPlainTextEdit
{
	Q_OBJECT
	Q_PROPERTY(int lineStep READ lineStep WRITE setLineStep)
	Q_PROPERTY(int zeroPadding READ zeroPadding WRITE setZeroPadding)
	Q_PROPERTY(bool zeroStart READ zeroStart WRITE setZeroStart)

	public:
		enum GutterMark
		{
			Cell = 0,
			Breakpoint
		};

	private:
		class Gutter: public QWidget
		{
			public:
				Gutter(CodeEdit *textEdit);

				QSize sizeHint() const;
				int prefferedWidth() const;

			protected:
				void paintEvent(QPaintEvent *event);
				void mousePressEvent(QMouseEvent *event);

			private:
				CodeEdit *m_textEdit;
		};

	public:
		explicit CodeEdit(QWidget *parent = 0);
		~CodeEdit();

		void setLineStep(const int &lineStep);
		int lineStep() const;

		void setZeroPadding(const int &zeroPadding);
		int zeroPadding() const;

		void setZeroStart(bool zeroStart);
		bool zeroStart() const;

		void addMark(const int &line, const GutterMark &mark);
		void removeMark(const int &line, const GutterMark &mark);
		void removeMarks(const GutterMark &mark);

		void setMarkPixmap(const GutterMark &mark, const QPixmap &pixmap);

		bool canRedo();
		bool canUndo();

	signals:
		void gutterClicked(const int &line, const GutterMark &mark);
		void fileDropped(const QString &fileName);
		void focusChanged(bool focus);

	protected:
		void keyPressEvent(QKeyEvent *event);
		void resizeEvent(QResizeEvent *event);
		void focusInEvent(QFocusEvent *event);
		void focusOutEvent(QFocusEvent *event);
		bool canInsertFromMimeData(const QMimeData *source) const;
		void insertFromMimeData(const QMimeData *source);

	protected slots:
		void updateGutterWidth();
		void updateGutter(const QRect &rect, const int &yDistance);
		void setRedoAvailable(bool redoAvailable);
		void setUndoAvailable(bool undoAvailable);

	protected:
		Gutter *m_gutter;
		int m_lineStep;
		int m_zeroPadding;
		bool m_zeroStart;
		bool m_undoAvailable;
		bool m_redoAvailable;
		QHash<GutterMark, QPixmap> m_markPixmaps;
		QMultiMap<int, GutterMark> m_marks;
};

#endif // CODEEDIT_H
