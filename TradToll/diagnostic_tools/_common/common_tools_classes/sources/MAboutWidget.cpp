/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:	 MAboutWidget.cpp												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include "MAboutWidget.h"
#include "MTracer.h"
#include "MHelpFuncs.h"
#include "MConfig.h"
#include "MDefines.h"

#include "ui_MAboutWidgetB.h"

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

MAboutWidget::MAboutWidget(QWidget *parent):QWidget(parent), m_ui(new Ui::MAboutWidgetB)
{
	m_ui->setupUi(this);

	connect(m_ui->btnCloseAbout, SIGNAL(released()), this, SLOT(onCloseBtnClicked()));

	// Load dialog stylesheet file if path to that file isn't empty and if that file isn't empty.
	if (!MConfig::getCfg()->getDialogStylesheetPath().isEmpty() && !MHelpFuncs::loadFileContent(MConfig::getCfg()->getDialogStylesheetPath()).isEmpty())
		MHelpFuncs::setFileCSSToWidget(MConfig::getCfg()->getDialogStylesheetPath(), this);
}

MAboutWidget::~MAboutWidget()
{

}

void MAboutWidget::setAppVersion(QString sVersion)
{
	m_sAppVersion = sVersion;
	retranslate();
}


void MAboutWidget::setLogoPix(QString sLogoPath)
{
	m_ui->lblAboutDlgLogo->setPixmap(QPixmap(sLogoPath));
}


void MAboutWidget::onCloseBtnClicked()
{
	this->hide();
	emit canceled();
}

void MAboutWidget::retranslate()
{
	m_ui->retranslateUi(this);

	m_ui->lblAboutLabel1->setText(QString(tr(ABOUT_DIALOG_LABEL1)).arg(m_sAppVersion));

}

/*-------------------------------- END OF FILE ------------------------------*/