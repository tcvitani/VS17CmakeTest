
#include "MIhmAction.h"
#include "MTracer.h"
#include <QStringList>


MIhmAction * MIhmAction::createActionFromString(QString sActionID, QString sActionString)
{
	MIhmAction * pRetVal = NULL;

	QStringList sLst = sActionString.split(QString("|"));

	if(sLst.size()>=2)
	{
		QString sType = sLst.at(0);
		QString sParamValue = sLst.at(1);
		pRetVal = createAction(sActionID, sType, sParamValue, "");

	}

	return pRetVal;
}


MIhmAction * MIhmAction::createAction(QString sActionID, QString sType, QString sParamValue, QString sCursorID)
{
	MIhmAction * pRetVal = NULL;

	bool bOK;

	int iType = sType.toInt(&bOK);
	if(bOK)
	{
		enumIhmActionTypes eTyp;

		switch(iType)
		{
		case 0:
			eTyp = enuSendKey;
			break;
		case 1:
			eTyp = enuOpenLink;
			break;
		case 2:
			eTyp = enuExecuteCmd;
			break;
		case 3:
			eTyp = enuSendCmdReq;
			break;
		default:
			TRACE_W(QString("MIhmAction::createAction: Undefined action type string [%1] for ActionID: %2!").
										arg(sType).
										arg(sActionID));
			return NULL;
		}
		
		pRetVal = new MIhmAction();
		pRetVal->m_sActionID = sActionID;
		pRetVal->m_eType = eTyp;
		pRetVal->m_sParamValue = sParamValue;
		pRetVal->m_sCursorID = sCursorID;
	}

	return pRetVal;
}


bool MIhmAction::operator<(const MIhmAction& rightAction) const
{
	return (this->m_sActionID < rightAction.getActionID());
}

bool MIhmAction::operator==(const MIhmAction& rightAction) const
{
	return (this->m_sActionID == rightAction.getActionID());
}
