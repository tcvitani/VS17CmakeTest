#ifndef MIHM_CONFIG_ACTIONS_H
#define MIHM_CONFIG_ACTIONS_H

#include<QList>
#include<QString>

class MIhmAction;



class MIhmConfigActions
{

public:
	MIhmConfigActions();
	~MIhmConfigActions();

	bool loadConfigFromFile(QString sConfigFilePath);
	
	MIhmAction * findAction(QString sActionID);
	
	QString getCursorIdForAction(QString sActionID);

	static bool lessThan(const MIhmAction* x1, const MIhmAction* x2);

private:
	QList<MIhmAction*> m_lstAvailableActions;
};


#endif


