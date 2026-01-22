/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MStyledTitleBar.h												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef COMMON_TOOLS_MStyledTitleBar_H
#define COMMON_TOOLS_MStyledTitleBar_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QWidget>
#include <QAbstractButton>

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/
#if defined COMMON_TOOLS_CLASSES_EXPORT
	#define COMMON_TOOLS_CLASSES_DLLSPEC  Q_DECL_EXPORT
#else
	#define COMMON_TOOLS_CLASSES_DLLSPEC Q_DECL_IMPORT
#endif

/*-------------------------------- TYPEDEFS:  -------------------------------*/

class COMMON_TOOLS_CLASSES_DLLSPEC MStyledTitleBar : public QWidget

{
    Q_OBJECT

public:

    MStyledTitleBar(QWidget *parentDialog, QWidget *parentFrame, QAbstractButton *pbtnMinimize, QAbstractButton *pbtnMaximize, QAbstractButton *pbtnClose);
	void setButtonsPixmaps(const QPixmap &pxMaximize, const QPixmap &pxMinimize, const QPixmap &pxRestore, const QPixmap &pxClose);
	
// 	void setText(const QString &);
public slots:
    void showSmall();
    void showMaxRestore();

signals:
	void dialogResized();
	void message(QString);

protected:
	void mousePressEvent(QWidget * w, QMouseEvent *e);
	void mouseMoveEvent(QWidget * w, QMouseEvent *e);
	void mouseReleaseEvent(QWidget * w, QMouseEvent *e);
	bool eventFilter(QObject *obj, QEvent *event);

private:
	void displayMessage(QString sMsg);

	QAbstractButton *m_pbtnMinimize;
	QAbstractButton *m_pbtnMaximize;
	QAbstractButton *m_pbtnClose;

    QPixmap restorePix, maxPix;
    bool bIsMaximized;

	QPoint m_old_pos;
	QPoint m_start_pos;
    bool m_mouse_down; 
	bool m_bIsMouseDownAtTitle;
    bool left, right, bottom;
	bool m_bResizeStarted;
	QWidget * m_parentDialog;
	QWidget * m_parentFrame;


	QPixmap m_pxMaximize, m_pxMinimize, m_pxRestore, m_pxClose;

	QPoint m_normalPos;
	QSize m_normalSize;

};



#endif

