#ifndef MINST_TRACER_H
#define MINST_TRACER_H

#include <windows.h>

#ifdef __cplusplus

	#include <qobject.h>
	#include <qstring.h>
	#include <qmutex.h>

	extern "C"
	{
#endif

	#include <trc.h>

#ifdef __cplusplus
	}
#endif


#define MInstTrtacer_FichierError     ERR_FILE=__FILE__,\
                                      ERR_LINE=__LINE__,\
                                      DEFINE_MInstTracer_FichierError

#define TRACE_INSTANCE_MAX 20
#define IHM_REG_VAL_MAX_TRACE_SIZE_MB	   "MaxTraceSizeMB"



	#ifdef __cplusplus

		#define TRACE_D(Inst, MSSGE) {MInstTracer::doTRACE_D(Inst, MSSGE);}
		#define TRACE_W(Inst, MSSGE)  {MInstTracer::doTRACE_W(Inst, MSSGE,__FILE__,__LINE__);}


		class MInstTracer
		{
		public:
			MInstTracer(short int siInstId, QString sBallID);
			~MInstTracer();
	
			static MInstTracer * getTracer(short int siInstId);

			static void doTRACE_D(short int siInstId, QString sMessage);
			static void doTRACE_W(short int siInstId, QString sMessage, QString sFileName, int iLineNumber);

			static bool Init(short int siInstId, QString sBallID, int iMaxTraceSize = 1);
			static void Deinit(short int siInstId);

			bool isTrcEnabled(){return m_bTrcEnabled;};
			bool isErrEnabled(){return m_bErrEnabled;};

			void setMaxTraceSize(int iMaxTraceSize);
		private:
			//void lock();
			//void unlock();
	
			void log_debug(QString sMessage);
			void log_warning(QString sMessage,
									QString sFileName,
									int iLineNumber);
			bool initData(int iMaxTraceSize);
			void deinitData();

			static MInstTracer* m_apTracer[TRACE_INSTANCE_MAX];

			bool m_bTrcEnabled;
			bool m_bErrEnabled;

			QString m_sBallID;
			DWORD m_dwMaxSizeBytes;

			bool CreatePath(QString sPath);

			QString m_sERR_PATH;
			QString m_sTRC_PATH; 
			QString m_sFullTrcFileName;
			QString m_sFullErrFileName;
	
			//QMutex m_TrcMutex;

			TRC_EMETTEUR m_emetTrace;
			TRC_EMETTEUR m_emetError;

			short int m_siInstId;
		};


		extern "C"
		{

	#endif


		BOOL WINAPI MInstTracer_InitTrace(const char *pcBal, short siInstId, int iTraceFileMaxSizeM);
		void WINAPI MInstTracer_FichierTrace(short siInstId, const char *szFmt, ...);
		void WINAPI DEFINE_MInstTracer_FichierError(short siInstId, const char *szFmt, ...);
		void WINAPI MInstTracer_DeinitTrace(short siInstId);

		extern const char *ERR_FILE;
		extern int  ERR_LINE;

	#ifdef __cplusplus
		}
	#endif

#endif


