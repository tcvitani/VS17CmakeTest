#include "MInputDialogReq.h"
#include "MLineEdit.h"
#include "MIhmConfigGeneral.h"
#include <QKeyEvent>
#include <QRegularExpressionValidator>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}


MLineEdit::MLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
	m_parentWidget = parent;
	m_bRightMask = false;
	m_bNumber = false;
	m_bDecimal = false;
	m_bLeftPadZeroes = false;
	m_bBlockRecursion = false;
	m_validator = NULL;
	m_bSigned = false;

	setContextMenuPolicy(Qt::NoContextMenu);
}

MLineEdit::~MLineEdit()
{
	if(m_validator != NULL)
			delete m_validator;
}

void MLineEdit::onCursorPositionChanged (int iOld, int iNew)
{
	if(m_bRightMask && !m_bBlockRecursion)
	{
		int iSize = displayText().size();
		
		m_bBlockRecursion = true;
		if(iNew!=iSize)
			setCursorPosition(iSize);
		m_bBlockRecursion = false;
	}
}

void MLineEdit::focusInEvent(QFocusEvent * event)
{
	if(m_bRightMask && inputMask()!="")
	{
		int iSize = displayText().size();
		setCursorPosition(iSize);
	}	
	else if(inputMask()!="")
	{
		setCursorPosition(0);
	}
	else
	{
		setSelection(0, displayText().size());
	}

	QLineEdit::focusInEvent(event);
}




void MLineEdit::setRightMask(QString sMask, QString sMaskPlaceholder)
{
	m_bRightMask = true;
	m_iRightMaskSize = sMask.size();
	m_cRightMaskPlaceholder = (sMaskPlaceholder.size()>0)?sMaskPlaceholder.at(0):QChar(' ');

	m_chDecimalSeparator = MIhmConfigGeneral::getCfg()->getDefaultDecimalSeparator();
	m_iRightMaskSeparatorPosition = sMask.indexOf("."); //The '.' must be the char to be used in formats...It is later replaced by the default separator set in reg.
	
	if(m_iRightMaskSeparatorPosition>0)
		m_iRightMaskCurNumDecimals = m_iRightMaskSize - m_iRightMaskSeparatorPosition - 1;
	else
		m_iRightMaskCurNumDecimals = 0;

	if(m_bSigned)
		m_sRightMask = sMask.mid(1);
	else
		m_sRightMask = sMask;

	connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(onCursorPositionChanged(int , int)));
}



void MLineEdit::rightMaskKeyHandler(QKeyEvent * event)
{
	bool bProcessed = false;
	QString sKey = event->text();
	QString s = displayText();
	int iCur = cursorPosition();
	QChar chKey = '\0';
	
	if(sKey.size()>0)
		chKey = sKey.at(0);
	
	enuKeyAction aeActionsToPerform[10];
	int iNumActions = rightMaskGenerateKeyActions(aeActionsToPerform, iCur, chKey, event->key(), s);
	
	//execute key action
	if(iNumActions>0)
		bProcessed = executeKeyActions(aeActionsToPerform, iNumActions, sKey);
	

	if(!bProcessed)
	{
		QLineEdit::keyPressEvent(event);
	}

	setCursorPosition(displayText().size());
	
	s = displayText();
	
	if(s.size()<m_iRightMaskSize && m_cRightMaskPlaceholder!=QChar(' '))
	{
		QString sFill = QString(m_iRightMaskSize, m_cRightMaskPlaceholder);
		s = sFill + s;
	}

	if(s.size()>m_iRightMaskSize)
	{	
		s = s.right(m_iRightMaskSize);
		setText(s);
	}
	
	s = displayText();

	if(m_iRightMaskSeparatorPosition>0 && m_cRightMaskPlaceholder==QChar(' '))
	{
		//-------------------------------------------------
		//fix starting with multiple zeroes
		//-------------------------------------------------
		iCur = cursorPosition();
		
		if(removeLeadingZeroes(s, iCur))
		{
			setText(s);
			setCursorPosition(iCur);
		}
	}

}

