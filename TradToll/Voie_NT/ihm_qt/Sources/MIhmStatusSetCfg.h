#ifndef MIHM_STATUS_SET_CFG_H
#define MIHM_STATUS_SET_CFG_H

#include <QObject>


extern "C" {
	#include <noyau.h>
	#include <ihm.h>
}

#define IHM_REG_STATUS_SET  "StatusSet"            


#define IHM_STATUS_OBJ_SEPARATOR_CHAR  '|'
#define IHM_STATUS_OBJ_ID_SEPARATOR_CHAR ':'
#define IHM_STATUS_OBJ_ACTIONS_SEPARATOR_CHAR ','

struct SStatusSetGroupObject{
	enum_ihm_objets             objet;
	QString		sAdditionalParameters;	//TO DO should be used for menu...
};

class MStatusSetGroup
{
public:
	MStatusSetGroup(QString sGroupID){m_sGroupID = sGroupID;};
	~MStatusSetGroup();

	bool appendObject(QString sCfgSetting);
	const QString & getID() const{return (const QString &)m_sGroupID;};
	
	QList <SStatusSetGroupObject *> *getIdList(){return &lstStatusSetGroupIds;};
private:
	QString m_sGroupID; 
	QList <SStatusSetGroupObject *> lstStatusSetGroupIds;

};


class StatusSetGroupList
{
public:
	StatusSetGroupList();
	~StatusSetGroupList();
	
	inline void append(MStatusSetGroup*p)
		{m_plstOrderedStatusSetGroups->append(p);};
	
	QList<MStatusSetGroup*> *m_plstOrderedStatusSetGroups;

	static bool lessThan(const MStatusSetGroup* x1, const MStatusSetGroup* x2); 

	bool find(QString  &sGroupName, MStatusSetGroup ** pFound);

	void sort();
};



class MIhmStatusSetCfg:public QObject
{
    Q_OBJECT
public:
	MIhmStatusSetCfg();
	~MIhmStatusSetCfg();
	void Init(QString sConfigKey);

	MStatusSetGroup * getStatusSetGroup(QString sGroupName);
private:

	bool MIhmStatusSetLoadGroupsFromRegister(IN char * pcKey);
	bool MIhmAnalyseStatusSetGroup( IN char* szGroupName, IN char * szDesc);

	StatusSetGroupList m_lstStatusSetGroups;
};


#endif	// MIHM_STATUS_SET_CFG_H
/*---------------------------- END OF FILE -------------------------*/
