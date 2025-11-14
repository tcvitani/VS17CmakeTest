#ifndef MIHM_VIRTUAL_OBJECTS_H
#define MIHM_VIRTUAL_OBJECTS_H



#include <QString>
#include <QStringList>
#include <QList>
#include <QRect>
#include <QDomDocument>
#include "MPoliceValue.h"

extern "C" {
	#include "ihm.h"
}


class MIhmVirtualObject
{
	public:
		MIhmVirtualObject(){ m_bObjUpdated = false; m_bVisible = true; m_bObjInitialized = false; m_bEnabled = false;};
		//MIhmVirtualObject * newCopy();
		virtual ~MIhmVirtualObject(){};

		enum enumVirtualObjectType{
			enuIhmNoVirtualObjType = 0,  
			enuIhmContainer,
			enuIhmIco,
			enuIhmText,
			enuIhmButton,
			enuIhmList,
			enuIhmTableViewReq, //affiche - show file in table view
			enuIhmHMenuViewReq, //affiche - show menu from xml file
			enuIhmVideoViewReq, //affiche - show video from cam or scaner using CAM modules (available for desktop only)
			enuIhmRestreamVideoViewReq, //affiche - show video from restream server
			enuIhmTabView,
			enuIhmDynConfig,
			enuIhmDynStrDetection,
			enuIhmAsyncInput,		//M_IHM_ASYNC_INPUT service handling
			enuIhmInputDialogReq, //sasie visu service handling ...
			enuIhmInputDlgExReq //sasie_visu_ex service handling ...
		};

		enum enumVirtualObjectId{
			enuIhmNoVirtualObjId = 0,  //generic
			
			enuIhmVirtFirstObject,	

			//text labels
			enuIhmVirtAllObject = enuIhmVirtFirstObject,	
			enuIhmVirtPlazaName,
			enuIhmVirtPlazaName1,
			enuIhmVirtPlazaName2,
			enuIhmVirtPlazaName3,
			
			enuIhmVirtPlazaNameTitle,
			enuIhmVirtPlazaNbr,
			enuIhmVirtPlazaNbrTitle,
			enuIhmVirtLaneNbr,
			enuIhmVirtLaneNbrTitle,
			enuIhmVirtLaneName,          
			enuIhmVirtLaneName1,          
			enuIhmVirtLaneName2,          
			enuIhmVirtLaneNameTitle,	

			enuIhmVirtCollectorID,
			enuIhmVirtCollectorName,

			enuIhmVirtTxtSpare1,
			enuIhmVirtTxtSpare2,
			enuIhmVirtTxtSpare3,
			enuIhmVirtTxtSpare4,
			enuIhmVirtTxtSpare5,
			enuIhmVirtTxtSpare6,
			enuIhmVirtTxtSpare7,
			enuIhmVirtTxtSpare8,
			enuIhmVirtTxtSpare9,
			enuIhmVirtTxtSpare10,

			enuIhmVirtTxtSpare11,
			enuIhmVirtTxtSpare12,
			enuIhmVirtTxtSpare13,
			enuIhmVirtTxtSpare14,
			enuIhmVirtTxtSpare15,
			enuIhmVirtTxtSpare16,
			enuIhmVirtTxtSpare17,
			enuIhmVirtTxtSpare18,
			enuIhmVirtTxtSpare19,
			enuIhmVirtTxtSpare20,

			enuIhmVirtTxtSpare21,
			enuIhmVirtTxtSpare22,
			enuIhmVirtTxtSpare23,
			enuIhmVirtTxtSpare24,
			enuIhmVirtTxtSpare25,

			enuIhmVirtTxtSpare26,
			enuIhmVirtTxtSpare27,
			enuIhmVirtTxtSpare28,
			enuIhmVirtTxtSpare29,
			enuIhmVirtTxtSpare30,

			enuIhmVirtTxtSpare31,
			enuIhmVirtTxtSpare32,
			enuIhmVirtTxtSpare33,
			enuIhmVirtTxtSpare34,
			enuIhmVirtTxtSpare35,