int  MLineEdit::rightMaskGenerateKeyActions(OUT enuKeyAction *eActionsToPerform, 
											int iCur, 
											QChar chCharEntered, 
											int iQtEventKey, 
											QString& s)
{
	int iNumActions = 0;
	int iSize = s.size();
	QChar chFirst;

	if(iSize > 0)
		chFirst	= s.at(0);
	else
		chFirst = '\0';

	QChar chSecond = '\0';
		
	if(s.size()>1)	
		chSecond = s.at(1);
		

		if(chCharEntered.isDigit() ) //if key is number
		{
			if(iSize<m_iRightMaskSize)
			{
				eActionsToPerform[iNumActions++] = enuInsertR; 
				eActionsToPerform[iNumActions++] = enuMoveSeparatorR;
			}
			else 
			{
				if(m_iRightMaskCurNumDecimals>0)
					eActionsToPerform[iNumActions++] = enuIgnore; 
				else
					eActionsToPerform[iNumActions++] = enuInsertR; 
			}
		}
		else if(iQtEventKey == Qt::Key_Backspace || iQtEventKey == Qt::Key_Delete)
		{
			if(iSize > m_iRightMaskCurNumDecimals + 2)
			{
				eActionsToPerform[iNumActions++] = enuDeleteL;
				eActionsToPerform[iNumActions++] = enuAppendZeroL;
				eActionsToPerform[iNumActions++] = enuMoveSeparatorL;
			}
			else
			{
				eActionsToPerform[iNumActions++] = enuDeleteL;
				eActionsToPerform[iNumActions++] = enuAppendZeroL;
				eActionsToPerform[iNumActions++] = enuMoveSeparatorL;
			}
		}
		else
		{
			eActionsToPerform[iNumActions++] = enuIgnore;
		}	

	
	return iNumActions;
}



