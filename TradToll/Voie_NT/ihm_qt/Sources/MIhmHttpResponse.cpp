
#include <MIhmHttpResponse.h>
#include <QStringList>
#include <QTime>
#include <QUrl>
#include <MHelpFuncs.h>
#include <MTracer.h>
#include "MIhmConfigWeb.h"


const QByteArray MIhmHttpResponse::baHeaderNoProxy = QByteArray("HTTP/1.1 200 ok\nContent-type: text/html\n");
const QByteArray MIhmHttpResponse::baHeaderWithProxy = QByteArray("HTTP/1.1 200 ok\nContent-type: text/html\nContent-Length: %lenght%\r\n\r\n");
const QByteArray MIhmHttpResponse::baError = QByteArray("HTTP/1.1 404 Not Found\n\r\n\r\n");


//----------------------------------------------------

MIhmHttpRspError::MIhmHttpRspError()
{

}

bool MIhmHttpRspError::isValid()
{
	return true;
}

bool MIhmHttpRspError::getRspData(QByteArray * pData)
{
	*pData = baError;

	return isValid();
}




MIhmHttpRspLoginOK::MIhmHttpRspLoginOK(QString sSessionID, QString sLoginRedirectPage, bool bProxyUsed)
{
	setSessionID(sSessionID);
	m_sLoginRedirectPage = sLoginRedirectPage;
	m_bProxyUsed = bProxyUsed;
}

bool MIhmHttpRspLoginOK::isValid()
{
	if(m_sSessionID!="" && m_sLoginRedirectPage!="")
		return true;
	else
		return false;
}

bool MIhmHttpRspLoginOK::getRspData(QByteArray * pData)
{
	QByteArray ba;


	if(!m_bProxyUsed)
	{
	// 	HTTP/1.1 200 ok
	// 	Refresh: 0; url=http://www.example.com/
	// 	Content-type: text/html
		ba = baHeaderNoProxy;
		ba += QByteArray("Refresh: 0; url=%redirect%?session_id=%session%\n");
		ba.replace("%redirect%", m_sLoginRedirectPage.toLatin1());
		ba.replace("%session%", m_sSessionID.toLatin1());
	}
	else
	{
		ba = baHeaderWithProxy;

		QByteArray baContent;
		baContent = QByteArray("%redirect%?session_id=%session%");
		baContent.replace("%redirect%", m_sLoginRedirectPage.toLatin1());
		baContent.replace("%session%", m_sSessionID.toLatin1());
		
		int iSize = baContent.size();
		QByteArray baSize = (QString("%1").arg(iSize)).toLatin1();
		ba.replace("%lenght%", baSize);
		
		ba += baContent;
	}

	
	*pData = ba;

	return isValid();
}

//-------------------------------------

MIhmHttpRspLoginNOK::MIhmHttpRspLoginNOK(QString sErrMessage, QString sLoginRedirectPage, bool bProxyUsed)
{
	m_sErrMessage = sErrMessage;
	m_sLoginFailRedirectPage = sLoginRedirectPage;
	m_bProxyUsed = bProxyUsed;

}

bool MIhmHttpRspLoginNOK::isValid()
{
		return true;
}

bool MIhmHttpRspLoginNOK::getRspData(QByteArray * pData)
{
	QByteArray ba;

	QString sMessage = QUrl::toPercentEncoding(m_sErrMessage);

	if(!m_bProxyUsed)
	{
		ba = baHeaderNoProxy;
		ba += QByteArray("Refresh: 0; url=%redirect%?message=%message%\n");
		ba.replace("%redirect%", m_sLoginFailRedirectPage.toLatin1());
		ba.replace("%message%", sMessage.toLatin1());
	}
	else
	{
		ba = baHeaderWithProxy;

		QByteArray baContent;
		baContent = QByteArray("%redirect%?message=%message%");
		baContent.replace("%redirect%", m_sLoginFailRedirectPage.toLatin1());
		baContent.replace("%message%", sMessage.toLatin1());
		
		int iSize = baContent.size();
		QByteArray baSize = (QString("%1").arg(iSize)).toLatin1();
		ba.replace("%lenght%", baSize);
		
		ba += baContent;
	}

	*pData = ba;

	return isValid();
}

//------------------------------------------------------------
//------------------------------------------------------------



MIhmHttpRspRefresh::MIhmHttpRspRefresh(QString sSessionID)
{
	setSessionID(sSessionID);
}

bool MIhmHttpRspRefresh::isValid()
{
	if(m_sSessionID!="")
		return true;
	else
		return false;
}

