#include <qdatetime.h>
#include <QSettings>
#include <QFile>

#include "mtracer.h"

#include <windows.h>

extern "C"
{
	#include <CSRLC32.H>
};

MTracer* MTracer::m_pTracer = NULL;

#define DEFAULT_ERR_PATH "c:\\csr\\_errors\\"
#define DEFAULT_TRC_PATH "c:\\csr\\_traces\\"

void MTracer::lock()
{
	m_TrcMutex.lock();
}

void MTracer::unlock()
{
	m_TrcMutex.unlock();
}

MTracer::MTracer()
{
	m_hTrace = INVALID_HANDLE_VALUE;;
	m_hError = INVALID_HANDLE_VALUE;;

	m_bTrcEnabled = false;
	m_bErrEnabled = false;
}

void MTracer::doTRACE_D(QString sMessage)
{
	MTracer * pTrc = MTracer::getTracer();

	if(pTrc!=NULL)
	{
		if(pTrc->isTrcEnabled())
		{
			pTrc->lock();
				pTrc->log_debug(sMessage);
			pTrc->unlock();
		}
	}
}

void MTracer::doTRACE_W(QString sMessage, QString sFileName, int iLineNumber)
{
	MTracer * pTrc = MTracer::getTracer();

	if(pTrc!=NULL)
	{
		if(pTrc->isTrcEnabled())
		{
			pTrc->lock();
				pTrc->log_warning(sMessage, sFileName, iLineNumber);
			pTrc->unlock();
		}
	}
}


void MTracer::initData(MTracer * pTracer, int iMaxTraceSize)
{
	QString sRegPathKey = QString("%1%2%3%4").arg("HKEY_LOCAL_MACHINE\\").arg(CSR_REG_KEYn_CSRBASE).arg(CSR_REG_KEYn_LANE_BASE).arg(CSR_REG_KEYn_CONFIG);
 	QSettings regValues(sRegPathKey, QSettings::NativeFormat);
	pTracer->m_sERR_PATH = regValues.value(QString("%1").arg(CSR_REG_KEYv_ERRORPATH), QString(DEFAULT_ERR_PATH)).toString();
	pTracer->m_sTRC_PATH = regValues.value(QString("%1").arg(CSR_REG_KEYv_TRACEPATH), QString(DEFAULT_TRC_PATH)).toString();

	QSettings regValues2(QString("%1TRC\\").arg(CSR_REG_KEYn_CSRBASE), QSettings::NativeFormat);
	int iErr = regValues2.value(QString("%1.ERR").arg(m_sBallID), 0).toInt();
	int iTrc = regValues2.value(QString("%1.TRC").arg(m_sBallID), 0).toInt();
	
	pTracer->m_bTrcEnabled = (iTrc!=0)?true:false;
	pTracer->m_bErrEnabled = (iErr!=0)?true:false;

	if(pTracer->m_bErrEnabled)
	{
		pTracer->m_sFullErrFileName = QString("%1\\%2.ERR").arg(m_sERR_PATH).arg(m_sBallID);

		pTracer->m_hError = CreateFile(m_sFullErrFileName.utf16(),   
			GENERIC_WRITE,                // open for writing 
			FILE_SHARE_READ,              
			NULL,                         // no security 
			OPEN_ALWAYS,                  // open or create 
			FILE_ATTRIBUTE_NORMAL,        // normal file 
			NULL);                        // no attr. template 

		if (pTracer->m_hError == INVALID_HANDLE_VALUE) 
		{ 
			;
		} 
		else
			SetFilePointer(m_pTracer->m_hError,0,0, FILE_END);
	}

	if(pTracer->m_bTrcEnabled)
	{
		pTracer->m_sFullTrcFileName = QString("%1\\%2.TRC").arg(m_sTRC_PATH).arg(m_sBallID);

		pTracer->m_hTrace = CreateFile(m_sFullTrcFileName.utf16(),   // open TWO.TXT 
			GENERIC_WRITE,                // open for writing 
			FILE_SHARE_READ,               
			NULL,                         // no security 
			OPEN_ALWAYS,                  // open or create 
			FILE_ATTRIBUTE_NORMAL,        // normal file 
			NULL);                        // no attr. template 

		if (pTracer->m_hTrace == INVALID_HANDLE_VALUE) 
		{ 
			;
		} 
		else
			SetFilePointer(pTracer->m_hTrace,0,NULL, FILE_END);

	}

	pTracer->setMaxTraceSize(iMaxTraceSize);
	
}

void MTracer::Init(QString &sBallID, int iMaxTraceSize)
{
	if(MTracer::m_pTracer!=NULL)
		return;
	
	MTracer::m_pTracer = new MTracer();
	m_pTracer->m_sBallID = sBallID;

	m_pTracer->initData(MTracer::m_pTracer, iMaxTraceSize);

	qInstallMsgHandler(myMessageOutput);
	
}