int  MLineEdit::generateKeyActions(OUT enuKeyAction *eActionsToPerform, 
								   int iCur, 
								   QChar chCharEntered, 
								   int iQtEventKey, 
								   QString& s)
{
	int iNumActions = 0;
	int iSep = s.indexOf(m_chDecimalSeparator);
	int iCurNumDecimals = s.size() - iSep - 1;
	//-------------------------------------------------------------
	// First handle if something was selected ...

		int iSelStart = -1;
		int iSelEnd = -1;

		if(hasSelectedText()&& 
			(chCharEntered.isDigit() ||
			chCharEntered == '-' ||
			isSeparator(chCharEntered) ||
			iQtEventKey == Qt::Key_Backspace || 
			iQtEventKey == Qt::Key_Delete))
		{
			iSelStart = selectionStart();
			iSelEnd = iSelStart + selectedText().size();

			if(iSelStart>=0 && iSelEnd>iSelStart && iSelEnd<=s.size()) //if any valid selection
			{
				iCur = iSelStart;
				QString sLeft, sRight;

				if(iSelStart > 0)
					sLeft = s.left(iSelStart);
				else
					sLeft = "";
				
				if(iSelEnd<s.size())
					sRight = s.mid(iSelEnd); //get everiting right of iSelEnd
				else
					sRight = "";

				QChar chFirst = s.at(0);

				if((iSelStart<iSep && iSelEnd<=iSep) || (iSelStart>iSep && iSelEnd>iSep))
				{
					s = sLeft + sRight;
					setText(s);
					setCursorPosition(iCur);

				}
				else if(iSelStart<=iSep && iSelEnd>iSep)
				{
					
					if(isSeparator(chCharEntered))
					{
						s = sLeft + chCharEntered +  sRight;

					}
					else
					{
						s = sLeft +  sRight;

						if(s.size()>m_iNumDigits && chFirst!='-')
							s = s.left(m_iNumDigits);
						else if(s.size()>m_iNumDigits+1 && chFirst=='-')
							s = s.left(m_iNumDigits+1);
					}

					setText(s);
					setCursorPosition(iCur);

				}

				//to avoid executing these keys after the selection is deleted
				if(iQtEventKey == Qt::Key_Backspace || iQtEventKey == Qt::Key_Delete)
				{
					eActionsToPerform[iNumActions++] = enuIgnore; 
					return iNumActions;
				}
				
				//recalculate iCurNumDecimals
				iSep = s.indexOf(m_chDecimalSeparator);
				if(iSep>=0)
					iCurNumDecimals = s.size() - iSep - 1;
				else
					iCurNumDecimals = 0;
			}

		}
	//-----------------------------------------------
	//no more selection
	if(s.size()==0)	 
	{
		if(chCharEntered == '-' && m_bSigned)
			eActionsToPerform[iNumActions++] = enuInsertR;
		else if(chCharEntered.isDigit())
			eActionsToPerform[iNumActions++] = enuInsertR;
		else if(isSeparator(chCharEntered))
			eActionsToPerform[iNumActions++] = enuInsertR;
	}
	else //if size >0
	{
		QChar chFirst = s.at(0);
		QChar chSecond = '\0';

		if(s.size()>1)	
			chSecond = s.at(1);

		if(iCur<=iSep || iSep<0) //if cursor is at the left side of the separator character
		{
			if(chCharEntered.isDigit()) //if key is number
			{
				if(iCur==0 && chFirst == '0' && iCur+1 == iSep)
					eActionsToPerform[iNumActions++] = enuOverwriteR;
				else if(iCur==0 && chFirst == '-' && getNumDigits(s) < m_iNumDigits)
				{
					eActionsToPerform[iNumActions++] = enuOverwriteR;
				}
				else if(iCur==0 && chFirst == '-' && chCharEntered != '0' && getNumDigits(s) >= m_iNumDigits)
				{
					eActionsToPerform[iNumActions++] = enuDeleteR;
					eActionsToPerform[iNumActions++] = enuOverwriteR;
				}
				else if(iCur==0 && chFirst == '-' && chCharEntered == '0' && getNumDigits(s) >= m_iNumDigits)
				{
					eActionsToPerform[iNumActions++] = enuDeleteR;
				}
				else if(iCur==0 && chCharEntered == '0' && chFirst!='0' && iSep == 1)
				{
					eActionsToPerform[iNumActions++] = enuOverwriteR;
				}
				else if(iCur==0 && chCharEntered == '0' && chFirst!='0' && iSep != 1 && m_iNumDigits>getNumDigits(s))
				{
					eActionsToPerform[iNumActions++] = enuInsertR;
				}
				else if(iCur==1)
				{	
					if(chFirst == '0' && iCur==iSep )
						eActionsToPerform[iNumActions++] = enuOverwriteL;
					else if(chFirst == '-' && chSecond == '0' && iCur+1 == iSep)
						eActionsToPerform[iNumActions++] = enuOverwriteR;
					else if(chFirst == '-' && chSecond != '0' && chSecond != m_chDecimalSeparator && chCharEntered == '0' && m_iNumDigits>getNumDigits(s))
						eActionsToPerform[iNumActions++] = enuInsertR;
					else if(chFirst == '-' && chSecond == '0' && iCur == iSep)
						eActionsToPerform[iNumActions++] = enuOverwriteL;
					else if(chFirst == '-' && chSecond == m_chDecimalSeparator && iCur == iSep && m_iNumDigits>getNumDigits(s))
						eActionsToPerform[iNumActions++] = enuInsertL;
					else if(getNumDigits(s) < m_iNumDigits && ((iCur < iSep) || (iSep<0)))
						eActionsToPerform[iNumActions++] = enuInsertR; 
					else if(getNumDigits(s) == m_iNumDigits && ((iCur < iSep) || (iSep<0)))
						eActionsToPerform[iNumActions++] = enuOverwriteR; 
					else if(getNumDigits(s) < m_iNumDigits && iCur == iSep )
						eActionsToPerform[iNumActions++] = enuInsertL; 

				}
				else if(iCur==2 && chFirst == '-' && chSecond == '0' && m_iNumDecimals>0)
				{
					eActionsToPerform[iNumActions++] = enuOverwriteL;
				}
				else if(m_iNumDigits>getNumDigits(s))
				{
					eActionsToPerform[iNumActions++] = enuInsertL;
				}
				else if(getNumDigits(s)==m_iNumDigits && ((iCur < iSep)||(iSep<0)) && 
					((chFirst != '-' && iCur < m_iNumDigits) || (chFirst == '-' && iCur < m_iNumDigits+1)))
				{
					eActionsToPerform[iNumActions++] = enuOverwriteR;
				}else
					eActionsToPerform[iNumActions++] = enuIgnore;
			}
			else if(isSeparator(chCharEntered))
			{ 
				if (iCur==iSep)
					eActionsToPerform[iNumActions++] = enuMoveCursorR;
				else if(iCur==0 && chFirst == '-')
					eActionsToPerform[iNumActions++] = enuIgnore;
				else if(iCur<iSep)
				{
					QString sRest = s.mid(iCur);
					sRest.replace(m_chDecimalSeparator, "");
					sRest = sRest.left(m_iNumDecimals);

					s = s.left(iCur) + m_chDecimalSeparator + sRest;

					setText(s);
					setCursorPosition(iCur+1);

					eActionsToPerform[iNumActions++] = enuIgnore;
				}


			}
			else if(iQtEventKey == Qt::Key_Backspace)
			{
				if(iCur<=iSep && iCur>0)
				{
					if(iCur==1 && chFirst != '-' && getNumDigits(s)==1)
						eActionsToPerform[iNumActions++] = enuReplaceWithZeroL;
					else if(iCur==2 && chFirst == '-' &&  getNumDigits(s)==1)
						eActionsToPerform[iNumActions++] = enuReplaceWithZeroL;
					else if(iCur==1 && chFirst == '-')
						eActionsToPerform[iNumActions++] = enuDeleteL;
					else if(iCur==1 && chFirst != '-' && getNumDigits(s)>1)
						eActionsToPerform[iNumActions++] = enuDeleteL;
					else if(iCur>1 && getNumDigits(s)>1)
						eActionsToPerform[iNumActions++] = enuDeleteL;

				}
			}
			else if(iQtEventKey == Qt::Key_Delete)
			{
					if(iCur==iSep)
						eActionsToPerform[iNumActions++] = enuMoveCursorR;
					else if(iCur==0 && chFirst != '-' && getNumDigits(s)==1)
						eActionsToPerform[iNumActions++] = enuReplaceWithZeroR;
					else if(iCur==1 && chFirst == '-' &&  getNumDigits(s)==1)
						eActionsToPerform[iNumActions++] = enuReplaceWithZeroR;
					else if(iCur==0 && chFirst == '-')
						eActionsToPerform[iNumActions++] = enuDeleteR;
					else if(iCur==0 && chFirst != '-' && getNumDigits(s)>1)
						eActionsToPerform[iNumActions++] = enuDeleteR;
					else if(iCur>0 && getNumDigits(s)>1)
						eActionsToPerform[iNumActions++] = enuDeleteR;

			}
			else if(chCharEntered == '-' && m_bSigned)
			{
				if(iCur==0 && chFirst.isDigit())
					eActionsToPerform[iNumActions++] = enuInsertR;
				else
					eActionsToPerform[iNumActions++] = enuIgnore;

			}
				
		}
		else //	iCur>iSep cursor at the right side of separator character
		{
			if(chCharEntered.isDigit()) //if key is number
			{
				if(iCurNumDecimals<m_iNumDecimals)
					eActionsToPerform[iNumActions++] = enuInsertR; 
				else if(iCurNumDecimals==m_iNumDecimals && iCur-iSep<=m_iNumDecimals)
				{	
					eActionsToPerform[iNumActions++] = enuOverwriteR;
				}
				else if(!hasSelectedText()) 
				{	
					eActionsToPerform[iNumActions++] = enuIgnore;
				}
						
			}
			else if(iQtEventKey == Qt::Key_Backspace)//if key is not digit
			{
				if(iCur - 1 > iSep)
						eActionsToPerform[iNumActions++] = enuDeleteL;
				else if (iCur - 1 == iSep )
				{
					eActionsToPerform[iNumActions++] = enuMoveCursorL;
				}
			}
			else if(iQtEventKey == Qt::Key_Delete)
			{
				if(iCurNumDecimals>m_iNumDecimals)
					eActionsToPerform[iNumActions++] = enuDeleteR; 
				else if(iCurNumDecimals==m_iNumDecimals && iCur<=iSep+m_iNumDecimals)
				{	
					eActionsToPerform[iNumActions++] = enuDeleteR;
				}
				else if(iCurNumDecimals==m_iNumDecimals && iCur>iSep+m_iNumDecimals)
				{
					eActionsToPerform[iNumActions++] = enuIgnore;
				}
			}
		} // end	iCur>iSep cursor behind separator character

	}//	end if(iSelStart = -1) //no selection


	
	return iNumActions;
}

