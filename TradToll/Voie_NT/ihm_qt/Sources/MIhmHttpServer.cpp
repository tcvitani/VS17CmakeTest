
#include <MIhmHttpServer.h>
#include <MClientSocket.h>
#include <QTcpSocket>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>

MIhmHttpServer::MIhmHttpServer(quint16 port, QObject* parent)
        : QTcpServer(parent)
{
    listen(QHostAddress::Any, port);
}


MIhmHttpServer::~MIhmHttpServer()
{
	if(this->isListening())
		this->close();



}

void MIhmHttpServer::stop()
{
	if(this->isListening())
		this->close();

}


void MIhmHttpServer::incomingConnection( int socketDescriptor )
{
	emit newConnection(socketDescriptor);
}
