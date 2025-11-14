#ifndef MIHM_CONFIG_DYNAMIC_H
#define MIHM_CONFIG_DYNAMIC_H


#include <QString>


//Default data read from dynamic config xml
class MIhmConfigDynamic 
{
	public:
		MIhmConfigDynamic();
		~MIhmConfigDynamic();
	
		bool loadConfigFromFile(QString sConfigFilenameAndPath);
		int getMaxWebClients(){return m_iMaxWebClients;};
		int getMaxMiniWebClients(){return m_iMaxMiniWebClients;};
		int getIdleTimeoutDsk(){return m_iIdleTimeoutDsk;};
		int getIdleTimeoutWeb(){return m_iIdleTimeoutWeb;};
		int getTakeCtrlTimeout(){return m_iTakeCtrlTimeout;};


	private:
		bool getIntValue(QString sParamValue, QString sParamName, int &iRetVal);
	
		int	m_iTakeCtrlTimeout;

		int	m_iMaxWebClients;
		int	m_iMaxMiniWebClients;
		int m_iIdleTimeoutDsk;
		int m_iIdleTimeoutWeb;
};






#endif


