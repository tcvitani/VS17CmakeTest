
#include <QStringList>
#include <QUrl>
#include <QRegularExpression>
#include <MIhmHttpRequest.h>
#include <MHelpFuncs.h>
#include <MTracer.h>

#define IHM_WEB_REQUEST_LOGIN			"Login"
#define IHM_WEB_REQUEST_LOGOUT			"Logout"
#define IHM_WEB_REQUEST_REFRESH_ALL		"RefreshAll"
#define IHM_WEB_REQUEST_REFRESH_PART	"RefreshPart"
#define IHM_WEB_REQUEST_REFRESH_ACK		"RefreshAck"
#define IHM_WEB_REQUEST_DIALOG_RESPONSE "DialogResponse"
#define IHM_WEB_REQUEST_KEY_COMMAND		"KeyCommandsUrl"
#define IHM_WEB_REQUEST_TAKE_CONTROL	"TakeOverControl"
#define IHM_WEB_REQUEST_RETURN_CONTROL	"ReturnControl"
#define IHM_WEB_REQUEST_CLICK_COMMAND	"ClickCommandUrl"
#define IHM_WEB_REQUEST_COMMENTS_UPDATE	"CommentsUpdate"
#define IHM_WEB_REQUEST_GET_DYN_IMAGE_PATH		"GetDynImagePath"

#define IHM_WEB_REQUEST_MINI_WEB_REFRESH_ALL	"MiniWebRefreshAll"
#define IHM_WEB_REQUEST_MINI_WEB_REFRESH_PART	"MiniWebRefreshPart"



#define IHM_WEB_REQUEST_LOGIN_PARAM_USER "username"
#define IHM_WEB_REQUEST_LOGIN_PARAM_PWD "password"
#define IHM_WEB_REQUEST_LOGIN_PARAM_CONTROL "takeover"
#define IHM_WEB_REQUEST_LOGIN_PARAM_PROXY "proxy"
#define IHM_WEB_REQUEST_CLIENT_SESSION_ID "client_session_id"
#define IHM_WEB_REQUEST_APP_ROOT_URL "app_root_url"

#define IHM_WEB_REQUEST_PARAM_CTRL_ACK_LIST "controls_state_update"





#define IHM_WEB_REQUEST_SESSION_ID "session_id"
#define IHM_WEB_REQUEST_SOURCE_PAGE "source_page"
#define IHM_WEB_REQUEST_DIALOG_ID "dialog_id"


#define IHM_WEB_REQUEST_DIALOG_RESP_PARAM	"dialog_response"
#define IHM_WEB_REQUEST_DIALOG_CANCELED		"cancelled"

#define IHM_WEB_REQUEST_DIALOG_TIMESTAMP_PARAM "time_stamp"

#define IHM_WEB_REQUEST_REFRESH_ACK_CTRL		"control_id"
#define IHM_WEB_REQUEST_REFRESH_ACK_TIMESTAMP	"time_stamp"

#define IHM_WEB_REQUEST_KEY_COMMAND_KEY			"key_value"
#define IHM_WEB_REQUEST_CLICK_COMMAND_CTRL_ID	"control_id"
#define IHM_WEB_REQUEST_CLICK_COMMAND_PARAM		"param"

#define IHM_WEB_REQUEST_COMMENTS_UPDATE_DATA	"data"

#define IHM_WEB_REQUEST_DYN_IMAGE_OBJ_NAME	"image"


MIhmHttpRequest::MIhmHttpRequest()
{
	m_bValid = false;
	m_eType = enuReqUnknown;
	m_bTakeControl = false;
	m_iAskedKey = 0;
	m_bProxyUsed = false;
}

MIhmHttpRequest::~MIhmHttpRequest()
{
	while(!m_lstAckObj.isEmpty())
		delete m_lstAckObj.takeFirst();
}

MIhmHttpRequest::MIhmHttpRequest(const QByteArray& ba)
{
	m_eType = enuReqUnknown;
	m_bTakeControl = false;
	m_iAskedKey = 0;
	m_bValid = false;

	parse(ba);

}

