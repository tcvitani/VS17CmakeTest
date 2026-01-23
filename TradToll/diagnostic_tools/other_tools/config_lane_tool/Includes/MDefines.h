/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
/* FILE:     MDefines.h														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CFG_LANE_TOOL_DEFINES_H
#define CFG_LANE_TOOL_DEFINES_H

/*-------------------------------- INCLUDES:  -------------------------------*/

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

#define MAX_LANE_TYPES			20
#define MAX_DEV_GROUPS			30
#define MAX_DEV_ITEMS			50
#define MAX_LANE_DIRECTIONS		5
#define MAX_PLAZA_ITEMS			100
#define MAX_LANE_ITEMS			50
#define MAX_NB_PARAM			50
#define MAX_NB_IP_ADAPTERS		25
#define MAX_NB_IP_ADDRESSES		15
#define MAX_NB_NETWORK_CONFIG	10

// This is defined like this because of translation. If translation is not used, it should be defined like usually
#define CFG_LANE_TOOL_CANNOT_LOAD_REG		"CFG_LANE_TOOL_CANNOT_LOAD_REG"			//"Cannot load registry"
#define CFG_LANE_TOOL_ABOUT_LABEL1			"CFG_LANE_TOOL_ABOUT_DIALOG_LABEL1"		//"Version: %1"
#define CFG_LANE_TOOL_LABEL_OK				"CFG_LANE_TOOL_LABEL_OK"				//"OK"
#define CFG_LANE_TOOL_LABEL_CANCEL			"CFG_LANE_TOOL_LABEL_CANCEL"			//"CANCEL"
#define CFG_LANE_TOOL_LABEL_YES				"CFG_LANE_TOOL_LABEL_YES"
#define CFG_LANE_TOOL_LABEL_NO				"CFG_LANE_TOOL_LABEL_NO"
#define CFG_LANE_TOOL_MSG_QUIT_APP			"CFG_LANE_TOOL_MSG_QUIT_APP"
#define CFG_LANE_TOOL_MSG_QUIT_APP_UNSAVED	"CFG_LANE_TOOL_MSG_QUIT_APP_UNSAVED"
#define CFG_LANE_TOOL_MSG_SAVE				"CFG_LANE_TOOL_MSG_SAVE"
#define CFG_LANE_TOOL_PARAMS				"CFG_LANE_TOOL_PARAMETER"
#define CFG_LANE_TOOL_PARAM_VALUE			"CFG_LANE_TOOL_PARAM_VALUE"
#define CFG_LANE_TOOL_NO_DEFINED_CONFIG		"CFG_LANE_TOOL_NO_DEFINED_CONFIG"
#define CFG_LANE_TOOL_NOT_ALL_PARAM_SAVED	"CFG_LANE_TOOL_NOT_ALL_PARAM_SAVED"
#define CFG_LANE_TOOL_NOT_ALL_PARAM_READ	"CFG_LANE_TOOL_NOT_ALL_PARAM_READ"
#define CFG_LANE_TOOL_TABS_WILL_BE_UPDATED	"CFG_LANE_TOOL_TABS_WILL_BE_UPDATED"
#define CFG_LANE_TOOL_PLAZA_NUM_NOT_FOUND	"CFG_LANE_TOOL_PLAZA_NUM_NOT_FOUND"
#define CFG_LANE_TOOL_LANE_NUM_NOT_FOUND	"CFG_LANE_TOOL_LANE_NUM_NOT_FOUND"
#define CFG_LANE_TOOL_MSG_FORCE_SHUTDOWN	"CFG_LANE_TOOL_MSG_FORCE_SHUTDOWN"
#define CFG_LANE_TOOL_MSG_QUESTION_SHUTDOWN	"CFG_LANE_TOOL_MSG_QUESTION_SHUTDOWN"
#define CFG_LANE_TOOL_MSG_CHANGES_SAVED		"CFG_LANE_TOOL_MSG_CHANGES_SAVED"
#define CFG_LANE_TOOL_QUESTION_COLD_START	"CFG_LANE_TOOL_QUESTION_COLD_START"

#define	CFG_LANE_TOOL_INI_TOOL_PARAMS		"CFG_LANE_TOOL_PARAMS"
#define CFG_LANE_TOOL_LANE_TYPES			"LANE_TYPES"
#define CFG_LANE_TOOL_LANE_DIRECTIONS		"LANE_DIRECTIONS"
#define CFG_LANE_TOOL_MANUAL_CONFIG			"MANUL_CONFIGURATION"
#define CFG_LANE_TOOL_PLAZA_SPEC_PARAM		"SPECIFIC_PLAZA_PARAMETERS"
#define CFG_LANE_TOOL_LANE_SPEC_PARAM		"SPECIFIC_LANE_PARAMETERS"