			enuIhmVirtTxtSpare36,
			enuIhmVirtTxtSpare37,
			enuIhmVirtTxtSpare38,
			enuIhmVirtTxtSpare39,
			enuIhmVirtTxtSpare40,

			enuIhmVirtTxtSpare41,
			enuIhmVirtTxtSpare42,
			enuIhmVirtTxtSpare43,
			enuIhmVirtTxtSpare44,
			enuIhmVirtTxtSpare45,

			enuIhmVirtTxtSpare46,
			enuIhmVirtTxtSpare47,
			enuIhmVirtTxtSpare48,
			enuIhmVirtTxtSpare49,
			enuIhmVirtTxtSpare50,

			enuIhmVirtTxtSpare51,
			enuIhmVirtTxtSpare52,
			enuIhmVirtTxtSpare53,
			enuIhmVirtTxtSpare54,
			enuIhmVirtTxtSpare55,

			enuIhmVirtTxtSpare56,
			enuIhmVirtTxtSpare57,
			enuIhmVirtTxtSpare58,
			enuIhmVirtTxtSpare59,
			enuIhmVirtTxtSpare60,

			enuIhmVirtTransactionNbr,
			enuIhmVirtTransactionNbrTitle,
			enuIhmVirtReceiptNbr,
			enuIhmVirtReceiptNbrTitle,
			enuIhmVirtAxles,
			enuIhmVirtAxlesTitle,
			enuIhmVirtBalance,
			enuIhmVirtBalanceTitle,
			enuIhmVirtClass,
			enuIhmVirtClassTitle,  
			enuIhmVirtAddRevenue,
			enuIhmVirtAddRevenueTitle,

			enuIhmVirtNbCar,
			enuIhmVirtEntry,
			enuIhmVirtEntryTitle,
			enuIhmVirtCurrency,
			enuIhmVirtCurrencyTitle,

			enuIhmVirtFare,
			enuIhmVirtFareTitle,
			enuIhmVirtSale,
			enuIhmVirtSaleTitle,
			enuIhmVirtSaleDue,
			enuIhmVirtSaleDueTitle,
			enuIhmVirtSaleDuePaid,
			enuIhmVirtTransactionType,
			enuIhmVirtTransactionTypeTitle,
			enuIhmVirtFareType,
			enuIhmVirtFareTypeTitle,

			enuIhmVirtTxtModeTitle,
			enuIhmVirtTxtMode,
			enuIhmVirtTxtCollectorTitle,
			enuIhmVirtTxtInCtrlUserTitle,
			enuIhmVirtTxtInCtrlUser,		
			enuIhmVirtTxtInCtrlUser1,		
			enuIhmVirtTxtTrsContainerTitle,	
			enuIhmVirtTxtAlarmsTitle,
			enuIhmVirtTxtTrsHistoryTitle,
			enuIhmVirtTxtAlertesTitle,	
			enuIhmVirtTxtCamNumPic,	
			enuIhmVirtTxtScanNumPic,	
			enuIhmVirtTxtNbCarSAS,

			//instructions
			enuIhmVirtErrPayment,
			enuIhmVirtPayment,
			enuIhmVirtRemark,
			enuIhmVirtInstruction,   
			enuIhmVirtInstruction2,
			enuIhmVirtCurrentDate,

			// Icons
			enuIhmVirtIcoLaneStatus,
			enuIhmVirtIcoLaneMode,
			enuIhmVirtIcoCollector,       
			enuIhmVirtIcoEntryGate,			
			enuIhmVirtIcoExitGate,		
			enuIhmVirtIcoCar,
			enuIhmVirtIcoTrafficLight,
			enuIhmVirtIcoViolation,       
			enuIhmVirtIcoEntryLoop,		
			enuIhmVirtIcoExitLoop,
			enuIhmVirtIcoBeacon,
			enuIhmVirtIcoEntOpticalBarrier,
			
