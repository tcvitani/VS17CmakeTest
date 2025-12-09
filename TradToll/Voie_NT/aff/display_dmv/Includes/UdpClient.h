#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <QUdpSocket>
#include <QObject>

class UdpClient : public QUdpSocket
{
	Q_OBJECT
public:
	UdpClient(short int siInstId,
			  bool bFullTrace,
			  QString DeviceAddr,
			  quint16 DevicePort,
			  QString LocalAddr,
			  quint16 LocalPort,
			  QObject *parent = 0);

	~UdpClient();

	void createConnection();
	void sendDatagram(const QByteArray &datagram);

signals:
	void receivedDatagram(const QByteArray &datagram);

	private slots:
	void readPendingDatagrams();

private:
	QUdpSocket *clientSocket;
	QHostAddress bcastAddress;
	QHostAddress recvAddress;

	QByteArray m_baInputBuffer;
	QByteArray m_baOutputBuffer;

	QString m_sServerAddress;
	quint16 m_iServerPort;
	QString m_sLocalAddress;
	quint16 m_iLocalPort;

	short int m_siInstId;
	bool m_bFullTrace;
};

#endif