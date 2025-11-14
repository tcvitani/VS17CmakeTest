#include <QtGui>
#include <QtUiTools>
#include <QDialogButtonBox>

#include "MIhmConfigGeneral.h"
#include "MFormInput.h"
#include "MInputDialogReq.h"
#include "MIhmLanguages.h"
#include "MCtlEdit.h"
#include "MCtlList.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

 

MFormInput::MFormInput(QWidget *pParentWindow)
    : QWidget(pParentWindow)
{

	setWindowFlags(Qt::Window
					| Qt::CustomizeWindowHint);

	m_bTemplateLoaded = false;
	ui_lblTitle = NULL;
	m_sResult = "";
	m_gridLayout = NULL;
	ui_buttonBox = NULL;
	m_pCtlList = NULL;
	ui_buttonOK = NULL;
	ui_buttonCancel = NULL;
	ui_buttonBox = NULL;

	m_pParentWindow = pParentWindow;

	m_bInitialized = false;

	if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);
}


MFormInput::~MFormInput()
{

	if(m_pCtlList!=NULL)
	{
		delete m_pCtlList;
	}
	

	qDeleteAll(m_lstEditCtl);
}


void MFormInput::focusFirstInput()
{
//if in control set the focus to the first available input control
	if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_FIELD)
	{
		MCtlEdit * pCurrent;
		bool bExit = false;
		//init edit controls
		for(int i = 0; i<m_lstEditCtl.size(); ++i)
		{
			pCurrent = m_lstEditCtl.at(i);

			int iNumItems = pCurrent->getNumItems();

			for(int j = 0; j<iNumItems; j++)
			{
				QLineEdit * pEdit = (QLineEdit *)pCurrent->getItemWidget(0);
				if(pEdit->isEnabled()&&!pEdit->isReadOnly())
				{
					pEdit->setFocus();
					bExit = true;
					break;
				}
			}
			
			if(bExit)
				break;
		}
	}
	else if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_CHOICE ||
		m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_BIGCHOICE)
	{
		if(m_pCtlList!=NULL)
		{
			QListWidget * pLst = m_pCtlList->getListWidget();
			
			if (pLst!=NULL)
				if(pLst->isEnabled())
				{
					pLst->setFocus();
				}
		}
	}
	else
		this->setFocus();
}




void MFormInput::show()
{
	//Show only if template correctly loaded
    if(m_bTemplateLoaded)
	{
		//dialog alignment
		if(!m_oReq.isEmbeded())
			initFormAlignment(m_oReq.getAlignment());
		else
		{
			QWidget * pEmbedingWidget = m_pParentWindow->findChild<QWidget *>(m_oReq.getRelativeToObjectName());
			
			if(pEmbedingWidget!=NULL)
			{	
				QPoint ptWinPos = m_pParentWindow->mapToGlobal(pEmbedingWidget->pos());
				QSize siSize = pEmbedingWidget->size();
				this->setMaximumSize(siSize);
				this->setGeometry(QRect(ptWinPos,siSize));
			}
			else
			{
				TRACE_W(QString("MFormInput::show:Error unable to find visible object named:%1!")
									.arg(m_oReq.getRelativeToObjectName()));

			}
		}

		QWidget::show();
		focusFirstInput();
	}
	else
		TRACE_W(QString("MFormInput::show:Error template not loaded!"));
}


