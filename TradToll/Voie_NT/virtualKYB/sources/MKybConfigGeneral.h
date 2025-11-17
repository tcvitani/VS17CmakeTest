#ifndef MKyb_CONFIG_GENERAL_H
#define MKyb_CONFIG_GENERAL_H

#include <QString>
#include <QThread>
#include <MKybTypeSettings.h>

extern "C" {
	#include <noyau.h>
	#include <virt_kyb.h>

}

#define KYB_REG_VAL_MANUAL_STARTUP_POSITION		"ManualStartupPosition"
#define KYB_REG_VAL_STARTUP_POSITION_LEFT		"StartupPositionLeft"
#define KYB_REG_VAL_STARTUP_POSITION_TOP		"StartupPositionTop"
#define KYB_REG_VAL_SHOW_TITLE_BAR				"ShowTitleBar"
#define KYB_REG_VAL_UI_TEMPLATE_ROOT			"UITemplateRoot"
#define KYB_REG_VAL_CFG_FILES_ROOT				"CfgFilesRoot"
#define KYB_REG_VAL_ANI_THREAD_PRIORITY			"PriorityANIThread"
#define IHM_REG_VAL_HIDE_CURSOR					"HideCursor"

//------------------------------------------------------------------
//General configuration data container class
//The 	read from registry
//------------------------------------------------------------------
class MKybConfigGeneral 
{
	
	friend class MKybAniInitData;
	
	public:
		MKybConfigGeneral();
		~MKybConfigGeneral();
		
		QString getModuleMboxName(){return m_sModuleMboxName;};
		QString getModuleConfigKey(){return m_sModuleConfigKey;};

		//used to initialize all configuration files at the app start
		bool loadConfigFromRegistry(QString sMboxName); 

		static MKybConfigGeneral * getCfg(){return m_pGenCfg;};

		static QString getCleanPath(QString sPath, QString sSubPath);
		QThread::Priority getAniPriority(){ return m_eAniThreadPriority;};

		QString getCfgFilesRoot(){return m_sCfgFilesRoot;};
		QString getUIFilesRoot(){return m_sUITemplateRoot;};

		MKybTypeSettings * getKybTypeSettings(QString sCfgId);

		bool shouldHideCursor(){ return m_bHideCursor; };
	private:
		bool loadKeyboardConfigurations();

		QThread::Priority m_eAniThreadPriority; 
		
		static MKybConfigGeneral * m_pGenCfg;


		QString m_sModuleMboxName;
		QString m_sModuleConfigKey;	

		QString m_sUITemplateRoot;
		QString	m_sCfgFilesRoot;
		
		QList <MKybTypeSettings *> m_lstKybSettings;

		bool	m_bHideCursor;

};






#endif


