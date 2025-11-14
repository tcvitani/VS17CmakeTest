
#ifndef MIHM_HTTP_RESPONSE_H
#define MIHM_HTTP_RESPONSE_H


#include <QString>
#include <QByteArray>
#include <QStringList>


//Class MIhmHttpResponse is used to format the Http response data

class MIhmHttpResponse
{
public:
	enum enuIhmHttpRspType{
		enuRspUnknown,
		enuRspError,
		enuRspLoginOK,
		enuRspLoginNOK,
		enuRspLogout,
		enuRspRedirect,
		enuRspRefresh,
		enuRspRefreshAckOK,
		enuRspDynImagePath,
		enuRspMiniWebErr
	};

	virtual enuIhmHttpRspType getType() = 0;
	virtual bool isValid()=0;
	virtual bool getRspData(QByteArray * pData) = 0;
	static const QByteArray baHeaderNoProxy;
	static const QByteArray baHeaderWithProxy;
	static const QByteArray baError;
	
protected:
	void setSessionID(QString s){ m_sSessionID = s;};
	QString m_sSessionID;
	
};


class MIhmHttpRspError:public MIhmHttpResponse
{
	public:
		MIhmHttpRspError();

		enuIhmHttpRspType getType(){return enuRspError;};
		bool isValid();	
		bool getRspData(QByteArray * pData);
	
};

class MIhmHttpRspLoginOK:public MIhmHttpResponse
{
	public:
		MIhmHttpRspLoginOK(QString sSessionID, QString sLoginRedirectPage, bool bProxyUsed);

		enuIhmHttpRspType getType(){return enuRspLoginOK;};
		bool isValid();	
		bool getRspData(QByteArray * pData);
	
	private:
		QString m_sLoginRedirectPage;
		bool	m_bProxyUsed;
};


class MIhmHttpRspLoginNOK:public MIhmHttpResponse
{
	public:
		MIhmHttpRspLoginNOK(QString sErrMessage, QString sLoginRedirectPage, bool bProxyUsed);

		enuIhmHttpRspType getType(){return enuRspLoginNOK;};
		bool isValid();	
		bool getRspData(QByteArray * pData);
	
	private:
		QString m_sErrMessage;
		QString m_sLoginFailRedirectPage;
		bool	m_bProxyUsed;
};

class MIhmHttpRspRefresh:public MIhmHttpResponse
{
	public:
		MIhmHttpRspRefresh(QString sSessionID);

		enuIhmHttpRspType getType(){return enuRspRefresh;};
		bool isValid();	
		bool getRspData(QByteArray * pData);
	
		void addStyleCommand(QString sControlName, QString sControlStyle, QString sTimeStamp);
		void addHtmlCommand(QString sControlName, QString sControlHtml, QString sTimeStamp);
		void addExecScriptCommand(QString sControlName, QString sScriptToExecute, QString sTimeStamp);

		
		void addShowDlgCommand(QString sIhmControlName, QString sControlHtml, QString sTimeStamp);
		void addHideDlgCommand(QString sIhmControlName, QString sTimeStamp);

		void addChangeModeCommand(QString sMode, QString sTimeStamp);
		void addOpenLinkCommand(QString sCtrlId,
								QString sLinkUrl, 
								int iTargetDlgIndex,
								QString sButtonText,
								QString sTimeStamp,
								bool bClose);

		void addShowHiPrioDlgCommand(QString sIhmControlName, QString sControlHtml, QString sTimeStamp);
		void addHideHiPrioDlgCommand(QString sIhmControlName, QString sTimeStamp);
		
	private:
		QStringList m_lstCommands;
};

class MIhmHttpRspRefreshAckOK:public MIhmHttpResponse
{
	public:
		MIhmHttpRspRefreshAckOK(QString sSessionID);

		enuIhmHttpRspType getType(){return enuRspRefreshAckOK;};
		bool isValid(){return true;};	
		bool getRspData(QByteArray * pData);
	
};


class MIhmHttpRspRedirect:public MIhmHttpResponse
{
	public:
		MIhmHttpRspRedirect(QString sPage);

		enuIhmHttpRspType getType(){return enuRspRedirect;};
		bool isValid();	
		bool getRspData(QByteArray * pData);
		
		void setRedirectPage(QString s){m_sRedirectPage = s;};
		void setMessage(QString s){m_sMessage = s;};

	private:
		QString m_sRedirectPage;
		QString m_sMessage;
};

class MIhmHttpRspMiniWebError:public MIhmHttpResponse
{
	public:
		MIhmHttpRspMiniWebError();

		enuIhmHttpRspType getType(){return enuRspMiniWebErr;};
		bool isValid(){return true;};	
		bool getRspData(QByteArray * pData);
		
	private:
};




class MIhmHttpRspDynImagePath:public MIhmHttpResponse
{
	public:
		MIhmHttpRspDynImagePath(bool bOK, QString sImagePath);
		enuIhmHttpRspType getType(){return enuRspDynImagePath;};
		bool isValid(){return true;};	
		bool getRspData(QByteArray * pData);

	private:
		bool m_bOK;
		QString m_sImagePath;
	
};


#endif
