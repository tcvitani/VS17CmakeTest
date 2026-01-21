/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator														 */
/* FILE:     FormMainTranslTest.h														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef FORMMAINTRANSLTEST_H
#define FORMMAINTRANSLTEST_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QApplication>

#include "ui_FormMainTranslTest.h"
#include <MTranslator.h>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


namespace Ui
{
class FormMainTranslTest;
}

class FormMainTranslTest : public QMainWindow
{
    Q_OBJECT
public:
    FormMainTranslTest(QApplication* pApp, MTranslator* pTrans, QWidget *parent = 0);
    virtual ~FormMainTranslTest();

	void changeEvent(QEvent* event);

private slots:
    void onCurrentIndexChanged(int index);
    void onBtnClear();

private:
    Ui::FormMainTranslTestB* m_ui;
    void addToMsgList(QString msg);
	QApplication* m_pApp;
	MTranslator* m_pTrans;
	void retranslate();
};

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
