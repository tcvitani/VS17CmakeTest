#include <qdatetime.h>
#include <QSettings>
#include <QFile>
#include <QDir>

#include "MInstTracer.h"
#include "MHelpFuncs.h"


#include <windows.h>

extern "C"
{
#include <csrlc32.h>
#include <reg.h>

	const char *ERR_FILE;
	int  ERR_LINE;
}


MInstTracer* MInstTracer::m_apTracer[TRACE_INSTANCE_MAX] = { 0 };


#define DEFAULT_ERR_PATH "c:\\csr\\_errors\\"
#define DEFAULT_TRC_PATH "c:\\csr\\_traces\\"

//void MInstTracer::lock()
//{
//	m_TrcMutex.lock();
//}
//
//void MInstTracer::unlock()
//{
//	m_TrcMutex.unlock();
//}

MInstTracer * MInstTracer::getTracer(short int siInstId)
{	
	return MInstTracer::m_apTracer[siInstId];
}



MInstTracer::MInstTracer(short int siInstId, QString sBallID)
{
	m_siInstId = siInstId;
	m_sBallID = sBallID;
	m_bTrcEnabled = false;
	m_bErrEnabled = false;
	MInstTracer::m_apTracer[siInstId] = this;
}

MInstTracer::~MInstTracer()
{
	deinitData();
	m_bTrcEnabled = false;
	m_bErrEnabled = false;
	MInstTracer::m_apTracer[m_siInstId] = NULL;
}

void MInstTracer::doTRACE_D(short int siInstId, QString sMessage)
{
	MInstTracer * pTrc = MInstTracer::getTracer(siInstId);

	if(pTrc!=NULL)
	{
		if(pTrc->isTrcEnabled())
		{
			pTrc->log_debug(sMessage);
		}
	}
}

void MInstTracer::doTRACE_W(short int siInstId, QString sMessage, QString sFileName, int iLineNumber)
{
	MInstTracer * pTrc = MInstTracer::getTracer(siInstId);

	if(pTrc!=NULL)
	{
		if(pTrc->isTrcEnabled())
		{
				pTrc->log_warning(sMessage, sFileName, iLineNumber);
		}
	}
}

bool MInstTracer::CreatePath(QString sPath)
{
	QDir oDir;

	return oDir.mkpath(sPath);
}

