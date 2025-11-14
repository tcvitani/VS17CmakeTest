

#ifndef MINPUT_DIALOG_REQ_H
#define MINPUT_DIALOG_REQ_H

#include <QString>

#define SAISIE_CAT_FIELD_NUM "0"
#define SAISIE_CAT_ALARM_NUM "1"
#define SAISIE_CAT_CHOICE_NUM "2"
#define SAISIE_CAT_BIGCHOICE_NUM "3"
#define SAISIE_CAT_FIELD_TXT "FIELD"
#define SAISIE_CAT_ALARM_TXT "ALARM"
#define SAISIE_CAT_CHOICE_TXT "CHOICE"
#define SAISIE_CAT_BIGCHOICE_TXT "CHOICE"

#define SAISIE_MAX_CHAMPS 50



class MInputDialogReq
{
public:
	// ' Caractere qui, s'il est en debut d'une valeur texte
	// ' indique que le texte qui suit est un code de libelle
	// ' qu'il doit etre decode
	static const char cSAISIE_DECODE_TEXTE;

	// 	Caractere utilise pour demander le RAZ d'une zone de saisie
	// 	lors de la tabulation du premier caratere
	static const char cSAISIE_RAZ_ZONE_SAISIE;

	//Separateur principal dans les chaines descriptives
	static const char cSAISIE_SEPARATEUR_CHAMP;

	//Separateur secondaire dans les chaines descriptives
	static const char cSAISIE_SEPARATEUR_PARAM;

	//Separateur tertiare dans les listes d'items
	static const char cSAISIE_SEPARATEUR_ITEM;

	static const char cSAISIE_VEROU; //= "!"


//-----------------------------------------------------
//  public Constants
//-----------------------------------------------------
	static const char cFORMAT_CHAR_NUMBER;
	static const char cFORMAT_CHAR_STRING;
	static const char cFORMAT_CHAR_MASK;
	static const char cFORMAT_CHAR_RIGHTMASK;
	static const char cFORMAT_CHAR_PASSWORD;
	static const char cFORMAT_CHAR_FUNCTION;
//-----------------------------------------------------

	

	enum enuDialogInputType
	{
		enuIHMSAISIE_VISUSEUL = 0, 
		enuIHMSAISIE_SAISIE = 1, 
		enuIHMSAISIE_MARQUEUR_TOUS = 2,
		enuIHMSAISIE_MARQUEUR_ACTIF = 3
	};

	enum enuDialogType
	{
		enuSAISIE_CAT_FIELD = 0,
		enuSAISIE_CAT_ALARM = 1,
		enuSAISIE_CAT_CHOICE = 2,
		enuSAISIE_CAT_BIGCHOICE =3
	};

	MInputDialogReq():m_bValid(false),
					m_bDefinitionOK(false), 
					m_bEmbeded(false),
					m_lNbInputFields(0){};
	~MInputDialogReq(){};
	MInputDialogReq(MInputDialogReq &obj);
	MInputDialogReq &operator=( const MInputDialogReq & right );	

	unsigned long getDialogInputType(){ return (unsigned long)m_eInputType;};
	QString getDialogInputTypeStr();
	enuDialogType getDialogType(){return m_eDialogType;};
	unsigned long getRetBalId() { return m_retBalId; };
	unsigned char getButtons() { return m_ucButtons; };
	
	//returns an auto generated ID used to identify requests from one another
	// (in case of the same request we would have two different ReqIDs)
	QString getDlgReqID(){return m_sDlgReqId;}

	int getNumInputFileds(){return m_lNbInputFields;};

	bool hasOKButton();
	bool hasCancelButton();

	QString getDescription() {return m_sDescription;};
	QString getDefinition() {return m_sDefinition;};
	QString getFieldDescription(int iIndex);
	QString getFieldDefinition(int iIndex);
	
	QString getDialogId(){return m_sDialogId;};
	QString getDialogTitle();
	QString getColorAttributes(){return m_sColorAttributes;};
	Qt::Alignment getAlignment(){return m_flagsAligment;}; 
	QString getUITemplate() { return m_sUITemplate;}

	bool isDefinitionOK(){return m_bDefinitionOK;};
	bool isValid(){return m_bValid;};	
	void setDialogInputType(unsigned long dwInputType);
	void setButtons(unsigned char buttons);
	void setDescription(QString sDescription);
	bool setDefinition(QString &sDefinition);
	void setRetBalId(unsigned long retBalId);
	
	bool isEmbeded(){return m_bEmbeded;};
	QString getRelativeToObjectName(){return m_sRelativeToObjectName;};

	bool operator== ( const MInputDialogReq & right ) const;	


	bool verifyResponseFormat(QString sDlgResult, QString &sNewDescription);

	static QString readInputDialogDefinition(QString sModuleCfgKey, QString sDialogID);	
private:
	bool parseDescription();
	void generateRequestID();
	bool verifyField(QString sFormat, QString sResult);
	QString replaceDescResult(QString sOldDesc, QString sFieldData);
	
	bool m_bEmbeded;
	QString m_sRelativeToObjectName;
	Qt::Alignment m_flagsAligment;


	QString m_sColorAttributes;

	enuDialogInputType m_eInputType;
	enuDialogType	m_eDialogType;
	unsigned char  m_ucButtons;
	unsigned long m_retBalId;
	
	QString m_sDescription;
	QString m_sDefinition;
	QString m_sDialogId;
	QString m_sDialogTitle;
	QString m_sUITemplate;
	int m_lNbInputFields;

	bool m_bValid;
	bool m_bDefinitionOK;
	
	//used to identify the request  
	QString m_sDlgReqId;
};



#endif
