/***************** (v) 2014 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 QT_translator													 */
/* FILE:	 FormMainTranslTest.cpp												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include "formmaintransltest.h"

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*---------------------------------- CODE: ----------------------------------*/

FormMainTranslTest::FormMainTranslTest(QApplication* pApp, MTranslator* pTrans, QWidget *parent):
    QMainWindow(parent),
    m_ui(new Ui::FormMainTranslTestB)
{
    m_ui->setupUi(this);

	m_pApp = pApp;
	m_pTrans = pTrans;

	m_ui->languagesCombo->addItems(pTrans->getLangNativeNamesList());
    m_ui->languagesCombo->setCurrentIndex(pTrans->getCurrentLangIndex());
    
    connect(m_ui->languagesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(m_ui->clearButton, SIGNAL(released()), this, SLOT(onBtnClear()));
}

FormMainTranslTest::~FormMainTranslTest()
{
	if (m_ui != NULL)
	{
		delete m_ui;
		m_ui = NULL;
	}
}

void FormMainTranslTest::onCurrentIndexChanged(int index)
{
	m_pTrans->setLanguage(index);
	QString statusText = m_pTrans->getLabelTranslation("LOG_SELECTED_INDEX");

	QString sTemp = statusText.arg(index).arg(m_pTrans->getLangIDsList().at(index)).arg(m_pTrans->getLangNativeNamesList().at(index));
	addToMsgList(sTemp);

	m_ui->statusbar->showMessage(QString("%1 - %2").arg(index).arg(m_pTrans->getLangNativeNamesList().at(index)));

	QCoreApplication::removeTranslator(m_pTrans);
	QCoreApplication::installTranslator(m_pTrans);
}

void FormMainTranslTest::addToMsgList(QString msg)
{
  m_ui->listWidget->addItem(msg);
  m_ui->listWidget->scrollToBottom();
}

void FormMainTranslTest::onBtnClear()
{
  m_ui->listWidget->clear();
}

void FormMainTranslTest::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		QString s2 = m_pTrans->translate("FormMainTranslTestB", "LABEL_APP_WINDOW_TITLE", nullptr);
		QString s = QCoreApplication::translate("FormMainTranslTestB", "LABEL_APP_WINDOW_TITLE", nullptr);
		// retranslate designer form (single inheritance approach)
		m_ui->retranslateUi(this);
		
		// retranslate other widgets which weren't added in designer
		retranslate();
	}
	
	// remember to call base class implementation
	QMainWindow::changeEvent(event);
}

void FormMainTranslTest::retranslate()
{
	addToMsgList(QString(tr("LOG_FUNC_INVOKED")).arg("changeEvent"));
}