			enuIhmVirtIcoSpare1,
			enuIhmVirtIcoSpare2,
			enuIhmVirtIcoSpare3,	
			enuIhmVirtIcoSpare4,	
			enuIhmVirtIcoSpare5,	
			enuIhmVirtIcoSpare6,
			enuIhmVirtIcoSpare7,
			enuIhmVirtIcoSpare8,
			enuIhmVirtIcoSpare9,
			enuIhmVirtIcoSpare10,
			enuIhmVirtIcoSpare11,
			enuIhmVirtIcoSpare12,
			enuIhmVirtIcoSpare13,
			enuIhmVirtIcoSpare14,
			enuIhmVirtIcoSpare15,
			enuIhmVirtIcoSpare16,
			enuIhmVirtIcoSpare17,
			enuIhmVirtIcoSpare18,
			enuIhmVirtIcoSpare19,
			enuIhmVirtIcoSpare20,
			enuIhmVirtIcoSpare21,
			enuIhmVirtIcoSpare22,
			enuIhmVirtIcoSpare23,
			enuIhmVirtIcoSpare24,
			enuIhmVirtIcoSpare25,
			enuIhmVirtIcoSpare26,
			enuIhmVirtIcoSpare27,
			enuIhmVirtIcoSpare28,
			enuIhmVirtIcoSpare29,
			enuIhmVirtIcoSpare30,
			enuIhmVirtIcoSpare31,
			enuIhmVirtIcoSpare32,
			enuIhmVirtIcoSpare33,
			enuIhmVirtIcoSpare34,
			enuIhmVirtIcoSpare35,
			enuIhmVirtIcoSpare36,
			enuIhmVirtIcoSpare37,
			enuIhmVirtIcoSpare38,
			enuIhmVirtIcoSpare39,
			enuIhmVirtIcoSpare40,
			enuIhmVirtIcoSpare41,
			enuIhmVirtIcoSpare42,
			enuIhmVirtIcoSpare43,
			enuIhmVirtIcoSpare44,
			enuIhmVirtIcoSpare45,
			enuIhmVirtIcoSpare46,
			enuIhmVirtIcoSpare47,
			enuIhmVirtIcoSpare48,
			enuIhmVirtIcoSpare49,
			enuIhmVirtIcoSpare50,
			enuIhmVirtIcoCamera1,
			enuIhmVirtIcoCamera2,

			enuIhmVirtIcoBeacon2,
			enuIhmVirtIcoSignalLight,
			enuIhmVirtIcoExtOpticalBarrier,
			
			//Buttons
			enuIhmVirtBtnClass,
			enuIhmVirtBtnEntryPoint,
			enuIhmVirtBtnEntryPointNbr,
			enuIhmVirtBtnMode,
			enuIhmVirtBtnArrowOn,
			enuIhmVirtBtnArrowOff,
			enuIhmVirtBtnCrossOn,
			enuIhmVirtBtnCrossOff,
			enuIhmVirtBtnDsrcOn,
			enuIhmVirtBtnDsrcOff,
			enuIhmVirtBtnMagneticOn,
			enuIhmVirtBtnMagneticOff,
			enuIhmVirtBtnCollectorOn,
			enuIhmVirtBtnCollectorOff,
			enuIhmVirtBtnCamera,
			enuIhmVirtBtnScanner,
			enuIhmVirtBtnComment,
			enuIhmVirtBtnGabarite,
			enuIhmVirtBtnEntryGate,
			enuIhmVirtBtnExitGate,
			enuIhmVirtBtnCar3,
			enuIhmVirtBtnCar2,
			enuIhmVirtBtnCar,
			enuIhmVirtBtnEntryLoop,
			enuIhmVirtBtnExitLoop,
			enuIhmVirtBtnTblBeacon,

			enuIhmVirtBtnCamGrab,
			enuIhmVirtBtnCamHide,
			enuIhmVirtBtnScanGrab,
			enuIhmVirtBtnScanHide,
			enuIhmVirtBtnScanSend,
			enuIhmVirtBtnScanSave,
			enuIhmVirtBtnCommHide,
			enuIhmVirtBtnExternalDisplay,
			enuIhmVirtBtnExternalDisplay2,
			enuIhmVirtBtnTakeCtrl,
			enuIhmVirtBtnReturnCtrl,

