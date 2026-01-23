/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
/* FILE:	 MCfgLaneToolConfig.cpp											 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QApplication>
#include <QSettings>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QTextCodec>

#include "MDefines.h"
#include "MCfgToolConfig.h"
#include "MTracer.h"
#include "MRegUtils.h"
#include "MLangPickConfig.h"

extern "C"
{
	#include <reg.h>
};

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

#define DCP_TOOL_CONFIG_FILE_PATH					"csr_ConfigLane_Tool.ini"
#define DCP_TOOL_INI_INSTANCE_NAME					"InstanceName"

#define MENU_SETTINGS_INI_FILE_APP_LABELS			"APP_LIST_LABELS"
#define MENU_SETTINGS_INI_FILE_GROUP_LABEL			"GROUP_LABEL"
#define MENU_SETTINGS_INI_FILE_APP_RUN_CMD			"APP_RUN_CMD"
#define MENU_SETTINGS_INI_FILE_APP_ACTIVE			"APP_ACTIVE"
#define MENU_SETTINGS_INI_FILE_APP_INI_FILE_PATH	"APP_INI_FILE_PATH"
#define MENU_SETTINGS_INI_CMD_TYPE					"CMD_TYPE"
#define MENU_SETTINGS_INI_CMD_PARAMETERS			"CMD_PARAMETERS"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -----------------------------*/

MCfgLaneToolConfig::MCfgLaneToolConfig(QString sAppBaseConfigKeyName):
					MConfig(sAppBaseConfigKeyName),
					m_pLangPickCfg(NULL)
{

}

MCfgLaneToolConfig::~MCfgLaneToolConfig()
{
	if (m_pLangPickCfg != NULL)
		delete m_pLangPickCfg;
}

QString MCfgLaneToolConfig::getDefaultConfigFilePath()
{
	return QString("%1/%2")
		.arg(QApplication::applicationDirPath())
		.arg(DCP_TOOL_CONFIG_FILE_PATH);
}

