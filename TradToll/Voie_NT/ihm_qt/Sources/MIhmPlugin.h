#ifndef MIHM_PLUGIN_H
#define MIHM_PLUGIN_H

#include <QObject>


class MIhmPlugin:public QObject
{
    Q_OBJECT
public:
	MIhmPlugin();
	virtual ~MIhmPlugin();

	bool initializePlugin(QString sDllName, QString sPluginMailbox) = 0;
	QString getDllName(){ return sDllName; };


protected:
	QString sDllName;
	bool bLoaded;
	
};


#endif	
/*---------------------------- END OF FILE -------------------------*/