#define	CFG_LANE_TOOL_DO_NOT_SHOW_GENERAL	"DoNotShowGeneralParamTab"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_NETWORK	"DoNotShowNetworkParamTab"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_PLAZA_S	"DoNotShowPlazaSpecificParamTab"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_LANE_S	"DoNotShowLaneSpecificParamTab"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_DEVICES	"DoNotShowDevicesParamTab"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_CH_NUM	"DoNotShowChannelNumber"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_DUAL_LANE	"DoNotShowDualLane"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_LANE_DIR	"DoNotShowLaneDirection"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_FARE		"DoNotShowFarePoint"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_GAC	"DoNotShowBtnGeneralGetActualCfg"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_GDC	"DoNotShowBtnGeneralGetDefaultCfg"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_PSAC	"DoNotShowBtnPlazaSpecGetActualCfg"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_PSDC	"DoNotShowBtnPlazaSpecGetDefaultCfg"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_LSAC	"DoNotShowBtnLaneSpecGetActualCfg"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_LSDC	"DoNotShowBtnLaneSpecGetDefaultCfg"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_DAC	"DoNotShowBtnDevicesGetActualCfg"
#define	CFG_LANE_TOOL_DO_NOT_SHOW_BTN_DDC	"DoNotShowBtnDevicesGetDefaultCfg"

#define	CFG_LANE_TOOL_MIN_PLAZA_NUM			"MinPlazaNum"
#define	CFG_LANE_TOOL_MAX_PLAZA_NUM			"MaxPlazaNum"
#define	CFG_LANE_TOOL_MIN_LANE_NUM			"MinLaneNum"
#define	CFG_LANE_TOOL_MAX_LANE_NUM			"MaxLaneNum"
#define	CFG_LANE_TOOL_MIN_CHANNEL_NUM		"MinChannelNum"
#define	CFG_LANE_TOOL_MAX_CHANNEL_NUM		"MaxChannelNum"
#define	CFG_LANE_TOOL_SPEC_P_STRING_LEN		"SpecParamStringMaxLength"
#define	CFG_LANE_TOOL_SPEC_P_NUM_INPUT_MASK	"SpecParamNumberInputMask"
#define	CFG_LANE_TOOL_SHOW_ALL_NET_ADAPTERS	"ShowAllNetworkAdapters"

#define	CFG_LANE_TOOL_REG_APP_PARAM			"RegKeyAppParameters"
#define	CFG_LANE_TOOL_REG_DEV_PARAM			"RegKeyDevParameters"
#define	CFG_LANE_TOOL_REG_MODULE_LAN		"RegKeyModule_LAN"
#define	CFG_LANE_TOOL_REG_MODULE_TFT		"RegKeyModule_TFT"

#define	CFG_LANE_TOOL_REG_VAL_PLAZA_NAME	"ValuePlazaName"
#define	CFG_LANE_TOOL_REG_VAL_PLAZA_NUMBER	"ValuePlazaNumber"
#define	CFG_LANE_TOOL_REG_VAL_LANE_NAME		"ValueLaneName"
#define	CFG_LANE_TOOL_REG_VAL_LANE_NUMBER	"ValueLaneNumber"
#define	CFG_LANE_TOOL_REG_VAL_LANE_TYPE		"ValueLaneType"
#define	CFG_LANE_TOOL_REG_VAL_MSG_SERVER	"ValueMessageServer"
#define	CFG_LANE_TOOL_REG_VAL_FILE_SERVER	"ValueFileServer"
#define	CFG_LANE_TOOL_REG_VAL_LANE_DIRECT	"ValueLaneDirection"
#define	CFG_LANE_TOOL_REG_VAL_FARE_POINT	"ValueFarePoint"
#define	CFG_LANE_TOOL_REG_VAL_DUAL_LANE		"ValueDualLane"
#define	CFG_LANE_TOOL_REG_VAL_HOST_NAME		"ValueHostName"
#define	CFG_LANE_TOOL_REG_VAL_NV_HOST_NAME	"ValueNVHostName"
#define	CFG_LANE_TOOL_REG_EDIT_COMMAND		"RegEditCmd"
#define	CFG_LANE_TOOL_SHUTDOWN_COMMAND		"ShutDownCmd"
#define	CFG_LANE_TOOL_SET_DHCP_COMMAND		"SetDhcpCmd"
#define	CFG_LANE_TOOL_SET_IP_ADDR_COMMAND	"SetIpAddressCmd"
#define	CFG_LANE_TOOL_SET_WORK_GROUP_CMD	"UpdateWorkGroupCmd"
#define	CFG_LANE_TOOL_WIN_FIREWALL_COMMAND	"WindowsFirewallCmd"
#define	CFG_LANE_TOOL_REG_KEY_FOR_HOST_NAME	"RegKeyForHostName"
#define	CFG_LANE_TOOL_FORCE_SHUTDOWN		"ForcePcShutDown"
#define	CFG_LANE_TOOL_MSG_SHUTDOWN			"ShutDownPcMessage"
#define	CFG_LANE_TOOL_ASK_FOR_COLD_START	"AskForColdStart"

