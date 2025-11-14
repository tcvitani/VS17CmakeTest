
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFrame>
#include <QKeyEvent>

#include <QtUiTools>

#include "MFormAbout.h"
#include "MIhmConfigGeneral.h"
#include "MTracer.h"
#include "MHelpFuncs.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <reg.h>
	#include "ihm.h"
}

#define REG_VAL_CUSTOM_BUTTONS "\\CustomButtons"
#define REG_VAL_CUSTOM_BUTTONS_LAST_CMD "LastCommand"
#define XML_CUSTOM_BTN_ROOT "custom_button_list"
#define XML_CUSTOM_BTN_UI "ui_file"
#define XML_CUSTOM_BTN_SHOW_GENERIC "show_generic"
#define XML_CUSTOM_BTN_OPTION "option"
#define XML_CUSTOM_BTN_TEXT "text"
#define XML_CUSTOM_BTN_VALUE "value"


MFormAbout::MFormAbout(QWidget *parent)
    : QDialog(parent)
{

	setWindowFlags(Qt::Dialog
					| Qt::WindowTitleHint
					| Qt::WindowStaysOnTopHint);

	//setWindowModality(Qt::ApplicationModal);
	m_bTemplateLoaded = false;
	m_bShowGeneric = false;
	m_frExecute = NULL;
	m_btnExec = NULL;
	m_txtExecuteLine = NULL;

	if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);
}

MFormAbout::~MFormAbout()
{

}


bool MFormAbout::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormAbout::loadTemplate: sUIFilePath:%1").arg(sUIFilePath));
	
	QFile file(sUIFilePath);
	if(file.open(QFile::ReadOnly))
	{
		m_dynFormWidget = loader.load(&file, this);
		file.close();

		QVBoxLayout * layout = new QVBoxLayout(this);
		layout->addWidget(m_dynFormWidget);
		
		layout->setSpacing(0);
		this->setContentsMargins(0,0,0,0);
		layout->setContentsMargins(1,1,1,1);

		m_btnClose = this->findChild<QPushButton*>("btnOK");  //qFindChild<QPushButton*>(this, "btnOK");
 		m_frButtonsFrame = this->findChild<QFrame*>("frButtonsFrame"); //qFindChild<QFrame*>(this, "frButtonsFrame");
		m_btnExec = this->findChild<QPushButton*>("btnExec");
 		m_txtExecuteLine = this->findChild<QLineEdit*>("txtExecuteLine");
		m_frExecute = this->findChild<QFrame*>("frExecute");


		if(m_btnClose!=NULL && 
				m_frButtonsFrame != NULL)
		{	

			QObject::connect(m_btnClose,SIGNAL(clicked()), this, SLOT(onButtonClose()));
			
			this->setWindowTitle(m_dynFormWidget->windowTitle());
			m_bTemplateLoaded = true;
			bRetVal = true;
		}


		if(m_txtExecuteLine != NULL && m_btnExec!=NULL && m_bShowGeneric)
		{
			QObject::connect(m_txtExecuteLine,SIGNAL(returnPressed()), this, SLOT(onButtonExecute()));
			QObject::connect(m_btnExec,SIGNAL(clicked()), this, SLOT(onButtonExecute()));

			m_txtExecuteLine->setText(getLastCommand());
		}
		else
		{
			if(m_txtExecuteLine != NULL) 
				m_txtExecuteLine->hide();
			if(m_btnExec != NULL) 
				m_btnExec->hide();
			if(m_frExecute != NULL) 
				m_frExecute->hide();
		}
	}
	
	if(!bRetVal)
	{
		TRACE_W(QString("MFormInput::loadTemplate: Error loading %1").arg(sUIFilePath));
		
	}

	return bRetVal;
}