bool MInstTracer::initData(int iMaxTraceSize)
{
	setMaxTraceSize(iMaxTraceSize);

	QString sRegPathKey = QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\LaneController\\Config\\");
	QSettings regValues(sRegPathKey, QSettings::NativeFormat);
	m_sERR_PATH = regValues.value(QString("%1").arg("ErrorPath"), QString(DEFAULT_ERR_PATH)).toString();
	m_sTRC_PATH = regValues.value(QString("%1").arg("TracePath"), QString(DEFAULT_TRC_PATH)).toString();

	QSettings regValues2(QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\CSRoute\\TRC\\"), QSettings::NativeFormat);
	QString sNameErr = QString("%1.ERR").arg(m_sBallID);
	int iErr = regValues2.value(sNameErr, 1).toInt();
	regValues2.setValue(sNameErr, iErr);
	QString sNameTrc = QString("%1.TRC").arg(m_sBallID);
	int iTrc = regValues2.value(sNameTrc, 1).toInt();
	regValues2.setValue(sNameTrc, iTrc);

	m_bTrcEnabled = (iTrc != 0) ? true : false;
	m_bErrEnabled = (iErr != 0) ? true : false;

	m_sFullErrFileName = QString("%1\\%2.ERR").arg(m_sERR_PATH).arg(m_sBallID);
	m_sFullTrcFileName = QString("%1\\%2.TRC").arg(m_sTRC_PATH).arg(m_sBallID);

	if (!QFile::exists(m_sFullErrFileName))
		if (!CreatePath(m_sERR_PATH))
			m_bErrEnabled = false;

	///* creation du repertoire de traces */
	if (!QFile::exists(m_sFullTrcFileName))
		if (!CreatePath(m_sTRC_PATH))
			m_bTrcEnabled = false;

	if (m_bErrEnabled)
	{
		/* initialise le fichier de debug */
		TRC_Initialise_Trace((char*)"DEBUG", m_sFullErrFileName.toLatin1().data(),
			TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION | TRC_OPT_TEXTE_SEUL,
			(TRC_EMETTEUR *)&m_emetError);

		TRC_Taille_Max_Fichier(m_emetError, m_dwMaxSizeBytes);

		/* teste le fichier de debug */
		TRC_Trace_Texte(m_emetError, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, (char *)" => Debut du debug");
	}

	if (m_bTrcEnabled)
	{
		DWORD dwERR = TRC_Initialise_Trace((char*)"TRACE", m_sFullTrcFileName.toLatin1().data(), TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, (TRC_EMETTEUR *)&m_emetTrace);
		
		dwERR = TRC_Taille_Max_Fichier(m_emetTrace, m_dwMaxSizeBytes);
	}
	//-------------------------

	return true;
}

bool MInstTracer::Init(short int siInstId, QString sBallID, int iMaxTraceSize)
{
	if (MInstTracer::getTracer(siInstId) == NULL)
	{
		MInstTracer *pTracer = new MInstTracer(siInstId, sBallID);
		pTracer->initData(iMaxTraceSize);
	}

	return true;
}

void MInstTracer::deinitData()
{
	if (m_emetError != NULL)
	{
		TRC_Termine_Trace(m_emetError); 
		m_emetError = NULL;
	}

	if (m_emetTrace != NULL)
	{
		TRC_Termine_Trace(m_emetTrace);
		m_emetTrace = NULL;
	}
}


void MInstTracer::Deinit(short int siInstId)
{
	MInstTracer * pTrc = MInstTracer::getTracer(siInstId);

	if (pTrc != NULL)
	{
		delete pTrc;
	}
}


void MInstTracer::log_debug(QString sMessage)
{
	if(!m_bTrcEnabled)
		return;
	QString sPid = QString("PROC%1-THR%2:").arg(GetCurrentProcessId()).arg(GetCurrentThreadId());
	QString sMsg = sPid + MHelpFuncs::escapeStringForSprintf(sMessage);
	
	TRC_Trace_V(m_emetTrace, TRC_OPT_MASK, NULL, 0, sMsg.toLatin1().data(), NULL);

}



void MInstTracer::log_warning(QString sMessage, QString sFileName, int iLineNumber)
{
	if(!m_bErrEnabled)
		return;

	QString sPid = QString("PROC%1-THR%2:").arg(GetCurrentProcessId()).arg(GetCurrentThreadId());
	QString sMsg = sPid;

	sMsg += "WARNING: " + MHelpFuncs::escapeStringForSprintf(sMessage);
	sMsg += " | ";
	sMsg += "File: ";
	sMsg += sFileName; 
	sMsg += " | ";
	sMsg += "Line: ";
	sMsg += QString::number(iLineNumber);
	
	//writeWarning(sMsg);
	//writeDebug(sMsg);
	TRC_Trace_V(m_emetTrace, TRC_OPT_MASK, NULL, 0, sMsg.toLatin1().data(), NULL);
	TRC_Direct_Trace_V(m_emetError, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, NULL, 0, sMsg.toLatin1().data(), NULL);

}


void MInstTracer::setMaxTraceSize(int iMaxTraceSizeMb) 
{  
	if (iMaxTraceSizeMb>1000)
		iMaxTraceSizeMb = 1000;
	if (iMaxTraceSizeMb<1)
		iMaxTraceSizeMb = 1;

		m_dwMaxSizeBytes = 1048576 * iMaxTraceSizeMb;
}



/*******************************************************************/
// function to be used from C
/*******************************************************************/
BOOL WINAPI MInstTracer_InitTrace(const char *pcBal, short siInstId, int iTraceFileMaxSizeMB)
{
	if (!MInstTracer::Init(siInstId, pcBal, iTraceFileMaxSizeMB))
		return FALSE;
	else
		return TRUE;
}


void WINAPI MInstTracer_DeinitTrace(short siInstId)
{
	MInstTracer::Deinit(siInstId);
}

#define MAX_STRING 16384

void WINAPI MInstTracer_FichierTrace(short siInstId, const char *szFmt, ...)
{
	va_list args;
	char szString[MAX_STRING];

	va_start(args, szFmt);
	_vsnprintf_s(szString, sizeof(szString), sizeof(szString), szFmt, args);

	MInstTracer::doTRACE_D(siInstId, QString::fromLocal8Bit(szString));

	va_end(args);
}



void WINAPI DEFINE_MInstTracer_FichierError(short siInstId,
	const char *szFmt, ...)
{
	va_list ap;
	char szString[MAX_STRING] = "*ERREUR* ";
	const char *DBG_FILE;
	int  DBG_LINE;

	DBG_FILE = ERR_FILE;
	DBG_LINE = ERR_LINE;

	/* ecrit les infos */
	va_start(ap, szFmt);

	_vsnprintf_s(&szString[strlen(szString) - 1], (sizeof(szString) - strlen(szString) - 1), MAX_STRING, szFmt, ap);

	MInstTracer::doTRACE_W(siInstId, QString::fromLocal8Bit(szString), QString::fromLocal8Bit(DBG_FILE), DBG_LINE);

	va_end(ap);
}