bool MIhmHttpRequest::parse(const QByteArray& ba)
{
	m_baRawData = ba;
	m_bValid = false;
	bool bIsPOST = false;

	//verify if it is POST
	if(m_baRawData.indexOf("POST")==0)
		bIsPOST = true;
	else
	{
		if(m_baRawData.indexOf("GET")==0 || m_baRawData.indexOf("OPTIONS")==0)
			bIsPOST = false;
		else
		{
			TRACE_WEB_D( "MIhmHttpRequest::parse: Invalid request format!");
			return false;
		}
	}


	QList <SHttpParameter*> prmList; 
	QByteArray baRequest;

	if(bIsPOST)
	{
		int iIndexOfReqParamsStart = m_baRawData.indexOf("/");
		int iIndexOfReqTypeEnd = m_baRawData.indexOf("HTTP", iIndexOfReqParamsStart+1);

		if(iIndexOfReqParamsStart>iIndexOfReqTypeEnd)
		{
			TRACE_WEB_D( "MIhmHttpRequest::parse: Invalid request format!");
			return false;
		}

 		baRequest = m_baRawData.mid(iIndexOfReqParamsStart+1, iIndexOfReqTypeEnd-iIndexOfReqParamsStart-1);
		baRequest = baRequest.trimmed();

		//Line[10]: Content-Length: 41 
		int iIndexOfContentLenght = m_baRawData.indexOf("Content-Length:");

		if(iIndexOfContentLenght==-1)
			iIndexOfContentLenght = m_baRawData.indexOf("content-length:"); //retry with lowercase string
		
		int iIndexOfSpace = m_baRawData.indexOf(" ", iIndexOfContentLenght);
		int iIndexOfEOL = m_baRawData.indexOf("\n", iIndexOfSpace);
		
		bool bOK;
		QByteArray baLenght = m_baRawData.mid(iIndexOfSpace, iIndexOfEOL - iIndexOfSpace -1);
		int iParamsLenght = baLenght.toInt(&bOK);

		if(!bOK)
		{
			TRACE_WEB_W( QString("MIhmHttpRequest::parse: Invalid content lenght: %1").arg(m_baRawData.mid(iIndexOfSpace, iIndexOfEOL - iIndexOfSpace).data()));
			return false;
		}

		int iLastLine = m_baRawData.lastIndexOf("\n", -2);

		if(m_baRawData.count()-iLastLine < iParamsLenght)
		{
			TRACE_WEB_W( "MIhmHttpRequest::parse: Invalid request format!");
			return false;
		}

		QByteArray baParameters;
		baParameters = m_baRawData.mid(iLastLine+1, iParamsLenght);

		extractAllParams(prmList, baParameters);

	}
	else //if GET
	{
		int iIndexOfReqParamsStart = m_baRawData.indexOf("/");
		int iIndexOfReqParamsEnd = m_baRawData.indexOf(" ", iIndexOfReqParamsStart+1);
		int iIndexOfQuest = m_baRawData.indexOf("?", iIndexOfReqParamsStart+1);

		if(iIndexOfReqParamsStart>iIndexOfReqParamsEnd||
			iIndexOfQuest>iIndexOfReqParamsEnd||
			iIndexOfReqParamsStart>iIndexOfQuest)
		{
			TRACE_WEB_D( "MIhmHttpRequest::parse: Invalid request format!");
			return false;
		}

		baRequest = m_baRawData.mid(iIndexOfReqParamsStart+1, iIndexOfQuest-iIndexOfReqParamsStart-1);
		QByteArray baParameters = m_baRawData.mid(iIndexOfQuest+1, iIndexOfReqParamsEnd-iIndexOfQuest-1);
		
		extractAllParams(prmList, baParameters);
	}



		if(baRequest == IHM_WEB_REQUEST_LOGIN)
		{
			m_eType = enuReqLogin;
 			m_sLogin = getParameterValue(IHM_WEB_REQUEST_LOGIN_PARAM_USER, prmList);
 			m_sPwd = getParameterValue(IHM_WEB_REQUEST_LOGIN_PARAM_PWD, prmList);
 			m_bTakeControl = (getParameterValue(IHM_WEB_REQUEST_LOGIN_PARAM_CONTROL, prmList)=="on");
			m_bProxyUsed = (getParameterValue(IHM_WEB_REQUEST_LOGIN_PARAM_PROXY, prmList)!="no_proxy");
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);

			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_REFRESH_ALL)
		{
			m_eType = enuReqRefreshAll;
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);

			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_REFRESH_PART)
		{
			m_eType = enuReqRefreshPart;
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);
			
			
			QString sAckObjList = getParameterValue(IHM_WEB_REQUEST_PARAM_CTRL_ACK_LIST, prmList);
			extractObjAckTimestamps(sAckObjList);
			
			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_REFRESH_ACK)
		{
			m_eType = enuReqRefreshAck;
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			m_sAckObjectName = getParameterValue(IHM_WEB_REQUEST_REFRESH_ACK_CTRL, prmList); ;
			m_sAckTimestamp = getParameterValue(IHM_WEB_REQUEST_REFRESH_ACK_TIMESTAMP, prmList);;
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);
			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_DIALOG_RESPONSE)
		{
			m_eType = enuReqDialogResponse; //dialog response
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sWebDialogId = getParameterValue(IHM_WEB_REQUEST_DIALOG_ID, prmList);
			m_sDialogResponse = getParameterValue(IHM_WEB_REQUEST_DIALOG_RESP_PARAM, prmList);
			m_sDialogTimeStamp = getParameterValue(IHM_WEB_REQUEST_DIALOG_TIMESTAMP_PARAM, prmList);
			
			QString sCanceled = getParameterValue(IHM_WEB_REQUEST_DIALOG_CANCELED, prmList);
			m_bDialogCanceled = (sCanceled.compare("false", Qt::CaseInsensitive)==0)?false:true;
								
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);
			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_KEY_COMMAND)
		{
			m_eType = enuReqKeyDown; 
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			QString sKey = getParameterValue(IHM_WEB_REQUEST_KEY_COMMAND_KEY, prmList);
			bool bOK;
			m_iAskedKey = sKey.toInt(&bOK);

			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);
			if(bOK)
				m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_CLICK_COMMAND)
		{
			m_eType = enuReqClickCommand; 
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			m_sClickCtrlId = getParameterValue(IHM_WEB_REQUEST_CLICK_COMMAND_CTRL_ID, prmList);
			m_sClickParam = getParameterValue(IHM_WEB_REQUEST_CLICK_COMMAND_PARAM, prmList);
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);

			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_COMMENTS_UPDATE)
		{
			m_eType = enuReqCommentsUpdate; 
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			m_sCommentsUpdateData = getParameterValue(IHM_WEB_REQUEST_COMMENTS_UPDATE_DATA, prmList);
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);
			
			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_GET_DYN_IMAGE_PATH)
		{
			m_eType = enuReqGetDynImagePath; 
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sDynImageObjName = getParameterValue(IHM_WEB_REQUEST_DYN_IMAGE_OBJ_NAME, prmList);
			m_bValid = true;
		}
		else if(baRequest == IHM_WEB_REQUEST_MINI_WEB_REFRESH_ALL)
		{
			m_eType = enuReqMiniWebRefreshAll;
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);

			m_bValid = true;
		}		
		else if(baRequest == IHM_WEB_REQUEST_MINI_WEB_REFRESH_PART)
		{
			m_eType = enuReqMiniWebRefreshPart;
			m_sSessionID = getParameterValue(IHM_WEB_REQUEST_SESSION_ID, prmList);
			m_sClientSessionID = getParameterValue(IHM_WEB_REQUEST_CLIENT_SESSION_ID, prmList);
			m_sSourcePage = getParameterValue(IHM_WEB_REQUEST_SOURCE_PAGE, prmList);
			m_sAppRootUrl = getParameterValue(IHM_WEB_REQUEST_APP_ROOT_URL, prmList);

			QString sAckObjList = getParameterValue(IHM_WEB_REQUEST_PARAM_CTRL_ACK_LIST, prmList);
			extractObjAckTimestamps(sAckObjList);

			m_bValid = true;
		}		
		else 
		{
			m_eType = enuReqUnknown;
			TRACE_WEB_W(QString("MIhmHttpRequest::parseResponse:ERROR: Request %1 not recognized!")
								.arg(baRequest.data()));
		}

		if(m_sClientSessionID=="")
		{
			TRACE_WEB_W(QString("MIhmHttpRequest::parse: Invalid parameter %1!").arg(IHM_WEB_REQUEST_CLIENT_SESSION_ID));
		}

		while(!prmList.isEmpty())
			delete prmList.takeLast();
	
	return m_bValid;
}



