
#include <QtUiTools>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>


#include "MFormTakeOverDlg.h"
#include "MIhmConfigGeneral.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <reg.h>
	#include "ihm.h"
}


MFormTakeOverDlg::MFormTakeOverDlg(QWidget *parent)
    : MFormClientDlg(parent)
{
	m_eDlgType = enuNone;
}

MFormTakeOverDlg::~MFormTakeOverDlg()
{

}

bool MFormTakeOverDlg::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormTakeOverDlg::loadTemplate: sUIFilePath:%1")
									.arg(sUIFilePath));
	
	QFile file(sUIFilePath);
	if(file.open(QFile::ReadOnly))
	{
		m_dynFormWidget = loader.load(&file, this);
		file.close();
		

		m_frContainer = this->findChild<QFrame*>("frContainer");
		m_btnOK = this->findChild<QPushButton*>("btnOK");
 		m_btnCancel = this->findChild<QPushButton*>("btnCancel");
		m_lblTitle = this->findChild<QLabel*>("lblTitle");
		m_lblMessageText = this->findChild<QLabel*>("lblMessageText");

		int iWidth = m_dynFormWidget->width();
		int iHeight = m_dynFormWidget->height();

		QPalette pal1 = m_dynFormWidget->palette();
		this->setAttribute(Qt::WA_WindowPropagation, false);
		this->setPalette(pal1);
		
		QVBoxLayout * layout = new QVBoxLayout(this);
		this->setContentsMargins(0,0,0,0);
		layout->addWidget(m_frContainer);
		layout->setSpacing(0);
		layout->setContentsMargins(0,0,0,0);
		
		if(m_btnOK!=NULL && 
				m_btnCancel!=NULL && 
				m_lblTitle != NULL &&
				m_lblMessageText != NULL)
		{	
			
			QObject::connect(m_btnOK,SIGNAL(clicked()), this, SLOT(onButtonOK()));
			QObject::connect(m_btnCancel,SIGNAL(clicked()), this, SLOT(onButtonCancel()));
			this->resize(iWidth, iHeight);
			
			//init input widget list
			m_lstInputWidgets.append(m_btnOK);
			m_lstInputWidgets.append(m_btnCancel);
			
			m_bTemplateLoaded = true;
			bRetVal = true;
		}

	}
	
	if(!bRetVal)
	{
		TRACE_W(QString("MFormTakeOverDlg::loadTemplate: Error loading %1")
									.arg(sUIFilePath));
	}

	return bRetVal;
}


bool MFormTakeOverDlg::initialize(enumDialogType eType, 
								  QString sInputTemplate, 
								  QString sEmbedTo,
								  QString sCSSPath)
{
	m_eDlgType = eType;
	m_sEmbedTo = sEmbedTo;

	if(sInputTemplate!="")
	{
		QString sTemplatePath = MIhmConfigGeneral::getCfg()->getUIFullPath(sInputTemplate);

		if(loadTemplate(sTemplatePath))
		{
			MHelpFuncs::setFileCSSToWidget(sCSSPath,this);

			installInputCtrlEventFilters();
			return true;
		}
		else
			TRACE_W(QString("MFormTakeOverDlg::initialize: template not loaded!"));
	}
	else
	{
		TRACE_W(QString("MFormTakeOverDlg::initialize: No UI template file defined for LoginDialogTemplate!"));
	}

	return false;
}




void MFormTakeOverDlg::update(QString sTitle, QString sMsg)
{
	if(m_bTemplateLoaded)
	{
		QString sOK; QString sCancel;

		//initialize translations
		MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();

		if (m_eDlgType == enuTakeOverMsg)
		{
			sOK = pLang->getCancelButtonTranslation(MIhmLanguages::enuTranslTargetDesktop);
			m_btnCancel->setVisible(false);
		}
		else if (m_eDlgType == enuTakeOverQuestion)
		{
			sOK = pLang->getTakeOverAcceptButtonTranslation(MIhmLanguages::enuTranslTargetDesktop);
			sCancel = pLang->getTakeOverRejectButtonTranslation(MIhmLanguages::enuTranslTargetDesktop);
			m_btnCancel->setVisible(true);
		}
		else if (m_eDlgType == enuClientInfoMsg)
		{
			sOK = pLang->getClientCloseButtonTranslation(MIhmLanguages::enuTranslTargetDesktop);
			m_btnCancel->setVisible(false);
		}
		else if (m_eDlgType == enuClientConnectingMsg)
		{
			sCancel = pLang->getCancelButtonTranslation(MIhmLanguages::enuTranslTargetDesktop);
			m_btnOK->setVisible(false);

			bool bShowCancelOnConnectingDlg = MIhmConfigGeneral::getCfg()->getShowCancelOnConnectingDlg();

			m_btnCancel->setVisible(bShowCancelOnConnectingDlg);
		}


		m_btnOK->setText(sOK);
		m_btnCancel->setText(sCancel);

		m_lblTitle->setText(sTitle);	
		m_lblMessageText->setText(sMsg);	
	}
}
