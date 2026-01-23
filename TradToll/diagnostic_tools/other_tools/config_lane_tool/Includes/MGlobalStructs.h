/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
/* FILE:     MGlobalStructs.h												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CFG_LANE_TOOL_GLOB_STRUCTS_H
#define CFG_LANE_TOOL_GLOB_STRUCTS_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QByteArray>
#include <QDateTime>

#include <windows.h>
#include <iphlpapi.h>

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef struct
{
	UCHAR	IpAddress[MAX_PATH];
	UCHAR	IpMask[MAX_PATH];
}
struct_ip_info;

typedef struct
{
	char			AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
	char			Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
	UINT			AddressLength;
	BYTE			Address[MAX_ADAPTER_ADDRESS_LENGTH];
	DWORD			Index;
	UINT			Type;
	UINT			DhcpEnabled;
	UCHAR			Gateway[MAX_PATH];
	UCHAR			DhcpServer[MAX_PATH];
	char			FriendlyName[MAX_PATH];

	bool			bUpdateIpSettings;

	DWORD			dwNbIpAddresses;
	struct_ip_info	sIpInfo[MAX_NB_IP_ADDRESSES];
}
struct_ip_adapter_info;

typedef struct
{
	QString	sLabel;
	QString	sDefaultValue;
	QString	sParamLabel;
	QString	sParamRegKey;
	QString	sParamName;
	QString	sParamType;
	bool	bParamRegKeyMustExist;
}
strucrt_spec_param;

typedef struct
{
	QString				sLabel;
	int					iLaneNum;

	int					iNbItems;
	strucrt_spec_param	sParams[MAX_NB_PARAM];
}
strucrt_lane_params;

typedef struct
{
	QString				sLabel;
	int					iPlazaNum;

	int					iNbItems;
	strucrt_lane_params	sLane[MAX_LANE_ITEMS];
}
strucrt_plaza_lane_params;

typedef struct
{
	int							iNbItems;
	strucrt_plaza_lane_params	sPlaza[MAX_PLAZA_ITEMS];
}
strucrt_spec_lane_params;

typedef struct
{
	QString				sLabel;
	int					iPlazaNum;

	int					iNbItems;
	strucrt_spec_param	sParams[MAX_NB_PARAM];
}
strucrt_spec_plaza_params;

typedef struct
{
	int							iNbPlazaItems;
	strucrt_spec_plaza_params	sPlazaParams[MAX_PLAZA_ITEMS];

	strucrt_spec_lane_params	sLaneParams;
}
strucrt_spec_params;

typedef struct
{
	QString				sLabel;

	QString				sAdapterName;
	QString				sIpAddress;
	QString				sSubnetMask;
	QString				sDefaultGateWay;

	bool				bAutoIpAddress;
	bool				bUpdateIpSettings;
}
strucrt_network_cfg;

typedef struct
{
	QString	sLaneLabel;

	int					iLaneNum;
	QString				sLaneName;
	QString				sLaneDirection;
	QString				sLaneType;
	QString				sFarePoint;
	QString				sComputerName;
	QString				sScriptCommand;
	QString				sWorkGroup;
	QString				sLabelNetworkCfg;

	bool				bDualLane;
	
	int					iNbNetCfg;
	strucrt_network_cfg	sNetworkConfig[MAX_NB_NETWORK_CONFIG];
}
strucrt_lane_cfg;

typedef struct
{
	QString				sPlazaCfgLabel;
	QString				sLaneCfgLabel;

	int					iPlazaNum;
	QString				sPlazaName;
	QString				sPlazaFileServer;
	QString				sPlazaMsgServer;

	int					iNbItems;
	strucrt_lane_cfg	sLaneConfig[MAX_LANE_ITEMS];
}
strucrt_plaza_cfg;

typedef struct
{
	int					iNbItems;
	strucrt_plaza_cfg	sPlazaConfig[MAX_PLAZA_ITEMS];
}
strucrt_manual_cfg;

typedef struct
{
	int		iNbItems;
	QString sLaneDirections[MAX_LANE_DIRECTIONS];
}
strucrt_lane_directions;

typedef struct
{
	QString sLabel;
	QString sSubKeyPath;
	QString sSubKeyName;
	QString sInstFile;
	QString sUninstFile;

	bool bIsDefaultChecked;
}
strucrt_device_item;

typedef struct
{
	QString				sLabel;

	int					iNbItems;
	strucrt_device_item	sDeviceItem[MAX_DEV_ITEMS];
}
strucrt_device_group, *PTR_DEV_GROUP;

typedef struct
{
	QString					sLabel;
	QString					sLaneType;
	QString					sFileDir;
	QString					sInstFile;
	QString					sUninstFile;
	QString					sDevGroupLabel;

	int						iNbItems;
	strucrt_device_group	sDeviceGroup[MAX_DEV_GROUPS];
}
strucrt_lane_type_item;

typedef struct
{
	int						iNbItems;
	strucrt_lane_type_item	sLaneTypeItem[MAX_LANE_TYPES];
}
strucrt_lane_type;

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#include <undef.h>
#endif

/*-------------------------------- END OF FILE ------------------------------*/