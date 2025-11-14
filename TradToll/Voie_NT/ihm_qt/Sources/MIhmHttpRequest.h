
#ifndef MIHM_HTTP_REQUEST_H
#define MIHM_HTTP_REQUEST_H


#include <QString>
#include <QByteArray>


struct SHttpParameter 
{
	QString sName;
	QString sValue;

};

//Class MIhmHttpRequest is used to parse the received http request data
// and to extract its type and parameters

class MIhmHttpRequest
{
public:
	MIhmHttpRequest();
	MIhmHttpRequest(const QByteArray& ba);
	~MIhmHttpRequest();

	enum enuIhmHttpReqType{
		enuReqUnknown,
		enuReqLogin,
		enuReqRefreshAll,
		enuReqRefreshPart,
		enuReqRefreshAck,
		enuReqKeyDown,
		enuReqDialogResponse,
		enuReqClickCommand,
		enuReqCommentsUpdate,
		enuReqGetDynImagePath,
		enuReqMiniWebRefreshAll,
		enuReqMiniWebRefreshPart
	};

	bool isValid(){return m_bValid;};
	bool parse(const QByteArray& ba);
	
	//all types
	enuIhmHttpReqType type() {return m_eType;};
	QString getSessionID() {return m_sSessionID;};
	QString getClientSessionID() {return m_sClientSessionID;};

	//enuReqLogin
	bool	isTakeControl() {return m_bTakeControl;};
	bool	isProxyUsed(){return m_bProxyUsed;};
	QString getLogin() {return m_sLogin;};
	QString getPwd() {return m_sPwd;};
	QString getSourcePage() {return m_sSourcePage;};
	QString getWebDialogId(){return m_sWebDialogId;};

	QString	getAckObjectName() {return m_sAckObjectName;};
	QString	getAckObjectTimestamp() {return m_sAckTimestamp;};

	int	getKeyCode() {return m_iAskedKey;};

	QString getDialogResponse();
	bool isDialogCanceled(){return m_bDialogCanceled;};

	static void extractAllParams(QList <SHttpParameter*> &prmList, const QByteArray & baStringParameters);
	static QString getParameterValue(QString sParamName, QList <SHttpParameter*> &prmList);
	static QString getDialogRspParamValue(QString sParamName, const QString & sDlgResponse);
	
	QString getClickCtrlId(){return m_sClickCtrlId;};
	QString getClickParam(){return m_sClickParam;};

	QString getCommentsUpdateData(){return m_sCommentsUpdateData;};

	QString getAppRootUrl(){return m_sAppRootUrl;};

	QString getDynImageObjectName(){ return m_sDynImageObjName;};

	QList <SHttpParameter *> * getAckObjectsList(){return &m_lstAckObj;};
private:
	
	bool m_bValid;

	enuIhmHttpReqType m_eType;
	
	QString m_sAppRootUrl;
	QString m_sClientSessionID;
	QString m_sSessionID;
	QString m_sSourcePage;
	QString	m_sLogin;
	QString	m_sPwd;

	// Id (string) used to identify the http request and related visible objects
	// We use names: "HighPrioDialogId", "InputDialogId", InputDilaogExId" 
	QString m_sWebDialogId; 


	bool m_bTakeControl;

	QString m_sAckObjectName;
	QString m_sAckTimestamp;

	//list of acknoledgments
	void extractObjAckTimestamps(QString sAckObjList);
	QList <SHttpParameter *> m_lstAckObj;

	QString m_sDialogResponse;
	bool m_bDialogCanceled;
	QString m_sDialogTimeStamp;

	int m_iAskedKey;

	QString m_sClickCtrlId;
	QString m_sClickParam;


	//Comments update
	QString m_sCommentsUpdateData;

	bool m_bProxyUsed;
	QByteArray m_baRawData;

	//GetDynImagePath
	QString m_sDynImageObjName;
};



#endif