bool MFormAbout::initialize(QString sCustomButtonsCfgFile)
{
	bool bOK = false;

	if(sCustomButtonsCfgFile!="")
 		bOK = loadOptionsFromFile(sCustomButtonsCfgFile);
	else
		bOK = loadOptionsFromFile(MIhmConfigGeneral::getCfg()->m_sAboutDlgDefaultCfg);
	
	if(bOK)
	{
		if(m_sInputTemplate.isEmpty())
			m_sInputTemplate = MIhmConfigGeneral::getCfg()->m_sAboutDlgTemplate;
		
		if(m_sInputTemplate!="")
		{
			QString sTemplatePath = MIhmConfigGeneral::getCfg()->getUIFullPath(m_sInputTemplate);
			
			if(loadTemplate(sTemplatePath))
			{
				initCustomButtons();
				
				return true;
			}
			else
				TRACE_W(QString("MFormAbout::initialize: template not loaded!"));
		}
		else
		{
			TRACE_W(QString("MFormAbout::initialize: No UI template file defined in AboutDialogTemplate!"));
		}
	}
	else
	{
		TRACE_W(QString("MFormAbout::loadOptionsFromFile: Error while loading options from file!"));
	}

	return false;
}


bool MFormAbout::loadOptionsFromFile(QString sSourceFile)
{
	QString sFullPath = MIhmConfigGeneral::getCfg()->getCommFileFullPath(sSourceFile);
	QString sXml = MHelpFuncs::loadFileContent(sFullPath);
	
	TRACE_D(QString( "MFormAbout::loadOptionsFromFile: ...."));
	
	if(sXml=="")
	{
		TRACE_W(QString( "MFormAbout::loadOptionsFromFile: Unable to load options list file %1").arg(sFullPath));
		return false;
	}
	
	QDomDocument m_xmlData;
	QString errorStr;
	int errorLine;
	int errorColumn;
	
	bool bRet = m_xmlData.setContent(sXml, &errorStr, &errorLine, &errorColumn);
	
	if(bRet)
	{
		MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
		QDomElement root = m_xmlData.documentElement();
		if(root.tagName() != XML_CUSTOM_BTN_ROOT)
		{
			TRACE_W(QString( "MFormAbout::loadOptionsFromFile:Invalid root element %1 for file %2!").
				arg(XML_CUSTOM_BTN_ROOT).
				arg(sFullPath));
			return false;
		}
		
		m_sInputTemplate = MHelpFuncs::getAttributeText(&root, XML_CUSTOM_BTN_UI);
		QString sTemp = MHelpFuncs::getAttributeText(&root, XML_CUSTOM_BTN_SHOW_GENERIC, true);
		
		if(sTemp == "1")
			m_bShowGeneric = true;
		else 
			m_bShowGeneric = false;

		//add options received in the description
		QString sValue,sText;
		QDomNodeList lstItems = root.childNodes();
		QDomNode currItem;
		
		for(int i=0;i<lstItems.count();i++)
		{
			currItem = lstItems.at(i); 
			if(currItem.nodeName()==XML_CUSTOM_BTN_OPTION)
			{
				sText = MHelpFuncs::getAttributeText(&currItem, XML_CUSTOM_BTN_TEXT);
				sValue = MHelpFuncs::getAttributeText(&currItem, XML_CUSTOM_BTN_VALUE);

				CustomButton *pBtnData = new CustomButton;
				QString sTranslation;
				pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetDesktop,sTranslation);

				pBtnData->m_sCaption = sTranslation;
				pBtnData->m_sActionId = sValue;
				m_lstCustomButtons.append(pBtnData);
			
			}
		}
		
		
	}
	else
	{
		QString sMsg = QString("MInputExField::loadOptionsFromFile:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
		TRACE_W(sMsg);
		return false;
	}	
	
	return true;
}		