			enuIhmVirtButton1,
			enuIhmVirtButton2,
			enuIhmVirtButton3,
			enuIhmVirtButton4,
			enuIhmVirtButton5,
			enuIhmVirtBtnSpeedLimitOn,
			enuIhmVirtBtnSpeedLimitOff,
			enuIhmVirtBtnWarningOn,
			enuIhmVirtBtnWarningOff,
			enuIhmVirtBtnDsrcTSAOn,
			enuIhmVirtBtnDsrcTSAOff,
			enuIhmVirtBtnSpare1,
			enuIhmVirtBtnSpare2,
			enuIhmVirtBtnSpare3,
			enuIhmVirtBtnSpare4,
			enuIhmVirtBtnSpare5,
			enuIhmVirtBtnSpare6,
			enuIhmVirtBtnSpare7,
			enuIhmVirtBtnSpare8,
			enuIhmVirtBtnSpare9,
			enuIhmVirtBtnSpare10,
			enuIhmVirtBtnSpare11,
			enuIhmVirtBtnSpare12,
			enuIhmVirtBtnSpare13,
			enuIhmVirtBtnSpare14,
			enuIhmVirtBtnSpare15,
			enuIhmVirtBtnSpare16,
			enuIhmVirtBtnSpare17,
			enuIhmVirtBtnSpare18,
			enuIhmVirtBtnSpare19,
			enuIhmVirtBtnSpare20,
			enuIhmVirtBtnSpare21,
			enuIhmVirtBtnSpare22,
			enuIhmVirtBtnSpare23,
			enuIhmVirtBtnSpare24,
			enuIhmVirtBtnSpare25,
			enuIhmVirtBtnSpare26,
			enuIhmVirtBtnSpare27,
			enuIhmVirtBtnSpare28,
			enuIhmVirtBtnSpare29,
			enuIhmVirtBtnSpare30,

			//lists
			enuIhmVirtLstAlarms,
			enuIhmVirtLstWarnings,

			// Config - enuIhmDynConfig
			enuIhmVirtConfLaneType,
			enuIhmVirtConfProjectName,
			enuIhmVirtConfProjectVersion,
			enuIhmVirtConfLanguage,
			enuIhmVirtConfMainDlgHidden,
			enuIhmVirtConfShiftType,
			
			//	StringDetection - enuIhmDynStringDetection
			enuIhmVirtStrDetectConfigID,
			
			// Async inputs - enuIhmAsyncInput
			enuIhmVirtAsyncInputsID,

			//Others:

			enuIhmVirtVideo, //visible video when showing video
			
			//Visible image when showing image
			//it is the same virtual object type like other icons
			// but also includes its own container
			enuIhmVirtImage,  
			enuIhmVirtImage2,
			
			enuIhmVirtInputDialogID,
			enuIhmVirtInputDlgExID,
			
			//to handle sowing file in table view 
			enuIhmVirtTableView1,
			enuIhmVirtTableView2,
			enuIhmVirtTableView3,

			//to handle showing menus on the main form
			enuIhmVirtHMenuView1,
			enuIhmVirtHMenuView2,
			
			//to handle showing video vindows on the main form
			enuIhmVirtRestreamVideoView1,
			enuIhmVirtRestreamVideoView2,

			enuIhmVirtCntHeaderGroup,
			enuIhmVirtCntInstructionGroup,
			enuIhmVirtCntModeGroup,
			enuIhmVirtCntPaymentGroup,
			enuIhmVirtCntTrsGroup,

			enuIhmVirtCntComments,
			enuIhmVirtCntCamera,
			enuIhmVirtCntScanner,

			enuIhmVirtCntSpare1,
			enuIhmVirtCntSpare2,
			enuIhmVirtCntSpare3,
			enuIhmVirtCntSpare4,
			enuIhmVirtCntSpare5,
			enuIhmVirtCntSpare6,
			enuIhmVirtCntSpare7,
			enuIhmVirtCntSpare8,
			enuIhmVirtCntSpare9,
			enuIhmVirtCntSpare10,
			enuIhmVirtCntSpare11,
			enuIhmVirtCntSpare12,
			enuIhmVirtCntSpare13,
			enuIhmVirtCntSpare14,
			enuIhmVirtCntSpare15,
			enuIhmVirtCntSpare16,
			enuIhmVirtCntSpare17,
			enuIhmVirtCntSpare18,
			enuIhmVirtCntSpare19,
			enuIhmVirtCntSpare20,
			enuIhmVirtCntMiniWeb,
			enuIhmVirtCntAlarmsGroup,

