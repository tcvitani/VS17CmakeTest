#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QString>
#include <QDebug>

class UdpServer : public QUdpSocket
{
	Q_OBJECT

public:
	explicit UdpServer(QString Addr,
					   quint16 Port,
					   quint16 EchoPort,
					   QObject *parent = 0);

	~UdpServer();

	void createConnection();
	void createEchoConnection();
	void sendDatagram(const QByteArray& datagram);
	void sendEchoDatagram(const QByteArray& datagram);
	void sendMessageProcessed(bool isMessageOK);
	
signals:
	void receivedDatagram(const QByteArray& datagram);

public slots:
	
private slots:
	void readPendingDatagrams();
	void echoDatagram();

private:
	QUdpSocket* udpSocket;
	QUdpSocket* echoSocket;
	QHostAddress m_hAddr;
	QHostAddress m_hClientAddr;

	QString m_Address;
	QString m_ClientAddress;
	quint16 m_Port;
	quint16 m_EchoPort;
	quint16 m_ClientPort;

	QByteArray m_InputBuffer;
};
