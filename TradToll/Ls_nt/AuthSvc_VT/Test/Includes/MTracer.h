/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MTracer.h														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef COMMON_TOOLS_TRACER_H
#define COMMON_TOOLS_TRACER_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QString.h>
#include <QMutex.h>
#include <windows.h>

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/


#define TRACE_D(MSSGE) {if(MTracer::getTracer()->isTrcEnabled())MTracer::doTRACE_D(MSSGE);}
#define TRACE_W(MSSGE)  {MTracer::doTRACE_W(MSSGE,__FILE__,__LINE__);}


/*-------------------------------- TYPEDEFS:  -------------------------------*/

class MTracer
{
	public:
		MTracer();
		~MTracer();
		
		static MTracer * getTracer();

		static void doTRACE_D(QString sMessage);
		static void doTRACE_W(QString sMessage, QString sFileName, int iLineNumber);

		static bool Init(QString &sTrcFilePath,
						QString &sErrFilePath, 
						QString &sFileNamePrefix, 
						int iMaxFileSize, 
						bool bTrcEnabled, 
						bool bErrEnabled);

		static void Deinit();

		void setMaxTraceSize(int iMaxTraceSize);
		bool isTrcEnabled(){return m_bTrcEnabled;};
		bool isErrEnabled(){return m_bErrEnabled;};

		static QString getTrcFilePath(){return MTracer::m_pTracer->m_sFullTrcFileName;};
		static QString getErrFilePath(){return MTracer::m_pTracer->m_sFullErrFileName;};


	protected:
		static void myMessageOutput( QtMsgType type, const char *msg );
		void writeDebug(QString sMsg);
		void writeWarning(QString sMsg);
		
	private:
		void lock();
		void unlock();

		void log_debug(QString sMessage);
		void log_warning(QString sMessage,
								QString sFileName,
								int iLineNumber);
		bool checkSize(HANDLE *phHandle, bool bIsTrace);
		bool initData(int iMaxTraceSize);

		static MTracer* m_pTracer;
		bool m_bTrcEnabled;
		bool m_bErrEnabled;

		HANDLE m_hTrace;
		HANDLE m_hError;
		
		QString m_sFileNamePrefix;
		unsigned int m_uiMaxSizeBytes;

		QString m_sTRC_PATH;
		QString m_sERR_PATH; 

		QString m_sFullTrcFileName;
		QString m_sFullErrFileName;
		
		QMutex m_TrcMutex;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/