void MLineEdit::decimalKeyHandler(QKeyEvent * event)
{
	bool bProcessed = false;
	QString sKey = event->text();
	QString s = displayText();
	int iCur = cursorPosition();
	QChar chKey = '\0';

	if(sKey.size()>0)
		chKey = sKey.at(0);

	enuKeyAction aeActionsToPerform[10];
	int iNumActions = generateKeyActions(aeActionsToPerform, iCur, chKey, event->key(), s);
		
	//execute key action
	if(iNumActions>0)
		bProcessed = executeKeyActions(aeActionsToPerform, iNumActions, sKey);
	
	if(!bProcessed)
	{
		QLineEdit::keyPressEvent(event);
	}
	
//Do the formating finalization
//check if decimal separator is missing and if number of decimals match
//-------------------------------------------------
	s = displayText();
	if(s=="" || s=="-") s+="0";

	//fix starting with multiple zeroes
	//-------------------------------------------------
	iCur = cursorPosition();
	
	if(removeLeadingZeroes(s, iCur))
	{
		setText(s);
		setCursorPosition(iCur);
	}


	//correct decimal number format
	//-------------------------------------------------
	bool bUpdateDecimal = false;
	int iSep;
	iSep = s.indexOf(m_chDecimalSeparator);
	iCur = cursorPosition();
	
	if(iSep==0) 
	{
		s = '0'+ s; 
		iCur++;
		
		bUpdateDecimal = true;
	}
	else if(iSep == 1)
	{
		if(s.at(0) == '-')
		{
			s = "-0" + s.mid(1);
			if(iCur>0)
				iCur++;

			bUpdateDecimal = true;
		}
	}

	iSep = s.indexOf(m_chDecimalSeparator);

	if(iSep == -1)
	{
		QString sSuFix = QString(m_chDecimalSeparator);
		sSuFix+=QString(m_iNumDecimals, QChar('0'));
		
		s += sSuFix;
		bUpdateDecimal = true;
	}
	else 
	{
		int iCurNumDecimals = s.size() - iSep - 1;

		if(iCurNumDecimals<m_iNumDecimals)
		{
			QString sSuFix = QString(m_iNumDecimals-iCurNumDecimals, QChar('0'));
			s += sSuFix;
			bUpdateDecimal = true;
		}
	}

	if(bUpdateDecimal)
	{
		setText(s);
		setCursorPosition(iCur);
	}
}