bool MFormInput::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormInput::loadTemplate: sUIFilePath:%1").arg(sUIFilePath));
	
	QFile file(sUIFilePath);
	if(file.open(QFile::ReadOnly))
	{
		m_dynFormWidget = loader.load(&file, this);
		file.close();
		QPalette pal1 = m_dynFormWidget->palette();
		this->setPalette(pal1);
		this->setAttribute(Qt::WA_WindowPropagation, false);

		ui_FormInput = this->findChild<QWidget*>("MFormInput");
		ui_FormInput->setPalette(pal1);

		m_layout = new QVBoxLayout(this);
		m_layout->addWidget(m_dynFormWidget);
		
		m_layout->setSpacing(0);
		this->setContentsMargins(0,0,0,0);
		m_layout->setContentsMargins(1,1,1,1);

		//find the widgets by name
		ui_buttonBox = this->findChild<QDialogButtonBox*>("buttonBox");
		
		if(ui_buttonBox == NULL)
		{
			ui_buttonOK = this->findChild<QPushButton*>("btnOK");
			ui_buttonCancel = this->findChild<QPushButton*>("btnCancel");

		}
		else
		{
			ui_buttonOK = ui_buttonBox->button(QDialogButtonBox::Ok);
			ui_buttonCancel = ui_buttonBox->button(QDialogButtonBox::Cancel);
		}

		ui_lblTitle = this->findChild<QLabel*>("lblTitle");

		ui_pfrInputContainer = this->findChild<QFrame*>("frInputFrame");
		ui_pfrBody = this->findChild<QFrame*>("frBody");
		ui_pfrTitle = this->findChild<QFrame*>("frTitle");

 		if(ui_buttonOK!=NULL && 
				ui_buttonCancel!=NULL && 
				ui_lblTitle != NULL &&
				ui_pfrBody != NULL &&
				ui_pfrTitle != NULL &&
				ui_pfrInputContainer != NULL)
		{	
			m_iInitialWidth = m_dynFormWidget->width();
			m_iInitialHeight = m_dynFormWidget->height();
			m_iInitialInputHeight = m_iInitialHeight - ui_lblTitle->height() - ui_buttonOK->height();

			QObject::connect(ui_buttonOK,SIGNAL(clicked()), this, SLOT(onButtonOK()));
			QObject::connect(ui_buttonCancel,SIGNAL(clicked()), this, SLOT(onButtonCancel()));

			m_bTemplateLoaded = true;
			bRetVal = true;
		}

	}
	
	if(!bRetVal)
	{
		TRACE_W(QString("MFormInput::loadTemplate: Error loading %1").arg(sUIFilePath));
	}

	return bRetVal;
}


void MFormInput::onButtonOK()
{
	if(doValidate())
		emit accepted();
}

void MFormInput::onButtonCancel()
{
	if(doCancel())
		emit rejected();
}



