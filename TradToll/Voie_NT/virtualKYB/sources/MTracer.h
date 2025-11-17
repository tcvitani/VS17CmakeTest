#ifndef MTRACER_H
#define MTRACER_H

#include <qobject.h>
#include <qstring.h>
#include <qapplication.h>
#include <qmutex.h>
#include <windows.h>

#define REG_VAL_MAX_TRACE_SIZE_MB	   "MaxTraceSizeMB"

#define TRACE_D(MSSGE) {if(MTracer::getTracer()->isTrcEnabled())MTracer::doTRACE_D(MSSGE);}
#define TRACE_W(MSSGE)  {MTracer::doTRACE_W(MSSGE,__FILE__,__LINE__);}

class MTracer
{
public:
	MTracer();
	~MTracer();
	
	static MTracer * getTracer(){return MTracer::m_pTracer;};

	static void doTRACE_D(QString sMessage);
	static void doTRACE_W(QString sMessage, QString sFileName, int iLineNumber);

	static void Init(QString &sBallID = QString("BL_VIRT_KYB"), int iMaxTraceSize = 1);
	static void Deinit(QString &sBallID = QString("BL_VIRT_KYB"));

	bool isTrcEnabled(){return m_bTrcEnabled;};
	void setMaxTraceSize(int iMaxTraceSize);

protected:
	static void myMessageOutput( QtMsgType type, const char *msg );
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