//fix starting with multiple zeroes
//-------------------------------------------------
bool MLineEdit::removeLeadingZeroes(QString &s, int& iCur)
{
	QChar chFirst, chSecond;
    bool bZeroesFixed = false;

	for (int i=0;i<s.size() - 1;i++)
	{
		chFirst = s.at(i);
		chSecond = s.at(i+1);
		
		if(chFirst == '-')
			continue;

		if(chFirst == '0' && chSecond.isDigit())
		{
			s = s.left(i) + s.mid(i+1,s.size()-i-1);
			
			if(iCur>i)
				iCur--; //move cursor left if it was on the right of removed '0' 
			
			i--;
			bZeroesFixed = true;
		}
		else
			break;
	}
	
	return bZeroesFixed;
}


void MLineEdit::numberKeyHandler(QKeyEvent * event)
{
	bool bProcessed = false;
	QString s = displayText();
	QString sKey = event->text();
	int iCur = cursorPosition();

	QChar chKey = '\0';

	if(sKey.size()>0)
		chKey = sKey.at(0);

	enuKeyAction aeActionsToPerform[10];
	int iNumActions = generateKeyActions(aeActionsToPerform, iCur, chKey, event->key(), s);
		
	//execute key action
	if(iNumActions>0)
		bProcessed = executeKeyActions(aeActionsToPerform, iNumActions, sKey);
	
	if(!bProcessed)
	{
		QLineEdit::keyPressEvent(event);
	}

	s = displayText();

	if(!m_bLeftPadZeroes)
	{
		//-------------------------------------------------
		//fix starting with multiple zeroes
		//-------------------------------------------------
		iCur = cursorPosition();
		
		if(removeLeadingZeroes(s, iCur))
		{
			setText(s);
			setCursorPosition(iCur);
		}
	}

}