bool MFormInput::initialize(MInputDialogReq * pReq)
{	//keep the pointer to 
	m_oReq = *pReq;

	TRACE_D(QString("MFormInput::initialize: Def:%1 Desc:%2")
					.arg(m_oReq.getDefinition())
					.arg(m_oReq.getDescription()));

	QString sInputTemplate = pReq->getUITemplate();
		
	if(sInputTemplate == "" )
		sInputTemplate = MIhmConfigGeneral::getCfg()->m_sDefaultInputTemplate;

	QString sTemplatePath = MIhmConfigGeneral::getCfg()->getUIFullPath(sInputTemplate);

	if(!loadTemplate(sTemplatePath))
	{
		TRACE_W(QString("MFormInput::initialize: template not loaded!"));
		return false;
	}
	
	switch(m_oReq.getDialogInputType())
	{
		case MInputDialogReq::enuIHMSAISIE_VISUSEUL:
			m_bProtected = true;
			m_bExternalKeyCatch = false;
			m_bAllExternalDevices = false;
			break;
		case MInputDialogReq::enuIHMSAISIE_SAISIE:
			m_bProtected = false;
			m_bExternalKeyCatch = false;
			m_bAllExternalDevices = false;
			break;
		case MInputDialogReq::enuIHMSAISIE_MARQUEUR_TOUS:
			m_bProtected = false;
			m_bExternalKeyCatch = true;
			m_bAllExternalDevices = true;
			break;
		case MInputDialogReq::enuIHMSAISIE_MARQUEUR_ACTIF:
			m_bProtected = false;
			m_bExternalKeyCatch = true;
			m_bAllExternalDevices = false;
			break;
		default:
			break;
	}
	
	m_bOkButton = m_oReq.hasOKButton();
	m_bCancelButton = m_oReq.hasCancelButton();

	if(!m_bOkButton&&!m_bCancelButton)
	{
		TRACE_W(QString("MFormInput::initialize: No buttons defined!Closing the input dialog."));
		return false;
	}


	QString sDefinition = m_oReq.getDefinition();

	QString sDlgTitle = m_oReq.getDialogTitle();
	MIhmConfigGeneral::getCfg()->getLanguages()->getLabelTranslation(sDlgTitle, MIhmLanguages::enuTranslTargetDesktop, sDlgTitle);
	
	ui_lblTitle->setText(sDlgTitle);

	m_iNumFields = m_oReq.getNumInputFileds();
	ui_lblTitle->setWordWrap(true);
	ui_lblTitle->setFrameShape((m_iNumFields==0)?QFrame::Box : QFrame::NoFrame);

	QFont fntTitle = ui_lblTitle->font();
	QFontMetrics fm(fntTitle);
	int textHeightInPixels = fm.height();
	int iNormalFontSize = textHeightInPixels*0.8;
	
	
	//extract colors
	initFormColors(m_oReq.getColorAttributes());

			
	if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_FIELD || 
			m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM)
	{

		//init edit controls
		for(int i = 0; i<m_iNumFields; i++)
		{
			QString sFieldDef = m_oReq.getFieldDefinition(i);
			QString sFieldDesc = m_oReq.getFieldDescription(i);
			
			MCtlEdit * pNewInput = new MCtlEdit(ui_pfrInputContainer);
			pNewInput->setIsAlarm(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM);


			if(!pNewInput->setDefinition(sFieldDef))
			{
				TRACE_W(QString("MFormInput::initialize: setDefinition failed for field:%1 Definition::%2 Description:%3").arg(i)
									.arg(sFieldDef)
									.arg(sFieldDesc));
				return false;
			}

			if(!pNewInput->setDescription(sFieldDesc))
			{
				TRACE_W(QString("MFormInput::initialize: setDescription failed for field:%1 Definition::%2 Description:%3")
									.arg(i)
									.arg(sFieldDef)
									.arg(sFieldDesc));
				return false;
			}

			pNewInput->setFont(fntTitle);
			pNewInput->setFontSize(iNormalFontSize);

			//Init control colors if default template form is used
			if(pReq->getUITemplate()=="")
			{
				if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM)
					pNewInput->setColors(m_sCol3,m_sCol3,m_sCol3,m_sCol3,m_sCol4,m_sCol4);
				else
					pNewInput->setColors(m_sCol3,"","",m_sCol3,m_sCol4,"");
			}
			
			m_lstEditCtl.append(pNewInput);

		}


	}
	else if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_CHOICE ||
		m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_BIGCHOICE)

	{
		//init list control
		QString sCaption = m_oReq.getFieldDefinition(0);

		m_pCtlList = new MCtlList(ui_pfrInputContainer);
		m_pCtlList->setCaption(sCaption);
		m_pCtlList->setFont(fntTitle);
		m_pCtlList->setFontSize(iNormalFontSize);

		if(pReq->getUITemplate()=="")
		{
			m_pCtlList->setColors(m_sCol3,"","","");
		}
		
		for(int i = 0; i<m_iNumFields; i++)
		{
			QString sFieldDesc = m_oReq.getFieldDescription(i);
			m_pCtlList->addListItemDesc(sFieldDesc);
		}

	}


	//if default input template is used update the dialog colors with the 
	// color attributes defined in registry with
	if(pReq->getUITemplate()=="")
	{
		updateFormColors();
	}
	

	arrangeButtons();

	//to arrange dynamically created controls to the template form  container
	//and resize the form correctly
	arrangeControls(); 
	
	//to handle the calculations 
	installCalculationHandler();

	m_bInitialized = true;
	return true;
}

