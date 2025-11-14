

#ifndef MIHM_SESSION_USER_DATA_H
#define MIHM_SESSION_USER_DATA_H

#include <QString>


class MIhmSessionUserData
{
public:
	MIhmSessionUserData();
	~MIhmSessionUserData();

	static MIhmSessionUserData * findByWebSessionID(QList <MIhmSessionUserData*> *plst, QString sWebSessionId);
	static MIhmSessionUserData * findByIntSessionID(QList <MIhmSessionUserData*> *plst, QString sInternalSessionId);
	static bool removeIhmSession(QList <MIhmSessionUserData*> *plst, MIhmSessionUserData *pToRemove, bool bDelete);
	MIhmSessionUserData& operator=(const MIhmSessionUserData& right);
	
	QString m_sInternalSessionID;
	QString m_sWebSessionID;
	QString m_sUserId;
	QString m_sUserName;
	QString m_sSecCode;
	bool m_bIsSecCodePwd;
	bool m_bAuthorized;
	bool m_bIsDesktopUser;
};




#endif
