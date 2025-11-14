
#include <QtUiTools>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>


#include "MHelpFuncs.h"
#include "MFormLogin.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <reg.h>
	#include "ihm.h"
}


MFormLogin::MFormLogin(QWidget *parent)
    : MFormClientDlg(parent)
{
	m_bLDAPOffline = false;

		

}

MFormLogin::~MFormLogin()
{

}

//if bLDAPOnline==true the sSecCodeis password otherwise it is the secure user id number
void MFormLogin::getLoginData(QString &sName, QString &sSecCode, bool &bLDAPOffline)
{
	sName = m_txtUserName->text();
	sSecCode = m_txtPassword->text();
	bLDAPOffline = m_bLDAPOffline;
}

bool MFormLogin::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormLogin::loadTemplate: sUIFilePath:%1")
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
		m_lblLoginName = this->findChild<QLabel*>("lblLoginName");
		m_lblPassword = this->findChild<QLabel*>("lblPassword");
 		m_txtUserName = this->findChild<QLineEdit*>("txtUserName");
 		m_txtPassword = this->findChild<QLineEdit*>("txtPassword");
		m_lblMessage = this->findChild<QLabel*>("lblMessage");

		int iWidth = m_dynFormWidget->width();
		int iHeight = m_dynFormWidget->height();
		QPalette pal1 = m_dynFormWidget->palette();
		this->setAttribute(Qt::WA_WindowPropagation, false);
		this->setPalette(pal1);

		
		if(m_btnOK!=NULL && 
				m_btnCancel!=NULL && 
				m_lblTitle != NULL &&
				m_lblLoginName != NULL &&
				m_lblPassword != NULL &&
				m_txtUserName != NULL &&
				m_txtPassword != NULL &&
				m_lblMessage != NULL &&
				m_frContainer != NULL)
		{
			
			QVBoxLayout * layout = new QVBoxLayout(this);
			this->setContentsMargins(0,0,0,0);
			layout->addWidget(m_frContainer);
			layout->setSpacing(0);
			layout->setContentsMargins(0,0,0,0);
			
			QObject::connect(m_btnOK,SIGNAL(clicked()), this, SLOT(onButtonOK()));
			QObject::connect(m_btnCancel,SIGNAL(clicked()), this, SLOT(onButtonCancel()));
			this->resize(iWidth, iHeight);
			
			//init input widget list
			m_lstInputWidgets.append(m_txtUserName);
			m_lstInputWidgets.append(m_txtPassword);
			m_lstInputWidgets.append(m_btnOK);
			m_lstInputWidgets.append(m_btnCancel);

			m_bTemplateLoaded = true;
			bRetVal = true;
		}

	}
	
	if(!bRetVal)
	{
		TRACE_D(QString("MFormLogin::loadTemplate: Error loading %1")
									.arg(sUIFilePath));
	}

	return bRetVal;
}


bool MFormLogin::initialize(QString sLoginName, 
							QString sInputTemplate, 
							QString sEmbedTo, 
							QString sCSSPath, 
							bool bLoginFieldEnabled,
							bool bLDAPOffline)
{
	m_sEmbedTo = sEmbedTo;
	m_bLDAPOffline = bLDAPOffline;

	if(sInputTemplate!="")
	{
		QString sTemplatePath = MIhmConfigGeneral::getCfg()->getUIFullPath(sInputTemplate);
		

		if(loadTemplate(sTemplatePath))
		{
			MHelpFuncs::setFileCSSToWidget(sCSSPath,this);
			//initialize translations
			MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
			m_btnOK->setText(pLang->getLoginOKButtonTranslation(MIhmLanguages::enuTranslTargetDesktop));	
			m_btnCancel->setText(pLang->getLoginCancelButtonTranslation(MIhmLanguages::enuTranslTargetDesktop));	
			
			QString sTemp = pLang->getLoginTitleTranslation(MIhmLanguages::enuTranslTargetDesktop);
			m_lblTitle->setText(sTemp);	

			sTemp = pLang->getLoginNameTranslation(MIhmLanguages::enuTranslTargetDesktop);
			m_lblLoginName->setText(sTemp);	 

			if(!bLDAPOffline)
			{
				sTemp = pLang->getLoginPwdTranslation(MIhmLanguages::enuTranslTargetDesktop);
				m_lblPassword->setText(sTemp);	 

				sTemp = pLang->getLoginMessageTranslation(MIhmLanguages::enuTranslTargetDesktop);
				m_lblMessage->setText(sTemp);	 
			}
			else
			{
				sTemp = pLang->getLoginSecNumberTranslation(MIhmLanguages::enuTranslTargetDesktop);
				m_lblPassword->setText(sTemp);	 

				sTemp = pLang->getLoginLDAPOfflineMessageTranslation(MIhmLanguages::enuTranslTargetDesktop);
				m_lblMessage->setText(sTemp);	 
			}

			m_txtUserName->setText(sLoginName);
			m_txtUserName->setEnabled(bLoginFieldEnabled);

			if(bLoginFieldEnabled)
				m_txtUserName->setFocus();
			else
				m_txtPassword->setFocus();

			installInputCtrlEventFilters();
			return true;
		}
		else
			TRACE_W(QString("MFormLogin::initialize: template not loaded!"));
	}
	else
	{
		TRACE_W(QString("MFormLogin::initialize: No UI template file defined for LoginDialogTemplate!"));
	}

	return false;
}