void MFormAbout::initCustomButtons()
{
	
		if(m_lstCustomButtons.size()>0)
		{
			m_Layout = new QVBoxLayout(m_frButtonsFrame);
		}	
			
		CustomButton *pCurrent;

		for(int i = 0; i<m_lstCustomButtons.size(); ++i)
		{
			pCurrent = m_lstCustomButtons.at(i);

			QPushButton *pPushButton = new QPushButton();
			pPushButton->setText(pCurrent->m_sCaption);
			pCurrent->m_pWidget = pPushButton;
			pPushButton->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
			QObject::connect(pPushButton,SIGNAL(clicked()), this, SLOT(onCustomButtonClicked()));

			m_Layout->addWidget(pPushButton);
		}

}


void MFormAbout::keyPressEvent (QKeyEvent * event)
{
	int iKey = event->key();

	if(iKey == Qt::Key_Escape)
	{
		onButtonClose();
	}
	else if(iKey == Qt::Key_Up)
	{
		//move focus up
		//moveFocus(true);
	}
	else if(iKey == Qt::Key_Down)
	{
		//move focus down
		//moveFocus(false);
	}
	else
		QWidget::keyPressEvent(event);
}



void MFormAbout::onButtonExecute()
{
	if(m_txtExecuteLine!=NULL)
	{
		QString sProgram =  m_txtExecuteLine->text();
		
		//save to registry
		setLastCommand(sProgram);

		//execute the generic action
		emit action("GENERICEXECUTE",sProgram); //predefined action 
	}
}




void MFormAbout::onButtonClose()
{
	reject();
}

void MFormAbout::onCustomButtonClicked()
{
    QWidget *src = qobject_cast<QWidget *>(sender());	
	CustomButton *pCurrent;

	for(int i = 0; i<m_lstCustomButtons.size(); ++i)
	{
		pCurrent = m_lstCustomButtons.at(i);

		if(	pCurrent->m_pWidget == src)
		{
			QString sProgram = pCurrent->m_sActionId;
			emit action(sProgram,"");
			break;
		}

	}
}


QString MFormAbout::getLastCommand()
{
	QString sLastCommand;
    char pcRegKey[IHM_LG_LIGNE_MAX];
	char szTemp[IHM_LG_LIGNE_MAX];
	DWORD dwValeurLen;

	QString sRegKeyActions = MIhmConfigGeneral::getCfg()->getModuleConfigKey() + REG_VAL_CUSTOM_BUTTONS;

	strcpy_s(pcRegKey, sizeof(pcRegKey), sRegKeyActions.toLatin1().data());

	dwValeurLen = sizeof (szTemp) ;
	if ((REG_Lire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)REG_VAL_CUSTOM_BUTTONS_LAST_CMD, 
                        szTemp, 
                        &dwValeurLen )) != ERROR_SUCCESS)
	{
        TRACE_W(QString("MFormAbout::getLastCommand: Error key %1[%2]").arg(pcRegKey).arg(REG_VAL_CUSTOM_BUTTONS_LAST_CMD));
	}
	else
		sLastCommand = szTemp;

	return sLastCommand;

}

bool MFormAbout::setLastCommand(QString sCmd)
{
    char pcRegKey[IHM_LG_LIGNE_MAX];
	char pcValeur[IHM_LG_LIGNE_MAX];

	QString sRegKeyActions = MIhmConfigGeneral::getCfg()->getModuleConfigKey() + REG_VAL_CUSTOM_BUTTONS;
	strcpy_s(pcRegKey, sizeof(pcRegKey), sRegKeyActions.toLatin1().data());

	strcpy_s(pcValeur, sizeof(pcValeur), sCmd.toLatin1().data());

	if ((REG_Ecrire_Chaine (
                        CSR_REG_KEYi_ROOT, 
                        pcRegKey,
                        (char*)REG_VAL_CUSTOM_BUTTONS_LAST_CMD, 
                        pcValeur)) != ERROR_SUCCESS)
	{
        TRACE_W(QString("MFormAbout::getLastCommand: Error writing key %1[%2]").arg(pcRegKey).arg(REG_VAL_CUSTOM_BUTTONS_LAST_CMD));
		return false;
	}

	return true;
}

