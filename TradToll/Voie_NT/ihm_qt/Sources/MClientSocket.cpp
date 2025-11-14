
#include <QtNetwork>

#include "MClientSocket.h"
#include "MIhmHttpRequest.h"
#include "MHelpFuncs.h"
#include "MTracer.h"


MClientSocket::MClientSocket(QObject *parent)
    : QTcpSocket(parent)
{
	connect(this, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

}

MClientSocket::~MClientSocket()
{
	disconnect(this, SIGNAL(connected()), this, SLOT(onConnected()));
	disconnect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

}


void MClientSocket::onConnected()
{

}


bool MClientSocket::isConnected()
{
	if(this->state() == QAbstractSocket::ConnectedState)
	{
		return true;
	}
	else
		return false;
}




bool MClientSocket::sendData(const QByteArray &baOutData) 
{
	
	if(this->isConnected())
	{
		if(MTracer::getWebTracer()->isTrcEnabled())
			TRACE_WEB_D(QString("MClientSocket::sendData:Send data: %1")
									.arg(MHelpFuncs::ByteArrayToAnsi(baOutData)));
		
		if(this->write(baOutData)!=-1)
			return true;
		else
			TRACE_WEB_W(QString("MClientSocket::sendData:Unable to send data!"));

	}
	else
		TRACE_WEB_W(QString("MClientSocket::sendData:Unable to send data! Client disconnected"));

	return false;
}



void MClientSocket::onReadyRead()
{
	int iSize = this->bytesAvailable();
	
	if(iSize>0)
	{
		QByteArray baDataRead;
		baDataRead = this->read(iSize);
		
		m_baCompletePackage += baDataRead;
		
		if(MTracer::getWebTracer()->isTrcEnabled())
		 TRACE_WEB_D(QString("MClientSocket::onReadyRead: DataRead: %1")
										.arg(MHelpFuncs::ByteArrayToAnsi(baDataRead)));


		if(isComplete())
		{
			TRACE_WEB_D(QString("MClientSocket::onReadyRead:Complete package! Buffer deleted!"));

			emit receivedData(m_baCompletePackage);
			m_baCompletePackage = "";
		}
		else
		{
			TRACE_WEB_D(QString("MClientSocket::onReadyRead:Incomplete package!"));
		}
	}	
}

bool MClientSocket::isComplete()
{
	if(m_baCompletePackage.indexOf("POST")!=0)
	{
		return true;
	}

	//Line[10]: Content-Length: 41 
	int iIndexOfContentLenght = m_baCompletePackage.indexOf("Content-Length:");
	int iIndexOfSpace = m_baCompletePackage.indexOf(" ", iIndexOfContentLenght);
	int iIndexOfEOL = m_baCompletePackage.indexOf("\n", iIndexOfSpace);
	
	bool bOK;
	QByteArray baLenght = m_baCompletePackage.mid(iIndexOfSpace, iIndexOfEOL - iIndexOfSpace -1);
	int iParamsLenght = baLenght.toInt(&bOK);

	if(!bOK)
	{
		return true; //return complete package since is no good format to wait rest
	}

	int iEndOfHeader = m_baCompletePackage.indexOf(MHelpFuncs::HexStrToByteArray("0D0A0D0A"));

	if(iEndOfHeader!=-1)
	{
		if(m_baCompletePackage.count()-iEndOfHeader < iParamsLenght)
			return false;
	}

	return true;
}

