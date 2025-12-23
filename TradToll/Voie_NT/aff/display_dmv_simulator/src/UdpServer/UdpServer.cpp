#include "UdpServer.h"
#include <QThread>

UdpServer::UdpServer(QString Addr,
					 quint16 Port,
					 quint16 EchoPort,
					 QObject *parent) : QUdpSocket(parent)
{
	m_Address = Addr;
	m_Port = Port;
	m_EchoPort = EchoPort;
	m_ClientAddress = "";
	m_ClientPort = 0;

	udpSocket = new QUdpSocket(this);
	echoSocket = new QUdpSocket(this);

	m_hAddr = QHostAddress(m_Address);
	m_hClientAddr = QHostAddress(m_ClientAddress);

	connect(udpSocket, &QUdpSocket::readyRead, this, &UdpServer::readPendingDatagrams);
	connect(echoSocket, &QUdpSocket::readyRead, this, &UdpServer::echoDatagram);

	if (udpSocket->bind(m_hAddr, m_Port))
	{
		qInfo() << "Succesfully created UDP socket on address:" << m_hAddr.toString() << "and port:" << m_Port;
	}
	else
	{
		qDebug() << "Failed to bind to port" << m_Port;
	}

	if (echoSocket->bind(m_hAddr, m_EchoPort))
	{
		qInfo() << "Succesfully created ECHO socket on address:" << m_hAddr.toString() << "and port:" << m_EchoPort;
	}
	else
	{
		qDebug() << "Failed to bind to port" << m_EchoPort;
	}
}

UdpServer::~UdpServer()
{
	if (this != nullptr)
	{
		this->deleteLater();
	}
}

void UdpServer::createConnection()
{
	if (udpSocket->bind(m_hAddr, m_Port))
	{
		qInfo() << "Succesfully created UDP socket on address:" << m_hAddr.toString() << "and port:" << m_Port;
		udpSocket->connectToHost(m_hAddr, m_Port);
	}
	else
	{
		qDebug() << "Failed to bind to port" << m_Port;
	}
}

void UdpServer::createEchoConnection()
{
	if (echoSocket->bind(m_hAddr, m_EchoPort))
	{
		qInfo() << "Succesfully created ECHO socket on address:" << m_hAddr.toString() << "and port:" << m_EchoPort;
		udpSocket->connectToHost(m_hAddr, m_EchoPort);
	}
	else
	{
		qDebug() << "Failed to bind to port" << m_EchoPort;
	}
}

void UdpServer::sendDatagram(const QByteArray& datagram)
{
	udpSocket->write(datagram);
}

void UdpServer::sendEchoDatagram(const QByteArray& datagram)
{
	echoSocket->write(datagram);
}

void UdpServer::readPendingDatagrams()
{
	QByteArray datagram;

	while (udpSocket->hasPendingDatagrams())
	{
		datagram.resize(udpSocket->pendingDatagramSize());

		QHostAddress sender;
		quint16 senderPort;

		udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
	
		m_hClientAddr = sender;
		m_ClientPort = senderPort;
	}

	emit receivedDatagram(datagram);
}

void UdpServer::echoDatagram()
{
	QByteArray datagram;
	QHostAddress sender;
	quint16 senderPort = 0;

	while (echoSocket->hasPendingDatagrams())
	{
		datagram.resize(echoSocket->pendingDatagramSize());
		echoSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
	}

	if (datagram.size() > 0 && senderPort > 0)
	{
		qint64 bytesSent = echoSocket->writeDatagram(datagram.data(), sender, senderPort);
		if (bytesSent == -1)
		{
			qWarning() << "UdpServer::echoDatagram() - Failed to write datagram:" << echoSocket->errorString();
		}

	}
}

void UdpServer::sendMessageProcessed(bool isMessageOK)
{
	// 07641ec8014eabc9ff8600050000000000049e ==> Example of an OK message	

	QByteArray response("\x07\x64\x1E\xC8\x01\x4E\xAB\xC9\xFF\x86\x00\x05"); // Setting header
	QByteArray genericDescription(5, '\x00');
	int checksum = 0;
	QString stringChecksum;
	QByteArray byteChecksum;

	if (isMessageOK)
	{
		response.append('\x00'); // 0x00 - No Error
		response.append(genericDescription);
	}
	else
	{
		response.append('\x1A'); // 0x1A - Error in absolute command, this is generic
		response.append(genericDescription);
	}

	for (char byte : response)
	{
		checksum += static_cast<unsigned char>(byte);
	}
	
	stringChecksum = QString::number(checksum, 16);
	byteChecksum = QByteArray::fromHex(stringChecksum.toUtf8()).rightJustified(2, '\x00');

	response.append(byteChecksum);

	// Sleep for 20ms before sending, that is usually how much time it takes for the actual device

	QThread::msleep(20);

	qint64 bytesSent = udpSocket->writeDatagram(response, m_hClientAddr, m_ClientPort);
	if (bytesSent == -1)
	{
		qWarning() << "UdpServer::onMessageProcessed() - Failed to write datagram:" << udpSocket->errorString();
	}
}