#define	CFG_LANE_TOOL_CONTEXT_FILE			"ContextFile"
#define CFG_LANE_TOOL_SCRIPT_PATH			"ScriptPath"
#define CFG_LANE_TOOL_DEFAULT_SCRIPT_FILE	"DefaultScriptFile"

#define CFG_LANE_TOOL_LANE_TYPE				"LaneType"
#define CFG_LANE_TOOL_LANE_NUMBER			"LaneNumber"
#define CFG_LANE_TOOL_LANE_NAME				"LaneName"
#define CFG_LANE_TOOL_LANE_DIRECTION		"LaneDirection"
#define CFG_LANE_TOOL_LANE_FARE_POINT		"FarePoint"
#define CFG_LANE_TOOL_LANE_DUAL_LANE		"DualLane"
#define CFG_LANE_TOOL_LANE_COMPUTER_NAME	"ComputerName"
#define CFG_LANE_TOOL_SCRIPT_FILE			"ScriptFile"
#define CFG_LANE_TOOL_LANE_WORK_GROUP		"WorkGroup"
#define CFG_LANE_TOOL_LANE_NETWORK_CONFIG	"NetworkConfig"
#define CFG_LANE_TOOL_LANE_ADAPTER_NAME		"AdapterName"
#define CFG_LANE_TOOL_LANE_DEFAULT_GATEWAY	"DefaultGateWay"
#define CFG_LANE_TOOL_LANE_AUTO_IP_ADDRESS	"ObtainIpAddressAuto"
#define CFG_LANE_TOOL_LANE_IP_ADDRESS_LABEL	"IpAdresses"
#define CFG_LANE_TOOL_LANE_IP_ADDRESS		"IpAddress"
#define CFG_LANE_TOOL_LANE_SUBNET_MASK		"SubNetMask"
#define CFG_LANE_TOOL_LANE_UPD_IP_SET_DEF	"UpdateIpSettingsByDefault"

#define CFG_LANE_TOOL_LANE_CONFIG			"LaneConfig"
#define CFG_LANE_TOOL_PLAZA_NUMBER			"PlazaNumber"
#define CFG_LANE_TOOL_PLAZA_NAME			"PlazaName"
#define CFG_LANE_TOOL_PLAZA_FILE_SEVER		"PlazaFileServer"
#define CFG_LANE_TOOL_PLAZA_MSG_SEVER		"PlazaMsgServer"
#define CFG_LANE_TOOL_FILE_DIR				"ConfigRegFileDir"
#define CFG_LANE_TOOL_INST_FILE				"RegFile_Inst"
#define CFG_LANE_TOOL_UNINST_FILE			"RegFile_Uninst"
#define CFG_LANE_TOOL_DEVICE_GROUP			"Devices_Group"
#define CFG_LANE_TOOL_SUBKEY_PATH			"SubKeyRegPath"
#define CFG_LANE_TOOL_SUBKEY_NAME			"SubKeyRegName"
#define CFG_LANE_TOOL_IS_DEFAULT			"IsDefaultChecked"
#define CFG_LANE_TOOL_DEFAULT_VALUE			"DefaultValue"
#define CFG_LANE_TOOL_PARAM_LABEL			"ParameterLabel"
#define CFG_LANE_TOOL_PARAM_REG_KEY			"ParameterRegKey"
#define CFG_LANE_TOOL_PARAM_MUST_EXIST		"ParameterRegKeyMustExist"
#define CFG_LANE_TOOL_PARAM_NAME			"ParameterName"
#define CFG_LANE_TOOL_PARAM_TYPE			"ParameterType"

#define CFG_LANE_TOOL_PARAM_NUMBER			"Number"
#define CFG_LANE_TOOL_PARAM_STRING			"String"
#define CFG_LANE_TOOL_PARAM_IP_ADDRESS		"IpAddress"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/