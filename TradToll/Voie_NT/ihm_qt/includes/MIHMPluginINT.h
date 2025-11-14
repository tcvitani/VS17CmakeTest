#ifndef MIHM_PLUGIN_INT_H
#define MIHM_PLUGIN_INT_H

#include <QtPlugin>

QT_BEGIN_NAMESPACE
	class MIHMPluginDlgLogicINT;
	class QString;
QT_END_NAMESPACE


class MIHMPluginINT 
{
public:
	virtual ~MIHMPluginINT(){};

	virtual bool initialize(QString sCfgRegKey) = 0;
	virtual QString getLastError() = 0;

	virtual MIHMPluginDlgLogicINT * getPluginDlgLogic(int iDlgLogicType) = 0;
	virtual void releasePluginDlgLogic(MIHMPluginDlgLogicINT *pDlgLogic) = 0;

};

QT_BEGIN_NAMESPACE
	
	Q_DECLARE_INTERFACE(MIHMPluginINT, "com.sanefIts.IHM_Plugin_interface/1.0")
	
QT_END_NAMESPACE


#endif	
/*---------------------------- END OF FILE -------------------------*/
