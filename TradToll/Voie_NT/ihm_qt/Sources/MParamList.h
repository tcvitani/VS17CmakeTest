#ifndef MParamList_H
#define MParamList_H

#include <QString>
#include <QList>



struct MParam
{
	QString param_name;
	QString param_value;
};



class MParamList
{
	
public:
	MParamList();
	~MParamList();
	
	bool loadListFromRegistry(QString sRegKey);
	QString getParam(QString sParamName);
	int getCount(){return m_lstParams.count();};
	MParam * at(int i){ return (MParam *)m_lstParams.at(i);}
private:

	MParam * newParam(QString sParamName, QString sValue);

	QList <MParam*> m_lstParams;
};


#endif


