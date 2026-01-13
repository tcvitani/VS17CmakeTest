/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:	 MTracer.cpp													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QObject.h>
#include <QDateTime.h>
#include <QSettings>
#include <QDir>
#include <QApplication.h>


#include "mtracer.h"
#include <Shlobj.h>
/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

MTracer* MTracer::m_pTracer = NULL;
#define APP_SETTINGS_SUB_FOLDER "\\EMOVIS_ITS_TestTools"

/*-------------------------------- CODE:      -------------------------------*/

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


MTracer * MTracer::getTracer()
{
	return MTracer::m_pTracer;
}


void MTracer::doTRACE_D(QString sMessage)
{
	MTracer * pTrc = MTracer::getTracer();

	if (pTrc != NULL)
	{
		if(pTrc->isTrcEnabled())
		{
			pTrc->lock();
				pTrc->log_debug(sMessage);
			pTrc->unlock();
		}
	}
}

void MTracer::doTRACE_W(QString sMessage, 
						QString sFileName, 
						int iLineNumber)
{
	MTracer * pTrc = MTracer::getTracer();

	if(pTrc!=NULL)
	{
		if(pTrc->isErrEnabled())
		{
			pTrc->lock();
				pTrc->log_warning(sMessage, sFileName, iLineNumber);
			pTrc->unlock();
		}
	}
}

bool MTracer::initData(int iMaxTraceSize)
{
	bool bRetVal = true;

	if(m_bErrEnabled)
	{
		m_sFullErrFileName = QDir::cleanPath(QString("%1\\%2.ERR")
			.arg(m_sERR_PATH)
			.arg(m_sFileNamePrefix));

		m_hError = CreateFile(m_sFullErrFileName.utf16(),   
								GENERIC_WRITE,                // open for writing 
								FILE_SHARE_READ,              
								NULL,                         // no security 
								OPEN_ALWAYS,                  // open or create 
								FILE_ATTRIBUTE_NORMAL,        // normal file 
								NULL);                        // no attr. template 

		if (m_hError == INVALID_HANDLE_VALUE) 
			bRetVal = false;
		else
			SetFilePointer(m_hError,0,0, FILE_END);
	}

	if(m_bTrcEnabled)
	{
		m_sFullTrcFileName = QDir::cleanPath(QString("%1\\%2.TRC")
			.arg(m_sTRC_PATH)
			.arg(m_sFileNamePrefix));

		m_hTrace = CreateFile(m_sFullTrcFileName.utf16(),   // open
								GENERIC_WRITE,                // open for writing 
								FILE_SHARE_READ,               
								NULL,                         // no security 
								OPEN_ALWAYS,                  // open or create 
								FILE_ATTRIBUTE_NORMAL,        // normal file 
								NULL);                        // no attr. template 

		if (m_hTrace == INVALID_HANDLE_VALUE) 
			bRetVal = false;
		else
			SetFilePointer(m_hTrace,0,NULL, FILE_END);

	}

	setMaxTraceSize(iMaxTraceSize);

	return bRetVal;
}

bool MTracer::Init(QString &sTrcFilePath,
				   QString &sErrFilePath,
				   QString &sFileNamePrefix, 
				   int iMaxFileSize, 
				   bool bTrcEnabled, 
				   bool bErrEnabled)
{
	unsigned short szAppDataPath[MAX_PATH];

	
	
	if(sTrcFilePath.isEmpty()||sErrFilePath.isEmpty())
	{
		if(SUCCEEDED(SHGetFolderPath(NULL, 
									 CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
									 NULL, 
									 SHGFP_TYPE_CURRENT, 
									 szAppDataPath))) 
		{
			
			if(sTrcFilePath.isEmpty())
			{
				sTrcFilePath = QString::fromWCharArray(szAppDataPath);
				sTrcFilePath.append(APP_SETTINGS_SUB_FOLDER);
			}	

			if(sErrFilePath.isEmpty())
			{
				sErrFilePath = QString::fromWCharArray(szAppDataPath);
				sErrFilePath.append(APP_SETTINGS_SUB_FOLDER);
			}	
		}
	}


	if (MTracer::m_pTracer != NULL)
	{
		delete MTracer::m_pTracer;
		MTracer::m_pTracer = NULL;
	}

	QDir dirTrc(sTrcFilePath);

	if (!dirTrc.exists())
	{
		if(!dirTrc.mkpath(sTrcFilePath))
			sTrcFilePath = ".";
	}

	QDir dirErr(sErrFilePath);
	
	if (!dirErr.exists())
	{
		if(!dirErr.mkpath(sErrFilePath))
			sErrFilePath = ".";
	}

	m_pTracer = new MTracer();
	m_pTracer->m_sTRC_PATH = sTrcFilePath;
	m_pTracer->m_sERR_PATH = sErrFilePath;
	m_pTracer->m_sFileNamePrefix = sFileNamePrefix;
	m_pTracer->m_bTrcEnabled = bTrcEnabled;
	m_pTracer->m_bErrEnabled = bErrEnabled;

	if (m_pTracer->initData(iMaxFileSize))
	{
		qInstallMsgHandler(myMessageOutput);
		return TRUE;
	}

	return FALSE;	
}

void MTracer::Deinit()
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
	QString sMsg = "\r\n" + sTime;
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
	
	sMsg = "\r\nWARNING: " + sMsg;
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
		m_uiMaxSizeBytes = 1048576 * iMaxTraceSize; 
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
		
		if(dwSizeLow==INVALID_FILE_SIZE || dwSizeLow >= m_uiMaxSizeBytes)	
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

/*-------------------------------- END OF FILE ------------------------------*/