void MFormInput::installCalculationHandler()
{
	//first verify if there is any calculations needed
	bool bCalculationNeeded = false;

	MCtlEdit * pCurrent;
	//init edit controls
	for(int i = 0; i<m_lstEditCtl.size(); ++i)
	{
		pCurrent = m_lstEditCtl.at(i);
		if(pCurrent->getFormatType()== MCtlEdit::enuFORMAT_TYPE_FUNCTION)
		{
			bCalculationNeeded = true;
			break;
		}
	}

	
	if(bCalculationNeeded)
	{
		for(int i = 0; i<m_lstEditCtl.size(); ++i)
		{
			pCurrent = m_lstEditCtl.at(i);

			if(pCurrent->getFormatType()!= MCtlEdit::enuFORMAT_TYPE_FUNCTION)
			{
				int iNumItems = pCurrent->getNumItems();

				for(int j = 0; j<iNumItems; j++)
				{
					MLineEdit * pEdit = (MLineEdit *)pCurrent->getItemWidget(j);
					connect((QObject*)pEdit,SIGNAL(textChanged(const QString &)),this,SLOT(onLineEditTextChanged(const QString &)));
				}
			}
		}
	}
}

bool MFormInput::getItemPos(MLineEdit * pEdit, int *piRow, int *piCol)
{
	*piRow = -1;
	*piCol = -1;

	MCtlEdit * pCurrent;
	for(int i = 0; i<m_lstEditCtl.size(); ++i)
	{
		pCurrent = m_lstEditCtl.at(i);

		int iNumItems = pCurrent->getNumItems();

		for(int j = 0; j<iNumItems; j++)
		{
			MLineEdit * pCurrEdit = (MLineEdit *)pCurrent->getItemWidget(j);

			if(pEdit == pCurrEdit)
			{
				*piRow = i;
				*piCol = j;
				return true;
			}
		}
		
	}

	return false;

}




void MFormInput::onLineEditTextChanged(const QString &sNewText)
{
	Q_UNUSED(sNewText)
	int iRow,iCol;
	MLineEdit * pEdit = (MLineEdit *)(qobject_cast<QWidget *>(sender()));

	if(getItemPos(pEdit, &iRow, &iCol))
	{
		updateCalculationsInCol(iCol);
	}

}

void MFormInput::updateCalculationsInCol(int iCol)
{
	MCtlEdit * pCurrent;
	QStringList lstColData;
	
	for(int i = 0; i<m_lstEditCtl.size(); ++i)
	{
		lstColData.append("");

		pCurrent = m_lstEditCtl.at(i);
		
		if(pCurrent!=NULL)
			if(pCurrent->getFormatType()!= MCtlEdit::enuFORMAT_TYPE_FUNCTION)
			{
				lstColData.replace(i, pCurrent->getItemText(iCol));
			}

	}

	for(int i = 0; i<m_lstEditCtl.size(); ++i)
	{
		pCurrent = m_lstEditCtl.at(i);
				
		if(pCurrent->getFormatType()== MCtlEdit::enuFORMAT_TYPE_FUNCTION)
		{
			pCurrent->updateCalculation(&lstColData, iCol);
		}
	}
}





void MFormInput::updateFormColors()
{
	QString sStyle = MIhmConfigColor::createBkgColorStyle(m_sCol1);
	sStyle += MIhmConfigColor::createColorStyle(m_sCol2);

	if(sStyle!="")
	{
		ui_lblTitle->setStyleSheet(sStyle);
		ui_pfrTitle->setStyleSheet(sStyle);
	}

	sStyle = MIhmConfigColor::createBkgColorStyle(m_sCol3);
	sStyle += MIhmConfigColor::createColorStyle(m_sCol4);

	if(sStyle!="")
	{
		this->setStyleSheet(sStyle);
	}

	QString sStyleFont;
	QFontInfo fntInfo = ui_lblTitle->font();
	sStyleFont = QString("font:%1 %2pt \"%3\";").arg(fntInfo.weight()).arg(fntInfo.pointSize()).arg(fntInfo.family()); 
	
	ui_buttonCancel->setStyleSheet(sStyleFont);
	ui_buttonOK->setStyleSheet(sStyleFont);

}

