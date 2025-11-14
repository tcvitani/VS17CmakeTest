#ifndef MIHM_LANGUAGES_H
#define MIHM_LANGUAGES_H


#include<QList>
#include<QString>


//-------------------------------------
//non dynamic text labels
//-------------------------------------
#define IHM_LABEL_BTN_OK_TEXT 		"BTN_OK_TEXT"
#define IHM_LABEL_BTN_CANCEL_TEXT 	"BTN_CANCEL_TEXT"

#define IHM_LABEL_BTN_LOGIN_OK_TEXT 		"BTN_LOGIN_OK_TEXT"
#define IHM_LABEL_BTN_LOGIN_CANCEL_TEXT 	"BTN_LOGIN_CANCEL_TEXT"
#define IHM_LABEL_BTN_CLIENT_CLOSE_TEXT 	"BTN_CLIENT_CLOSE_TEXT"




//TAKE OVER LABELS AND TRANSLATIONS ....
#define IHM_LABEL_BTN_TAKE_OVER_ACCEPT 	"BTN_TAKE_OVER_ACCEPT"
#define IHM_LABEL_BTN_TAKE_OVER_REJECT 	"BTN_TAKE_OVER_REJECT"
#define IHM_LABEL_TITLE_LOGIN_DIALOG 	"LABEL_TITLE_LOGIN_DIALOG"
#define IHM_LABEL_LOGIN_DIALOG_MESSAGE 	"LABEL_LOGIN_DIALOG_MESSAGE"


#define IHM_LABEL_ERR_MSG_TOO_MANY_USERS_CONNECTED 		"LABEL_ERR_MSG_TOO_MANY_USERS_CONNECTED"
#define IHM_LABEL_TAKE_CTRL_REJECTED_BY_USER_MESSAGE 	"LABEL_TAKE_CTRL_REJECTED_BY_USER_MESSAGE"
#define IHM_LABEL_LOGIN_DIALOG_MESSAGE_LDAP_OFFLINE 	"LABEL_LOGIN_DIALOG_MESSAGE_LDAP_OFFLINE"
#define IHM_LABEL_ERR_SESSION_IDLE_TIMEOUT			 	"LABEL_ERR_SESSION_IDLE_TIMEOUT"
#define IHM_LABEL_ERR_SESSION_NOT_FOUND			 		"LABEL_ERR_SESSION_NOT_FOUND"


#define IHM_LABEL_LOGIN_DIALOG_NAME						"LABEL_LOGIN_DIALOG_NAME"
#define IHM_LABEL_LOGIN_DIALOG_PWD						"LABEL_LOGIN_DIALOG_PWD"
#define IHM_LABEL_LOGIN_DIALOG_SEC_NUM					"LABEL_LOGIN_DIALOG_SEC_NUM"

#define IHM_LABEL_TITLE_TAKE_OVER_DIALOG_QUESTION		"LABEL_TITLE_TAKE_OVER_DIALOG_QUESTION"
#define IHM_LABEL_TAKE_OVER_DIALOG_QUESTION				"LABEL_TAKE_OVER_DIALOG_QUESTION"

#define IHM_LABEL_TAKE_OVER_WAITING_DLG_TITLE			"LABEL_TAKE_OVER_WAITING_DLG_TITLE"
#define IHM_LABEL_TAKE_OVER_WAITING_DLG_MSG				"LABEL_TAKE_OVER_WAITING_DLG_MSG"

#define IHM_LABEL_TAKE_CTRL_FAILURE_DLG_TITLE			"LABEL_TAKE_CTRL_FAILURE_DLG_TITLE"


#define IHM_LABEL_BTN_TAKE_CTRL							"LABEL_BTN_TAKE_CTRL"
#define IHM_LABEL_BTN_RETURN_CTRL						"LABEL_BTN_RETURN_CTRL"
#define IHM_LABEL_BTN_IDENTIF							"LABEL_BTN_IDENTIF"
#define IHM_LABEL_BTN_END_IDENTIF						"LABEL_BTN_END_IDENTIF"


#define IHM_LABEL_IN_CONTROL							"LABEL_IN_CONTROL"
#define IHM_LABEL_NOT_IN_CONTROL						"LABEL_NOT_IN_CONTROL"
//-------------------------------------------------

class LanguageLabel
{
public:
	bool operator<(const LanguageLabel &x) const { return m_sLabelName < x.m_sLabelName; }