void MIhmHttpRspRefresh::addStyleCommand(QString sControlName, QString sControlStyle, QString sTimeStamp)
{
	QString sNewCommand;
	
// 		{
// 			"command_type":"UpdateControlStyle",
// 			"id":"%control%",
// 			"time_stamp":"%timestamp%",
// 			"style":"%style%"
// 		}

	sNewCommand = QString("{\"command_type\":\"UpdateControlStyle\",\"id\":\"%control%\",");
	sNewCommand += QString("\"time_stamp\":\"%timestamp%\",\"style\":\"%style%\" }");

	sNewCommand = sNewCommand.replace("%control%",sControlName);
	sNewCommand = sNewCommand.replace("%timestamp%",sTimeStamp);
	sNewCommand = sNewCommand.replace("%style%",MHelpFuncs::escapeString(sControlStyle));
	

	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);
}

void MIhmHttpRspRefresh::addHtmlCommand(QString sControlName, QString sControlHtml, QString sTimeStamp)
{
	QString sNewCommand;

// 		{
// 			"command_type":"UpdateControl",
// 			"id":"%control%",
// 			"time_stamp":"%timestamp%",
// 			"html":"%html%"
// 		}

	sNewCommand = QString("{\"command_type\":\"UpdateControl\",\"id\":\"%control%\",");
	sNewCommand += QString("\"time_stamp\":\"%timestamp%\",\"html\":\"%html%\" }");

	sNewCommand = sNewCommand.replace("%control%",sControlName);
	sNewCommand = sNewCommand.replace("%timestamp%",sTimeStamp);
	sNewCommand = sNewCommand.replace("%html%",MHelpFuncs::escapeString(sControlHtml));
	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);
}

void MIhmHttpRspRefresh::addExecScriptCommand(QString sControlName, QString sScriptToExecute, QString sTimeStamp)
{
	QString sNewCommand;

// 		{
// 			"command_type":"ExecuteScript",
// 			"id":"%control%",
// 			"time_stamp":"%timestamp%",
// 			"script":"%script%"
// 		}

	sNewCommand = QString("{\"command_type\":\"ExecuteScript\",\"id\":\"%control%\",");
	sNewCommand += QString("\"time_stamp\":\"%timestamp%\",\"script\":\"%script%\" }");

	sNewCommand = sNewCommand.replace("%control%",sControlName);
	sNewCommand = sNewCommand.replace("%timestamp%",sTimeStamp);
	sNewCommand = sNewCommand.replace("%script%",MHelpFuncs::escapeString(sScriptToExecute));
	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);
}




void MIhmHttpRspRefresh::addChangeModeCommand(QString sMode, QString sTimeStamp)
{
	QString sNewCommand;

// 	{
// 		"command_type":"ChangeMode",
// 		"mode":"readonly",     //or "incontrol"
// 		"time_stamp":"2009-12-03T14:16:44+01:00"
// 	}

	sNewCommand = QString("{\"command_type\":\"ChangeMode\",\"mode\":\"%mode%\",");
	sNewCommand += QString("\"time_stamp\":\"%timestamp%\"}");

	sNewCommand = sNewCommand.replace("%timestamp%",sTimeStamp);
	sNewCommand = sNewCommand.replace("%mode%",MHelpFuncs::escapeString(sMode));
	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);

}

void MIhmHttpRspRefresh::addOpenLinkCommand(QString sCtrlId,
											QString sLinkUrl, 
											int iTargetDlgIndex,
											QString sButtonText,
											QString sTimeStamp,
											bool bClose)
{
	QString sNewCommand;

// 	{
// 		"command_type":"OpenLink",
//		"id":"%control_id%",
// 		"window_id":"1",     
//		"url":"http://",
//		"button_text":"trlabrla",
//		"dlg_close":1,	
// 		"time_stamp":"2009-12-03T14:16:44+01:00"
// 	}

	sNewCommand = QString("{\"command_type\":\"OpenLink\",\"id\":\"%control_id%\",\"url\":\"%url%\",\"window_id\":\"%window_id%\",\"button_text\":\"%button_text%\",\"dlg_close\":\"%dlg_close%\",");
	sNewCommand += QString("\"time_stamp\":\"%timestamp%\"}");

	sNewCommand = sNewCommand.replace("%timestamp%",sTimeStamp);
	sNewCommand = sNewCommand.replace("%control_id%",sCtrlId);
	sNewCommand = sNewCommand.replace("%url%",MHelpFuncs::escapeString(sLinkUrl));
	sNewCommand = sNewCommand.replace("%window_id%",QString("%1").arg(iTargetDlgIndex));
	sNewCommand = sNewCommand.replace("%button_text%", sButtonText);
	sNewCommand = sNewCommand.replace("%dlg_close%", bClose?"1":"0");
	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);

}


