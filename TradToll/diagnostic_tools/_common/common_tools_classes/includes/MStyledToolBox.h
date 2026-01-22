/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MStyledToolBox.h												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef COMMON_TOOLS_MStyledToolBox_H
#define COMMON_TOOLS_MStyledToolBox_H


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QtGui>
#include <QToolBox>
#include <QPushButton>
#include <QAbstractButton>
#include <QTimer>
/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/
#if defined COMMON_TOOLS_CLASSES_EXPORT
	#define COMMON_TOOLS_CLASSES_DLLSPEC  Q_DECL_EXPORT
#else
	#define COMMON_TOOLS_CLASSES_DLLSPEC Q_DECL_IMPORT
#endif

/*-------------------------------- TYPEDEFS:  -------------------------------*/


class COMMON_TOOLS_CLASSES_DLLSPEC MStyledToolBox : public QToolBox

{
    Q_OBJECT

public:
    MStyledToolBox(QWidget *parentDialog);
    ~MStyledToolBox();

	int addGroup(const QString & text );
	int addGroupItem( int indexGroup, const QString & text , QPushButton **ppButton = NULL);
	void collapseToolboxMenu();
	void retranslate();
	void setFixedWidth(int iWidth);
	
	void setAutoHide(bool b){m_bAutoHide = b;};
	void setAutoHideTimeout(uint i){m_uiAutoHideTimeout = i;};

public slots:
	void show();
	void hide();

private slots:
	void onToolboxButtonClicked();
	void onCurrentGroupChanged(int index);
	void onMouseLeaveTimeout();

protected:
	void leaveEvent( QEvent * event );
	void enterEvent( QEnterEvent * event );

private:
	void updateToolboxButtonList();
	void groupClicked(int index);
	void updateToolboxPages();
	QTimer m_tmMouseOut;
	bool m_bAutoHide;
	uint m_uiAutoHideTimeout;
	bool m_bMouseIn;

	QWidget * m_parentDialog;

	QList<QAbstractButton *> m_lstToolboxButtons;
	QStringList sGroupLabels;

	int iCurrentGroup;
};



#endif

