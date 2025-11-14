
#include <MCtlEdit.h>
#include <QJSEngine>
#include "MLineEdit.h"
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <MInputDialogReq.h>
#include <MHelpFuncs.h>
#include <MIhmConfigGeneral.h>
#include <QRegularExpressionValidator>
#include <MTracer.h>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
}


#define TWIPS_PER_PIXEL 12
#define DEFAULT_LINE_HEIGHT_PIXEL 24



MCtlEdit::MCtlEdit(QWidget *parent)
{
	m_lblCaption = NULL;
	m_bIsAlarm = NULL;
	m_parentWidget = parent;
	
	m_font = m_parentWidget->font();
	m_bPasword = false;
	m_eFormatType = enuFORMAT_TYPE_INVALID;
	m_iNumDecimals = 0;
	m_chDecimalSeparator = '.';
	
//     ChampEdt(lIndex).Visible = True
//     ChampEdt(lIndex).AutoAdvance = True
//     ChampEdt(lIndex).Locked = gbProtected
//     ChampEdt(lIndex).LabelSize = 50
//     ChampEdt(lIndex).HasLed = False

}

MCtlEdit::~MCtlEdit()
{

}

void MCtlEdit::setFont( const QFont & fnt)
{
	m_font = fnt;
	updateFont();
}

void MCtlEdit::setFontSize(int iSize)
{
	if(iSize<=0)
		return;
	
	m_font.setPixelSize(iSize);

	updateFont();
}

void MCtlEdit::updateFont()
{
	if (m_lblCaption!=NULL) 
		m_lblCaption->setFont(m_font);

	MLineEdit *pEdit;

	for(int i=0;i<m_lstLineEdits.size();++i)
	{
		pEdit = m_lstLineEdits.at(i);
		if(pEdit!=NULL)
			pEdit->setFont(m_font);
	}

}