void MIhmHttpRspRefresh::addShowDlgCommand(QString sIhmControlName, QString sControlHtml, QString sTimeStamp)
{
	QString sNewCommand;

// 		{
// 			"command_type":"ShowDialog",
//			"dialog_id":"%ihm_control_name%", //name used by ihm to process the dialog result
// 			"time_stamp":"%timestamp%",
// 			"html":"%html%"
// 		}

	sNewCommand = QString("{\"command_type\":\"ShowDialog\",\"dialog_id\":\"%1\",")
							.arg(sIhmControlName);
	sNewCommand += QString("\"time_stamp\":\"%1\",\"html\":\"%2\" }")
							.arg(sTimeStamp)
							.arg(MHelpFuncs::escapeString(sControlHtml));
	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);

}



void MIhmHttpRspRefresh::addHideDlgCommand(QString sIhmControlName, QString sTimeStamp)
{
	QString sNewCommand;

// 		{
// 			"command_type":"HideDialog",
//			"dialog_id":"%ihm_control_name%", //name used by ihm to process the dialog result
// 			"time_stamp":"%timestamp%",
// 		}

	sNewCommand = QString("{\"command_type\":\"HideDialog\",\"dialog_id\":\"%1\",")
							.arg(sIhmControlName);
	sNewCommand += QString("\"time_stamp\":\"%1\"}")
							.arg(sTimeStamp);

	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);

}

void MIhmHttpRspRefresh::addShowHiPrioDlgCommand(QString sIhmControlName, QString sControlHtml, QString sTimeStamp)
{
	QString sNewCommand;

// 		{
// 			"command_type":"ShowHighPriorityDialog",
//			"dialog_id":"%ihm_control_name%", //name used by ihm to process the dialog result
// 			"time_stamp":"%timestamp%",
// 			"html":"%html%"
// 		}
	sNewCommand = QString("{\"command_type\":\"ShowHighPriorityDialog\",\"dialog_id\":\"%1\",")
							.arg(sIhmControlName);
	sNewCommand += QString("\"time_stamp\":\"%1\",\"html\":\"%2\" }")
							.arg(sTimeStamp)
							.arg(MHelpFuncs::escapeString(sControlHtml));

	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);

}

void MIhmHttpRspRefresh::addHideHiPrioDlgCommand(QString sIhmControlName, QString sTimeStamp)
{
	QString sNewCommand;

// 		{
// 			"command_type":"HideHighPriorityDialog",
//			"dialog_id":"%ihm_control_name%", //name used by ihm to process the dialog result
// 			"time_stamp":"%timestamp%",
// 		}

	sNewCommand = QString("{\"command_type\":\"HideHighPriorityDialog\",\"dialog_id\":\"%1\",")
							.arg(sIhmControlName);
	sNewCommand += QString("\"time_stamp\":\"%1\"}")
							.arg(sTimeStamp);
	
	if(sNewCommand!="")
		m_lstCommands.append(sNewCommand);

}

bool MIhmHttpRspRefresh::getRspData(QByteArray * pData)
{


	//this is content:
// {
// 	"commands":
// 	[
// 		{
// 			"command_type":"UpdateTime",
// 			"value":"14:16:44.125",
// 			"format":"HH:mm:ss",
// 		},
// 		{
// 			"command_type":"UpdateControl",
// 			"id":"IcoExitGate",
// 			"time_stamp":"2009-12-03T14:16:44+01:00",
// 			"html":"<div id=\"IcoExitGate\" class=\"control image\" style=\"\"><div class=\"image-holder\"><img src=\".\/images\/gate_down.gif\" \/><\/div><\/div>"
// 		},
// 		{
// 			"command_type":"UpdateControlStyle",
// 			"id":"TxtClass",
// 			"time_stamp":"2009-12-03T14:18:09+01:00",
// 			"style":"color: #ffff;"
// 		}
// 	]
// }

	QByteArray ba;

	ba = baHeaderWithProxy; 

	//prepare content
	QByteArray baContent;
		
	baContent = QByteArray("{\"commands\":["); //start of commands response
	
	baContent += QByteArray("{\"command_type\":\"UpdateTime\", \"value\":\"");
	baContent += QTime::currentTime().toString("HH:mm:ss.zzz").toLatin1();
	baContent += QByteArray("\", \"format\":\"");
	QString sTimeFormat = MIhmConfigWeb::getCfg()->getDefaultTimeFormat();			
	baContent += sTimeFormat.toLatin1();
	baContent += QByteArray("\" }");

	
	for(int i=0; i<m_lstCommands.size();i++)
	{
		baContent += QByteArray(",");
		baContent += m_lstCommands.at(i).toLatin1();
	}

	baContent += QByteArray("] }");
	

	int iSize = baContent.size();
	QByteArray baSize = (QString("%1").arg(iSize)).toLatin1();
	ba.replace("%lenght%", baSize);
	
	ba += baContent;

	
	*pData = ba;

	return isValid();
}
	