void MLineEdit::keyPressEvent(QKeyEvent * event)
{

	if(m_bRightMask && m_sRightMask!="")
	{
		rightMaskKeyHandler(event);
	}
	else if(m_bNumber && !m_bDecimal)
	{
		numberKeyHandler(event);
	}
	else if(m_bNumber && m_bDecimal)
	{
		decimalKeyHandler(event);
	}
	else 
	{
		QLineEdit::keyPressEvent(event);
	}
}

int MLineEdit::getNumDigits(QString sDisplayedText)
{	
	int iNum = 0;
	
	sDisplayedText = sDisplayedText.trimmed();
	
	if(sDisplayedText.size()>0)
	{
		if(sDisplayedText.at(0) == '-')
			sDisplayedText = sDisplayedText.mid(1);

		int iSep = sDisplayedText.indexOf(m_chDecimalSeparator);
		if(iSep>0)
			iNum = iSep;	
		else
			iNum = sDisplayedText.size();
	}

	return iNum;
}


bool MLineEdit::executeKeyActions(enuKeyAction *aeActions, int iNumActions, QString sKey)
{
	bool bProcessed = false;

	for(int i=0;i<iNumActions;i++)
	{
		int iCur = cursorPosition();
		QString s = displayText();

		if(aeActions[i] == enuOverwriteR)
		{
			s.replace(iCur,1,sKey.at(0));
			setText(s);
			setCursorPosition(iCur+1);
			bProcessed = true;
		} 
		else if(aeActions[i] == enuOverwriteL)
		{
			s.replace(iCur-1,1,sKey.at(0));
			setText(s);
			setCursorPosition(iCur);
			bProcessed = true;
		}
		else if(aeActions[i] == enuInsertR)
		{
			s = s.left(iCur) + sKey.at(0) + s.mid(iCur);
			setText(s);
			setCursorPosition(iCur+1);
			bProcessed = true;
		}
		else if(aeActions[i] == enuInsertL)
		{
			s = s.left(iCur) + sKey.at(0) + s.mid(iCur);
			setText(s);
			setCursorPosition(iCur+1);
			bProcessed = true;
		}		
		else if(aeActions[i] == enuReplaceWithZeroL)
		{
			s.replace(iCur-1,1,'0');
			setText(s);
			setCursorPosition(iCur-1);
			bProcessed = true;
		}
		else if(aeActions[i] == enuReplaceWithZeroR)
		{
			s.replace(iCur,1,'0');
			setText(s);
			setCursorPosition(iCur+1);
			bProcessed = true;
		}
		else if(aeActions[i] == enuDeleteL)
		{
			s = s.left(iCur-1) + s.mid(iCur);
			setText(s);
			setCursorPosition(iCur-1);
			bProcessed = true;
		}	
		else if(aeActions[i] == enuDeleteR)
		{
			s = s.left(iCur) + s.mid(iCur+1);
			setText(s);
			setCursorPosition(iCur);
			bProcessed = true;
		}	
		else if(aeActions[i] == enuMoveCursorL)
		{
			setCursorPosition(iCur-1);
			bProcessed = true;
		}
		else if(aeActions[i] == enuMoveCursorR)
		{
			setCursorPosition(iCur+1);
			bProcessed = true;
		}
		else if(aeActions[i] == enuMoveSeparatorR)
		{
			int iSeparatorPosition = s.indexOf(m_chDecimalSeparator);
			if(iSeparatorPosition>=0)
			{
				s = s.left(iSeparatorPosition) + s.mid(iSeparatorPosition+1,1) + m_chDecimalSeparator + s.mid(iSeparatorPosition+2);
				setText(s);
				bProcessed = true;
			}	
		}
		else if(aeActions[i] == enuMoveSeparatorL)
		{
			int iSeparatorPosition = s.indexOf(m_chDecimalSeparator);
			if(iSeparatorPosition>1)
			{
				s = s.left(iSeparatorPosition-1) + m_chDecimalSeparator + s.at(iSeparatorPosition-1) + s.mid(iSeparatorPosition+1);
				setText(s);
				bProcessed = true;
			}	
			
		}		
		else if(aeActions[i] == enuAppendZeroL)
		{
				s.prepend('0');
				setText(s);
				bProcessed = true;
		}		
		


		else if(aeActions[i] == enuIgnore)
		{
			bProcessed = true;
		}
	}


	return bProcessed;
}



