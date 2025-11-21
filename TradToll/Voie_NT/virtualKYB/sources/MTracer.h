#ifndef MTRACER_H
#define MTRACER_H

#include <qobject.h>
#include <qstring.h>
#include <qapplication.h>
#include <qmutex.h>
#include <windows.h>

#define IHM_REG_VAL_MAX_TRACE_SIZE_MB	   "MaxTraceSizeMB"
#define IHM_REG_VAL_MAX_TRACE_SIZE_WEB_MB  "MaxTraceSizeWebMB"


#define TRACE_D(MSSGE) {if(MTracer::getTracer()->isTrcEnabled())MTracer::doTRACE_D(MSSGE);}
#define TRACE_W(MSSGE)  {MTracer::doTRACE_W(MSSGE,__FILE__,__LINE__);}
#define TRACE_WEB_D(MSSGE) {if(MTracer::getWebTracer()->isTrcEnabled())MTracer::doTRACE_WEB_D(MSSGE);}
#define TRACE_WEB_W(MSSGE) {MTracer::doTRACE_WEB_W(MSSGE,__FILE__,__LINE__);}


class MTracer
{
public:
	MTracer();
	~MTracer();
	
	static MTracer * getTracer(){return MTracer::m_pTracer;};
	static MTracer * getWebTracer(){return MTracer::m_pWebTracer;};

	static void doTRACE_D(QString sMessage);
	static void doTRACE_W(QString sMessage, QString sFileName, int iLineNumber);
	static void doTRACE_WEB_D(QString sMessage);
	static void doTRACE_WEB_W(QString sMessage, QString sFileName, int iLineNumber);

	static void Init(QString sBallID = QString("BL_IHM_PEAGER"), int iMaxTraceSize = 1);
	static void Deinit(QString sBallID = QString("BL_IHM_PEAGER"));
	static void InitWeb(QString sBallID = QString("BL_IHM_PEAGER_WEB"), int iMaxTraceSize = 1);
	static void DeinitWeb(QString sBallID = QString("BL_IHM_PEAGER_WEB"));

	bool isTrcEnabled(){return m_bTrcEnabled;};
	void setMaxTraceSize(int iMaxTraceSize);

protected:
	static void myMessageOutput( QtMsgType type, const QMessageLogContext& ctx, const QString& msg);
	void writeDebug(QString sMsg);
	void writeWarning(QString sMsg);
	
private:
	void lock();
	void unlock();

	bool isErrEnabled(){return m_bErrEnabled;};
	
	void log_debug(QString sMessage);
	void log_warning(QString sMessage,
							QString sFileName,
							int iLineNumber);
	bool checkSize(HANDLE *phHandle, bool bIsTrace);
	void initData(MTracer * pTracer, int iMaxTraceSize);

	static MTracer* m_pTracer;
	static MTracer* m_pWebTracer;

	bool m_bTrcEnabled;
	bool m_bErrEnabled;

	HANDLE m_hTrace;
	HANDLE m_hError;
	
	QString m_sBallID;
	DWORD m_dwMaxSizeBytes;

	QString m_sERR_PATH; 
	QString m_sTRC_PATH; 
	QString m_sFullTrcFileName;
	QString m_sFullErrFileName;
	
	QMutex m_TrcMutex;
};


#endif