//------------------------------------------------------------
//------------------------------------------------------------

MIhmHttpRspRefreshAckOK::MIhmHttpRspRefreshAckOK(QString sSessionID)
{
	setSessionID(sSessionID);
}

bool MIhmHttpRspRefreshAckOK::getRspData(QByteArray * pData)
{
	QByteArray ba;

	ba = baHeaderWithProxy; 

	//prepare content
	QByteArray baContent;
	baContent = QByteArray("{\"commands\":["); //start of commands response
	baContent += QByteArray("{\"command_type\":\"UpdateTime\", \"value\":\"");
	baContent += QTime::currentTime().toString("hh:mm:ss.zzz").toLatin1();
	baContent += QByteArray("\" }");
	baContent += QByteArray("] }");
	
	int iSize = baContent.size();
	QByteArray baSize = (QString("%1").arg(iSize)).toLatin1();
	ba.replace("%lenght%", baSize);
	
	ba += baContent;


	*pData = ba;

	return true;
}
//------------------------------------------------------------
//------------------------------------------------------------




MIhmHttpRspRedirect::MIhmHttpRspRedirect(QString sPage)
{
	setRedirectPage(sPage);	
	m_sMessage = "";
}

bool MIhmHttpRspRedirect::isValid()
{
	if(m_sRedirectPage!="")
		return true;
	else
		return false;
}

bool MIhmHttpRspRedirect::getRspData(QByteArray * pData)
{

// {
// 	"commands":
// 	[
// 		{
// 			"command_type":"Redirect",
// 			"destination":"blocked-mode.html"
// 		}
// 
// 	]
// }

	QByteArray ba;
	ba = baHeaderWithProxy; 

	//prepare content
	QByteArray baContent;

	baContent = QByteArray("{\"commands\":["); //start of commands response
	baContent += QByteArray("{\"command_type\":\"Redirect\", \"destination\":\"");
	baContent += m_sRedirectPage.toLatin1();
	
	if(m_sMessage!="")
	{
		QByteArray baMsg = QByteArray("?message=%message%");
		baMsg.replace("%message%", MHelpFuncs::escapeString(m_sMessage).toLatin1());
		baContent += baMsg;
	}	

	baContent += QByteArray("\" }");
	baContent += QByteArray("] }");

	int iSize = baContent.size();
	QByteArray baSize = (QString("%1").arg(iSize)).toLatin1();
	ba.replace("%lenght%", baSize);
	
	ba += baContent;
	
	*pData = ba;

	return isValid();
}
	
//------------------------------------------------------------------------------

MIhmHttpRspMiniWebError::MIhmHttpRspMiniWebError()
{

}

bool MIhmHttpRspMiniWebError::getRspData(QByteArray * pData)
{
	QByteArray ba;
	ba = QByteArray("HTTP/1.1 299 ok\nContent-type: text/html\nContent-Length: %lenght%\r\n\r\n");

	//prepare content
	QByteArray baContent;

	baContent = QByteArray("{\"commands\":["); //start of commands response
	baContent += QByteArray("] }");

	int iSize = baContent.size();
	QByteArray baSize = (QString("%1").arg(iSize)).toLatin1();
	ba.replace("%lenght%", baSize);

	ba += baContent;
	*pData = ba;

	return true;
}

//------------------------------------------------------------------------------
MIhmHttpRspDynImagePath::MIhmHttpRspDynImagePath(bool bOK, QString sImagePath)
{
	m_bOK = bOK;
	m_sImagePath = sImagePath;
}

		
bool MIhmHttpRspDynImagePath::getRspData(QByteArray * pData)
{	
	QByteArray ba;

	ba = baHeaderWithProxy; 

	//prepare content
	QByteArray baContent;
	baContent = QByteArray("{\"valid\":\""); //start of commands response
	baContent += (m_bOK)?QByteArray("true"):QByteArray("false");
	baContent += QByteArray("\",");
	baContent += QByteArray("\"path\":\"");
	baContent += MHelpFuncs::escapeString(m_sImagePath).toLatin1();
	baContent += QByteArray("\"}");
	
	int iSize = baContent.size();
	QByteArray baSize = (QString("%1").arg(iSize)).toLatin1();
	ba.replace("%lenght%", baSize);
	
	ba += baContent;

	*pData = ba;

	return true;	
}