/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
/* FILE:     MCfgLaneToolConfig.h											 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CFG_LANE_TOOL_CONFIG_H
#define CFG_LANE_TOOL_CONFIG_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QObject>
#include <QList>
#include <QString>
#include <QFont>
#include <QRect>
#include <QStringList>
#include <MTranslator.h>

#include "MDefines.h"
#include "MGlobalStructs.h"
#include "MHelpFuncs.h"
#include "MConfig.h"

extern "C"
{
	#include <noyau.h>
};

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

class MLangPickConfig;

/*-------------------------------- DEFINES:   -------------------------------*/

#define CFG_LANE_TOOL_LABEL_ERR_TITLE	"CONFIG LANE TOOL"
#define TOOL_GEN_SETTINGS_INI_FILE_KEY	"GENERAL_SETTINGS"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

class MToolEntryConfigData
{
	public:
		MToolEntryConfigData(QString sAppLabel):m_sAppLabel(sAppLabel),bActive(false),eCmdType(enuToolExec){};
		
		QString getCmdLine(){return sAppRunCmd;};
		QString getIniFilePAth(){return sAppIniFilePath;};

	enum enumCmdType
	{
		enuToolExec = 0,
		enuShellExec,
		enuOpenLocalUrl,
		enuOpenWebUrl
	};

	QString m_sAppLabel;
	QString sGroupLabel;
	QString sAppRunCmd;
	bool bActive;
	QString sAppIniFilePath;
	enumCmdType eCmdType;
	QString sCmdParameters;
};

class MCfgLaneToolConfig:public MConfig
{	
	public:
		MCfgLaneToolConfig(QString sAppBaseConfigKeyName);
		virtual ~MCfgLaneToolConfig();
		static MCfgLaneToolConfig * getCfg(){return (MCfgLaneToolConfig *)MConfig::getCfg();};

		bool loadConfig();

		virtual QString getDefaultConfigFilePath();

		MLangPickConfig * getLangPickConfig() { return m_pLangPickCfg; };
		
		QString getRegKeyAppParam(){ return m_RegKeyAppParam; }
		QString getRegKeyDevParam(){ return m_RegKeyDevParam; }
		QString getRegKeyModuleLAN(){ return m_RegKeyModuleLAN; }
		QString getRegKeyModuleTFT(){ return m_RegKeyModuleTFT; }

		QString getRegValPlazaName(){ return m_RegValPlazaName; }
		QString getRegValPlazaNumber(){ return m_RegValPlazaNumber; }
		QString getRegValLaneName(){ return m_RegValLaneName; }
		QString getRegValLaneNumber(){ return m_RegValLaneNumber; }
		QString getRegValLaneType(){ return m_RegValLaneType; }
		QString getRegValMsgServer(){ return m_RegValMsgServer; }
		QString getRegValFileServer(){ return m_RegValFileServer; }
		QString getRegValLaneDirection(){ return m_RegValLaneDirection; }
		QString getRegValFarePoint(){ return m_RegValFarePoint; }
		QString getRegValDualLane(){ return m_RegValDualLane; }
		QString getRegValHostName(){ return m_RegValHostName; }
		QString getRegValNVHostName(){ return m_RegValNVHostName; }

		QString getRegEditCommand(){ return m_RegEditCommand; }
		QString getShutDownCommand(){ return m_ShutDownCommand; }
		QString getSetDhcpCommand(){ return m_SetDhcpCommand; }
		QString getSetIpAddressCommand(){ return m_SetIpAddressCommand; }
		QString getWindowsFirewallCommand(){ return m_WindowsFirewallCommand; }
		QString getUpdateWorkGroupCommand(){ return m_UpdateWorkGroupCommand; }
		QString getRegKeyForHostName(){ return m_RegKeyForHostName; }

		QString getContextFilePath(){ return m_ContextFile; }
		QString getScriptPath(){ return m_ScriptPath; }
		QString getDefaultScriptFile(){ return m_DefaultScriptFile; }

		strucrt_lane_type * getLaneTypesConfig(){ return &m_LaneConfig; }
		strucrt_lane_directions * getLaneDirections(){ return &m_LaneDirections; }
		strucrt_manual_cfg * getManualConfig(){ return &m_ManualConfig; }
		strucrt_spec_params * getSpecParams(){ return &m_SpecParams; }

		bool doNotShowGeneralTab(){ return m_bDoNotShowGeneralTab; }
		bool doNotShowNetworkTab(){ return m_bDoNotShowNetworkTab; }
		bool doNotShowPlazaSpecificTab(){ return m_bDoNotShowPlazaSpecificTab; }
		bool doNotShowLaneSpecificTab(){ return m_bDoNotShowLaneSpecificTab; }
		bool doNotShowDevicesTab(){ return m_bDoNotShowDevicesTab; }
		bool doNotShowChannelNumber(){ return m_bDoNotShowChannelNumber; }
		bool doNotShowDualLane(){ return m_bDoNotShowDualLane; }
		bool doNotShowLaneDirection(){ return m_bDoNotShowLaneDirection; }
		bool doNotShowFarePoint(){ return m_bDoNotShowFarePoint; }
		bool doNotShowBtnGenActualCfg(){ return m_bDoNotShowBtnGenActualCfg; }
		bool doNotShowBtnGenDefaultCfg(){ return m_bDoNotShowBtnGenDefaultCfg; }
		bool doNotShowBtnPlazaActualCfg(){ return m_bDoNotShowBtnPlazaActualCfg; }
		bool doNotShowBtnPlazaDefaultCfg(){ return m_bDoNotShowBtnPlazaDefaultCfg; }
		bool doNotShowBtnLaneActualCfg(){ return m_bDoNotShowBtnLaneActualCfg; }
		bool doNotShowBtnLaneDefaultCfg(){ return m_bDoNotShowBtnLaneDefaultCfg; }
		bool doNotShowBtnDevActualCfg(){ return m_bDoNotShowBtnDevActualCfg; }
		bool doNotShowBtnDevDefaultCfg(){ return m_bDoNotShowBtnDevDefaultCfg; }