bool MCtlEdit::setDefinition(QString sDef)
{
	bool bOK = true;

//     ' DEFINITION DU CHAMP
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();

//     ' Extraire le nombre d'item ou le libelle
	QString sTemp = MHelpFuncs::getStringItem(sDef,0, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	int iItemsShift;

	if(MHelpFuncs::isInt(sTemp))
	{
		m_iNumItems = sTemp.toInt();
		iItemsShift = 1;
		
		sTemp = MHelpFuncs::getStringItem(sDef,1, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	}
	else
	{
		m_iNumItems = 1;
		iItemsShift = 0;
	}

	if(m_iNumItems==0)
	{
		TRACE_W(QString( "MCtlEdit::setDefinition: Error: m_iNumItems==0!"));
		return false;
	}

	//extract the label text
	if(sTemp == "")
	{
		m_sLabelText = "";
		m_bLocked = false;
	}
	else
	{
		if(sTemp.at(0) == MInputDialogReq::cSAISIE_VEROU)  // is '!' (is locked)
		{
			m_bLocked = true;
			sTemp = sTemp.mid(1);
		}
		else
			m_bLocked = false;

		pLang->getLabelTranslation(sTemp, MIhmLanguages::enuTranslTargetDesktop, m_sLabelText);
	}
	
	//extract the format
	m_sFormat = MHelpFuncs::getStringItem(sDef,1 + iItemsShift , QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);

	//  Extraire la taille min 
	sTemp = MHelpFuncs::getStringItem(sDef,2 + iItemsShift , QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	m_iMinLen = sTemp.toInt(&bOK,10);

	if(!bOK)
	{
		TRACE_W(QString("MCtlEdit::setDefinition: Error decoding field m_iMinLen!"));
		return false;
	}

	//extract the max len in format:" ~9^0.00|"
	sTemp = MHelpFuncs::getStringItem(sDef,3 + iItemsShift , QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	
	QString sMaxLen = MHelpFuncs::getStringItem(sTemp,0 , QString(MInputDialogReq::cSAISIE_RAZ_ZONE_SAISIE), true);
	m_iMaxLen = sMaxLen.toInt(&bOK,10);
	
	if(!bOK)
	{
		TRACE_W(QString("MCtlEdit::setDefinition: Error decoding field m_iMaxLen!"));
		return false;
	}
	
	
	// '  RAZ champ lors du 1er caractere saisie
	QString sDefaultValue = MHelpFuncs::getStringItem(sTemp,1 , QString(MInputDialogReq::cSAISIE_RAZ_ZONE_SAISIE), true);

	if(sDefaultValue=="")
	{
		m_bClearFieldWhenKeyPressedForFirstTime = false;
	}
	else
	{
		m_bClearFieldWhenKeyPressedForFirstTime = true;
		m_sDefaultValue = sDefaultValue;
	}

//     ' On extrait la hauteur en nombre de Twips du champ
//     ' si il n'est pas present on le prend par defaut comme etant une ligne
	sTemp = MHelpFuncs::getStringItem(sDef,4 + iItemsShift , QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	
	if(sTemp=="")
		m_iLineHeight = DEFAULT_LINE_HEIGHT_PIXEL;
	else
		m_iLineHeight = sTemp.toInt(&bOK,10)/TWIPS_PER_PIXEL;

	if(!createControls())
		return false;

	return true;
}


bool MCtlEdit::setFormat(QString sFormat)
{
	char cType;

	if(sFormat.size()<1) 
			return false;

	cType = sFormat.at(0).toUpper().toLatin1();
		
	if(cType == MInputDialogReq::cFORMAT_CHAR_PASSWORD)
	{
		if(sFormat.size()<2) 
				return false;
		
		sFormat = sFormat.mid(1);
		cType = sFormat.at(0).toUpper().toLatin1();
		m_bPasword = true;

		MLineEdit *pEdit;

		for(int i=0;i<m_lstLineEdits.size();++i)
		{
			pEdit = m_lstLineEdits.at(i);
			if(pEdit!=NULL)
			{
				if(m_bPasword)
					pEdit->setEchoMode(QLineEdit::Password);
				else
					pEdit->setEchoMode(QLineEdit::Normal);
			}
		}	
	}

	if(sFormat.size()<1) 
			return false;

	if(cType == MInputDialogReq::cFORMAT_CHAR_STRING)
	{
			if(sFormat.size()!=1) 
							return false;

			m_eFormatType = enuFORMAT_TYPE_STRING;

			QString sReg;
			//matches any character if minimal m_iMinLen and maximal m_iMaxLen
			sReg = QString(".{%1,%2}").arg(m_iMinLen).arg(m_iMaxLen);

			QRegularExpression rx(sReg);
			QRegularExpressionValidator *validator = new QRegularExpressionValidator(rx, m_parentWidget);
			
			MLineEdit *pEdit;

			for(int i=0;i<m_lstLineEdits.size();++i)
			{
				pEdit = m_lstLineEdits.at(i);
				if(pEdit!=NULL)
				{
					pEdit->setValidator(validator);
				}
			}


	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_MASK)
	{
			m_sMask = sFormat.mid(1);

			if(m_sMask!="")
			{
				m_eFormatType = enuFORMAT_TYPE_MASK;

				MLineEdit *pEdit;

				for(int i=0;i<m_lstLineEdits.size();++i)
				{
					pEdit = m_lstLineEdits.at(i);
					if(pEdit!=NULL)
						pEdit->setInputMask(m_sMask);
				}
			}
			else
				return false;
	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_RIGHTMASK)
	{
			m_sMask = sFormat.mid(1);

			if(m_sMask!="")
			{
				m_eFormatType = enuFORMAT_TYPE_RIGHTMASK;

				MLineEdit *pEdit;

				for(int i=0;i<m_lstLineEdits.size();++i)
				{
					pEdit = m_lstLineEdits.at(i);
					if(pEdit!=NULL)
					{
						pEdit->setRightMask(m_sMask,"0");
					}
				}
			}
			else
				return false;
	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_NUMBER)
	{
			if(sFormat.size()<=1) 
								return false;

			m_eFormatType = enuFORMAT_TYPE_NUMBER;

			MLineEdit *pEdit;
			
			for(int i=0;i<m_lstLineEdits.size();++i)
			{
				pEdit = m_lstLineEdits.at(i);
				if(pEdit!=NULL)
				{
					if(!pEdit->setNumberFormat(sFormat))
						return false;
				}
			}

	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_FUNCTION)
	{
			if(sFormat.size()<=1) 
								return false;

			m_eFormatType = enuFORMAT_TYPE_FUNCTION;
			m_cResultNumberFormat ='f';
			m_iResultPrecision = 0;

			QString sTemp = sFormat.mid(1);
			QStringList sLst = sTemp.split('^');

			if(sLst.size()==2)
			{
				sTemp = sLst.at(0);
				
				if(sTemp.size()==2)
				{
					m_cResultNumberFormat = sTemp.at(0);
					m_iResultPrecision = sTemp.at(1).digitValue();
				}

				m_sFormula = sLst.at(1);
			}

			MLineEdit *pEdit;
			
			for(int i=0;i<m_lstLineEdits.size();++i)
			{
				pEdit = m_lstLineEdits.at(i);
				if(pEdit!=NULL)
				{
					pEdit->setEnabled(false);
				}
			}

	}	
	else 
	{
		//default:
		TRACE_W(QString("MCtlEdit::setFormat: Unknown format %1!").arg(sFormat));
	}

	return true;
}

void MCtlEdit::updateCalculation(QStringList * psLst, int iCol)
{
	MLineEdit * pEdit=NULL;
	
	if(m_lstLineEdits.size()>iCol)
	{	
		pEdit = m_lstLineEdits.at(iCol);
	}

	if(pEdit==NULL)
	{
		TRACE_W(QString("MCtlEdit::updateCalculation: Error: Unable to find control in column %1 !").arg(iCol));
		return;
	}

	QString sEval = m_sFormula;
	QString sTemp;

	for(int i = 0; i<psLst->size(); i++)
	{
		sTemp = QString("%%1").arg(i+1);
		sEval = sEval.replace(sTemp, psLst->at(i));
	}

	
	QJSEngine myEngine;
	QJSValue result = myEngine.evaluate(sEval);
	 
	
	if (!myEngine.hasError()) {
		
		QString sResult = result.toString();

		if(m_iResultPrecision>0)
		{
			double d = sResult.toDouble();
			sResult = QString::number(d,m_cResultNumberFormat.toLatin1(),m_iResultPrecision);
			pEdit->setText(sResult);
		}
		else
			pEdit->setText(sResult);
	 }
	else
	{
		pEdit->setText("#ERROR");
		TRACE_D(QString("MCtlEdit::updateCalculation: Error evaluating: %1 !").arg(sEval));
	}
}


int MCtlEdit::getMaxWidth()
{
	int iMaxWidth = 0;
	QString sTemp;
	QFont fnt;
	int labelWidthInPixels;

	if(m_lblCaption!=NULL)
	{
		sTemp = m_lblCaption->text();
		fnt = m_lblCaption->font();
	}
	else
		fnt = QApplication::font();

	QFontMetrics fm(fnt);
	labelWidthInPixels = fm.size(Qt::TextSingleLine, sTemp).width();
	
	int textWidthInPixels;

	if(m_bLocked || m_bIsAlarm)
	{
		int iMaxWidthPx = 0;
		//get exact needed size
		for(int i=0; i<m_iNumItems; i++ )
		{
			sTemp = m_lstLineEdits.at(i)->text() + "M" ;
			iMaxWidthPx = qMax(iMaxWidthPx, fm.size(Qt::TextSingleLine, sTemp).width());
		}		
		
		textWidthInPixels = iMaxWidthPx * m_iNumItems;
	}
	else
	{
		int iMaxCharsPerEdit = qMin(m_iMaxLen,15); 
		sTemp = QString(iMaxCharsPerEdit, QChar('M'));
		textWidthInPixels = fm.size(Qt::TextSingleLine,sTemp).width() * m_iNumItems;
	}


	//the result:
	iMaxWidth += labelWidthInPixels + textWidthInPixels;
	iMaxWidth += (m_iNumItems + 2)*10; //some spacing ...

	return iMaxWidth;
}


bool MCtlEdit::createControls()
{

	if(m_lblCaption!=NULL)
		delete m_lblCaption;

	m_lblCaption = new QLabel(m_parentWidget);
	m_lblCaption->setText(m_sLabelText);
	
	if(m_sLabelText=="")
		m_lblCaption->setVisible(false);
	else	
		m_lblCaption->setVisible(true);

	//Recreate all the line edit controls 
	while(!m_lstLineEdits.isEmpty())
			delete m_lstLineEdits.takeFirst();

	MLineEdit *pEdit;

	for(int i=0;i<m_iNumItems;i++)
	{
		pEdit = new MLineEdit(m_parentWidget);
		m_lstLineEdits.append(pEdit);
		pEdit->setVisible(true);
		pEdit->setReadOnly(m_bLocked);
		if(m_bLocked)
			pEdit->setFocusPolicy(Qt::NoFocus);
	}

	if(!setFormat(m_sFormat))
	{
		TRACE_W(QString("MCtlEdit::createControls: Unable to set format:%s")
									.arg(m_sFormat));
		return false;
	}

	return true;
}

void MCtlEdit::setColors(QString sCaptionBack,
					QString sEditFocusBack,
					QString sEditNoFocusBack,
					QString sLockedBack,
					QString sCaptionFore,
					QString sEditFore)
{
	m_sCaptionBack = sCaptionBack;
	m_sLockedBack = sLockedBack;
	m_sCaptionFore = sCaptionFore;
	m_sEditFocusBack = sEditFocusBack;
	m_sEditNoFocusBack = sEditNoFocusBack;
	m_sEditFore = sEditFore;
	
	if(m_lblCaption!=NULL)
	{
		QString sStyle;
		
		if(m_sCaptionBack!="")
			sStyle += MIhmConfigColor::createBkgColorStyle(sCaptionBack);

		if(m_sCaptionFore!="")
			sStyle += MIhmConfigColor::createColorStyle(m_sCaptionFore);

		m_lblCaption->setStyleSheet(sStyle);
	}

	QString sStyle;

	if(m_bLocked)
	{
		if(m_sLockedBack!="")
			sStyle += QString("QLineEdit:read-only{%1}").arg(MIhmConfigColor::createBkgColorStyle(m_sLockedBack));
		else
		{
			QPalette pal = QApplication::palette();
			QColor col = pal.color(QPalette::Active, QPalette::Window);
			m_sLockedBack = col.name();
			sStyle += QString("QLineEdit:read-only{%1}").arg(MIhmConfigColor::createBkgColorStyle(m_sLockedBack));
		}

	}
	else
	{
		if(m_sEditNoFocusBack!="")
				sStyle += QString("QLineEdit:!focus{%1}").arg(MIhmConfigColor::createBkgColorStyle(m_sEditNoFocusBack));
		else
				sStyle += QString("QLineEdit:!focus{%1}").arg(MIhmConfigColor::createBkgColorStyle("FFFFFF"));

		if(m_sEditFore!="")
				sStyle += MIhmConfigColor::createColorStyle(m_sEditFore);

		if(m_sEditFocusBack!="")
				sStyle += QString("QLineEdit:focus{%1}").arg(MIhmConfigColor::createBkgColorStyle(m_sEditFocusBack));
		else
				sStyle += QString("QLineEdit:focus{%1}").arg(MIhmConfigColor::createBkgColorStyle("FFFFFF"));
	}



	if(sStyle!="")
	{
		QLineEdit *pEdit;
		
		for(int i=0;i<m_iNumItems;i++)
		{
			pEdit = m_lstLineEdits.at(i);
			pEdit->setStyleSheet(sStyle);
		}
	}
}



QWidget * MCtlEdit::getItemWidget(int i)
{
	QWidget * pRet = NULL;

	if(i<m_iNumItems)
		pRet = m_lstLineEdits.at(i);

	return pRet;
}



bool MCtlEdit::setDescription(QString sDesc)
{
	QString sTemp;
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();

//     ' VALEUR INITIALE DU CHAMP
	
//     ' Extraire le type et la valeur de la saisie

//     ' Type de la valeur e saisir (non utilise, mais e memoriser pour renvoyer la
//     ' valeur saisie)
	m_sUserInfo = MHelpFuncs::getStringItem(sDesc,0, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);

	if(m_sUserInfo =="")
	{
		TRACE_W(QString("MCtlEdit::setDescription: Error decoding m_sUserInfo!"));
		return false;
	}
	
//     Initial values
	QString sInitialValues = MHelpFuncs::getStringItem(sDesc,1, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), false);
	
	if(!m_bIsAlarm)
	{
		for(int i=0;i<m_iNumItems;i++)
		{
			sTemp = MHelpFuncs::getStringItem(sInitialValues,i, QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), false);

			if(sTemp!="")
			{
				//if first char is '?' translate the text 
				if(sTemp.at(0) == MInputDialogReq::cSAISIE_DECODE_TEXTE)
				{
					sTemp = sTemp.mid(1);
					pLang->getLabelTranslation(sTemp, MIhmLanguages::enuTranslTargetDesktop, sTemp);
				}
			
			}

			if(!setValidValue(sTemp, i))
			{
				TRACE_W(QString("MCtlEdit::setDescription: Error invalid initial value:%1 in item:%2!")
									.arg(sTemp)
									.arg(i));
				return false;
			}
		}
	}
	else //if it is an alarm dialog use the Caption label to show all the text
	{
		QString sMessage = m_sLabelText;

		for(int i=0;i<m_iNumItems;i++)
		{
			sTemp = MHelpFuncs::getStringItem(sInitialValues,i, QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), false);
			
			if(sTemp!="")
			{
				//if first char is '?' translate the text 
				if(sTemp.at(0) == MInputDialogReq::cSAISIE_DECODE_TEXTE)
				{
					sTemp = sTemp.mid(1);
					pLang->getLabelTranslation(sTemp, MIhmLanguages::enuTranslTargetDesktop, sTemp);
				}
			}
		
			if(sMessage!="")
				sMessage += QString("\n");
			sMessage += sTemp;

			m_lstLineEdits.at(i)->setVisible(false);
		}

		m_lblCaption->setText(sMessage);
		m_lblCaption->setAlignment(Qt::AlignHCenter);
		m_lblCaption->setVisible(true);
		
	}
	
	return true;
}

bool MCtlEdit::setValidValue(QString s, int i)
{
	const QValidator *v = m_lstLineEdits.at(i)->validator();
	m_chDecimalSeparator = MIhmConfigGeneral::getCfg()->getDefaultDecimalSeparator();

	if(getFormatType()==enuFORMAT_TYPE_NUMBER)
	{
		s.replace(QChar('.'), m_chDecimalSeparator);
	}

	//TODO verify the min and max size
	int iSize = s.size();
	
	if(iSize<m_iMinLen || iSize>m_iMaxLen)
		return false;

	if(m_iNumDecimals>0 && s.indexOf(m_chDecimalSeparator)==-1)
	{
		bool bOK;
		s.toInt(&bOK);
		
		if(bOK)
			s +=  QString("%1%2").arg(m_chDecimalSeparator).arg(QString(m_iNumDecimals, QChar('0')));	
		else
			return false;
	}


	if(v!=NULL && iSize>0)
	{
		int pos = 0;
		
		if(v->validate(s, pos)==QValidator::Acceptable)
			m_lstLineEdits.at(i)->setText(s);
		else
		{
			//invalid value  -- set default
			return false;
		}
	}
	else
		m_lstLineEdits.at(i)->setText(s);


	return true;
}

QString MCtlEdit::getItemText(int iItem)
{
	QString sResult;


	if(!m_bIsAlarm)
	{
		if(m_lstLineEdits.size()>iItem)
		{
			if(m_lstLineEdits.at(iItem)->echoMode()==QLineEdit::Normal)
				sResult = m_lstLineEdits.at(iItem)->displayText();
			else
				sResult = m_lstLineEdits.at(iItem)->text();
		}
	}
	else
		sResult = m_lblCaption->text();

	return sResult;
}


QString MCtlEdit::getResult()
{
	QString sRez;

	sRez = m_sUserInfo + QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM);
	
	for(int i=0; i<m_iNumItems; i++ )
	{
		if(i!=0) 
			sRez += QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM);
		
		sRez += getItemText(i);
	}
	
	return sRez;
}


bool MCtlEdit::isInputValid()
{
	if(m_bIsAlarm || m_bLocked)
		return true;

	QString sTemp;

	for(int i=0; i<m_iNumItems; i++ )
	{
		sTemp = m_lstLineEdits.at(i)->displayText();
		int iSize = sTemp.size();

		if(iSize < m_iMinLen)
		{
			TRACE_D(QString("MFormInput::doValidate: The value %1 is too short!").arg(sTemp));
			m_lstLineEdits.at(i)->setFocus();	
			return false;
		}

		if(!m_lstLineEdits.at(i)->hasAcceptableInput())
		{
			TRACE_D(QString("MFormInput::doValidate: The value %1 is not acceptable!").arg(sTemp));
			m_lstLineEdits.at(i)->setFocus();	
			return false;
		}
	}

	return true;
}