void MIhmHttpRequest::extractObjAckTimestamps(QString sAckObjList)
{
	if(!sAckObjList.isEmpty())
	{
		QStringList sLst = sAckObjList.split(";");
		QString sName, sValue;

		for(int i=0; i< sLst.count(); i++)
		{
			int pos = sLst.at(i).indexOf('=');

			if(pos>0)
			{
				SHttpParameter* pNewParam = new SHttpParameter;

				pNewParam->sName = sLst.at(i).left(pos);
				pNewParam->sValue = sLst.at(i).right( sLst.at(i).size() - pos - 1);

				m_lstAckObj.append(pNewParam);
			}
		}
	}

}
		 
		
void MIhmHttpRequest::extractAllParams(QList <SHttpParameter*> &prmList, const QByteArray & baStringParameters)
{
	QByteArray baRetVal;
	QByteArray baName, baValue;


	QList<QByteArray> baLstParams = baStringParameters.split('&');

	for(int i=0; i<baLstParams.size();i++)
	{
		QByteArray ba = baLstParams[i];

		int pos = ba.indexOf('=');
		if(pos>=0)
		{
			baName = ba.left(pos);
			baValue = ba.right( ba.size() - pos - 1);	
			
			SHttpParameter* pNewParam = new SHttpParameter;

			pNewParam->sName = baName;
			baValue = baValue.replace('+', ' ');
			pNewParam->sValue = QUrl::fromPercentEncoding(baValue);

			prmList.append(pNewParam);
		}

	}
}





QString MIhmHttpRequest::getParameterValue(QString sParamName, QList <SHttpParameter*> &prmList)
{
	QString sRetVal;

	for(int i=0; i<prmList.size();i++)
	{
		SHttpParameter *pCurrent = prmList.at(i);

		if(pCurrent->sName==sParamName)
		{
			sRetVal = pCurrent->sValue;
			break;
		}

	}

	return sRetVal;
}

QString MIhmHttpRequest::getDialogRspParamValue(QString sParamName, const QString & sDlgResponse)
{
	QString sRetVal;

	QString sRegDef = QString("%1~([^\\|]*)\\|").arg(sParamName);
	QRegularExpression rx(sRegDef);

	QRegularExpressionMatch match = rx.match(sDlgResponse);
	if (match.hasMatch())
		sRetVal = match.captured(1);

	return sRetVal;
	
}

QString MIhmHttpRequest::getDialogResponse()
{ 
	return m_sDialogResponse;

}