			enuIhmVirtTabControl1,
			enuIhmVirtTabControl2,
			enuIhmVirtTabControl3,
			enuIhmVirtTabControl4,

			enuIhmVirtLastObject
		};



		virtual enumVirtualObjectId  getId() const {return m_eID;};
		virtual enumVirtualObjectType getType() const = 0;

		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& obj);

		virtual void clearUpdated();
		virtual void setUpdated();
		virtual bool isUpdated() const;

		//returns false by default for all objects which does not use the Enabled/Disabled functionality
		//it is overloaded for objects that use service M_IHM_AFFICHAGE_STATUS
		virtual bool setEnabled(bool bEnabled){ Q_UNUSED(bEnabled) return false; };
		bool isEnabled(){return m_bEnabled;}

		void setInitialized();
		bool isInitialized(){return m_bObjInitialized;};


		static MIhmVirtualObject * findVirtualObjectByID(QList <MIhmVirtualObject*> *plst,   enumVirtualObjectId eID);
		static bool removeVirtualObjectByID(QList <MIhmVirtualObject*> *plst,   MIhmVirtualObject::enumVirtualObjectId eID);

		static enum_ihm_objets mapVirtObjIdToIhmObjId(enumVirtualObjectId eVirtId);
		static enumVirtualObjectId mapIhmObjIdToVirtObjId(enum_ihm_objets eIhmOBJId);

		bool setVisible(bool b);
		bool isVisible() const {return m_bVisible;};

		static QString getNameForId(enumVirtualObjectId eID);
protected:
		//If object received correct "Affichage" request the bVisible becomes true
		//The object will be visible at the resulting dialog 
		//if the configuration of the corresponding MIhmVisibleObject 
		// have the value m_bConfigVisible set to true
		// The value m_bConfigVisible is set in template xml for the form or in the registry key 
		// defining additional properties for the visible object
		bool m_bVisible; 
		
		bool m_bObjUpdated;
		bool m_bObjInitialized; //to know which one was never used

		enumVirtualObjectId		m_eID;
		

		bool m_bEnabled;
};


class MIhmVirtDynConfig:public MIhmVirtualObject
{

	public:
		MIhmVirtDynConfig(MIhmVirtDynConfig &obj);
		MIhmVirtDynConfig(enumVirtualObjectId eID);
		virtual ~MIhmVirtDynConfig(){};

		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);

		enumVirtualObjectType getType() const {return enuIhmDynConfig;};
		
		bool setDynConfigSetting(QString sDynConfigSetting);

		QString m_sDynConfigSetting;
};


class MIhmVirtContainer:public MIhmVirtualObject
{
	public:
		MIhmVirtContainer(MIhmVirtContainer &obj);
		MIhmVirtContainer(enumVirtualObjectId eID);
		virtual ~MIhmVirtContainer(){};

		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);

		enumVirtualObjectType getType() const{return enuIhmContainer;};

		bool setToolTip(QString sToolTip);
		QString getToolTip() {return m_sToolTip;};

		//to be updated when the POLICE service updates font or color
		MPoliceValue m_oPoliceValue;

		QString m_sToolTip;
};

class MIhmVirtIco:public MIhmVirtContainer
{
	public:
		MIhmVirtIco(MIhmVirtIco &obj);
		MIhmVirtIco(enumVirtualObjectId eID);
		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
		virtual ~MIhmVirtIco(){};
		
		enumVirtualObjectType getType() const{return enuIhmIco;};
		bool setImageID(QString sImage);	
		virtual bool setEnabled(bool b);

		QString m_sImageID; //used to get the file name from the configuration
};


class MIhmVirtText:public MIhmVirtContainer
{
	public:
		MIhmVirtText(MIhmVirtText &obj);
		MIhmVirtText(enumVirtualObjectId eID);
		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
		virtual ~MIhmVirtText(){};
	
		enumVirtualObjectType getType() const{return enuIhmText;};
		bool setTextID(QString sText);	
		virtual bool setEnabled(bool b);


