

#include <QStringList>
#include <QFont>

#include "MPoliceValue.h"
#include "MHelpFuncs.h"
#include "MTracer.h"


extern "C" {
	#include <ihm.H>
};


MPoliceValue::MPoliceValue()
{
	m_iUnderline = UNDEFINED_POLICE_VALUE;
	m_iItalic = UNDEFINED_POLICE_VALUE;
	m_iStrikeOut = UNDEFINED_POLICE_VALUE;
	m_sFontFamily = "";

	m_iPointSize = UNDEFINED_POLICE_VALUE;
	m_iFontWeight = UNDEFINED_POLICE_VALUE;

}

MPoliceValue::~MPoliceValue()
{
	

}

MPoliceValue& MPoliceValue::operator=(const MPoliceValue& right)
{
	this->m_sFontFamily = MHelpFuncs::deepCopy(right.m_sFontFamily);
	this->m_iPointSize = right.m_iPointSize;
	this->m_iUnderline = right.m_iUnderline;
	this->m_iItalic = right.m_iItalic;
	this->m_iStrikeOut = right.m_iStrikeOut;
	this->m_sForeColor = MHelpFuncs::deepCopy(right.m_sForeColor);
	this->m_iFontWeight = right.m_iFontWeight;
	this->m_sBckColor = MHelpFuncs::deepCopy(right.m_sBckColor);
	
	return *this;
}


bool MPoliceValue::isEmpty()
{
	if(!isFontValid() && m_sForeColor == "" && m_sBckColor=="")
		return true;
	else
		return false;
}


bool MPoliceValue::isFontValid()
{
	if(m_sFontFamily!=""
		|| m_iUnderline != UNDEFINED_POLICE_VALUE
		|| m_iItalic != UNDEFINED_POLICE_VALUE
		|| m_iStrikeOut != UNDEFINED_POLICE_VALUE
		|| m_iPointSize != UNDEFINED_POLICE_VALUE
		|| m_iFontWeight != UNDEFINED_POLICE_VALUE)
		return true;
	else
		return false;
}



void MPoliceValue::updateWithString(QString sAdditionalValues)
{
	QStringList lst;

	lst = sAdditionalValues.split(IHM_REG_FONT_SEPARATOR);

	if(lst.size()>0)
	{
		QString sFamily = lst.at(0);

		if(sFamily!="")
		{
			QFont fnt(sFamily);

			if(fnt.exactMatch())
			{
				m_sFontFamily = sFamily;
			}
			else
			{
				TRACE_W(QString("MPoliceValue::updateWithString: Invalid font value %1").arg(sFamily));
			}
		}
	}


	if(lst.size()>1)
	{
		QString sPointSize = lst.at(1);

		if(sPointSize!="")
		{
			bool bOK;
			int iSize = sPointSize.toInt(&bOK);
			
			if(bOK && iSize>0)
				m_iPointSize = iSize;
			else
			{
				TRACE_W(QString("MPoliceValue::updateWithString: Invalid Point Size value %1").arg(sPointSize));
			}
		}
	}

	if(lst.size()>2)
	{
		QString sWeight = lst.at(2);

		if(sWeight!="")
		{
			bool bOK;
			int iWeight = sWeight.toInt(&bOK);

			if(bOK && iWeight>0 && iWeight<=100)
				m_iFontWeight = iWeight;
			else
			{
				TRACE_W(QString("MPoliceValue::updateWithString: Invalid Font Weight value %1").arg(sWeight));
			}
		}
	}

	if(lst.size()>3)
	{
		QString sUnderline = lst.at(3);

		if(sUnderline!="")
		{
			bool bOK;
			int iUnderline = sUnderline.toInt(&bOK);

			if(bOK && (iUnderline==0 || iUnderline ==1))
				m_iUnderline = iUnderline;
			else
			{
				TRACE_W(QString("MPoliceValue::updateWithString: Invalid Underline value %1").arg(sUnderline));
			}
		}
	}
	
	if(lst.size()>4)
	{
		QString sItalic = lst.at(4);

		if(sItalic!="")
		{
			bool bOK;
			int iItalic = sItalic.toInt(&bOK);

			if(bOK && (iItalic==0 || iItalic ==1))
				m_iItalic = iItalic;
			else
			{
				TRACE_W(QString("MPoliceValue::updateWithString: Invalid Italic value %1").arg(sItalic));
			}
		}
	}

	if(lst.size()>5)
	{
		QString sStrikeOut = lst.at(5);

		if(sStrikeOut!="")
		{
			bool bOK;
			int iStrikeOut = sStrikeOut.toInt(&bOK);

			if(bOK && (iStrikeOut==0 || iStrikeOut ==1))
				m_iStrikeOut = iStrikeOut;
		}
	}


	if(lst.size()>6)
	{
		QString sForeColor = lst.at(6);

		if(sForeColor!="")
		{
			bool bOK;
			int iForeCol = sForeColor.toInt(&bOK,16);

			if(bOK)
			{
				m_sForeColor = QString("#%1").arg(iForeCol,6,16,QChar('0'));
			}
			else
			{
				m_sForeColor = sForeColor;
				TRACE_D(QString("MPoliceValue::updateWithString: Invalid Foreground Color value [%1]?").arg(sForeColor));
			}
		}
		else
		{
			m_sForeColor = "";
		}
	}

	if(lst.size()>7)
	{
		QString sBckCol= lst.at(7);

		if(sBckCol!="")
		{
			bool bOK;
			int iBckCol = sBckCol.toInt(&bOK,16);

			if(bOK)
			{
				m_sBckColor = QString("#%1").arg(iBckCol,6,16,QChar('0'));
			}
			else
			{
				m_sBckColor = sBckCol;
				TRACE_D(QString("MPoliceValue::updateWithString: Invalid Background Color value [%1]?").arg(sBckCol));
			}
		}
		else
		{
			m_sBckColor = "";
		}
	}


}


QString MPoliceValue::getColorStyle()
{
	QString sValue;
	
	if(m_sForeColor!="")
	{
		sValue += QString("color: %1;").arg(m_sForeColor);
	}

	if(m_sBckColor!="")
	{
		sValue += QString("background-color: %1;").arg(m_sBckColor);
	}

	return sValue;

}
