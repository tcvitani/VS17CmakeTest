#include <UdpClient.h>

UdpClient::UdpClient(short int siInstId,
					 bool bFullTrace,
					 QString DeviceAddr,
					 quint16 DevicePort,
					 QString LocalAddr,
					 quint16 LocalPort,
					 QObject *parent) : QUdpSocket(parent)
{
	m_siInstId = siInstId;
	m_bFullTrace = bFullTrace;
	m_sServerAddress = DeviceAddr;
	m_iServerPort = DevicePort;
	m_sLocalAddress = LocalAddr;
	m_iLocalPort = LocalPort;

	clientSocket = new QUdpSocket(this);
	bcastAddress = QHostAddress(m_sServerAddress);
	recvAddress = QHostAddress(m_sLocalAddress);

	connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

UdpClient::~UdpClient()
{

}

void UdpClient::createConnection()
{
	clientSocket->bind(recvAddress, m_iLocalPort);
	clientSocket->connectToHost(bcastAddress, m_iServerPort);
}

void UdpClient::sendDatagram(const QByteArray &datagram)
{
	createConnection();
	clientSocket->write(datagram);
}

void UdpClient::readPendingDatagrams()
{
	QByteArray datagram;

	while (clientSocket->hasPendingDatagrams())
	{
		datagram.resize(clientSocket->pendingDatagramSize());
		clientSocket->readDatagram(datagram.data(), datagram.size());
	}

	emit receivedDatagram(datagram);
}