bool MLineEdit::isSeparator(QChar ch)
{
	if(ch == '.' 
		|| ch == ',' 
		|| (ch == m_chDecimalSeparator &&  m_chDecimalSeparator!='\0'))
		return true;
	else
		return false;
}



void MLineEdit::setTxtInputMask(QString sMask)
{
	setInputMask(sMask);

	//QString sReg;

	////get reg exp string and extract format data to be used is valid call for the 
	//if(generateMaskValidator(sMask, sReg))
	//{
	//	//create mask validator with regular expression
	//	QRegExp rx(sReg);
	//	QRegularExpressionValidator *validator = new QRegularExpressionValidator(rx, this);
	//	
	//	if(m_validator != NULL)
	//		delete m_validator;

	//	m_validator = validator;
	//}
	//
}


const QValidator * MLineEdit::getValidator()
{
	const QValidator * pValidator = this->validator();

	//if(pValidator==NULL)
	//{
	//	pValidator = m_validator;
	//}

	return pValidator;
}




bool MLineEdit::setNumberFormat(QString &sFormat)
{

	QString sReg;

	//get reg exp string and extract format data to be used on
	//key events to program behaviour
	if(generateNumberValidator(sFormat, sReg))
	{
		//create number validator with regular expression
		QRegularExpression rx(sReg);
		QRegularExpressionValidator *validator = new QRegularExpressionValidator(rx, this);
		
		this->setValidator(validator);
	}
	else 
		return false;

	return true;
}


