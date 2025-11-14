

#ifndef MINPUT_DIALOG_EX_REQ_H
#define MINPUT_DIALOG_EX_REQ_H

#include <QString>
#include "MInputExField.h"
#include "MInputExReq.h"

class QDomNode;


class MInputDialogExReq:public MInputExReq
{
public:

//-----------------------------------------------------
	enum enuDialogExType
	{
		enuSAISIE_EX_UNKNOWN = 0,
		enuSAISIE_EX_GENERIC ,
		enuSAISIE_EX_SEL_FILE,
		enuSAISIE_EX_SEL_FOLDER,
		enuSAISIE_EX_KEYBOARD_TEST,
		enuSAISIE_EX_CUSTOM_PLUGIN_DLG,
		enuSAISIE_EX_SEL_LAST_TYPE = enuSAISIE_EX_CUSTOM_PLUGIN_DLG
	};

	MInputDialogExReq();
	~MInputDialogExReq();
	MInputDialogExReq(MInputDialogExReq &obj);

	void init(QString sModuleCfgKey, QString sDialogID);
	void setRetBalId(unsigned long retBalId);
		
	QString getDialogId(){return m_sDialogId;};
	enuDialogExType getDialogType(){return m_eDialogType;};
	unsigned long getRetBalId() { return m_retBalId; };

	//returns an auto generated ID used to identify requests from one another
	// (in case of the same request we would have two different ReqIDs)
	QString getDlgReqID(){return m_sDlgReqId;}

	bool hasOKButton();
	bool hasCancelButton();

	QString getDskUITemplate() { return m_sDskUITemplate;}
	QString getWebUITemplate() { return m_sWebUITemplate;}


	int getPosX(){return m_iPosX;};
	int getPosY(){return m_iPosY;};

	bool isEmbedded(){return m_bEmbedded;};
	QString getRelativeToObjectName(){return m_sRelativeToObjectName;};

	bool operator== ( const MInputDialogExReq & right ) const;	
	
	virtual MInputDialogExReq& operator=(const MInputDialogExReq& right);

	static QString readInputDialogDefinition(QString sModuleCfgKey, QString sDialogID);	

	QList <MInputExField *> * getInputFields(){ return &m_lstInputFields;};

	QString getDskCSSFile(){return m_sDskCSSFile;};

	QString getVirKybConfig(){ return m_sVirtKeybConfigToUse;};
	int getVirtPosX(){return m_iVirtKybPosX;};
	int getVirtPosY(){return m_iVirtKybPosY;};

	bool setCustomDlg_Parameters(QString sParams, int is_file);
	QString getCustomDlg_Parameters(){ return m_sCustomDlg_Parameters; };
	
	QString getCustomDlg_PluginName(){ return m_sCustomDlg_PluginName; };
	
	int getCustomDlg_DlgId(){ return m_iCustomDlg_DlgId; };

private:
	void generateRequestID();

	bool m_bEmbedded;
	QString m_sRelativeToObjectName;

	//if not embedded to an object
	int m_iPosX;
	int m_iPosY;

	enuDialogExType	m_eDialogType;
	unsigned long m_retBalId;
	
	bool m_bHasOKButton;
	bool m_bHasCancelButton;

	
	QString m_sDialogId;
	QString m_sDskUITemplate;
	QString m_sWebUITemplate;
	//used to identify the request  
	QString m_sDlgReqId;

	//Virtual keyboard costumization
	QString m_sVirtKeybConfigToUse;
	int m_iVirtKybPosX;
	int m_iVirtKybPosY;
	//------------------------------------------------------------
	//NOTE: assure that every new member is copied in operator = 
	// to assure that the data are copied to the TCL interfaces (MInputDialogExReq& operator=)
	//------------------------------------------------------------

	//For custom dialogs (enuSAISIE_EX_CUSTOM_PLUGIN_DLG)
	QString m_sCustomDlg_PluginName;
	int m_iCustomDlg_DlgId;
	QString m_sCustomDlg_Parameters;
};



#endif
