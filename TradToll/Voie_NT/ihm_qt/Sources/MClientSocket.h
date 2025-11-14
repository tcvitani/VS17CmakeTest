
#ifndef MCLIENT_SOCKET_H
#define MCLIENT_SOCKET_H

#include <QTcpSocket>

/*!
	\class MClientSocket
	\brief Used to encapsulate the receiving of http requests.

	Encapsulates the receiving of complete http requests including parameters.
	It detects the http request type and decides when to signal that a complete package was received.
	
	\author Tihomir Cvitanic
	\version 1.0
	\date    2010/01/20
	\sa  
*/
class MClientSocket : public QTcpSocket
{
    Q_OBJECT

public:
    MClientSocket(QObject *parent = 0);
    ~MClientSocket();
	//!Response data to the client
    /*!
      Function used to send response data to the client
      \param QByteArray data to be sent to the client.
      \return True if the data were successfully sent.
	*/
	bool sendData(const QByteArray &baOutData);

	//!Check if the client is in QAbstractSocket::ConnectedState
	bool isConnected();

signals:
    //! Signal emitted when a new complete http package was received
	void receivedData(const QByteArray &);
    //! Signal emitted when there was an error detected with the client socket
	void socketError(QAbstractSocket::SocketError);
	
private slots:
	void onConnected();
	void onReadyRead();
	
private:
	bool isComplete();
	QByteArray m_baCompletePackage;

};


#endif