	QString m_sLabelName;
	QString m_sLabelTranslation;
	QString m_sLabelTranslationForDsk;
	QString m_sLabelTranslationForWeb;
};


class LanguageList;


class MIhmLanguages
{

public:
	MIhmLanguages();
	~MIhmLanguages();

	enum enumTranslTarget{
		enuTranslTargetNoTransform = 0,  
		enuTranslTargetDesktop,  
		enuTranslTargetWeb  
	};

	//reads all the language entries from registry and 
	//sets the selected language to default
// 	bool initFromReg(QString sDefltLanguage);
	
	bool initFromFile(QString sLangFilePath, QString sDefltLanguage, bool bUsedForWeb);
		
	bool setLanguage(QString sLanguage);
	QString getLanguage(){ return m_sSelectedLanguage; };

	//returns the translation of the label for the currently selected language
	bool getLabelTranslation(QString sLabelName, enumTranslTarget eTranslType, QString &sRetVal);

	//These labels are used as text for the old Saisie Input dialogs
	QString getOKButtonTranslation(enumTranslTarget eTranslType);
	QString getCancelButtonTranslation(enumTranslTarget eTranslType);
	
	//These labels are used for the Login client dialog
	QString getLoginOKButtonTranslation(enumTranslTarget eTranslType);
	QString getLoginCancelButtonTranslation(enumTranslTarget eTranslType);
	QString getClientCloseButtonTranslation(enumTranslTarget eTranslType);

	
	//TAKE OVER PROCEDURE LABELS:
	QString getTakeOverAcceptButtonTranslation(enumTranslTarget eTranslType);
	QString getTakeOverRejectButtonTranslation(enumTranslTarget eTranslType);

	QString getLoginTitleTranslation(enumTranslTarget eTranslType);
	QString getLoginMessageTranslation(enumTranslTarget eTranslType);
	QString getLoginLDAPOfflineMessageTranslation(enumTranslTarget eTranslType);
	QString getLoginNameTranslation(enumTranslTarget eTranslType);
	QString getLoginPwdTranslation(enumTranslTarget eTranslType);
	QString getLoginSecNumberTranslation(enumTranslTarget eTranslType);
	QString getInControlTranslation(enumTranslTarget eTranslType);
	QString getNotInControlTranslation(enumTranslTarget eTranslType);

	QString getTakeOverQuestionTitleTranslation(enumTranslTarget eTranslType);
	QString getTakeOverQuestionTranslation(enumTranslTarget eTranslType, QString sUserName);
	
	QString getTakeOverInfoTitleTranslation(enumTranslTarget eTranslType);
	QString getTakeOverInfoTranslation(enumTranslTarget eTranslType);

	QString getTakeCtrlFailureDlgTitleTranslation(enumTranslTarget eTranslType);
	
	QString getTakeCtrlRejectedByUserMsgTranslation(enumTranslTarget eTranslType);
	
	QString getLabelErrMsgTooManyUsersConnected(enumTranslTarget eTranslType);
	QString getLabelErrSessionIdleTimeout(enumTranslTarget eTranslType);
	QString getLabelErrSessionNotFound(enumTranslTarget eTranslType);

	//BUTTONS
	QString getBtnLabelIdentification(enumTranslTarget eTranslType);
	QString getBtnLabelEndIdentification(enumTranslTarget eTranslType);
	QString getBtnLabelTakeControl(enumTranslTarget eTranslType);
	QString getBtnLabelReturnControl(enumTranslTarget eTranslType);

private:
// 	bool loadLanguageLabels(QList<LanguageLabel*> *pLst, QString sLangRegKey);
	
	QString m_sSelectedLanguage;

	
	QList<LanguageList*> m_lstAllLanguages;
	
	LanguageList * m_plstSelectedLanguage;

	
};


class LanguageList
{
public:
	LanguageList();
	~LanguageList();

	QList<LanguageLabel*> *m_plstLanguage;
	QString m_sLanguageName;

	static bool lessThan(const LanguageLabel* x1, const LanguageLabel* x2); 

	bool getTranslation(QString sLabelName, MIhmLanguages::enumTranslTarget eTranslType, QString &sResult);
	bool findLanguageLabel(QString sLabelName, LanguageLabel ** pFound);

	void sort();
};



#endif


