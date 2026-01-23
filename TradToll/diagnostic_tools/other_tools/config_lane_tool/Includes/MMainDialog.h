/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
/* FILE:     MMainDialog.h													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CFG_LANE_TOOL_MAIN_DIALOG_H
#define CFG_LANE_TOOL_MAIN_DIALOG_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QDialog>
#include "MDefines.h"
#include "MGlobalStructs.h"

extern "C"
{
	#include <noyau.h>
};

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

class QHBoxLayout;
class MStyledTitleBar;
class MStyledToolBox;
class MStyledLangPick;
class MAboutWidget;
class MToolEntryConfigData;
class QRegularExpressionValidator;
class QLineEdit;
class QTableWidget;
/*-------------------------------- DEFINES:   -------------------------------*/
namespace Ui {
	class MMainDialogB;
} // namespace Ui


/*-------------------------------- TYPEDEFS:  -------------------------------*/

class MMainDialog: public QDialog
{
    Q_OBJECT

	public:
		MMainDialog(QWidget *parent = 0);
		~MMainDialog();

	private slots:
		void onBtnTools();
		void onDialogResized();
		void onLangSelectionChanged(QString);
		void onBtnAbout();
		void onClose();
		void onBtnQuit();
		void onBtnSave();
		void onBtnGeneralGetCurrent();
		void onBtnGeneralGetDefault();
		void onBtnNetworkGetCurrent();
		void onBtnNetworkGetDefault();
		void onBtnDevGetCurrent();
		void onBtnDevGetDefault();
		void onBtnPlazaGetCurrent();
		void onBtnPlazaGetDefault();
		void onBtnLaneGetCurrent();
		void onBtnLaneGetDefault();
		void onMenuItemClicked();
		void onLaneNumChanged(int iValue);
		void onPlazaNumChanged(int iValue);
		void onComboBoxNetCard(QString sValue);
		void onCheckBoxAutoIp(bool bChecked);
		void onCheckBoxUpdateIp(bool bChecked);

	protected:
		void changeEvent(QEvent* event);
		void retranslate();

	private:
		void updateMainToolboxSizeAndPos();
		void updateAboutDlgSizeAndPos();
		void updateSettingsDlgSizeAndPos(QWidget * wToPosition);
		void createMainToolBox();
		void createLangPick();
		void createAboutDlg();

		void initConfiguration();

		void updateGeneralTab(int iPlazaIdx, int iLaneIdx);
		void updateNetworkTab(int iPlazaIdx, int iLaneIdx);
		void updateDevicesTab(int iPlazaIdx, int iLaneIdx);
		void updatePlazaSpecParamTab(int iPlazaNum);
		void updateLaneSpecParamTab(int iPlazaNum, int iLaneNum);

		void clearNetworkTab();
		void updaterNetworkTabIpAddress(bool bSetDisabled);
		void setNetworkTabIpAddress(QString sIpAddress, QString sIpMask, QString sDefaultGateway);
	
		void saveConfiguration();
		bool saveGeneralConfig();
		bool getGeneralConfig();
		bool getNetworkConfig();
		bool getPlazaAndMsgServers();
		bool getComputerNameAndWorkgroup();
		bool saveNetworkConfig();
		bool getAdapterData();
		bool saveLaneTypeConfig();
		bool getLaneTypeConfig();
		bool saveSpecParamsConfig();
		bool getSpecParamsConfig(int iPlazaNum, int iLaneNum);
		bool savePlazaSpecParamsConfig();
		bool getPlazaSpecParamsConfig(int iPlazaNum);
		bool saveLaneSpecParamsConfig();
		bool getLaneSpecParamsConfig(int iPlazaNum, int iLaneNum);
		void mergeRegFiles(QString sRegDir, QString sRegFiles);
		void executeCommand(QString sCommand);
		void ExecuteCommandRstrSettings(strucrt_lane_cfg *pFileToProcess);
		void ExecuteCmd(QString sCommand);

		QString getCurrentPlazaName(CHAR *pRegPath, bool *pStatus);
		uint getCurrentPlazaNumber(CHAR *pRegPath, bool *pStatus);
		uint getCurrentLaneNumber(CHAR *pRegPath, bool *pStatus);

		bool raiseMsgBox(QWidget *pParent,
						QString sMessage,
						QString sBtnText, 
						QString sBtnCancelText, 
						int iMsgBoxIcon);

		bool fileExists(QString sPath);

		QRegularExpressionValidator* ipValidator(QWidget *pParent);

		QLineEdit * createLineEditForSpecParam(QWidget *pParent, QString sParamType);

		bool getAllNetworkInterfaces();

		void updateAndShowIpAddressData(strucrt_network_cfg *pNetCfg);
		void updateNetCfgToApplyWithCurrentData(QString sAdapterName);
		void updateCheckBoxUpdateIp(bool bUpdateIpSettings);

		strucrt_network_cfg * getNetCfgToApplyInfo(QString sAdapterName);

		void resetNetCfgToApply();
		void updateNetCfgToApplyWithActualCfg(struct_ip_adapter_info *pAdapterInfo);
		void updateNetCfgToApplyWithDefaultCfg(int iPlazaIdx, int iLaneIdx);

		void addTableRow(QTableWidget *pTable,
						QString sParamLabel,
						QString sParamValue,
						QString sParamType);


		bool doesParamRegKeyExist(char *pRegPath);

		uint	m_uiFirstPlazaNum;
		uint	m_uiFirstLaneNum;
		uint	m_uiPlazaNumForPlazaSpecParam;
		uint	m_uiPlazaNumForLaneSpecParam;
		uint	m_uiLaneNumForLaneSpecParam;
	
		QString m_sLaneTypeForDevParam;
		
		QString	m_sLastSelectedAdapterName;

		char	m_szGeneralAppParamReg[300];
		char	m_szTftParamReg[300];
		char	m_szLanParamReg[300];

		bool	m_bReadActualCfg;
		bool	m_bMakeColdStart;
		bool	m_bConfigSaved;
		bool	m_bPlazaLaneFirstTimeChange;
		bool	m_bNetTabActualCfg;
		bool	m_bInitDone;

		ulong					ulNbAdapters;
		struct_ip_adapter_info	sAdaptersInfo[MAX_NB_IP_ADAPTERS];

		int						iNbNetCfg;
		strucrt_network_cfg		sNetCfgToApply[MAX_NB_NETWORK_CONFIG];

		Ui::MMainDialogB	*m_ui;
		MStyledTitleBar		*m_pTitle;
		QString				m_sTitle;
	
		MStyledToolBox								*m_pMainToolBox;
		QMap <QPushButton *, MToolEntryConfigData*>	mapToolConfig;

		MStyledLangPick *m_LangPick;
		MAboutWidget	*m_pAboutDlg;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/