bool MLineEdit::generateNumberValidator(QString sMaskFormat, QString& sReg)
{
	int iSize = sMaskFormat.size();
	
	if(iSize<=1) 
			return false;
	
	QString sMask, sPart;
	m_bSigned = false;
	m_bLeftPadZeroes = false;
	int iPos = 1;
	QChar chCurrent = sMaskFormat.at(iPos);

	if(chCurrent == IHM_FIELD_FORMAT_CHAR_SIGNED)
	{
		sMask = "-?";
		iPos++;
		m_bSigned = true;
		if(iSize<=iPos) return false;
		chCurrent = sMaskFormat.at(iPos);
	}
	
	if(chCurrent == IHM_FIELD_FORMAT_CHAR_ZERO)
	{
		m_bLeftPadZeroes = true;
		iPos++;
		
		if(iSize<=iPos) 
			return false;
		chCurrent = sMaskFormat.at(iPos);
	}

	sPart = "";
	
	while(chCurrent.isDigit())
	{
		sPart += chCurrent;
		iPos++;
		if(iSize<=iPos) 
			break;
		chCurrent = sMaskFormat.at(iPos);
	}

	bool bOK;
	QString sMask2;

	m_iNumDigits = sPart.toInt(&bOK);

	if(bOK)
	{
		if(m_bLeftPadZeroes)
			sMask2 = QString("[0-9]{1,%2}").arg(m_iNumDigits);
		else
			sMask2 = QString("([1-9][0-9]{0,%1}|[0])").arg(m_iNumDigits - 1);
	}
	else
		return false;


	sMask += sMask2;

	if(iSize<=iPos) 
	{
		sReg = sMask;
		m_bNumber = true;
		m_bDecimal = false;
		return true;  
	}

	m_chDecimalSeparator = MIhmConfigGeneral::getCfg()->getDefaultDecimalSeparator();

	iPos++;
	
	if(iSize<=iPos) 
			return false;
	
	chCurrent = sMaskFormat.at(iPos);

	sPart = "";
	while(chCurrent.isDigit())
	{
		sPart += chCurrent;
		
		iPos++;
		if(iSize<=iPos) 
			break;
		chCurrent = sMaskFormat.at(iPos);
	}

	m_iNumDecimals = sPart.toInt(&bOK);
	
	if(bOK)
	{
		sMask2 = QString("[\\d]{%1,%2}").arg(m_iNumDecimals).arg(m_iNumDecimals);
	}
	else
		return false;

	sMask +=  QString("%1%2").arg("[\\.,]").arg(sMask2);
	sReg = sMask;

	m_bNumber = true;
	m_bDecimal = true;
	return true;
}



//
//bool MLineEdit::generateMaskValidator(QString sMaskFormat, QString& sReg)
//{
//	int iSize = sMaskFormat.size();
//	
//	if(iSize<=1) 
//			return false;
//	
//	QChar chDefault = ' ';
//	QString sMask = sMaskFormat;
//
//	if (sMaskFormat.at(iSize-2)==';')
//	{
//		chDefault = sMaskFormat.at(iSize-1);
//		sMask = sMaskFormat.left(iSize-2);
//	}
//	
//	
//	QChar ch;
//	
//	for(int i=0;i<sMask.size();i++)
//	{
//		ch = sMask.at(i);
//		
//		switch(ch.toLatin1())
//		{
//			case 'A':
//				sReg += "[A-Za-z]";
//			break;
//			case 'a':
//				sReg += "[A-Za-z]?";
//			break;
//			case 'N':
//				sReg += "[A-Za-z0-9]";
//			break;
//			case 'n':
//				sReg += "[A-Za-z0-9]?";
//			break;
//			case 'X':
//				sReg += "[.]";
//			break;
//			case 'x':
//				sReg += "[.]?";
//			break;
//			case '9':
//				sReg += "[0-9]";
//			break;
//			case '0':
//				sReg += "[0-9]?";
//			break;
//			case 'D':
//				sReg += "[1-9]";
//			break;
//			case 'd':
//				sReg += "[1-9]?";
//			break;
//			case '#':
//				sReg += "[0-9+-]?";
//			break;
//			case 'H':
//				sReg += "[0-9A-Fa-f]";
//			break;
//			case 'h':
//				sReg += "[0-9A-Fa-f]?";
//			break;
//			case 'B':
//				sReg += "[0-1]";
//			break;
//			case 'b':
//				sReg += "[0-1]?";
//			break;
//
//			default:
//				QString sForRegExp;
//				
//				if(getRegExEscapeForChar(ch, sForRegExp))
//					sReg += sForRegExp;
//		}
//	}
//
//	return true;
//}



bool MLineEdit::getRegExEscapeForChar(QChar chInMask, QString &sForRegExp)
{
	QString sCharsToEscape = "AaNnXx90Dd#HhBb";
	QString sCharsToIgnore = "><!\\";

	if(sCharsToEscape.indexOf(chInMask)>-1)
	{
		sForRegExp = QString("[\\%1]").arg(chInMask);
		return true;
	}
	else if(sCharsToIgnore.indexOf(chInMask)>-1)
	{
		return false;
	}
	else
	{
		sForRegExp = QString("[%1]").arg(chInMask);
		return true;
	}

	return false;
}