		//used to determine the translation text from the language configuration 
		QString m_sTextID; 

};

class MIhmVirtButton:public MIhmVirtContainer
{
	public:
		MIhmVirtButton(MIhmVirtButton &obj);
		MIhmVirtButton(enumVirtualObjectId eID);
		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
		virtual ~MIhmVirtButton(){};
		

		enumVirtualObjectType getType() const{return enuIhmButton;};
		bool setTextID(QString sText);	
		bool setImageID(QString sImage);	
		virtual bool setEnabled(bool b);

		QString m_sImageID; 
		QString m_sTextID;
};




class MIhmVirtList:public MIhmVirtContainer
{
	public:
		MIhmVirtList(MIhmVirtList &obj);
		MIhmVirtList(enumVirtualObjectId eID);
		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
		virtual ~MIhmVirtList(){};

		enumVirtualObjectType getType() const{return enuIhmList;};

		//list of items ids
		QStringList m_slistIDs; 

};


class MStrDetectionPair;

class MIhmVirtStrDetectConfig:public MIhmVirtualObject
{
	public:
		MIhmVirtStrDetectConfig(MIhmVirtStrDetectConfig &obj);
		MIhmVirtStrDetectConfig();
 		virtual ~MIhmVirtStrDetectConfig();

		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);

		enumVirtualObjectType getType() const{return enuIhmDynStrDetection;};
		
		bool updateDetectionPair(MStrDetectionPair* pNewPair);
		void removeDetectionPair(MStrDetectionPair* pNewPair);

	
	QList <MStrDetectionPair*> m_lstActivePairs;
};



class MInputDialogReq;

//For SAISIE_VISU service request handling ...
//It buffers the  SAISIE_VISU requests 
//When a request is processed it is removed from the list 
class MIhmVirtInputDialogReq:public MIhmVirtualObject
{
	public:
		MIhmVirtInputDialogReq(MIhmVirtInputDialogReq &obj);
		MIhmVirtInputDialogReq();
 		virtual ~MIhmVirtInputDialogReq();

		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);

		enumVirtualObjectType getType() const{return enuIhmInputDialogReq;};
		
		void addReq(MInputDialogReq* pReq);
		void removeReq(MInputDialogReq* pReq);
		bool updateReqDescription(MInputDialogReq* pReq, QString sCurrInputResult);
		void clearAllReq();
		bool getCurrentReq(MInputDialogReq* pReq);
		bool isCancel() const;
		void setCancel();
		bool isEmpty();
		void confirmCanceled();

	private:
		bool m_bIsCancel;
		QList <MInputDialogReq*> m_lstSasieVisuRequests;
};


class MInputDialogExReq;
//For SAISIE_VISU_EX service request handling ...
//It buffers the  SAISIE_VISU_EX requests 
//When a request is processed it is removed from the list 
class MIhmVirtInputDlgExReq:public MIhmVirtualObject
{
	public:
		MIhmVirtInputDlgExReq(MIhmVirtInputDlgExReq &obj);
		MIhmVirtInputDlgExReq();
 		virtual ~MIhmVirtInputDlgExReq();

		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);

		enumVirtualObjectType getType() const{return enuIhmInputDlgExReq;};
		
		void addReq(MInputDialogExReq* pReq);
		bool removeReq(MInputDialogExReq* pReq);
		MInputDialogExReq* takeReq(QString sDlgId); //Do not forget to delete the request
		MInputDialogExReq* takeFirstReq();
		MInputDialogExReq* getFirstReq();

		bool updateReqDescription(MInputDialogExReq* pReq, QString sInputDescUpdate);
		bool getCurrentExReq(MInputDialogExReq* pReq);
		bool isCancel() const;
		void setCancel();
		bool isEmpty();
		void confirmCanceled();
		
	private:
		void clearAllReq();
		bool m_bIsCancel;
		QList <MInputDialogExReq*> m_lstSasieVisuRequests;
};






//-----------------------------------------
//MIhmVirtTableViewReq
//-----------------------------------------
class MRowsCommand;
class MTableRow;