void MFormInput::arrangeButtons()
{
	//arrange buttons

	if(ui_buttonBox!=NULL)
	{
		if((!m_bOkButton && m_bCancelButton)||
			(m_bOkButton && !m_bCancelButton))
		{
			ui_buttonBox->setCenterButtons(true);
		}
		else
			ui_buttonBox->setCenterButtons(false);

	}
	//else
		//do not center buttons 

	ui_buttonOK->setVisible(m_bOkButton);
	ui_buttonCancel->setVisible(m_bCancelButton);
	
	MIhmLanguages *	pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	ui_buttonOK->setText(pLang->getOKButtonTranslation(MIhmLanguages::enuTranslTargetDesktop));
	ui_buttonCancel->setText(pLang->getCancelButtonTranslation(MIhmLanguages::enuTranslTargetDesktop));

}


void MFormInput::updateControlsReadOnlyState(bool m_bReadOnly)
{
	TRACE_D(QString("MFormInput::updateControlsReadOnlyState ..."));
	
	if(ui_buttonBox == NULL)
	{
		if(ui_buttonOK!=NULL)
			ui_buttonOK->setEnabled(!m_bReadOnly);
		if(ui_buttonCancel!=NULL)
			ui_buttonCancel->setEnabled(!m_bReadOnly);
	}
	else
	{
		ui_buttonBox->setEnabled(!m_bReadOnly);
	}

	if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM ||
		m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_FIELD)
	{
		MCtlEdit * pCurrent;
		//init edit controls
		for(int i = 0; i<m_lstEditCtl.size(); ++i)
		{
			pCurrent = m_lstEditCtl.at(i);
			
			int iNumItems = pCurrent->getNumItems();

			for(int j = 0; j<iNumItems; j++)
			{
				QLineEdit * pEdit = (QLineEdit *)pCurrent->getItemWidget(j);
				pEdit->setEnabled(!m_bReadOnly);
			}
		}
	}
	else if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_CHOICE ||
		m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_BIGCHOICE)
	{
		if(m_pCtlList!=NULL)
		{
			QListWidget *pLst = m_pCtlList->getListWidget();

			if(pLst!=NULL)
				pLst->setEnabled(!m_bReadOnly);
		}
	}
}



void MFormInput::arrangeControls()
{
	if(m_gridLayout!=NULL)
		delete m_gridLayout;

	m_gridLayout = new QGridLayout(ui_pfrInputContainer);
	m_gridLayout->setVerticalSpacing(2);

	int iVertSpacing = m_gridLayout->verticalSpacing();
	int iInputLineHeightSum = iVertSpacing;
	int iMaxWidth = 0;

	if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM ||
		m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_FIELD)
	{
		MCtlEdit * pCurrent;
		//init edit controls
		for(int i = 0; i<m_lstEditCtl.size(); ++i)
		{
			pCurrent = m_lstEditCtl.at(i);
			m_gridLayout->addWidget(pCurrent->getLabelWidget(), i,0);
			
			iInputLineHeightSum = iInputLineHeightSum + iVertSpacing + pCurrent->getLineHeight();
			iMaxWidth = qMax(iMaxWidth, pCurrent->getMaxWidth());
			
			int iNumItems = pCurrent->getNumItems();

			for(int j = 0; j<iNumItems; j++)
			{
				QLineEdit * pEdit = (QLineEdit *)pCurrent->getItemWidget(j);
				
				//in case of alarm hide the line edit border
				if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM && pEdit!=NULL)
						pEdit->setFrame(false);

				m_gridLayout->addWidget(pEdit, i,j+1);
				pEdit->installEventFilter(m_pParentWindow);

			}
		}
	}
	else if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_CHOICE ||
		m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_BIGCHOICE)
	{
		if(m_pCtlList!=NULL)
		{
			m_gridLayout->addWidget(m_pCtlList->getLabelWidget(), 1,0);
			m_gridLayout->addWidget(m_pCtlList->getListWidget(), 2,0);

			m_pCtlList->getListWidget()->setFocusPolicy(Qt::StrongFocus);
			m_pCtlList->getListWidget()->setFocus();

			iInputLineHeightSum = m_pCtlList->getHeight() + iVertSpacing * 2;
			iMaxWidth = m_pCtlList->getWidth();
			m_pCtlList->getListWidget()->installEventFilter(m_pParentWindow);
		}
	}
	

	//Adjust focus policy ...
	ui_buttonOK->clearFocus();
	ui_buttonCancel->clearFocus();
	ui_buttonOK->setFocusPolicy(Qt::NoFocus);
	ui_buttonCancel->setFocusPolicy(Qt::NoFocus);
			
	if(ui_buttonBox != NULL)
	{
		ui_buttonBox->clearFocus();
		ui_buttonBox->setFocusPolicy(Qt::NoFocus);
	}


	if(!m_oReq.isEmbeded())	
	{
		//Adjust the dialog size ....
		int iDialogHeight; 
		
		if(m_iNumFields>0)
		{
			if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_CHOICE ||
			m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_BIGCHOICE)
			{
				iDialogHeight = ui_lblTitle->height() + ui_buttonOK->height() 
								//+ m_iInitialInputHeight
								+ iInputLineHeightSum;

			}
			else
				iDialogHeight = ui_lblTitle->height() + ui_buttonOK->height() 
								+ (m_iInitialInputHeight - 1 * (iInputLineHeightSum / m_iNumFields))
								+ iInputLineHeightSum;
			
		}
		else
			iDialogHeight = m_iInitialHeight;

		int iDialogWidth = qMax(iMaxWidth, m_iInitialWidth);
		
		//assure to be smaller than main window
		iDialogHeight = qMin(iDialogHeight,parentWidget()->height());
		iDialogWidth = qMin(iDialogWidth,parentWidget()->width());

		this->resize(iDialogWidth,iDialogHeight);
	}
}