void MTracer::Deinit(QString &sBallID)
{
	if(MTracer::m_pTracer!=NULL)
	{
		delete MTracer::m_pTracer;
		MTracer::m_pTracer = NULL;
	}
}

MTracer::~MTracer()
{
	if(m_hTrace != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(m_hTrace);
	}
	if(m_hError != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(m_hError);
	}
}

void MTracer::log_debug(QString sMessage)
{
	if(!m_bTrcEnabled)
		return;

	QString sTime = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzz");
	QString sMsg = "\n" + sTime;
	sMsg += " ";
	sMsg += sMessage;
	
	writeDebug(sMsg);
}



void MTracer::log_warning(QString sMessage, QString sFileName, int iLineNumber)
{
	if(!m_bErrEnabled)
		return;

	QString sTime = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzz");
	QString sMsg = sTime + " " + sMessage;
	
	sMsg = "\nWARNING: " + sMsg;
	sMsg += " | ";
	sMsg += "File: ";
	sMsg += sFileName; 
	sMsg += " | ";
	sMsg += "Line: ";
	sMsg += QString::number(iLineNumber);
	
	writeWarning(sMsg);
	writeDebug(sMsg);
}



void MTracer::writeDebug(QString sMsg)
{
	if(checkSize(&m_hTrace, true)) 
	{
		DWORD dwBytesWritten;

		QByteArray ba = sMsg.toAscii();
		
		if(ba.size()>0)
		{
			WriteFile(m_hTrace, ba.data(), ba.size(), 
				&dwBytesWritten, NULL); 
		}
	}
}

void MTracer::writeWarning(QString sMsg)
{
	if(checkSize(&m_hError,false)) 
	{
		DWORD dwBytesWritten;
		QByteArray ba = sMsg.toAscii();
		
		if(ba.size()>0)
		{
			WriteFile(m_hError,  ba.data(), ba.size(),
				&dwBytesWritten, NULL); 
		}
	}
}


void MTracer::setMaxTraceSize(int iMaxTraceSize) 
{  
	if(iMaxTraceSize>1000)
		iMaxTraceSize = 1000;	
	if(iMaxTraceSize<1)
		iMaxTraceSize = 1;	

	lock();	
		m_dwMaxSizeBytes = 1048576 * iMaxTraceSize; 
	unlock();
}


bool MTracer::checkSize(HANDLE *phHandle, bool bIsTrace)
{
	DWORD dwSizeLow, dwSizeHigh;
	QString sCurrName, sNewName;

	if(bIsTrace)
	{
		sCurrName= m_sFullTrcFileName;
		sNewName = m_sFullTrcFileName;
		sNewName.append(".old");
	}
	else
	{
		sCurrName= m_sFullErrFileName;
		sNewName = m_sFullErrFileName;
		sNewName.append(".old");
	}

	if(*phHandle != INVALID_HANDLE_VALUE)
	{
		dwSizeLow = GetFileSize (*phHandle, &dwSizeHigh) ;
		
		if(dwSizeLow==INVALID_FILE_SIZE || dwSizeLow >= m_dwMaxSizeBytes)	
		{

			CloseHandle(*phHandle);
			
			QFile::remove(sNewName);
				
			if(QFile::rename(sCurrName, sNewName))
			{
				*phHandle = CreateFile(sCurrName.utf16(),   
					GENERIC_WRITE,                // open for writing 
					FILE_SHARE_READ,                            // do not share 
					NULL,                         // no security 
					OPEN_ALWAYS,                  // open or create 
					FILE_ATTRIBUTE_NORMAL,        // normal file 
					NULL);                        // no attr. template 

				if (*phHandle == INVALID_HANDLE_VALUE) 
				{ 
					; //NOP
				} 
				else
					SetFilePointer(*phHandle,0,NULL, FILE_END);

			}
		}
	}
	else
	{
		*phHandle = CreateFile(sCurrName.utf16(),   
			GENERIC_WRITE,                // open for writing 
			FILE_SHARE_READ,                            // do not share 
			NULL,                         // no security 
			OPEN_ALWAYS,                  // open or create 
			FILE_ATTRIBUTE_NORMAL,        // normal file 
			NULL);                        // no attr. template 

		if (*phHandle == INVALID_HANDLE_VALUE) 
		{ 
			; //NOP
		} 
		else
			SetFilePointer(*phHandle,0,NULL, FILE_END);

	}
		

	return *phHandle != INVALID_HANDLE_VALUE;
}

void MTracer::myMessageOutput( QtMsgType type, const char *msg )
{
	switch(type)
	{
	case QtWarningMsg:
		doTRACE_W(msg,"",0);
		break;
	case QtFatalMsg:
		doTRACE_W(msg,"",0);
		break;
    }
}
