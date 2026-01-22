/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MStyledLangPick.h												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef COMMON_TOOLS_StyledLangPick_H
#define COMMON_TOOLS_StyledLangPick_H

/*-------------------------------- INCLUDES:  -------------------------------*/
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/
#if defined COMMON_TOOLS_CLASSES_EXPORT
	#define COMMON_TOOLS_CLASSES_DLLSPEC  Q_DECL_EXPORT
#else
	#define COMMON_TOOLS_CLASSES_DLLSPEC Q_DECL_IMPORT
#endif

/*-------------------------------- TYPEDEFS:  -------------------------------*/


class COMMON_TOOLS_CLASSES_DLLSPEC MStyledLangButton: public QFrame
{
    Q_OBJECT

public:
	MStyledLangButton(QWidget *parent, QString sUnselectedImagePath, QString sSelectedImagePath, QString sTextToDisplay, QString sID);
	~MStyledLangButton();

	QString getId(){return m_sID;};

	void setSelected(bool bSelected);
signals:
	void clicked(QString);

protected:
	void mouseReleaseEvent (QMouseEvent * event);

private:
	QLabel * m_lblFlag;
	QLabel * m_lblLang;
	QPixmap * m_pColPixmap;
	QPixmap * m_pMonoPixmap;

	QString m_sID; //string to send when clicked
};



class COMMON_TOOLS_CLASSES_DLLSPEC MStyledLangPick : public QFrame

{
    Q_OBJECT

public:

    MStyledLangPick(QWidget *parentFrame);
	void appendLanguage(QString sColourImagePath, QString sMonoImagePath, QString sTextToDisplay, QString sLangID);
	
	void setAlignment(Qt::Alignment alignment);
	void setSelectedLang(QString sLangID);
	QString getSelectedLang(){return  m_sSelectedLangID;};
signals:
	void selectionChanged(QString);
	
public slots:
	void toogleSelectingState();

private slots:
	void onLangSelected(QString sLangID);
	void displaySelectingState();
	void displaySelectedState();


protected:

private:
	QHBoxLayout *m_horizontalLayout;
	QWidget * m_parentFrame;

	QList <MStyledLangButton*> m_lstButtons;

	QString m_sSelectedLangID;

	bool m_bStateSelectingInProgress;
};



#endif