void MFormInput::initFormColors(QString sColorAttributes)
{
	
	if(sColorAttributes!="")	
	{
		m_sCol1 = MHelpFuncs::getStringItem(sColorAttributes,0,QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), true);
		m_sCol2 = MHelpFuncs::getStringItem(sColorAttributes,1,QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), true);
		m_sCol3 = MHelpFuncs::getStringItem(sColorAttributes,2,QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), true);
		m_sCol4 = MHelpFuncs::getStringItem(sColorAttributes,3,QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), true);
	}

}




bool MFormInput::initFormAlignment(Qt::Alignment eAlign)
{
	int x,y;

	QWidget *parent = parentWidget();
	
	if(parent==NULL)
	{
		return false;
	}
	
	int iWidth, iHeight;

	iWidth = m_dynFormWidget->width();
	iHeight = m_dynFormWidget->height();

	if(eAlign.testFlag(Qt::AlignLeft))
        x = parentWidget()->x();
	else if(eAlign.testFlag(Qt::AlignRight))
        x = parentWidget()->x() + (parentWidget()->width() - iWidth);
	else //(eAlign & Qt::AlignHCenter != 0)
        x = parentWidget()->x() + ((parentWidget()->width() - iWidth)/2);
	
    if(eAlign.testFlag(Qt::AlignTop))
        y = parentWidget()->y();
	else if(eAlign.testFlag(Qt::AlignBottom))
        y = parentWidget()->y() + (parentWidget()->height() - iHeight);
	else //(eAlign & Qt::AlignVCenter != 0)
        y = parentWidget()->y() + ((parentWidget()->height() - iHeight)/2);
 
	this->move(x,y);

	return true; 
}