class MIhmVirtTableViewReq:public MIhmVirtualObject
{
	public:
		MIhmVirtTableViewReq(MIhmVirtTableViewReq &obj);
		MIhmVirtTableViewReq(enumVirtualObjectId eID);
 		virtual ~MIhmVirtTableViewReq();
		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
		

		enumVirtualObjectType getType() const{return enuIhmTableViewReq;};
	
		void appendNewCommand(QString sData, enum_aff_table_command eCmd, int iMaxRows);

		quint64 generateTimeStamp();
		void clearCommandList();
		quint64 getLastUpdate()
			{return m_uiLastUpdateTimestamp;};

		QList <MRowsCommand *> * getRowCommandList()
			{
				return &m_lstRowsCommands;
			}

private:
	bool readRowsFromBuffer(QString sXmlData, QList <MTableRow*> *plstNewRows);
	bool OpenXMLBuffer(QString sXmlData);
	bool readParsedRows(QList <MTableRow*> *plstNewRows);

	QList <MRowsCommand *> m_lstRowsCommands;
	quint64 m_uiLastUpdateTimestamp;
	QDomDocument m_xmlData;

	int m_iTimeStampCounter;

};


//enuIhmMenuViewReq
class MIhmVirtHMenuViewReq:public MIhmVirtualObject
{
public:
	MIhmVirtHMenuViewReq(MIhmVirtHMenuViewReq &obj);
	MIhmVirtHMenuViewReq(enumVirtualObjectId eID);
	virtual ~MIhmVirtHMenuViewReq();
	virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
	
	
	enumVirtualObjectType getType() const{return enuIhmHMenuViewReq;};
	bool setMenuContent(QString sMenuContent);
	bool setUseExternalFile(bool b);
	bool setDoNotReloadMenu(bool b);

	QString m_sMenuContent; //used to load the XML representing menu 
	bool m_bUseExternalFile; // use file (from the application output folder)
	bool m_bDoNotReloadMenu; //do not reload menu (for the optimization)
};

//enuIhmVideoViewReq
class MIhmVirtVideoViewReq:public MIhmVirtualObject
{
public:
	MIhmVirtVideoViewReq(MIhmVirtVideoViewReq &obj);
	MIhmVirtVideoViewReq(enumVirtualObjectId eID);
	virtual ~MIhmVirtVideoViewReq();
	virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
	
	enumVirtualObjectType getType() const{return enuIhmVideoViewReq;};

	int m_nSource;
	int m_nZoom;

	//used for RVideo objects to init client visible objects (mjpeg control)
	bool m_bVideoOn;
	QString m_sStreamSourceUrlLocal;
	QString m_sStreamSourceUrlRemote;
	QString m_sBaseUrl;	
};




//enuIhmTabView
class MIhmVirtTabView:public MIhmVirtContainer
{
	public:
		MIhmVirtTabView(MIhmVirtTabView &obj);
		MIhmVirtTabView(enumVirtualObjectId eID);
		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);
		virtual ~MIhmVirtTabView(){};

		enumVirtualObjectType getType() const{return enuIhmTabView;};
		virtual bool setEnabled(bool b);

		//list of items ids
		QStringList m_slstTabText; 
		int m_iCurrentIndex;
};




//Virtual object used for asynchronous input
//It is set with SRV_TYP_SET message of the M_IHM_ASYNC_INPUT service
//On every change (by IHM user), the data are emit with SRV_TYP_NOUVEL_ETAT message 
//to the user "main" application and updated to this object
class MInputAsyncExReq;

class MIhmVirtAsyncInput:public MIhmVirtualObject
{

	public:
		MIhmVirtAsyncInput(MIhmVirtAsyncInput &obj);
		MIhmVirtAsyncInput();
		virtual ~MIhmVirtAsyncInput();

		virtual MIhmVirtualObject& operator=(const MIhmVirtualObject& right);

		enumVirtualObjectType getType() const {return enuIhmAsyncInput;};
		
		bool initialize(QString sLaneType);
		void update(QString sDescription, int is_file);
	    
		void updateInputValues(QString sEncStrValues);

		QString getEncStrValues();

	MInputAsyncExReq *m_pAsyncReq;

};

#endif