		bool forcePcShutDown(){ return m_bForcePcShutDown; }
		bool shutDownPcMessage(){ return m_bShutDownPcMessage; }
		bool askForColdStart(){ return m_bAskForColdStart; }

		int getMinPlazaNum(){ return m_iMinPlazaNum; }
		int getMaxPlazaNum(){ return m_iMaxPlazaNum; }

		int getMinLaneNum(){ return m_iMinLaneNum; }
		int getMaxLaneNum(){ return m_iMaxLaneNum; }

		int getMinChannelNum(){ return m_iMinChannelNum; }
		int getMaxChannelNum(){ return m_iMaxChannelNum; }

		int getSpecParamStringMaxLength(){ return m_iSpecParamStringMaxLength; }
		QString getSpecParamNumberInputMask(){ return m_sSpecParamNumberInputMask; }

		bool showAllNetworkAdapters(){ return m_bShowAllNetworkAdapters; }

		bool getPlazaIndex(int iPlazaNum, int *pPlazaIdx);
		bool getLaneIndex(int iPlazaIdx, int iLaneNum, int *pLaneIdx);

		QList <MToolEntryConfigData*> * getToolEntryCfgList(){return &m_lstToolEntryCfg;};

	private:
		MLangPickConfig		*m_pLangPickCfg;

		QString				m_sInstanceName;

		QString				m_RegKeyAppParam;
		QString				m_RegKeyDevParam;
		QString				m_RegKeyModuleLAN;
		QString				m_RegKeyModuleTFT;

		QString				m_RegValPlazaName;
		QString				m_RegValPlazaNumber;
		QString				m_RegValLaneName;
		QString				m_RegValLaneNumber;
		QString				m_RegValLaneType;
		QString				m_RegValMsgServer;
		QString				m_RegValFileServer;
		QString				m_RegValLaneDirection;
		QString				m_RegValFarePoint;
		QString				m_RegValDualLane;
		QString				m_RegValHostName;
		QString				m_RegValNVHostName;

		QString				m_RegEditCommand;
		QString				m_ShutDownCommand;
		QString				m_SetDhcpCommand;
		QString				m_SetIpAddressCommand;
		QString				m_WindowsFirewallCommand;
		QString				m_UpdateWorkGroupCommand;
		QString				m_RegKeyForHostName;

		QString				m_ContextFile;
		QString				m_ScriptPath;
		QString				m_DefaultScriptFile;

		bool				m_bDoNotShowGeneralTab;
		bool				m_bDoNotShowNetworkTab;
		bool				m_bDoNotShowPlazaSpecificTab;
		bool				m_bDoNotShowLaneSpecificTab;
		bool				m_bDoNotShowDevicesTab;
		bool				m_bDoNotShowChannelNumber;
		bool				m_bDoNotShowDualLane;
		bool				m_bDoNotShowLaneDirection;
		bool				m_bDoNotShowFarePoint;
		bool				m_bDoNotShowBtnGenActualCfg;
		bool				m_bDoNotShowBtnGenDefaultCfg;
		bool				m_bDoNotShowBtnPlazaActualCfg;
		bool				m_bDoNotShowBtnPlazaDefaultCfg;
		bool				m_bDoNotShowBtnLaneActualCfg;
		bool				m_bDoNotShowBtnLaneDefaultCfg;
		bool				m_bDoNotShowBtnDevActualCfg;
		bool				m_bDoNotShowBtnDevDefaultCfg;

		bool				m_bForcePcShutDown;
		bool				m_bShutDownPcMessage;
		bool				m_bAskForColdStart;

		int					m_iMinPlazaNum;
		int					m_iMaxPlazaNum;

		int					m_iMinLaneNum;
		int					m_iMaxLaneNum;

		int					m_iMinChannelNum;
		int					m_iMaxChannelNum;

		int					m_iSpecParamStringMaxLength;
		QString				m_sSpecParamNumberInputMask;

		bool				m_bShowAllNetworkAdapters;

		strucrt_lane_type		m_LaneConfig;
		strucrt_lane_directions	m_LaneDirections;
		strucrt_manual_cfg		m_ManualConfig;
		strucrt_spec_params		m_SpecParams;

		//-------------------------------

		QStringList						getAppListLabels(){ return m_sAppListLabels; }
		QStringList						m_sAppListLabels;
		QList <MToolEntryConfigData*>	m_lstToolEntryCfg;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/