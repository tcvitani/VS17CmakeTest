#ifndef MIHM_ACTION_H
#define MIHM_ACTION_H

#include<QString>

class MIhmAction
{
public:
	enum enumIhmActionTypes
	{
		enuUnknown = -1,
		enuSendKey = 0, 
		enuOpenLink,
		enuExecuteCmd,
		enuSendCmdReq
	};

	static MIhmAction * createActionFromString(QString sActionID, QString sActionString);

	static MIhmAction * createAction(QString sActionID, QString sType, QString sParamValue, QString sCursorID);
	
	QString getActionID() const {return m_sActionID;};
	QString getParamValue() const {return m_sParamValue;};
	QString getCursorID() const {return m_sCursorID;};
	enumIhmActionTypes getType() const {return m_eType;};
	
	void setActionID(QString sId) {m_sActionID = sId;};
	bool operator<(const MIhmAction& rightAction) const;
	bool operator==(const MIhmAction& rightAction) const;
private:
	enumIhmActionTypes m_eType;
	QString m_sActionID;
	QString m_sParamValue;
	QString m_sCursorID;
};


#endif