bool MCfgLaneToolConfig::loadConfig()
{
	QString			sValue;
	QStringList		sChildKeys;
	int				iIdx = 0;
	int				iIdx2 = 0;
	int				iIdx3 = 0;

	TRACE_D("MCfgLaneToolConfig::loadConfig()" );

	if (!MConfig::loadConfig())
		return false;
	
	QTextCodec *codec = QTextCodec::codecForName("Windows-1250");//!!! to set to the correct System settings (Windows-1250)
	QTextCodec::setCodecForLocale(codec);

	// GET .INI CONFIG FILE
    QSettings sSettings (getConfigFilePath(), QSettings::IniFormat);
	//sSettings.setIniCodec("UTF-8");

	//////////////////////////////////////////////////////////////////////////////////

	sSettings.beginGroup(TOOL_GEN_SETTINGS_INI_FILE_KEY);

	// GET INSTANCE NAME FROM .INI FILE
	m_sInstanceName = sSettings.value(DCP_TOOL_INI_INSTANCE_NAME, "").toString();

	sSettings.endGroup();

	///////////////////////////////////////////////////////////////////////////

	sSettings.beginGroup(CFG_LANE_TOOL_INI_TOOL_PARAMS);
	
	// GENERAL PARAMETERS TAB
	m_bDoNotShowGeneralTab = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_GENERAL, "true").toBool();

	// NETWORK PARAMETERS TAB
	m_bDoNotShowNetworkTab = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_NETWORK, "true").toBool();

	// SPECIFIC PLAZA PARAMETERS TAB
	m_bDoNotShowPlazaSpecificTab = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_PLAZA_S, "true").toBool();

	// SPECIFIC LANE PARAMETERS TAB
	m_bDoNotShowLaneSpecificTab = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_LANE_S, "true").toBool();

	// DEVICES TAB
	m_bDoNotShowDevicesTab = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_DEVICES, "true").toBool();

	// Channel number
	m_bDoNotShowChannelNumber = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_CH_NUM, "true").toBool();

	// Dual lane
	m_bDoNotShowDualLane = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_DUAL_LANE, "true").toBool();

	// Lane direction
	m_bDoNotShowLaneDirection = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_LANE_DIR, "true").toBool();

	// Fare point
	m_bDoNotShowFarePoint = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_FARE, "true").toBool();

	// button "general tab - get actual configuration"
	m_bDoNotShowBtnGenActualCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_GAC, "true").toBool();

	// button "general tab - get default configuration"
	m_bDoNotShowBtnGenDefaultCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_GDC, "true").toBool();

	// button "devices tab - get actual configuration"
	m_bDoNotShowBtnDevActualCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_DAC, "true").toBool();

	// button "devices tab - get default configuration"
	m_bDoNotShowBtnDevDefaultCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_DDC, "true").toBool();

	// button "plaza specifica tab - get actual configuration"
	m_bDoNotShowBtnPlazaActualCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_PSAC, "true").toBool();

	// button "plaza specifica tab - get default configuration"
	m_bDoNotShowBtnPlazaDefaultCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_PSDC, "true").toBool();

	// button "lane specifica tab - get actual configuration"
	m_bDoNotShowBtnLaneActualCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_LSAC, "true").toBool();

	// button "lane specifica tab - get default configuration"
	m_bDoNotShowBtnLaneDefaultCfg = sSettings.value(CFG_LANE_TOOL_DO_NOT_SHOW_BTN_LSDC, "true").toBool();

	// min. and max. plaza number
	m_iMinPlazaNum = sSettings.value(CFG_LANE_TOOL_MIN_PLAZA_NUM, "1").toInt();
	m_iMaxPlazaNum = sSettings.value(CFG_LANE_TOOL_MAX_PLAZA_NUM, "999").toInt();

	// min. and max lane number
	m_iMinLaneNum = sSettings.value(CFG_LANE_TOOL_MIN_LANE_NUM, "1").toInt();
	m_iMaxLaneNum = sSettings.value(CFG_LANE_TOOL_MAX_LANE_NUM, "999").toInt();

	// min. and max channel number
	m_iMinChannelNum = sSettings.value(CFG_LANE_TOOL_MIN_CHANNEL_NUM, "1").toInt();
	m_iMaxChannelNum = sSettings.value(CFG_LANE_TOOL_MAX_CHANNEL_NUM, "999").toInt();

	m_iSpecParamStringMaxLength = sSettings.value(CFG_LANE_TOOL_SPEC_P_STRING_LEN, "50").toInt();
	
	// show all network adapters
	m_bShowAllNetworkAdapters = sSettings.value(CFG_LANE_TOOL_SHOW_ALL_NET_ADAPTERS, "true").toBool();

	// max. value of specific parameter (number)
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_SPEC_P_NUM_INPUT_MASK, "999999999").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_SPEC_P_NUM_INPUT_MASK)
			.arg(getConfigFilePath()));

		return false;
	}
	m_sSpecParamNumberInputMask = sValue;

	// GET GENERAL PARAMETERS REGISTRY KEY
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_APP_PARAM, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_APP_PARAM)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegKeyAppParam = sValue;

	// GET DEVICES REGISTRY KEY
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_DEV_PARAM, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_DEV_PARAM)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegKeyDevParam = sValue;

	// GET LAN MODULE REGISTRY KEY
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_MODULE_LAN, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_MODULE_LAN)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegKeyModuleLAN = sValue;

	// GET TFT MODULE REGISTRY KEY
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_MODULE_TFT, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_MODULE_TFT)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegKeyModuleTFT = sValue;

	// GET PLAZA NAME REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_PLAZA_NAME, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_PLAZA_NAME)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValPlazaName = sValue;

	// GET PLAZA NUMBER REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_PLAZA_NUMBER, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_PLAZA_NUMBER)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValPlazaNumber = sValue;

	// GET LANE NAME REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_LANE_NAME, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_LANE_NAME)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValLaneName = sValue;

	// GET LANE NUMBER REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_LANE_NUMBER, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_LANE_NUMBER)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValLaneNumber = sValue;

	// GET LANE TYPE REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_LANE_TYPE, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_LANE_TYPE)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValLaneType = sValue;

	// GET MESSAGE SERVER REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_MSG_SERVER, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_MSG_SERVER)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValMsgServer = sValue;

	// GET FILE SERVER REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_FILE_SERVER, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_FILE_SERVER)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValFileServer = sValue;

	// GET LANE DIRECTION REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_LANE_DIRECT, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_LANE_DIRECT)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValLaneDirection = sValue;

	// GET FARE POINT REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_FARE_POINT, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_FARE_POINT)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValFarePoint = sValue;

	// GET DUAL LANE REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_DUAL_LANE, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_DUAL_LANE)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValDualLane = sValue;

	// GET HOSTNAME REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_HOST_NAME, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_HOST_NAME)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValHostName = sValue;

	// GET NV HOSTNAME REGISTRY VALUE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_VAL_NV_HOST_NAME, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_VAL_NV_HOST_NAME)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegValNVHostName = sValue;

	// GET "REGEDIT" COMMAND
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_EDIT_COMMAND, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_EDIT_COMMAND)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegEditCommand = sValue;
	
	// GET "SHUT DOWN" COMMAND
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_SHUTDOWN_COMMAND, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_SHUTDOWN_COMMAND)
			.arg(getConfigFilePath()));

		return false;
	}
	m_ShutDownCommand = sValue;

	// GET "SET DHCP" COMMAND
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_SET_DHCP_COMMAND, "").toString();
	if (sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_SET_DHCP_COMMAND)
			.arg(getConfigFilePath()));

		return false;
	}
	m_SetDhcpCommand = sValue;

	// GET "SET IP ADDRESS" COMMAND
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_SET_IP_ADDR_COMMAND, "").toString();
	if (sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_SET_IP_ADDR_COMMAND)
			.arg(getConfigFilePath()));

		return false;
	}
	m_SetIpAddressCommand = sValue;

	// GET "SET WORK GROUP" COMMAND
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_SET_WORK_GROUP_CMD, "").toString();
	if (sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_SET_WORK_GROUP_CMD)
			.arg(getConfigFilePath()));

		return false;
	}
	m_UpdateWorkGroupCommand = sValue;

	// GET "WINDOWS FIREWALL" COMMAND
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_WIN_FIREWALL_COMMAND, "").toString();
	if (sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_WIN_FIREWALL_COMMAND)
			.arg(getConfigFilePath()));

		return false;
	}
	m_WindowsFirewallCommand = sValue;

	// GET REGISTRY KEY FOR HOST NAME
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_REG_KEY_FOR_HOST_NAME, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_REG_KEY_FOR_HOST_NAME)
			.arg(getConfigFilePath()));

		return false;
	}
	m_RegKeyForHostName = sValue;

	// CONTEXT FILE
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_CONTEXT_FILE, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_CONTEXT_FILE)
			.arg(getConfigFilePath()));

		return false;
	}
	m_ContextFile = sValue;

	//SCRIPT PATH
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_SCRIPT_PATH, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_SCRIPT_PATH)
			.arg(getConfigFilePath()));