bool MFormInput::doValidate()
{
	QString sRes;

	if(m_bOkButton)
	{
		if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM ||
			m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_FIELD)
		{
			
			MCtlEdit * pCurrent;

			sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
			sRes += QString("%1").arg(m_oReq.getDialogId()); //                         gszId + _
			sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
			sRes += QString("%1").arg(m_iNumFields);
			sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
			//read the data from the input controls
			for(int i = 0; i<m_lstEditCtl.size(); ++i)
			{
				pCurrent = m_lstEditCtl.at(i);

				if(!pCurrent->isInputValid())
				{
					TRACE_D(QString("MFormInput::doValidate: The value in CtlEdit(%1) is too short!")
									.arg(i));
					return false;
				}

				sRes += pCurrent->getResult();
				sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
			}
			
			m_sResult = sRes;
		}
		else if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_CHOICE ||
			m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_BIGCHOICE)
		{
			if(m_pCtlList!=NULL)
			{
				QString sText; 
				QString sData;

				if(!m_pCtlList->getSelection(sText, sData))
				{
					TRACE_D(QString("MFormInput::doValidate: getSelection returned false!"));

					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
					sRes += QString("%1").arg(m_oReq.getDialogId()); //                         gszId + _
					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
					sRes += QString("0");					//                                 "0" + _
					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	

				}
				else
				{
					TRACE_D(QString("MFormInput::doValidate: getSelection returned Data:%1 Text:%2!")
									.arg(sData)
									.arg(sText));

					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
					sRes += QString("%1").arg(m_oReq.getDialogId()); //                         gszId + _
					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
					sRes += QString("1") ;					//                                 "1" + _
					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	
					sRes += sData;				                         
					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM); 	
					sRes += sText;				                         
					sRes += QString("%1").arg(MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP); 	

					//							MInputDialogReq::cSAISIE_SEPARATEUR_CHAMP;	
					//                          szData + _
					//                          SAISIE_SEPARATEUR_PARAM + _
					//                          szText + _
					//                          SAISIE_SEPARATEUR_CHAMP
				}

				m_sResult = sRes;
			}
		}
	

	}

	
	return m_bOkButton;
}


bool MFormInput::doCancel()
{
	if(m_bCancelButton)
	{
		m_sResult = m_oReq.getDescription();
	}

	return m_bCancelButton;
}


void MFormInput::keyPressEvent (QKeyEvent * event)
{
	int iKey = event->key();

	if(iKey == Qt::Key_Enter|| iKey == Qt::Key_Return)
	{
		if(doValidate())
			emit accepted();
	}
	else if(iKey == Qt::Key_Escape)
	{
		if(doCancel())
			emit rejected();

	}
	else if(iKey == Qt::Key_Up)
	{
		//move focus up
		moveFocus(true);
	}
	else if(iKey == Qt::Key_Down)
	{
		//move focus down
		moveFocus(false);
	}
	else
		QWidget::keyPressEvent(event);
}


void MFormInput::moveFocus(bool bUp)
{
	if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM ||
			m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_FIELD)
	{
		if(m_iNumFields>1)
		{
			int iCurrentIndex = -1;
			int iCurrentSubItem = -1;

			MCtlEdit * pCurrent;

			for(int i = 0; i<m_lstEditCtl.size(); ++i)
			{
				pCurrent = m_lstEditCtl.at(i);

				int iNumItems = pCurrent->getNumItems();

				for(int j = 0; j<iNumItems; j++)
				{
					QLineEdit * pEdit = (QLineEdit *)pCurrent->getItemWidget(j);

					if(pEdit == focusWidget())
					{
						iCurrentIndex = i;
						iCurrentSubItem = j;
						break;
					}
				}

				if(iCurrentIndex>0 && iCurrentSubItem>0)
					break;
			}

			if(iCurrentIndex>=0 && iCurrentSubItem>=0)
			{
				if(bUp)
				{
					iCurrentIndex = iCurrentIndex - 1;
					if(iCurrentIndex < 0)
						iCurrentIndex = m_iNumFields - 1;
					
				}
				else //down
				{
					iCurrentIndex = iCurrentIndex + 1;
					if(iCurrentIndex > m_iNumFields - 1 )
						iCurrentIndex = 0;

				}
				
				pCurrent = m_lstEditCtl.at(iCurrentIndex);
				if(pCurrent!=NULL)
				{
					QLineEdit * pEdit = (QLineEdit *)pCurrent->getItemWidget(iCurrentSubItem);
					if(pEdit!=NULL)
					{
						pEdit->setFocus();
					}
				}

			}
		}
	}
}