//		return false;
	}
	m_ScriptPath = sValue;

	//DEFAULT SCRIPT PATH
	sValue.clear();
	sValue = sSettings.value(CFG_LANE_TOOL_DEFAULT_SCRIPT_FILE, "").toString();
	if (sValue.isEmpty() || sValue.isNull())
	{
		TRACE_W(QString("MCfgLaneToolConfig::loadConfig() => Error loading [%1] from .ini file: %2")
			.arg(CFG_LANE_TOOL_DEFAULT_SCRIPT_FILE)
			.arg(getConfigFilePath()));

//		return false;
	}
	m_DefaultScriptFile = sValue;
	
	// FORCE PC SHUT DOWN
	m_bForcePcShutDown = sSettings.value(CFG_LANE_TOOL_FORCE_SHUTDOWN, "false").toBool();

	// SHUT DOWN PC - MESSAGE
	m_bShutDownPcMessage = sSettings.value(CFG_LANE_TOOL_MSG_SHUTDOWN, "true").toBool();

	// ASK FOR COLD START
	m_bAskForColdStart = sSettings.value(CFG_LANE_TOOL_ASK_FOR_COLD_START, "false").toBool();

    sSettings.endGroup();

	///////////////////////////////////////////////////////////////////////////

	// get lane types
	sSettings.beginGroup(CFG_LANE_TOOL_LANE_TYPES);

	m_LaneConfig.iNbItems = 0;

	sChildKeys = sSettings.childKeys();
	foreach(const QString &childKey, sChildKeys)
	{
		if (m_LaneConfig.iNbItems >= MAX_LANE_TYPES)
			break;

		m_LaneConfig.sLaneTypeItem[m_LaneConfig.iNbItems].sLabel = sSettings.value(childKey).toString();
		m_LaneConfig.iNbItems++;
	}

	sSettings.endGroup();

	if (m_LaneConfig.iNbItems == 0)
	{
		TRACE_W(QString("MCfgLaneToolConfig()::loadConfig() => There is no defined lane types!")
			.arg(getConfigFilePath()));

		return false;
	}

	// get configuration for each lane type
	for (iIdx = 0; iIdx < m_LaneConfig.iNbItems; iIdx++)
	{
		sSettings.beginGroup(m_LaneConfig.sLaneTypeItem[iIdx].sLabel);

		// get lane type
		m_LaneConfig.sLaneTypeItem[iIdx].sLaneType = sSettings.value(CFG_LANE_TOOL_LANE_TYPE, "").toString();

		// get path to the registry file that contains configuration
		m_LaneConfig.sLaneTypeItem[iIdx].sFileDir = sSettings.value(CFG_LANE_TOOL_FILE_DIR, "").toString();

		// get the registry file name that contains configuration to be applied
		m_LaneConfig.sLaneTypeItem[iIdx].sInstFile = sSettings.value(CFG_LANE_TOOL_INST_FILE, "").toString();

		// get the registry file name that contains configuration to be uninstalled
		m_LaneConfig.sLaneTypeItem[iIdx].sUninstFile = sSettings.value(CFG_LANE_TOOL_UNINST_FILE, "").toString();

		m_LaneConfig.sLaneTypeItem[iIdx].sDevGroupLabel = sSettings.value(CFG_LANE_TOOL_DEVICE_GROUP, "").toString();

		sSettings.endGroup();
	}

	// get all device groups for each lane type
	for (iIdx = 0; iIdx < m_LaneConfig.iNbItems; iIdx++)
	{
		if (!m_LaneConfig.sLaneTypeItem[iIdx].sDevGroupLabel.isEmpty())
		{
			sSettings.beginGroup(m_LaneConfig.sLaneTypeItem[iIdx].sDevGroupLabel);

			m_LaneConfig.sLaneTypeItem[iIdx].iNbItems = 0;

			sChildKeys = sSettings.childKeys();
			foreach(const QString &childKey, sChildKeys)
			{
				if (m_LaneConfig.sLaneTypeItem[iIdx].iNbItems >= MAX_DEV_GROUPS)
					break;

				m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[m_LaneConfig.sLaneTypeItem[iIdx].iNbItems].sLabel =
					sSettings.value(childKey).toString();

				m_LaneConfig.sLaneTypeItem[iIdx].iNbItems++;
			}

			sSettings.endGroup();
		}
	}

	// get all device for each device group type
	for (iIdx = 0; iIdx < m_LaneConfig.iNbItems; iIdx++)
	{
		for (iIdx2 = 0; iIdx2 < m_LaneConfig.sLaneTypeItem[iIdx].iNbItems; iIdx2++)
		{
			if (!m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sLabel.isEmpty())
			{
				sSettings.beginGroup(m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sLabel);

				m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems = 0;

				sChildKeys = sSettings.childKeys();
				foreach(const QString &childKey, sChildKeys)
				{
					if (m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems >= MAX_DEV_ITEMS)
						break;

					m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems].sLabel =
						sSettings.value(childKey).toString();

					m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems++;
				}

				sSettings.endGroup();
			}
		}
	}

	for (iIdx = 0; iIdx < m_LaneConfig.iNbItems; iIdx++)
	{
		for (iIdx2 = 0; iIdx2 < m_LaneConfig.sLaneTypeItem[iIdx].iNbItems; iIdx2++)
		{
			for (iIdx3 = 0; iIdx3 < m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].iNbItems; iIdx3++)
			{
				sSettings.beginGroup(m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sLabel);

				// get device registry sub key path
				m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sSubKeyPath = 
					sSettings.value(CFG_LANE_TOOL_SUBKEY_PATH, "").toString();

				// get device registry sub key name
				m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sSubKeyName =
					sSettings.value(CFG_LANE_TOOL_SUBKEY_NAME "").toString();

				// get the device registry file name that contains configuration to be applied
				m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sInstFile = 
					sSettings.value(CFG_LANE_TOOL_INST_FILE, "").toString();

				// get the device registry file name that contains configuration to be uninstalled
				m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].sUninstFile = 
					sSettings.value(CFG_LANE_TOOL_UNINST_FILE, "").toString();

				// is device defualt checked 
				m_LaneConfig.sLaneTypeItem[iIdx].sDeviceGroup[iIdx2].sDeviceItem[iIdx3].bIsDefaultChecked =
					sSettings.value(CFG_LANE_TOOL_IS_DEFAULT, "false").toBool();

				sSettings.endGroup();
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////

	// get lane directions
	sSettings.beginGroup(CFG_LANE_TOOL_LANE_DIRECTIONS);

	m_LaneDirections.iNbItems = 0;

	sChildKeys = sSettings.childKeys();
	foreach(const QString &childKey, sChildKeys)
	{
		if (m_LaneDirections.iNbItems >= MAX_LANE_DIRECTIONS)
			break;

		m_LaneDirections.sLaneDirections[m_LaneDirections.iNbItems] = sSettings.value(childKey).toString();
		m_LaneDirections.iNbItems++;
	}

	sSettings.endGroup();

	///////////////////////////////////////////////////////////////////////////

	// get manual configuration
	sSettings.beginGroup(CFG_LANE_TOOL_MANUAL_CONFIG);

	m_ManualConfig.iNbItems = 0;

	// get all defined plazas
	sChildKeys = sSettings.childKeys();
	foreach(const QString &childKey, sChildKeys)
	{
		if (m_ManualConfig.iNbItems >= MAX_PLAZA_ITEMS)
			break;

		m_ManualConfig.sPlazaConfig[m_ManualConfig.iNbItems].sPlazaCfgLabel = sSettings.value(childKey).toString();
		m_ManualConfig.iNbItems++;
	}

	sSettings.endGroup();
	
	// get plaza manual configuration
	for (iIdx = 0; iIdx < m_ManualConfig.iNbItems; iIdx++)
	{
		if (!m_ManualConfig.sPlazaConfig[iIdx].sPlazaCfgLabel.isEmpty())
		{
			sSettings.beginGroup(m_ManualConfig.sPlazaConfig[iIdx].sPlazaCfgLabel);

			// get plaza number
			m_ManualConfig.sPlazaConfig[iIdx].iPlazaNum = sSettings.value(CFG_LANE_TOOL_PLAZA_NUMBER, "").toInt();

			// get plaza name
			m_ManualConfig.sPlazaConfig[iIdx].sPlazaName = sSettings.value(CFG_LANE_TOOL_PLAZA_NAME, "").toString();

			// get plaza file server
			m_ManualConfig.sPlazaConfig[iIdx].sPlazaFileServer = sSettings.value(CFG_LANE_TOOL_PLAZA_FILE_SEVER, "").toString();

			// get plaza message server
			m_ManualConfig.sPlazaConfig[iIdx].sPlazaMsgServer = sSettings.value(CFG_LANE_TOOL_PLAZA_MSG_SEVER, "").toString();

			// get lane configuration label
			m_ManualConfig.sPlazaConfig[iIdx].sLaneCfgLabel = sSettings.value(CFG_LANE_TOOL_LANE_CONFIG, "").toString();

			sSettings.endGroup();
		}
	}

	// get all defined lanes for each of plazas
	for (iIdx = 0; iIdx < m_ManualConfig.iNbItems; iIdx++)
	{
		if (!m_ManualConfig.sPlazaConfig[iIdx].sLaneCfgLabel.isEmpty())
		{
			sSettings.beginGroup(m_ManualConfig.sPlazaConfig[iIdx].sLaneCfgLabel);

			m_ManualConfig.sPlazaConfig[iIdx].iNbItems = 0;

			sChildKeys = sSettings.childKeys();
			foreach(const QString &childKey, sChildKeys)
			{
				if (m_ManualConfig.sPlazaConfig[iIdx].iNbItems >= MAX_LANE_ITEMS)
					break;

				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[m_ManualConfig.sPlazaConfig[iIdx].iNbItems].sLaneLabel =
					sSettings.value(childKey).toString();

				m_ManualConfig.sPlazaConfig[iIdx].iNbItems++;
			}

			sSettings.endGroup();
		}
	}

	// get configuration for all lanes
	for (iIdx = 0; iIdx < m_ManualConfig.iNbItems; iIdx++)
	{
		for (iIdx2 = 0; iIdx2 < m_ManualConfig.sPlazaConfig[iIdx].iNbItems; iIdx2++)
		{
			if (!m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLaneLabel.isEmpty())
			{
				sSettings.beginGroup(m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLaneLabel);

				// get lane type
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLaneType = sSettings.value(CFG_LANE_TOOL_LANE_TYPE, "").toString();

				// get lane number
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].iLaneNum = sSettings.value(CFG_LANE_TOOL_LANE_NUMBER, "").toInt();

				// get lane name
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLaneName = sSettings.value(CFG_LANE_TOOL_LANE_NAME, "").toString();

				// get lane direction
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLaneDirection = sSettings.value(CFG_LANE_TOOL_LANE_DIRECTION, "").toString();

				// get fare point
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sFarePoint = sSettings.value(CFG_LANE_TOOL_LANE_FARE_POINT, "").toString();

				// is dual lane?
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].bDualLane = sSettings.value(CFG_LANE_TOOL_LANE_DUAL_LANE, "false").toBool();

				// computer name
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sComputerName = sSettings.value(CFG_LANE_TOOL_LANE_COMPUTER_NAME, "").toString();

				// script file
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sScriptCommand = sSettings.value(CFG_LANE_TOOL_SCRIPT_FILE, "").toString();

				// workgroup
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sWorkGroup = sSettings.value(CFG_LANE_TOOL_LANE_WORK_GROUP, "").toString();

				// network configuration label
				m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLabelNetworkCfg = sSettings.value(CFG_LANE_TOOL_LANE_NETWORK_CONFIG, "").toString();

				sSettings.endGroup();

				// get network configuration
				if (!m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLabelNetworkCfg.isEmpty())
				{
					sSettings.beginGroup(m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sLabelNetworkCfg);

					m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].iNbNetCfg = 0;

					sChildKeys = sSettings.childKeys();
					foreach(const QString &childKey, sChildKeys)
					{
						if (m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].iNbNetCfg >= MAX_NB_NETWORK_CONFIG)
							break;

						m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].iNbNetCfg].sLabel =
							sSettings.value(childKey).toString();

						m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].iNbNetCfg++;
					}

					sSettings.endGroup();

					for (iIdx3 = 0; iIdx3 < m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].iNbNetCfg; iIdx3++)
					{
						if (!m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].sLabel.isEmpty())
						{
							sSettings.beginGroup(m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].sLabel);

							// get network adapter name
							m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].sAdapterName = 
								sSettings.value(CFG_LANE_TOOL_LANE_ADAPTER_NAME, "").toString();

							// obtain an ip address auomatically
							m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].bAutoIpAddress =
								sSettings.value(CFG_LANE_TOOL_LANE_AUTO_IP_ADDRESS, "false").toBool();

							// update ip settings by default
							m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].bUpdateIpSettings =
								sSettings.value(CFG_LANE_TOOL_LANE_UPD_IP_SET_DEF, "false").toBool();

							// get ip addresses
							m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].sIpAddress =
								sSettings.value(CFG_LANE_TOOL_LANE_IP_ADDRESS, "").toString();

							// get subnet mask
							m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].sSubnetMask =
								sSettings.value(CFG_LANE_TOOL_LANE_SUBNET_MASK, "").toString();

							// get default gateway
							m_ManualConfig.sPlazaConfig[iIdx].sLaneConfig[iIdx2].sNetworkConfig[iIdx3].sDefaultGateWay =
								sSettings.value(CFG_LANE_TOOL_LANE_DEFAULT_GATEWAY, "").toString();

							sSettings.endGroup();
						}
					}
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////

	// get plaza specific parameters
	sSettings.beginGroup(CFG_LANE_TOOL_PLAZA_SPEC_PARAM);

	m_SpecParams.iNbPlazaItems = 0;

	sChildKeys = sSettings.childKeys();
	foreach(const QString &childKey, sChildKeys)
	{
		if (m_LaneConfig.iNbItems >= MAX_PLAZA_ITEMS)
			break;

		m_SpecParams.sPlazaParams[m_SpecParams.iNbPlazaItems].sLabel = sSettings.value(childKey).toString();
		m_SpecParams.iNbPlazaItems++;
	}

	sSettings.endGroup();

	for (iIdx = 0; iIdx < m_SpecParams.iNbPlazaItems; iIdx++)
	{
		if (!m_SpecParams.sPlazaParams[iIdx].sLabel.isEmpty())
		{
			sSettings.beginGroup(m_SpecParams.sPlazaParams[iIdx].sLabel);

			m_SpecParams.sPlazaParams[iIdx].iNbItems = 0;

			sChildKeys = sSettings.childKeys();
			foreach(const QString &childKey, sChildKeys)
			{
				if (m_SpecParams.sPlazaParams[iIdx].iNbItems >= MAX_NB_PARAM)
					break;

				if (childKey == QString(CFG_LANE_TOOL_PLAZA_NUMBER))
				{
					m_SpecParams.sPlazaParams[iIdx].iPlazaNum = sSettings.value(childKey).toInt();
				}
				else
				{
					m_SpecParams.sPlazaParams[iIdx].sParams[m_SpecParams.sPlazaParams[iIdx].iNbItems].sLabel =
						sSettings.value(childKey).toString();

					m_SpecParams.sPlazaParams[iIdx].iNbItems++;
				}
			}

			sSettings.endGroup();
		}
	}

	for (iIdx = 0; iIdx < m_SpecParams.iNbPlazaItems; iIdx++)
	{
		if (!m_SpecParams.sPlazaParams[iIdx].sLabel.isEmpty())
		{
			for (iIdx2 = 0; iIdx2 < m_SpecParams.sPlazaParams[iIdx].iNbItems; iIdx2++)
			{
				if (!m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].sLabel.isEmpty())
				{
					sSettings.beginGroup(m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].sLabel);

					// get parameter label
					m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].sParamLabel = sSettings.value(CFG_LANE_TOOL_PARAM_LABEL, "").toString();

					// get parameter default value
					m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].sDefaultValue = sSettings.value(CFG_LANE_TOOL_DEFAULT_VALUE, "").toString();

					// get parameter registry key
					m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].sParamRegKey = sSettings.value(CFG_LANE_TOOL_PARAM_REG_KEY, "").toString();

					// does parameter registry key must exist?
					m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].bParamRegKeyMustExist = sSettings.value(CFG_LANE_TOOL_PARAM_MUST_EXIST, "false").toBool();

					// get parameter name
					m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].sParamName = sSettings.value(CFG_LANE_TOOL_PARAM_NAME, "").toString();

					// get parameter type
					m_SpecParams.sPlazaParams[iIdx].sParams[iIdx2].sParamType = sSettings.value(CFG_LANE_TOOL_PARAM_TYPE, "").toString();

					sSettings.endGroup();
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////

	// get lane specific parameters
	sSettings.beginGroup(CFG_LANE_TOOL_LANE_SPEC_PARAM);

	m_SpecParams.sLaneParams.iNbItems = 0;

	sChildKeys = sSettings.childKeys();
	foreach(const QString &childKey, sChildKeys)
	{
		if (m_LaneConfig.iNbItems >= MAX_LANE_ITEMS)
			break;

		m_SpecParams.sLaneParams.sPlaza[m_SpecParams.sLaneParams.iNbItems].sLabel = sSettings.value(childKey).toString();
		m_SpecParams.sLaneParams.iNbItems++;
	}

	sSettings.endGroup();

	for (iIdx = 0; iIdx < m_SpecParams.sLaneParams.iNbItems; iIdx++)
	{
		if (!m_SpecParams.sLaneParams.sPlaza[iIdx].sLabel.isEmpty())
		{
			sSettings.beginGroup(m_SpecParams.sLaneParams.sPlaza[iIdx].sLabel);

			m_SpecParams.sLaneParams.sPlaza[iIdx].iNbItems = 0;

			sChildKeys = sSettings.childKeys();
			foreach(const QString &childKey, sChildKeys)
			{
				if (m_SpecParams.sLaneParams.sPlaza[iIdx].iNbItems >= MAX_LANE_ITEMS)
					break;

				if (childKey == QString(CFG_LANE_TOOL_PLAZA_NUMBER))
				{
					m_SpecParams.sLaneParams.sPlaza[iIdx].iPlazaNum = sSettings.value(childKey).toInt();
				}
				else
				{
					m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[m_SpecParams.sLaneParams.sPlaza[iIdx].iNbItems].sLabel =
						sSettings.value(childKey).toString();

					m_SpecParams.sLaneParams.sPlaza[iIdx].iNbItems++;
				}
			}

			sSettings.endGroup();
		}
	}

	for (iIdx = 0; iIdx < m_SpecParams.sLaneParams.iNbItems; iIdx++)
	{
		if (!m_SpecParams.sLaneParams.sPlaza[iIdx].sLabel.isEmpty())
		{
			for (iIdx2 = 0; iIdx2 < m_SpecParams.sLaneParams.sPlaza[iIdx].iNbItems; iIdx2++)
			{
				if (!m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sLabel.isEmpty())
				{
					sSettings.beginGroup(m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sLabel);

					m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems = 0;

					sChildKeys = sSettings.childKeys();
					foreach(const QString &childKey, sChildKeys)
					{
						if (m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems >= MAX_NB_PARAM)
							break;
		
						if (childKey == QString(CFG_LANE_TOOL_LANE_NUMBER))
						{
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].iLaneNum = sSettings.value(childKey).toInt();
						}
						else
						{
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems].sLabel =
								sSettings.value(childKey).toString();

							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems++;
						}
					}

					sSettings.endGroup();
				}
			}
		}
	}

	for (iIdx = 0; iIdx < m_SpecParams.sLaneParams.iNbItems; iIdx++)
	{
		if (!m_SpecParams.sLaneParams.sPlaza[iIdx].sLabel.isEmpty())
		{
			for (iIdx2 = 0; iIdx2 < m_SpecParams.sLaneParams.sPlaza[iIdx].iNbItems; iIdx2++)
			{
				if (!m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sLabel.isEmpty())
				{
					for (iIdx3 = 0; iIdx3 < m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].iNbItems; iIdx3++)
					{
						if (!m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sLabel.isEmpty())
						{
							sSettings.beginGroup(m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sLabel);

							// get parameter label
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamLabel =
								sSettings.value(CFG_LANE_TOOL_PARAM_LABEL, "").toString();

							// get parameter default value
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sDefaultValue = 
								sSettings.value(CFG_LANE_TOOL_DEFAULT_VALUE, "").toString();

							// get parameter registry key
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamRegKey = 
								sSettings.value(CFG_LANE_TOOL_PARAM_REG_KEY, "").toString();

							// does parameter registry key must exist?
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].bParamRegKeyMustExist = 
								sSettings.value(CFG_LANE_TOOL_PARAM_MUST_EXIST, "false").toBool();

							// get parameter name
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamName = 
								sSettings.value(CFG_LANE_TOOL_PARAM_NAME, "").toString();

							// get parameter type
							m_SpecParams.sLaneParams.sPlaza[iIdx].sLane[iIdx2].sParams[iIdx3].sParamType = 
								sSettings.value(CFG_LANE_TOOL_PARAM_TYPE, "").toString();

							sSettings.endGroup();
						}
					}					
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////

	//get language pick settings from the same ini file 
	m_pLangPickCfg = new MLangPickConfig(getConfigFilePath());
	
	if (!m_pLangPickCfg->loadConfig())
	{
		TRACE_W(QString("MCfgLaneToolConfig()::loadConfig() => Error loading lang pick config from file: %1")
			.arg(getConfigFilePath()));
		
		QMessageBox::critical(NULL, 
			MHelpFuncs::getProcessFileName(),
			QString("MCfgLaneToolConfig()::loadConfig() Error loading lang pick config from file:%1")
				.arg(getConfigFilePath()));
		
		return false;	
	}

	///////////////////////////////////////////////////////////////////////////

	// Loading application labels and parameters
	sSettings.beginGroup(MENU_SETTINGS_INI_FILE_APP_LABELS);

	// GET APP LABELS FROM .INI FILE
    sChildKeys = sSettings.childKeys();
    foreach (const QString &childKey, sChildKeys)
        m_sAppListLabels << sSettings.value(childKey).toString();

    sSettings.endGroup();

	foreach (QString sAppLabel, m_sAppListLabels)
	{
		sSettings.beginGroup(sAppLabel);
			
		MToolEntryConfigData * pNew = new MToolEntryConfigData(sAppLabel);
				
		sChildKeys = sSettings.childKeys();
		foreach (const QString &childKey, sChildKeys)
		{
			if (MENU_SETTINGS_INI_FILE_GROUP_LABEL == childKey)		
			{
				pNew->sGroupLabel = sSettings.value(childKey).toString(); 
			}
			else if (MENU_SETTINGS_INI_FILE_APP_RUN_CMD == childKey)	
			{
				pNew->sAppRunCmd = sSettings.value(childKey).toString();
			}
			else if (MENU_SETTINGS_INI_FILE_APP_ACTIVE == childKey)	
			{
				pNew->bActive = (sSettings.value(childKey).toInt() == 1)|| 
					(QString::compare(sSettings.value(childKey).toString(),"true", Qt::CaseInsensitive)==0);
			}
			else if (MENU_SETTINGS_INI_FILE_APP_INI_FILE_PATH == childKey)	
			{
				pNew->sAppIniFilePath = sSettings.value(childKey).toString();
			}
			else if (MENU_SETTINGS_INI_CMD_TYPE == childKey)	
			{
				pNew->eCmdType = (MToolEntryConfigData::enumCmdType)sSettings.value(childKey).toInt();
			}
			else if (MENU_SETTINGS_INI_CMD_PARAMETERS == childKey)	
			{
				pNew->sCmdParameters = sSettings.value(childKey).toString();
			}
		}
			
		m_lstToolEntryCfg.append(pNew);

		sSettings.endGroup();
	}

	//----------------------------------------------------------------

	return TRUE;
}

bool  MCfgLaneToolConfig::getPlazaIndex(int iPlazaNum, int *pPlazaIdx)
{
	int		iIdx = 0;
	bool	bRet = false;

	bRet = false;
	for (iIdx = 0; iIdx < m_ManualConfig.iNbItems; iIdx++)
	{
		if (m_ManualConfig.sPlazaConfig[iIdx].iPlazaNum == iPlazaNum)
		{
			*pPlazaIdx = iIdx;
			bRet = true;
			
			break;
		}
	}
	TRACE_W(QString("MCfgLaneToolConfig::getPlazaIndex() -> Plaza number is %1, Plaza Index is %2")
		.arg(iPlazaNum)
		.arg(iIdx));
	return bRet;
}

bool  MCfgLaneToolConfig::getLaneIndex(int iPlazaIdx, int iLaneNum, int *pLaneIdx)
{
	int		iIdx = 0;
	bool	bRet = false;

	bRet = false;
	for (iIdx = 0; iIdx < m_ManualConfig.sPlazaConfig[iPlazaIdx].iNbItems; iIdx++)
	{
		if (m_ManualConfig.sPlazaConfig[iPlazaIdx].sLaneConfig[iIdx].iLaneNum == iLaneNum)
		{
			*pLaneIdx = iIdx;
			bRet = true;

			break;
		}
	}
	TRACE_W(QString("MCfgLaneToolConfig::getLaneIndex() -> PlazaIdx is %1, Lane Num is %2, Result %3")
		.arg(iPlazaIdx)
		.arg(iLaneNum)
		.arg(bRet));

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/